#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

/*
 * References:
 * - https://pubs.opengroup.org/onlinepubs/9699919799/functions/exec.html
 * - https://www.baeldung.com/linux/exec-functions
 * - https://www.cse.cuhk.edu.hk/~ericlo/teaching/os/lab/4-ProcessBasic/execle.html
 */
int main(void)
{
	pid_t retval_fork = fork();

	switch (retval_fork) {
		case -1:
			fprintf(stderr, "fork failed\n");
			return 69;

		case 0:
			printf("Child (PID: %d)\n", getpid());

			/*
			 * Provide the environment variable
			 * to use in the executable.
			 */
			const char *const envp[] = {
				"LS_COLORS=ow=1;34;40",
				(char *)0
			};

			/*
			 * Should we use shell to be
			 * able to use the env variable
			 * in envp?
			 *
			 * It looks like we can only use
			 * environment variable related
			 * to the executable, and
			 * if we want to use environment
			 * variable not related to the
			 * executable, we need to use
			 * shell (?).
			 */
			int retval_execle = execle(
				"/bin/ls",
				"ls",
				"-1",
				"--color",
				(char *)0,
				envp
			);

			if (retval_execle == -1) {
				perror("execle() failed");
				return 69;
			}

			break;

		default:
			int wstatus;

			pid_t retval_waitpid = waitpid(
				retval_fork,
				&wstatus,
				0
			);

			if (retval_waitpid == -1) {
				perror("waitpid() failed");
				return 69;
			}

			printf(
				"Parent (PID: %d, child PID: %d, child_exit_code: %d)\n",
				getpid(),
				retval_waitpid,
				WEXITSTATUS(wstatus)
			);

			break;
	}

	return 0;
}
