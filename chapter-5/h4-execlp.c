#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

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
			 * Unlike execl(), we can provide
			 * only the executable name, instead
			 * of full path of the executable.
			 *
			 * If we provide the first argument
			 * with slash (/), which means we
			 * only provide with the executable
			 * name, execlp() and others exec
			 * function variants will search the
			 * executable in PATH environment
			 * variable of the current shell.
			 */
			if (
				execlp(
					"ls",
					"ls",
					"-1",
					(void *)0
				) == -1
			) {
				perror("execlp() failed");
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
