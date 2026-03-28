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
			 * Looks like one of the difference
			 * between exec function with `l`,
			 * like execl() here, and exec
			 * function with `v`, like execv() is
			 * that we are using variadic function
			 * instead of an array (or vector).
			 *
			 * We need to terminate the execl()
			 * with NULL. We also need to provide
			 * _the path_ for the executable file.
			 *
			 * If we mismatch the executable path
			 * (the first argument) with the
			 * name (the second argument), execl()
			 * will replace the message from the
			 * executable with the provided name
			 * in the second argument.
			 *
			 * For example, if we provide arguments
			 * like this:
			 * execl("/bin/wc", "ls", "-1", NULL)
			 *
			 * The error message of `wc`
			 * executable will become like this:
			 * ls: invalid option -- '1'
			 * Try 'ls --help' for more information.
			 *
			 * We can try running `wc` executable
			 * directly to see the correct error
			 * message, like this:
			 * wc -1
			 *
			 * References:
			 * - http://www.linuxonly.nl/docs/2/2_GCC_4_warnings_about_sentinels.html
			 * - https://stackoverflow.com/a/12122764
			 */
			if (execl("/bin/ls", "ls", "-1", NULL) == -1) {
				perror("execl() failed");
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
