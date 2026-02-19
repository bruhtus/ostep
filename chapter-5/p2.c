#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

/*
 * Reference:
 * https://pages.cs.wisc.edu/~remzi/OSTEP/cpu-api.pdf
 */
int main(void)
{
	printf("Start (PID: %d)\n", getpid());

	int rc = fork();

	if (rc < 0) {
		fprintf(stderr, "fork failed\n");
		return 1;
	} else if (rc == 0) {
		printf("Child (PID: %d)\n", getpid());
		return 42;
	} else {
		int wstatus;

		/*
		 * We are waiting for the child process
		 * to return first, before continuing
		 * the next instruction.
		 *
		 * Keep in mind that waitpid() might
		 * return _before_ the child process exit.
		 * According to man page, this system call
		 * will return immediately if the child
		 * process __changed__ state, not
		 * only when the child process finish but
		 * also when the child process resumed by
		 * a signal.
		 */
		int rc_wait = waitpid(rc, &wstatus, 0);

		/*
		 * `wstatus` store the status information
		 * from child process. When we return child
		 * process with exit code 42, `wstatus`
		 * will have value 10752 which is two 8-bit
		 * fields. So we are shifting by 8 bit like
		 * this `wstatus >> 8` to get the exit code,
		 * which is what the macro WEXITSTATUS()
		 * does.
		 *
		 * References:
		 * - https://stackoverflow.com/a/179652
		 * - https://stackoverflow.com/a/3659673
		 */
		printf(
			"Parent of %d (rc_wait: %d, PID: %d, wstatus: %d, child_exit_code: %d)\n",
			rc,
			rc_wait,
			getpid(),
			wstatus,
			WEXITSTATUS(wstatus)
		);
	}

	return 0;
}
