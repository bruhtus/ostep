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

			char cmd[] = "ls";
			char cmd_args[] = "-1";

			char *const args[] = {
				cmd,
				cmd_args,
				NULL
			};

			if (execvp(args[0], args) == -1) {
				perror("execvp() failed");
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
