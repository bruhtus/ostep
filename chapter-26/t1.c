#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

static int counter = 0;

void *my_thread(void *arg);

int main(void)
{
	pthread_t p1, p2;
	const char *err;
	int retval;

	printf("main: begin (counter: %d)\n", counter);

	retval = pthread_create(
		&p1,
		NULL,
		my_thread,
		"A"
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

	retval = pthread_create(
		&p2,
		NULL,
		my_thread,
		"B"
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

	retval = pthread_join(p1, NULL);
	if (retval != 0) {
		err = strerror(errno);
		printf(
			"pthread_join() failed: %s (line %d)\n",
			err,
			__LINE__
		);
		return 69;
	}

	retval = pthread_join(p2, NULL);
	if (retval != 0) {
		err = strerror(errno);
		printf(
			"pthread_join() failed: %s (line %d)\n",
			err,
			__LINE__
		);
		return 69;
	}

	printf(
		"main: done with all threads (counter: %d)\n",
		counter
	);

	return 0;
}

void *my_thread(void *arg)
{
	int i;

	const char *section = arg;

	printf("%s: begin\n", section);

	for (i = 0; i < 1e7; ++i)
		++counter;

	printf("%s: done\n", section);

	return NULL;
}
