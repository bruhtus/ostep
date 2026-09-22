#include <stdio.h>
#include <string.h>
#include <pthread.h>

void *my_thread(void *arg);

int main(void)
{
	pthread_t p1, p2;
	const char *err;
	int retval;

	char *p1_arg = "A";
	char *p2_arg = "B";

	puts("main: begin");

	/*
	 * We can make the thread running in sequential order
	 * by create the thread in specific order and
	 * immediately wait for those thread (after creation).
	 */
	retval = pthread_create(
		&p2,
		NULL,
		my_thread,
		p2_arg
	);
	if (retval != 0) {
		err = strerror(retval);
		printf(
			"pthread_create() failed: %s (line %d)\n",
			err,
			__LINE__
		);
		return 69;
	}

	retval = pthread_join(p2, NULL);
	if (retval != 0) {
		err = strerror(retval);
		printf(
			"pthread_join() failed: %s (line %d)\n",
			err,
			__LINE__
		);
		return 69;
	}

	retval = pthread_create(
		&p1,
		NULL,
		my_thread,
		p1_arg
	);
	if (retval != 0) {
		err = strerror(retval);
		printf(
			"pthread_create() failed: %s (line %d)\n",
			err,
			__LINE__
		);
		return 69;
	}

	/*
	 * Wait the pthread to terminate.
	 */
	retval = pthread_join(p1, NULL);
	if (retval != 0) {
		err = strerror(retval);
		printf(
			"pthread_join() failed: %s (line %d)\n",
			err,
			__LINE__
		);
		return 69;
	}

	puts("main: end");

	return 0;
}

void *my_thread(void *arg)
{
	printf("%s\n", (const char *)arg);
	return NULL;
}
