#define _GNU_SOURCE

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>

#include "../libs-local/measurement.h"

/*
 * Change this to see the effect of having more threads
 * than the CPU cores.
 */
#define MAX_THREADS 17

#define PTHREAD_MUTEX_LOCK(lock) \
	assert(!pthread_mutex_lock(lock))
#define PTHREAD_MUTEX_UNLOCK(lock) \
	assert(!pthread_mutex_unlock(lock))

/*
 * We don't need `static` specifier in type definition
 * such as `struct` or `enum`.
 *
 * We need cpu_lock just in case multiple threads is
 * running in the same CPU core. This is to prevent
 * increment the same counter in multiple threads within
 * the same CPU core (assuming one CPU core can have
 * multiple threads) and causing race condition.
 *
 * References:
 * - https://stackoverflow.com/a/2743984
 * - https://stackoverflow.com/q/57681154
 */
struct thread_args {
	unsigned *global_counter;
	unsigned *cpu_counter;
	pthread_mutex_t *global_lock;
	pthread_mutex_t *cpu_lock;
	unsigned threshold;
};

static void *thread_exec(void *);

int main(int argc, char *argv[])
{
	struct timespec start_time, end_time, result_time;
	struct thread_args args;
	pthread_t threads[MAX_THREADS];
	long i, thread_count;
	int retval;

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
	pthread_mutex_t global_lock = PTHREAD_MUTEX_INITIALIZER;

	args.global_counter = &global_counter;
	args.global_lock = &global_lock;
	args.threshold = 5;

	/*
	 * References:
	 * - https://www.reddit.com/r/C_Programming/comments/6zxnr1/how_to_find_the_number_of_cores_in_c/
	 */
	long num_cpus = sysconf(_SC_NPROCESSORS_ONLN);

	/*
	 * Is there any downside using variable-length array
	 * instead of using dynamic memory allocation like
	 * malloc()?
	 */
	unsigned cpu_counter[num_cpus];
	pthread_mutex_t cpu_lock[num_cpus];

	args.cpu_counter = cpu_counter;
	args.cpu_lock = cpu_lock;

	/*
	 * Initialize the local cpu counter and lock.
	 */
	for (i = 0; i < num_cpus; ++i) {
		args.cpu_counter[i] = 0;
		pthread_mutex_init(args.cpu_lock + i, NULL);
	}

	retval = clock_gettime(
		CLOCK_REALTIME,
		&start_time
	);
	assert(!retval);

	/*
	 * References:
	 * - https://github.com/xxyzz/ostep-hw/blob/master/29/simple_concurrent_counter.c
	 * - https://www.classes.cs.uchicago.edu/archive/2018/spring/12300-1/lab6.html
	 */
	for (i = 0; i < thread_count; ++i) {
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

	sub_timespec(
		start_time,
		end_time,
		&result_time
	);

	printf("global counter: %u\n", global_counter);

	printf(
		"time: %ld.%.9ld\n",
		result_time.tv_sec,
		result_time.tv_nsec
	);

	for (i = 0; i < num_cpus; ++i)
		pthread_mutex_destroy(args.cpu_lock + i);

	return 0;
}

static void *thread_exec(void *params)
{
	struct thread_args *args = params;
	unsigned i;

	pid_t thread_id = gettid();

	int current_cpu = sched_getcpu();
	assert(current_cpu != -1);

	PTHREAD_MUTEX_LOCK(args->cpu_lock + current_cpu);

	for (i = 0; i < args->threshold; ++i) {
		++(args->cpu_counter[current_cpu]);
		printf(
			"local counter: %u (cpu: %d, thread ID: %d)\n",
			args->cpu_counter[current_cpu],
			current_cpu,
			thread_id
		);
	}

	PTHREAD_MUTEX_LOCK(args->global_lock);
	*(args->global_counter) += args->cpu_counter[current_cpu];
	printf(
		"local to global: %u (cpu: %d, thread ID: %d)\n",
		*(args->global_counter),
		current_cpu,
		thread_id
	);
	PTHREAD_MUTEX_UNLOCK(args->global_lock);

	args->cpu_counter[current_cpu] = 0;

	PTHREAD_MUTEX_UNLOCK(args->cpu_lock + current_cpu);

	return NULL;
}
