#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
	int nice = 69;
	printf("nice (PID: %d): %d\n", getpid(), nice);

	int rc = fork();

	if (rc < 0) {
		fprintf(stderr, "fork failed\n");
		return 1;
	} else if (rc == 0) {
		printf(
			"nice (PID: %d): %d\n",
			getpid(),
			nice
		);

		/*
		 * Looks like when we changed the value
		 * in the child process, the value only
		 * changed in the child process and not
		 * in the parent process.
		 */
		nice = 42;
	} else {
		/*
		 * If we change the value before entering
		 * the child process, the value is gonna be
		 * transferred in the child process. But if we
		 * change the value in specific condition like
		 * this (only in the parent process), the
		 * value would not be transferred into the
		 * child process.
		 */
		nice = 69420;

		int rc_wait = waitpid(rc, NULL, 0);

		if (rc_wait < 1) {
			fprintf(stderr, "waidpid() failed\n");
			return 1;
		}
	}

	printf(
		"nice (PID: %d): %d\n",
		getpid(),
		nice
	);

	return 0;
}
