#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define NS_PER_SECOND 1000000000

void sub_timespec(
	struct timespec,
	struct timespec,
	struct timespec *
);

int main(void)
{
	struct timespec start_time, end_time, result_time;
	int buf[2], retval_gettime;

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
	if (read(STDIN_FILENO, buf, 2) == -1) {
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

/*
 * Reference:
 * https://stackoverflow.com/a/53708448
 */
void sub_timespec(
	struct timespec start,
	struct timespec end,
	struct timespec *result
)
{
	time_t sec = end.tv_sec - start.tv_sec;
	time_t nsec = end.tv_nsec - start.tv_nsec;

	/*
	 * Still not sure when nsec become negative, but
	 * it will become negative at some point (?).
	 */
	if (sec > 0 && nsec < 0) {
		nsec += NS_PER_SECOND;
		--sec;
	}

	result->tv_sec = sec;
	result->tv_nsec = nsec;
}
