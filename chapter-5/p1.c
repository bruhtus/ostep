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
	 * which process we are currently in. If we are
	 * in the parent process, it will have value of
	 * the child process ID (PID), and if we are in
	 * the child process, it will print out the return
	 * code of the fork() (?).
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
