#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>

#include "../libs-local/measurement.h"

#define TOTAL_PAGES 10

/*
 * Is this the correct implementation for
 * measuring the cost of accessing page?
 *
 * References:
 * - https://stackoverflow.com/a/46633494
 * - https://linuxvox.com/blog/measure-page-faults-from-a-c-program/
 */
int main(int argc, char *argv[])
{
	struct timespec start_time, end_time, result_time;
	char *endptr;
	const char *err;
	long i, trials;
	int j, retval_gettime;
	cpu_set_t mask;

	/*
	 * number of pages to touch = TOTAL_PAGES
	 * number of trials = how many times should the page
	 *		      accessed (multiple iterations).
	 *
	 */
	if (argc != 2) {
		printf(
			"Usage: %s <num_trials>\n",
			argv[0]
		);
		return 1;
	}

	const char *input_trials = argv[1];

	trials = strtol(input_trials, &endptr, 10);
	if (errno == ERANGE) {
		err = strerror(errno);
		printf(
			"strtol() failed: %s (line %d)\n",
			err,
			__LINE__
		);
		return 2;
	} else if (*input_trials == *endptr) {
		printf(
			"invalid character for number (line %d)\n",
			__LINE__);
		return 2;
	} else if (*endptr != '\0') {
		printf(
			"first invalid character: %c (line %d)\n",
			*endptr,
			__LINE__
		);
		return 2;
	} else if (trials < 0) {
		printf(
			"input should not be negative (line %d)\n",
			__LINE__
		);
		return 2;
	}

	CPU_ZERO(&mask);
	CPU_SET(0, &mask);

	if (sched_setaffinity(0, sizeof(mask), &mask) == -1) {
		perror("sched_setaffinity() failed");
		return 42;
	}

	retval_gettime = clock_gettime(
		CLOCK_REALTIME,
		&start_time
	);
	if (retval_gettime == -1) {
		err = strerror(errno);
		printf(
			"clock_gettime() failed: %s (line %d)\n",
			err,
			__LINE__
		);
		return 69;
	}

	int arr[TOTAL_PAGES] = {0};
	for (i = 0; i < trials; ++i) {
		/*
		 * Why adding this or not make
		 * the timing different between
		 * the first and second element?
		 */
		printf("trial %ld\n", i);

		for (j = 0; j < TOTAL_PAGES; ++j) {
			arr[j] += 1;

			retval_gettime = clock_gettime(
				CLOCK_REALTIME,
				&end_time
			);
			if (retval_gettime == -1) {
				err = strerror(errno);
				printf(
					"clock_gettime() failed: %s (line %d)\n",
					err,
					__LINE__
				);
				return 69;
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
	}

	return 0;
}
