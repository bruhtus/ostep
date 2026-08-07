#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

void *my_thread(void *);

int main(void)
{
	pthread_t thread;
	long long value;
	const char *err;
	int retval;

	/*
	 * We need to use `long long` type here
	 * because `long long` type has the same
	 * size as pointer in 64-bit machine.
	 */
	printf(
		"pointer size: %zu\nlong long size: %zu\n",
		sizeof(void *),
		sizeof(long long)
	);

	/*
	 * Passing value 100 as pointer to my_thread().
	 */
	retval = pthread_create(
		&thread,
		NULL,
		my_thread,
		(void *)100
	);
	if (retval != 0) {
		err = strerror(errno);
		printf(
			"pthread_create() failed: %s (line %d)\n",
			err,
			__LINE__
		);
		return 69;
	}

	retval = pthread_join(
		thread,
		(void **)&value
	);
	if (retval != 0) {
		err = strerror(errno);
		printf(
			"pthread_join() failed: %s (line %d)\n",
			err,
			__LINE__
		);
		return 69;
	}

	printf("Returned %lld\n", value);

	return 0;
}

void *my_thread(void *arg)
{
	long long value = (long long)arg;
	printf("%lld\n", value);

	/*
	 * Return the value but treat that as a pointer.
	 */
	return (void *)(value + 1);
}
