#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <pthread.h>
#include <unistd.h>

#include "../libs-local/utils.h"

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(*arr))

struct node_type {
	unsigned key;
	struct node_type *next_node;
};

struct list_type {
	struct node_type *head;
	pthread_mutex_t lock;
};

struct thread_args_insert_type {
	struct list_type *list;
	unsigned key;
};

struct thread_args_lookup_type {
	struct node_type *node;
	pthread_mutex_t *lock;
	unsigned key;
};

static void *thread_insert(void *);
static void *thread_lookup(void *);

/*
 * The mechanism used in this linked list is the prepend
 * mechanism, basically add the new node before the existing
 * node.
 *
 * Reference:
 * https://www.learnc.net/c-data-structures/c-linked-list/
 */
int main(void)
{
	long i;
	int retval;

	long num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
	if (num_cpus == -1) {
		print_err(errno, __LINE__, "sysconf()");
		return 69;
	}

	pthread_t thread_ids[num_cpus];

	struct thread_args_insert_type thread_args_insert[num_cpus];
	struct thread_args_lookup_type thread_args_lookup[num_cpus];

	pthread_mutex_t lookup_lock = PTHREAD_MUTEX_INITIALIZER;
	unsigned lookup_key = 6;

	/*
	 * When prepend the new node in concurrent, is
	 * the node order will be sequential or not?
	 * No, there is no guarantee that the new node
	 * prepended concurrently will be in sequential
	 * order.
	 */
	struct list_type list = {
		.head = NULL,
		.lock = PTHREAD_MUTEX_INITIALIZER,
	};

	for (i = 0; i < num_cpus; ++i) {
		thread_args_insert[i].list = &list;
		thread_args_insert[i].key = i;

		retval = pthread_create(
			thread_ids + i,
			NULL,
			thread_insert,
			thread_args_insert + i
		);
		if (retval) {
			num_cpus = i;
			print_err(
				retval,
				__LINE__,
				"pthread_create()"
			);
			break;
		}

		printf(
			"Thread ID %ld created!\n",
			thread_ids[i]
		);
	}

	/*
	 * Is there any guarantee that all threads will be
	 * finished in this scope? So that we do not need to
	 * use mutex after this scope (?).
	 */
	for (i = 0; i < num_cpus; ++i) {
		/*
		 * The threads run concurrently, starting
		 * either during or after pthread_create().
		 * Calling pthread_join() has nothing to do
		 * with starting the thread.
		 *
		 * Reference:
		 * https://stackoverflow.com/a/29039788
		 */
		retval = pthread_join(thread_ids[i], NULL);
		if (retval) {
			print_err(
				retval,
				__LINE__,
				"pthread_join()"
			);
			goto cleanup;
		}

		/*
		 * This will print out the last key in the
		 * list after some threads finished, and not
		 * each key after one thread finish.
		 *
		 * It looks like some of the threads is
		 * running after the creation, without going
		 * into this scope, which is not using
		 * pthread_join() to wait the thread to
		 * finish.
		 *
		 * So by the time we go into this scope,
		 * we are not waiting for all the
		 * threads to finished but only a few threads,
		 * or we are not even waiting for any threads
		 * because all threads has been finished
		 * before we can use the pthread_join() with
		 * the specified thread ID (?).
		 *
		 * The mutex here to prevent printing out of
		 * date head pointer when another thread
		 * adding the new node concurrently.
		 */
		retval = pthread_mutex_lock(&list.lock);
		if (retval) {
			print_err(
				retval,
				__LINE__,
				"pthread_mutex_unlock()"
			);
			goto cleanup;
		}

		printf("last key: %u\n", list.head->key);

		retval = pthread_mutex_unlock(&list.lock);
		if (retval) {
			print_err(
				retval,
				__LINE__,
				"pthread_mutex_unlock()"
			);
			goto cleanup;
		}
	}

	/*
	 * Search specific key in each of the node in parallel
	 * (assuming there is only one key in the entire
	 * linked list).
	 * Assign each node to its own thread, for example:
	 * thread 1 -> node 15 (head node)
	 * thread 2 -> node 14
	 * thread 3 -> node 13
	 * ...
	 * thread 16 -> node 0
	 */
	struct node_type *lookup_node = list.head;
	for (i = 0; i < num_cpus; ++i) {
		thread_args_lookup[i].lock = &lookup_lock;
		thread_args_lookup[i].node = lookup_node;
		thread_args_lookup[i].key = lookup_key;

		retval = pthread_create(
			thread_ids + i,
			NULL,
			thread_lookup,
			thread_args_lookup + i
		);
		if (retval) {
			num_cpus = i;
			print_err(
				retval,
				__LINE__,
				"pthread_create()"
			);
			break;
		}

		/*
		 * If lookup_node is NULL, we are at the end
		 * of linked list.
		 */
		lookup_node = lookup_node->next_node;
		if (!lookup_node)
			break;
	}

	void *thread_id_key = NULL;
	for (i = 0; i < num_cpus; ++i) {
		retval = pthread_join(
			thread_ids[i],
			&thread_id_key
		);
		if (retval) {
			print_err(
				retval,
				__LINE__,
				"pthread_join()"
			);
			goto cleanup;
		}

		/*
		 * Still not sure if early exit from
		 * pthread_join() loop is a good idea or not.
		 */
		if (thread_id_key)
			break;
	}

	if (thread_id_key)
		printf(
			"Key %u found in thread ID %lu\n",
			lookup_key,
			(pthread_t)thread_id_key
		);
	else
		printf("Key %u not found\n", lookup_key);

cleanup:
	int prev_retval = retval;

	/*
	 * Do we need to use mutex for free() the
	 * allocation?
	 */
	struct node_type *cursor = list.head;
	while (cursor) {
		struct node_type *tmp = cursor;
		cursor = cursor->next_node;
		free(tmp);
	}

	if (prev_retval)
		return 69;

	return 0;
}

