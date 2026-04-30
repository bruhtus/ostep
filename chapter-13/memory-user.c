#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
 * We can check the increase memory usage using
 * this command:
 * free --mega -h -L -s 1
 */
int main(int argc, char *(argv[]))
{
	long i;
	char *endptr;

	if (argc != 2) {
		printf(
			"%s need 1 argument (in megabytes)\n",
			argv[0]
		);
		return 69;
	}

	/*
	 * This is for `pmap` command.
	 * We can check the memory map for this
	 * program by running:
	 * pmap -x <PID>
	 */
	printf("PID: %d\n", getpid());

	long input = strtol(argv[1], &endptr, 10);

	if (errno == ERANGE) {
		perror("strtol() failed");
		return 69;
	} else if (*(argv[1]) == *endptr) {
		puts("invalid character for number");
		return 69;
	} else if (*endptr != '\0') {
		printf("first invalid character: %c\n", *endptr);
		return 69;
	}

	/*
	 * To show the effect of increase memory,
	 * the minimum input should be 50000000.
	 */
	long *arr = malloc(input * sizeof(*arr));

	for (i = 0; i < input; ++i) {
		arr[i] = 1000000;

		long mod = i % 1000000;

		if (mod == 0) {
			printf("sleep on i = %ld\n", i);
			sleep(1);
		}
	}

	free(arr);

	return 0;
}
