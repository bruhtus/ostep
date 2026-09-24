#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <sched.h>
#include <string.h>
#include <sys/resource.h>
#include <pthread.h>
#include <unistd.h>

static void *thread_exec(void *);

/*
 * Example of using SCHED_OTHER policy (default on linux) and
 * nice value.
 *
 * Still not sure if changing the nice value has any effect
 * on thread scheduling.
 */
int main(void)
{
	struct rlimit rlim;
	const char *err;
	long i;
	int retval;

	retval = getrlimit(
		RLIMIT_NICE,
		&rlim
	);
	if (retval == -1) {
		err = strerror(errno);
		printf(
			"getrlimit() failed: %s (line %d)\n",
			err,
			__LINE__
		);
		return 69;
	}

	/*
	 * If the maximum value is 0, that means we only
	 * able to increase the nice value in the positive
	 * direction, like 0, 1, 2, ..., 19.
	 */
	printf(
		"rlim_cur: %ld, rlim_max: %ld\n",
		rlim.rlim_cur,
		rlim.rlim_max
	);

	long num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
	if (num_cpus == -1) {
		err = strerror(errno);
		printf(
			"sysconf() failed: %s (line %d)\n",
			err,
			__LINE__
		);
		return 69;
	}

	pthread_t threads[num_cpus];

	for (i = 0; i < num_cpus; ++i) {
		/*
		 * How do we check if all threads already
		 * terminated when one thread creation failed
		 * and exit the program?
		 */
		retval = pthread_create(
			threads + i,
			NULL,
			thread_exec,
			(void *)i
		);
		if (retval) {
			err = strerror(retval);
			printf(
				"pthread_create() failed: %s (line %d)\n",
				err,
				__LINE__
			);
			return 69;
		}
	}

	for (i = 0; i < num_cpus; ++i) {
		retval = pthread_join(threads[i], NULL);
		if (retval) {
			err = strerror(retval);
			printf(
				"pthread_join() failed: %s (line %d)\n",
				err,
				__LINE__
			);
			return 69;
		}
	}

	return 0;
}

static void *thread_exec(void *params)
{
	const char *err;
	int retval;

	long idx = (long)params;

	int current_cpu = sched_getcpu();
	if (current_cpu == -1) {
		err = strerror(errno);
		printf(
			"sched_getcpu() failed: %s (line %d)\n",
			err,
			__LINE__
		);
		return NULL;
	}

	/*
	 * Keep in mind that idx variable is long which
	 * might cause integer overflow on nice() syscall.
	 *
	 * On linux, the nice value is per-thread attribute,
	 * which means that each thread in one process might
	 * have different nice value. That might not be the
	 * case on another system because POSIX standard
	 * define the nice value as a per-process attribute,
	 * not per-thread attribute.
	 */
	errno = 0;
	retval = nice(idx);
	if (retval == -1 && errno != 0) {
		err = strerror(errno);
		printf(
			"nice() failed: %s (line %d)\n",
			err,
			__LINE__
		);
		return NULL;
	}

	printf(
		"Thread number: %lu, CPU core: %d\n",
		idx,
		current_cpu
	);

	return NULL;
}
