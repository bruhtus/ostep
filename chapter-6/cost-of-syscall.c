#include <stdio.h>
#include <unistd.h>

#include "measurement.h"

/*
 * Reference:
 * https://stackoverflow.com/a/17126785 (macro vs enum)
 */
#define BUF_SIZE 2

int main(void)
{
	struct timespec start_time, end_time, result_time;
	int buf[BUF_SIZE], retval_gettime;

	retval_gettime = clock_gettime(
		CLOCK_REALTIME,
		&start_time
	);

	if (retval_gettime == -1) {
		perror("clock_gettime() failed");
		return 69;
	}

	/*
	 * Press Enter to finish the operation.
	 */
	if (read(STDIN_FILENO, buf, BUF_SIZE) == -1) {
		perror("read() failed");
		return 69;
	}

	retval_gettime = clock_gettime(
		CLOCK_REALTIME,
		&end_time
	);

	if (retval_gettime == -1) {
		perror("clock_gettime() failed");
		return 69;
	}

	sub_timespec(start_time, end_time, &result_time);

	printf(
		"%ld.%.9ld\n",
		result_time.tv_sec,
		result_time.tv_nsec
	);

	return 0;
}
