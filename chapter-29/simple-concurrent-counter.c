#define _GNU_SOURCE

#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "../libs-local/measurement.h"

#define COUNTER_MAX 100

#define PTHREAD_MUTEX_LOCK(lock) \
	assert(!pthread_mutex_lock(lock))
#define PTHREAD_MUTEX_UNLOCK(lock) \
	assert(!pthread_mutex_unlock(lock))

struct counter_info {
	pthread_mutex_t lock;
	unsigned value;
};

struct thread_info {
	struct counter_info *counter;
	pthread_t thread;
};

/*
 * We can use pthread_exit() to exit early from any function
 * other than thread_exec() which is the entry point for the
 * thread. It's similar to exit() which is used to exit early
 * from non main() function.
 *
 * Reference:
 * https://stackoverflow.com/questions/2251452/how-to-return-a-value-from-pthread-threads-in-c#comment2210136_2251479
 */
static void *thread_exec(void *);

/*
 * Reference:
 * https://github.com/xxyzz/ostep-hw/blob/master/29/simple_concurrent_counter.c
 */
int main(void)
{
	struct timespec start_time, end_time, result_time;
	pthread_attr_t attr;
	cpu_set_t cpu_set;
	long i, j;
	int retval;

	long num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
	assert(num_cpus != -1);

	/*
	 * Do we need to use static variable if we want to
	 * initialize mutex with PTHREAD_MUTEX_INITIALIZER?
	 *
	 * References:
	 * - https://stackoverflow.com/a/14320588
	 * - https://www.man7.org/linux/man-pages/man3/pthread_mutex_lock.3.html
	 */
	struct counter_info counter = {
		.value = 0,
		.lock = PTHREAD_MUTEX_INITIALIZER,
	};

	retval = pthread_attr_init(&attr);
	assert(!retval);

	retval = clock_gettime(
		CLOCK_REALTIME,
		&start_time
	);
	assert(!retval);

	for (i = 0; i < num_cpus; ++i) {
		long num_threads = i + 1;

		struct thread_info threads[num_threads];

		for (j = 0; j < num_threads; ++j) {
			threads[j].counter = &counter;

			CPU_ZERO(&cpu_set);
			CPU_SET(j, &cpu_set);

			/*
			 * Because we use pthread_create(),
			 * we might need to
			 * use pthread_setaffinity_np()
			 * instead of sched_setaffinity() (?).
			 */
			pthread_attr_setaffinity_np(
				&attr,
				sizeof(cpu_set),
				&cpu_set
			);

			retval = pthread_create(
				&threads[j].thread,
				&attr,
				thread_exec,
				threads + j
			);
			assert(!retval);
		}

		for (j = 0; j < num_threads; ++j) {
			retval = pthread_join(
				threads[j].thread,
				NULL
			);
			assert(!retval);
		}

		printf("Counter: %u\n", counter.value);
		counter.value = 0;

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

		printf(
			"time: %ld.%.9ld\n",
			result_time.tv_sec,
			result_time.tv_nsec
		);
	}

	return 0;
}

static void *thread_exec(void *params)
{
	struct thread_info *info = params;
	unsigned i;

	struct counter_info *counter = info->counter;

	int current_cpu = sched_getcpu();
	assert(current_cpu != -1);

	PTHREAD_MUTEX_LOCK(&counter->lock);

	printf(
		"CPU: %d, Thread ID: %d\n",
		current_cpu,
		gettid()
	);

	for (i = 0; i < COUNTER_MAX; ++i)
		++(counter->value);

	PTHREAD_MUTEX_UNLOCK(&counter->lock);

	return NULL;
}
