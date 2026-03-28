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

			char cmd_name[] = "ls";
			char cmd_args[] = "-1";

			char *const args[] = {
				cmd_name,
				cmd_args,
				(char *)0
			};

			/*
			 * If we mismatch the executable path
			 * (the first argument) with the name
			 * (the first element of args array),
			 * execv() will _replace_ the message
			 * from the executable with the
			 * provided name in the first element
			 * of args array.
			 */
			if (execv("/bin/ls", args) == -1) {
				perror("execv() failed");
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
