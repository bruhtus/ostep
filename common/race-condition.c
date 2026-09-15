#include <assert.h>
#include <stdio.h>
#include <stdatomic.h> // In case anyone want to check out atomic data structure.
#include <pthread.h>
#include <unistd.h>

#define PTHREAD_MUTEX_LOCK(lock) \
	assert(!pthread_mutex_lock(lock))
#define PTHREAD_MUTEX_UNLOCK(lock) \
	assert(!pthread_mutex_unlock(lock))

static long long accum;
static pthread_mutex_t accum_mutex = PTHREAD_MUTEX_INITIALIZER;

static void *square(void *);

/*
 * This is an example to replicate race condition.
 * Use mutex lock or atomic data structure to see the difference.
 *
 * Reference:
 * https://www.classes.cs.uchicago.edu/archive/2018/spring/12300-1/lab6.html
 */
int main(void)
{
	long i, max_threads;
	int retval;

	/*
	 * Use number of processors in the system as
	 * maximum threads spawn.
	 */
	max_threads = sysconf(_SC_NPROCESSORS_ONLN);
	assert(max_threads != -1);

	pthread_t threads[max_threads];

	for (i = 0; i < max_threads; ++i) {
		retval = pthread_create(
			threads + i,
			NULL,
			square,
			(void *)1
		);
		assert(!retval);
	}

	for (i = 0; i < max_threads; ++i) {
		retval = pthread_join(threads[i], NULL);
		assert(!retval);
	}

	printf("accum: %lld\n", accum);

	return 0;
}

/*
 * To trigger the race condition, we might need to run
 * this program quite A LOT of time.
 * For example (running the program 100000 times):
 * for i in {1..100000}; do ./common/race-condition; done | sort | uniq -c
 *
 * Or using A LOT of iteration in the source code like this.
 */
static void *square(void *params)
{
	long long i, num;

	num = (long long)params;

	/*
	 * There might some differences when using atomic
	 * data structure (such as atomic_llong) or mutex,
	 * but currently still not sure about that.
	 *
	 * In this case, using the atomic data structure
	 * should be enough. But when we need to do more
	 * complex operations, that is when we need to
	 * use mutex to protect those critical operations.
	 *
	 * Atomic data structure lists:
	 * https://en.cppreference.com/c/header/stdatomic
	 */
	/* PTHREAD_MUTEX_LOCK(&accum_mutex); */

	for (i = 0; i < 100000; ++i)
		accum += num + num;

	/* PTHREAD_MUTEX_UNLOCK(&accum_mutex); */

	return NULL;
}
