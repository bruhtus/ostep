#include "measurement.h"

#define NS_PER_SECOND 1000000000

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
