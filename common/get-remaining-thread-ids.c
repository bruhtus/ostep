#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "../libs-local/utils.h"

struct thread_args_st {
	pthread_t *arr;
	pthread_mutex_t *lock;
	long idx;
};

static void *thread_main(void *);

int main(void)
{
	long i, j, arr_len;
	int retval;

	arr_len = sysconf(_SC_NPROCESSORS_ONLN);
	if (arr_len == -1) {
		perror("sysconf() failed");
		return 69;
	}

	pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
	struct thread_args_st thread_args[arr_len];

	pthread_t arr[arr_len], remaining_arr[arr_len];

	/*
	 * Why remove the elements on the thread IDs array
	 * directly is a bad idea?
	 * Because as we remove the elements, the thread ID
	 * might not be generated yet. So the index position
	 * would be messed up.
	 *
	 * For example:
	 * - First, only 3 threads created:
	 *   [140737349940928, 140737341548224, ..., 0]
	 * - And then, as we move the thread id to the left,
	 *   we unintentionality replace the 3rd thread id
	 *   with 0 (the placeholder for 4th thread id?):
	 *   [140737341548224, 0, ...]
	 * - Finally, as we try move the last thread id,
	 *   we still have some array length left, so we
	 *   are trying to access the element outside of
	 *   array (?).
	 *   [0, 140737333155520]
	 */
	for (i = 0; i < arr_len; ++i) {
		thread_args[i].arr = arr;
		thread_args[i].lock = &lock;
		thread_args[i].idx = i;

		retval = pthread_create(
			arr + i,
			NULL,
			thread_main,
			thread_args + i
		);
		if (retval) {
			arr_len = i;
			print_err(
				retval,
				__LINE__,
				"pthread_create()"
			);
			break;
		}

		printf("i: %ld, thread_id: %ld\n", i, arr[i]);
	}

	/*
	 * Can we put the remaining array elements in the
	 * thread_main()?
	 *
	 * Things to consider is when some thread IDs already
	 * inside remaining_arr, what should we do?
	 * Replace the array value with the next value, and
	 * if it is the last element, we do nothing (?). And
	 * after that, decrement the total_remaining.
	 *
	 * How do we insert the remaining ID into the array
	 * without duplicate ID?
	 */
	for (i = 0, j = 0; i < arr_len; ++i) {
		/*
		 * There might be a scenario where the value
		 * in the current index of array has been
		 * modified by another thread at the same
		 * time we are trying to evaluate the array
		 * index value, so add mutex to prevent race
		 * condition.
		 */
		retval = pthread_mutex_lock(&lock);
		if (retval) {
			print_err(
				retval,
				__LINE__,
				"pthread_mutex_lock()"
			);
			return 69;
		}

		if (arr[i] != 0) {
			printf("arr[%ld]: %ld\n", i, arr[i]);
			remaining_arr[j] = arr[i];
			++j;
		}

		retval = pthread_mutex_unlock(&lock);
		if (retval) {
			print_err(
				retval,
				__LINE__,
				"pthread_mutex_unlock()"
			);
			return 69;
		}
	}

	/*
	 * The j iterator should be one more digit than the
	 * actual filled index.
	 */
	for (i = 0; i < j; ++i) {
		retval = pthread_join(remaining_arr[i], NULL);
		if (retval) {
			print_err(
				retval,
				__LINE__,
				"pthread_join()"
			);
			return 69;
		}
		printf(
			"pthread_join() thread_id: %ld\n",
			remaining_arr[i]
		);
	}

	return 0;
}

static void *thread_main(void *params)
{
	int retval;

	struct thread_args_st *args = params;

	retval = pthread_mutex_lock(args->lock);
	if (retval) {
		print_err(
			retval,
			__LINE__,
			"pthread_mutex_lock()"
		);
		return NULL;
	}

	args->arr[args->idx] = 0;

	retval = pthread_mutex_unlock(args->lock);
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