static void *thread_insert(void *params)
{
	int retval;

	struct thread_args_insert_type *args = params;
	struct list_type *list = args->list;

	int current_cpu = sched_getcpu();
	if (current_cpu == -1) {
		print_err(
			errno,
			__LINE__,
			"sched_getcpu()"
		);
		return NULL;
	}

	/*
	 * Assuming that malloc() is thread-safe, which means
	 * that each thread can call to it without worry
	 * about race conditions.
	 */
	struct node_type *new_node = malloc(
		sizeof(*new_node) * 1
	);
	if (new_node == NULL) {
		print_err(errno, __LINE__, "malloc()");
		return NULL;
	}

	new_node->key = args->key;

	retval = pthread_mutex_lock(&list->lock);
	if (retval) {
		print_err(
			retval,
			__LINE__,
			"pthread_mutex_lock()"
		);
		return NULL;
	}

	printf(
		"CPU: %d, key: %u\n",
		current_cpu,
		args->key
	);

	new_node->next_node = list->head;
	list->head = new_node;

	retval = pthread_mutex_unlock(&list->lock);
	if (retval) {
		print_err(
			retval,
			__LINE__,
			"pthread_mutex_unlock()"
		);
		return NULL;
	}

	return NULL;
}

static void *thread_lookup(void *params)
{
	int retval;

	struct thread_args_lookup_type *args = params;
	pthread_t thread_id_key = 0;

	if (args->node->key == args->key) {
		retval = pthread_mutex_lock(args->lock);
		if (retval) {
			print_err(
				retval,
				__LINE__,
				"pthread_mutex_lock()"
			);
			return NULL;
		}

		thread_id_key = pthread_self();
		printf("thread_id_key: %lu\n", thread_id_key);

		retval = pthread_mutex_unlock(args->lock);
		if (retval) {
			print_err(
				retval,
				__LINE__,
				"pthread_mutex_unlock()"
			);
			return NULL;
		}
	}

	return (void *)thread_id_key;
}
