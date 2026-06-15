#ifndef MEASUREMENT
#define MEASUREMENT

#include <time.h>

void sub_timespec(
	struct timespec,
	struct timespec,
	struct timespec *
);

#endif /* measurement.h included */
