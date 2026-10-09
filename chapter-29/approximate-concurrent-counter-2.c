#define _GNU_SOURCE

#include <errno.h>
#include <sched.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "../libs-local/utils.h"
#include "../libs-local/measurement.h"

#define COUNTER_MAX 100000

struct counter_info {
	pthread_mutex_t lock;
	unsigned value;
};

/*
 * local_counter is per-CPU core counter.
 */
struct thread_info {
	struct counter_info *global_counter;
	struct counter_info *local_counter;
	pthread_t thread_id;
	long sync_threshold;
};

static void *thread_exec(void *);
static void print_thread_err(
	int,
	int,
	const char *,
	int,
	pthread_t
);

int main(void)
{
	struct counter_info global_counter;
	struct timespec start_time, end_time, result_time;
	pthread_attr_t pthread_attr;
	cpu_set_t cpu_set;
	long i, j, last_iterator;
	int retval;

	long num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
	if (num_cpus == -1) {
		print_err(errno, __LINE__, "sysconf()");
		return 69;
	}

	struct counter_info local_counter[num_cpus];

	/*
	 * On linux pthread_mutex_init() always return 0.
	 */
	pthread_mutex_init(&global_counter.lock, NULL);
	global_counter.value = 0;

	retval = pthread_attr_init(&pthread_attr);
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

		printf("THREADS: %ld\n", num_threads);

		for (j = 0; j < num_threads; ++j) {
			pthread_mutex_init(
				&local_counter[j].lock,
				NULL
			);
			local_counter[j].value = 0;

			threads[j].local_counter = local_counter + j;
			threads[j].global_counter = &global_counter;
			/*
			 * Make sure that the sync_threshold
			 * is less than or equal to COUNTER_MAX
			 * so that the global counter will
			 * be updated.
			 */
			threads[j].sync_threshold = 5;

			CPU_ZERO(&cpu_set);
			CPU_SET(j, &cpu_set);

			retval = pthread_attr_setaffinity_np(
				&pthread_attr,
				sizeof(cpu_set),
				&cpu_set
			);
			if (retval) {
				last_iterator = j + 1;
				print_err(
					retval,
					__LINE__,
					"pthread_attr_setaffinity_np()"
				);
				goto cleanup;
			}

			retval = pthread_create(
				&threads[j].thread_id,
				&pthread_attr,
				thread_exec,
				threads + j
			);
			if (retval) {
				last_iterator = j + 1;
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
				threads[j].thread_id,
				NULL
			);
			if (retval) {
				last_iterator = j + 1;
				print_err(
					retval,
					__LINE__,
					"pthread_join()"
				);
				goto cleanup;
			}
		}

		last_iterator = num_threads;

		printf(
			"global counter: %u\n",
			global_counter.value
		);

		/*
		 * Reset the global counter for the next
		 * scenario.
		 */
		global_counter.value = 0;

		retval = clock_gettime(
			CLOCK_REALTIME,
			&end_time
		);
		if (retval == -1) {
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

	retval = pthread_attr_destroy(&pthread_attr);
	if (retval) {
		print_err(
			retval,
			__LINE__,
			"pthread_attr_destroy()"
		);
		return 69;
	}

	retval = pthread_mutex_destroy(&global_counter.lock);
	if (retval) {
		print_err(
			retval,
			__LINE__,
			"pthread_mutex_destroy()"
		);
		return 69;
	}

	for (i = 0; i < last_iterator; ++i) {
		retval = pthread_mutex_destroy(
			&local_counter[i].lock
		);
		if (retval) {
			print_err(
				retval,
				__LINE__,
				"pthread_mutex_destroy()"
			);
			return 69;
		}
	}

	if (prev_retval)
		return 69;

	return 0;
}

static void *thread_exec(void *params)
{
	unsigned i;
	int retval;

	struct thread_info *thread = params;
	struct counter_info *local_counter = thread->local_counter;
	struct counter_info *global_counter = thread->global_counter;

	int current_cpu = sched_getcpu();
	if (current_cpu == -1) {
		print_err(errno, __LINE__, "sched_getcpu()");
		return NULL;
	}

	retval = pthread_mutex_lock(&local_counter->lock);
	if (retval) {
		print_thread_err(
			retval,
			__LINE__,
			"pthread_mutex_lock()",
			current_cpu,
			thread->thread_id
		);
		return NULL;
	}

	for (i = 0; i < COUNTER_MAX; ++i) {
		++(local_counter->value);

		if (local_counter->value >= thread->sync_threshold) {
			retval = pthread_mutex_lock(&global_counter->lock);
			if (retval) {
				print_thread_err(
					retval,
					__LINE__,
					"pthread_mutex_lock()",
					current_cpu,
					thread->thread_id
				);
				return NULL;
			}

			global_counter->value += local_counter->value;

			retval = pthread_mutex_unlock(&global_counter->lock);
			if (retval) {
				print_thread_err(
					retval,
					__LINE__,
					"pthread_mutex_unlock()",
					current_cpu,
					thread->thread_id
				);
				return NULL;
			}

			local_counter->value = 0;
		}
	}

	retval = pthread_mutex_unlock(&local_counter->lock);
	if (retval) {
		print_thread_err(
			retval,
			__LINE__,
			"pthread_mutex_unlock()",
			current_cpu,
			thread->thread_id
		);
		return NULL;
	}

	return NULL;
}

static void print_thread_err(
	int err_num,
	int line_num,
	const char *func_name,
	int current_cpu,
	pthread_t thread_id
)
{
	print_err(err_num, line_num, func_name);
	dprintf(
		2,
		"CPU: %d, thread ID: %ld\n",
		current_cpu,
		thread_id
	);
}
