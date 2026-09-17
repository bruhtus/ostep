#define _GNU_SOURCE

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <pthread.h>
#include <unistd.h>

#include "../libs-local/measurement.h"

#define MAX_THREADS 17

#define PTHREAD_MUTEX_LOCK(lock) \
	assert(!pthread_mutex_lock(lock))
#define PTHREAD_MUTEX_UNLOCK(lock) \
	assert(!pthread_mutex_unlock(lock))

struct thread_args {
	unsigned *counter;
	pthread_mutex_t *lock;
};

static void *thread_exec(void *);

int main(int argc, char *argv[])
{
	struct timespec start_time, end_time, result_time;
	struct thread_args args;
	pthread_t threads[MAX_THREADS];
	int retval;
	long thread_count;
	long i;

	if (argc == 1) {
		thread_count = 1;
	} else if (argc > 1) {
		char *endptr;

		thread_count = strtol(argv[1], &endptr, 10);
		if (errno == ERANGE) {
			perror("strtol() failed");
			return 69;
		} else if (*(argv[1]) == *endptr) {
			puts("invalid character for number");
			return 69;
		} else if (*endptr != '\0') {
			printf("first invalid character: %c\n", *endptr);
			return 69;
		} else if (thread_count <= 0) {
			puts("thread count can't be negative or zero");
			return 69;
		} else if (thread_count > MAX_THREADS) {
			puts("thread count can't be more than MAX_THREADS");
			return 69;
		}
	}

	unsigned global_counter = 0;
	pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

	args.counter = &global_counter;
	args.lock = &lock;

	retval = clock_gettime(
		CLOCK_REALTIME,
		&start_time
	);
	/*
	 * Looks like assert() expression of false if the
	 * expression is equal to 0 (?).
	 */
	assert(!retval);

	printf("main: begin (counter: %u)\n", global_counter);

	for (i = 0; i < thread_count; ++i) {
		/*
		 * If pthread_create() failed, do we need
		 * to do some clean up?
		 */
		retval = pthread_create(
			threads + i,
			NULL,
			thread_exec,
			&args
		);
		assert(!retval);
	}

	for (i = 0; i < thread_count; ++i) {
		/*
		 * Putting the pthread_join() in separate
		 * iteration should make this operation
		 * parallel, but still not sure if this
		 * mechanism is the correct parallel
		 * implementation.
		 */
		retval = pthread_join(threads[i], NULL);
		assert(!retval);
	}

	retval = clock_gettime(
		CLOCK_REALTIME,
		&end_time
	);
	assert(!retval);

	sub_timespec(
		start_time,
		end_time,
		&result_time
	);

	printf("main: done (counter: %u)\n", global_counter);

	printf(
		"time: %ld.%.9ld\n",
		result_time.tv_sec,
		result_time.tv_nsec
	);

	return 0;
}

static void *thread_exec(void *params)
{
	struct thread_args *args = params;

	/*
	 * Is this the right mechanism for precise concurrent
	 * counter?
	 */
	PTHREAD_MUTEX_LOCK(args->lock);

	int current_cpu = sched_getcpu();
	assert(current_cpu != -1);

	printf(
		"CPU: %d, Thread ID: %d\n",
		current_cpu,
		gettid()
	);

	++(*(args->counter));

	PTHREAD_MUTEX_UNLOCK(args->lock);

	return NULL;
}
