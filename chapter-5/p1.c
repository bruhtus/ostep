#include <stdio.h>
#include <unistd.h>

/*
 * Reference:
 * https://pages.cs.wisc.edu/~remzi/OSTEP/cpu-api.pdf
 */
int main(void)
{
	printf("Start (PID: %d)\n", getpid());

	/*
	 * We are kind of copying the instruction
	 * after the call to fork() into another process.
	 *
	 * And the value of rc (which i assumed stands
	 * for "return code"?) will change depends on
	 * which process we are currently in.
	 *
	 * If the fork process succeed, the PID of child
	 * process is returned in the parent process and
	 * 0 is returned in the child process. And if the
	 * fork process failed, -1 is returned in the
	 * _parent process_.
	 */
	int rc = fork();

	printf("return code: %d\n", rc);

	if (rc < 0) {
		fprintf(stderr, "fork failed\n");
		return 1;
	} else if (rc == 0) {
		printf("Child (PID: %d)\n", getpid());
	} else {
		printf(
			"Parent of %d (PID: %d)\n",
			rc,
			getpid()
		);
	}

	return 0;
}
