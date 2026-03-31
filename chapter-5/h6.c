#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
	const char *err;
	pid_t retval_fork, retval_waitpid;

	retval_fork = fork();

	switch (retval_fork) {
		case -1:
			err = strerror(errno);

			printf(
				"fork() failed: %s (on line %d)\n",
				err,
				__LINE__
			);

			return 69;

		case 0:
			retval_fork = fork();

			if (retval_fork == -1) {
				err = strerror(errno);

				printf(
					"fork() failed: %s (on line %d)\n",
					err,
					__LINE__
				);

				return 69;
			}

			printf("Child (PID: %d)\n", getpid());

			retval_waitpid = waitpid(-1, NULL, 0);

			if (retval_waitpid == -1) {
				err = strerror(errno);

				printf(
					"waitpid() failed: %s (on line %d)\n",
					err,
					__LINE__
				);

				return 69;
			}

			break;

		default:
			/*
			 * It looks like using WNOHANG make
			 * the parent process not waiting
			 * the child process to
			 * terminate (?).
			 */
			retval_waitpid = waitpid(-1, NULL, WNOHANG);

			if (retval_waitpid == -1) {
				err = strerror(errno);

				printf(
					"waitpid() failed: %s (on line %d)\n",
					err,
					__LINE__
				);

				return 69;
			}

			printf("Parent (PID: %d)\n", getpid());
	}

	return 0;
}
