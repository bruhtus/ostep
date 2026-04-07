#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
 * We can use atomic_int (from stdatomic.h)
 * to make sure that the increment is atomic.
 */
static int counter;
static atomic_int atomic_counter;

void *worker(void *);

/*
 * References:
 * - OSTEP Section 2.3.
 * - https://learnxbyexample.com/c/atomic-counters/
 */
int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <value>\n", argv[0]);
		return 69;
	}

	pthread_t p1, p2;
	int retval_pthread;
	char *end_ptr;

	const char *loops_input = argv[1];

	/*
	 * Using strtol() instead of atoi() because
	 * it's hard to check the error when using
	 * atoi(), and also there's no indicator for
	 * integer overflow or underflow with atoi().
	 *
	 * Reference:
	 * https://sqlpey.com/c/fixing-atoi-issues/
	 */
	long loops = strtol(loops_input, &end_ptr, 10);

	if (errno == ERANGE) {
		perror("strtol() failed");
		return 69;
	} else if (end_ptr == loops_input) {
		fprintf(
			stderr,
			"No digits found in %s\n",
			loops_input
		);

		return 69;
	} else if (*end_ptr != '\0') {
		fprintf(
			stderr,
			"Further characters after number\n"
		);

		return 69;
	}

	printf("Initial value (non-atomic): %d\n", counter);
	printf("Initial value (atomic): %d\n", atomic_counter);

	long loops_per_thread[] = {
		loops,
		loops,
	};

	/*
	 * Passing the argument for worker()
	 * in the last argument of pthread_create().
	 *
	 * Reference:
	 * https://hpc-tutorials.llnl.gov/posix/passing_args/
	 */
	retval_pthread = pthread_create(
		&p1,
		NULL,
		worker,
		loops_per_thread
	);

	if (retval_pthread != 0) {
		fprintf(
			stderr,
			"Error creating pthread with code: %d\n",
			retval_pthread
		);

		return 69;
	}

	retval_pthread = pthread_create(
		&p2,
		NULL,
		worker,
		(loops_per_thread + 1)
	);

	if (retval_pthread != 0) {
		fprintf(
			stderr,
			"Error creating pthread with code: %d\n",
			retval_pthread
		);

		return 69;
	}

	retval_pthread = pthread_join(p1, NULL);

	if (retval_pthread != 0) {
		fprintf(
			stderr,
			"Error joining pthread with code: %d\n",
			retval_pthread
		);

		return 69;
	}

	retval_pthread = pthread_join(p2, NULL);

	if (retval_pthread != 0) {
		fprintf(
			stderr,
			"Error joining pthread with code: %d\n",
			retval_pthread
		);

		return 69;
	}

	printf("Final value (non-atomic): %d\n", counter);
	printf("Final value (atomic): %d\n", atomic_counter);

	return 0;
}

void *worker(void *arg)
{
	long i, loops;

	/*
	 * Reference:
	 * https://www.gnu.org/software/c-intro-and-ref/manual/html_node/Void-Pointers.html
	 */
	if (arg != NULL)
		loops = *((long *)arg);
	else
		loops = 100000;

	for (i = 0; i < loops; ++i) {
		++counter;
		++atomic_counter;
	}

	return NULL;
}
