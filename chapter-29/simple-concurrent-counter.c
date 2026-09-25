#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "../libs-local/measurement.h"

#define COUNTER_MAX 100

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
static void print_err(int, int, const char *);

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

	int retval = 0;

	long num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
	if (num_cpus == -1) {
		print_err(errno, __LINE__, "sysconf()");
		return 69;
	}

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
	if (retval) {
		print_err(
			retval,
			__LINE__,
			"pthread_attr_init()"
		);
		return 69;
	}

	retval = clock_gettime(
		CLOCK_REALTIME,
		&start_time
	);
	if (retval == -1) {
		print_err(errno, __LINE__, "clock_gettime()");
		goto cleanup;
	}

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
			retval = pthread_attr_setaffinity_np(
				&attr,
				sizeof(cpu_set),
				&cpu_set
			);
			if (retval) {
				print_err(
					retval,
					__LINE__,
					"pthread_attr_setaffinity_np()"
				);
				goto cleanup;
			}

			retval = pthread_create(
				&threads[j].thread,
				&attr,
				thread_exec,
				threads + j
			);
			if (retval) {
				print_err(
					retval,
					__LINE__,
					"pthread_create()"
				);
				goto cleanup;
			}
		}

		for (j = 0; j < num_threads; ++j) {
			retval = pthread_join(
				threads[j].thread,
				NULL
			);
			if (retval) {
				print_err(
					retval,
					__LINE__,
					"pthread_join()"
				);
				goto cleanup;
			}
		}

		printf("Counter: %u\n", counter.value);
		counter.value = 0;

		retval = clock_gettime(
			CLOCK_REALTIME,
			&end_time
		);
		if (retval) {
			print_err(
				errno,
				__LINE__,
				"clock_gettime()"
			);
			goto cleanup;
		}

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

cleanup:
	int prev_retval = retval;

	retval = pthread_attr_destroy(&attr);
	if (retval) {
		print_err(
			retval,
			__LINE__,
			"pthread_attr_destroy()"
		);
		return 69;
	}

	if (prev_retval)
		return 69;

	return 0;
}

static void *thread_exec(void *params)
{
	struct thread_info *info = params;
	unsigned i;
	int retval;

	struct counter_info *counter = info->counter;

	int current_cpu = sched_getcpu();
	if (current_cpu == -1) {
		print_err(errno, __LINE__, "sched_getcpu()");
		return NULL;
	}

	retval = pthread_mutex_lock(&counter->lock);
	if (retval) {
		print_err(
			retval,
			__LINE__,
			"pthread_mutex_lock()"
		);
		return NULL;
	}

	printf(
		"CPU: %d, Thread ID: %d\n",
		current_cpu,
		gettid()
	);

	for (i = 0; i < COUNTER_MAX; ++i)
		++(counter->value);

	retval = pthread_mutex_unlock(&counter->lock);
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

static void print_err(
	int err_num,
	int line_num,
	const char *func_name
)
{
	const char *err = strerror(err_num);

	printf(
		"%s failed: %s (line: %d)\n",
		func_name,
		err,
		line_num
	);
}
