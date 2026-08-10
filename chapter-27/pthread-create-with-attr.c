#include <stdio.h>
#include <stdatomic.h>
#include <pthread.h>

void *thread_start(void *);

int main(void)
{
	pthread_t thread_1, thread_2;
	pthread_attr_t attr;
	size_t stack_size;
	void *stack_addr;
	atomic_int accumulator;
	int retval;

	retval = pthread_attr_init(&attr);
	if (retval != 0) {
		perror("pthread_attr_init() failed");
		return 69;
	}

	retval = pthread_attr_setstacksize(
		&attr,
		100000
	);
	if (retval != 0) {
		perror("pthread_attr_setstacksize() failed");
		return 69;
	}

	retval = pthread_attr_getstack(
		&attr,
		&stack_addr,
		&stack_size
	);
	if (retval != 0) {
		perror("pthread_attr_getstack() failed");
		return 69;
	}

	printf("stack address: %p\n", stack_addr);
	printf("stack size: %zu\n", stack_size);

	retval = pthread_create(
		&thread_1,
		&attr,
		thread_start,
		&accumulator
	);
	if (retval != 0) {
		perror("pthread_create() failed");
		return 69;
	}

	retval = pthread_create(
		&thread_2,
		&attr,
		thread_start,
		&accumulator
	);
	if (retval != 0) {
		perror("pthread_create() failed");
		return 69;
	}

	retval = pthread_attr_destroy(&attr);
	if (retval != 0) {
		perror("pthread_attr_destroy() failed");
		return 69;
	}

	retval = pthread_join(thread_1, NULL);
	if (retval != 0) {
		perror("pthread_join() failed");
		return 69;
	}

	retval = pthread_join(thread_2, NULL);
	if (retval != 0) {
		perror("pthread_join() failed");
		return 69;
	}

	printf("accumulator: %u\n", accumulator);

	return 0;
}

void *thread_start(void *arg)
{
	atomic_int *accumulator = arg;

	++(*accumulator);

	return NULL;
}
