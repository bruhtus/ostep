#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
	pid_t retval_fork, retval_wait;

	retval_fork = fork();

	switch (retval_fork) {
		case -1:
			fprintf(stderr, "fork failed\n");
			return 69;

		case 0:
			/*
			 * The changes of variable's value
			 * in the child process will not
			 * be reflected on the parent
			 * process.
			 */
			retval_fork = fork();

			if (retval_fork == -1) {
				perror("fork() failed");
				printf(
					"On line: %d\n",
					__LINE__
				);
				return 69;
			}

			printf("Child (PID: %d)\n", getpid());

			retval_wait = wait(NULL);

			/*
			 * One of the failure condition for
			 * wait() is that when there's no
			 * child process from the current
			 * process.
			 */
			if (retval_wait == -1) {
				perror("wait() failed");
				return 69;
			}

			break;

		default:
			retval_wait = wait(NULL);

			if (retval_wait == -1) {
				perror("wait() failed");
				return 69;
			}

			printf("Parent (PID: %d)\n", getpid());

			break;
	}

	return 0;
}
