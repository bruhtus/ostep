#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
	const char *err;

	pid_t retval_fork = fork();

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
			if (close(STDOUT_FILENO) == -1) {
				perror("close() failed");
				return 69;
			}

			/*
			 * printf() will write the message
			 * to the stdout
			 * (standard output stream), but
			 * the stdout file descriptor
			 * already close, so printf()
			 * can not write any message to
			 * stdout. That's means this message
			 * will not be printed out in the
			 * output device.
			 */
			printf("Child PID: %d", getpid());

			break;

		default:
			pid_t retval_wait = wait(NULL);

			if (retval_wait == -1) {
				perror("wait() failed");
				return 69;
			}

			/*
			 * Parent and child process have
			 * independent file descriptor, so
			 * when we close the file descriptor
			 * in child process, the file
			 * descriptor in parent process did
			 * not get effected.
			 */
			printf(
				"Parent PID: %d, Child PID: %d\n",
				getpid(),
				retval_wait
			);

			break;
	}

	return 0;
}
