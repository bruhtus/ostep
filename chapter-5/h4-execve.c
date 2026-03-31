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
			 * We can also put the path of
			 * executable as the first element
			 * of argument vector, and use
			 * that directly in execve() as the
			 * path argument.
			 */
			char cmd[][8] = {
				"/bin/ls",
				"-1",
				"--color",
			};

			/*
			 * The first element is the
			 * executable name, and we need
			 * to terminated the array with
			 * null pointer.
			 */
			char *const args[] = {
				cmd[0],
				cmd[1],
				cmd[2],
				NULL,
			};

			char env_1[] = "LS_COLORS=ow=1;34;40";

			char *const envp[] = {
				env_1,
				NULL,
			};

			int retval_execve = execve(
				args[0],
				args,
				envp
			);

			if (retval_execve == -1) {
				perror("execve() failed");
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
