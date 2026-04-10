#define _GNU_SOURCE

#include <stdio.h>
#include <sched.h>
#include <unistd.h>

int print_affinity(cpu_set_t *, long);

/*
 * Reference:
 * https://stackoverflow.com/a/50117787
 */
int main(void)
{
	cpu_set_t mask;
	int retval_print;

	long nproc = sysconf(_SC_NPROCESSORS_ONLN);

	if (nproc == -1) {
		perror("sysconf() failed");
		return 69;
	}

	printf("nproc: %ld\n", nproc);

	retval_print = print_affinity(&mask, nproc);

	if (retval_print != 0)
		return retval_print;

	CPU_ZERO(&mask);

	/*
	 * Do we need to provide at least _one_ cpu
	 * in the mask set for sched_setaffinity()?
	 */
	CPU_SET(1, &mask);

	if (sched_setaffinity(0, sizeof(mask), &mask) == -1) {
		perror("sched_setaffinity() failed");
		return 69;
	}

	retval_print = print_affinity(&mask, nproc);

	if (retval_print != 0)
		return retval_print;

	return 0;
}

int print_affinity(cpu_set_t *mask, long nproc)
{
	long i;

	if (sched_getaffinity(0, sizeof(*mask), mask) == -1) {
		perror("sched_getaffinity() failed");
		return 69;
	}

	printf("sched_getaffinity() = ");

	for (i = 0; i < nproc; ++i)
		printf("%d ", CPU_ISSET(i, mask));

	printf("\n");

	int cpu = sched_getcpu();

	if (cpu == -1) {
		perror("sched_getcpu() failed");
		return 69;
	}

	printf("cpu: %d\n", cpu);

	return 0;
}
