/*
 * This is an experiment to see how CPU core for the main()
 * thread might change as we spawn new threads. This behavior
 * might not be consistent, so do not depend on it.
 */

#define _GNU_SOURCE

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <sched.h>
#include <unistd.h>

#include "../libs-local/utils.h"

static void *thread_exec(void *);

int main(void)
{
	int i, retval;

	long num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
	if (num_cpus == -1) {
		perror("sysconf() failed");
		return 69;
	}

	pthread_t thread_ids[num_cpus];

	for (i = 0; i < num_cpus; ++i) {
		retval = sched_getcpu();
		if (retval == -1) {
			perror("sched_getcpu() failed");
			return 69;
		}

		printf(
			"main() current CPU: %d, thread ID: %ld\n",
			retval,
			pthread_self()
		);

		/*
		 * Why we can not put `thread_ids + i` in both
		 * first and fourth argument but we can use
		 * `thread_ids + i` as first argument and
		 * `&thread_ids[i]` as fourth argument?
		 */
		retval = pthread_create(
			thread_ids + i,
			NULL,
			thread_exec,
			&thread_ids[i]
		);
		if (retval) {
			print_err(
				retval,
				__LINE__,
				"pthread_create()"
			);
			return 69;
		}

		/*
		 * This make running the thread in serialize
		 * instead of parallel, which looks like
		 * we are reusing the thread ID (?).
		 */
		retval = pthread_join(
			thread_ids[i],
			NULL
		);
		if (retval) {
			print_err(
				retval,
				__LINE__,
				"pthread_join()"
			);
			return 69;
		}
	}

	return 0;
}

static void *thread_exec(void *args)
{
	int retval;

	pthread_t *thread_id = args;

	retval = sched_getcpu();
	if (retval == -1) {
		perror("sched_getcpu() failed");
		return NULL;
	}

	printf(
		"  current CPU: %d, thread ID: %ld\n",
		retval,
		*thread_id
	);

	return NULL;
}
