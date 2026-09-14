#define _GNU_SOURCE

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <sched.h>
#include <unistd.h>

#include "../libs-local/measurement.h"

/*
 * Change this to see the effect of more threads than
 * the CPU cores available.
 */
#define MAX_THREADS 17

#define PTHREAD_MUTEX_LOCK(lock) \
	assert(!pthread_mutex_lock(lock))
#define PTHREAD_MUTEX_UNLOCK(lock) \
	assert(!pthread_mutex_unlock(lock))

struct thread_args {
	unsigned *global_counter;
	pthread_mutex_t *global_lock;
	unsigned threshold;
};

static void *thread_exec(void *);

int main(void)
{
	struct timespec start_time, end_time, result_time;
	struct thread_args args;
	pthread_t threads[MAX_THREADS];
	long i;
	int retval;

	unsigned global_counter = 0;
	pthread_mutex_t global_lock = PTHREAD_MUTEX_INITIALIZER;

	args.global_counter = &global_counter;
	args.global_lock = &global_lock;
	args.threshold = 5;

	retval = clock_gettime(
		CLOCK_REALTIME,
		&start_time
	);
	assert(!retval);

	for (i = 0; i < MAX_THREADS; ++i) {
		retval = pthread_create(
			threads + i,
			NULL,
			thread_exec,
			&args
		);
		assert(!retval);
	}

	for (i = 0; i < MAX_THREADS; ++i) {
		/*
		 * If we join the thread after creating
		 * the thread, it will become a sequential
		 * operation because we wait for the new
		 * thread to finish before spawning another
		 * thread.
		 *
		 * If we join the thread after creating
		 * all of the threads, it will become
		 * concurrent (?).
		 */
		retval = pthread_join(threads[i], NULL);
		assert(!retval);
	}

	retval = clock_gettime(
		CLOCK_REALTIME,
		&end_time
	);
	assert(!retval);

	printf("global counter: %u\n", global_counter);

	sub_timespec(
		start_time,
		end_time,
		&result_time
	);

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
	unsigned i, local_counter;

	/*
	 * If we use the local variable for the local
	 * counter, we don't need to have a lock because
	 * the each thread will have its own stack space
	 * that is independent from another thread.
	 *
	 * So we can safely increment this local counter.
	 */
	local_counter = 0;
	pid_t thread_id = gettid();

	int current_cpu = sched_getcpu();
	assert(current_cpu != -1);

	for (i = 0; i < args->threshold; ++i) {
		++local_counter;
		printf(
			"local counter: %u (cpu: %d, thread ID: %d)\n",
			local_counter,
			current_cpu,
			thread_id
		);
	}

	PTHREAD_MUTEX_LOCK(args->global_lock);

	*(args->global_counter) += local_counter;

	printf(
		"local to global: %u (cpu: %d, thread ID: %d)\n",
		*(args->global_counter),
		current_cpu,
		thread_id
	);

	PTHREAD_MUTEX_UNLOCK(args->global_lock);

	return NULL;
}
