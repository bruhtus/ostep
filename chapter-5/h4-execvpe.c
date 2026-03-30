/*
 * Looks like execvpe() is specific to glibc, so
 * we need to add _GNU_SOURCE to use it.
 */
#define _GNU_SOURCE

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
			 * Not sure if this is the right
			 * thing to do, because the first
			 * element is less than 8 characters.
			 * What would happen to the first
			 * element when it's not 8 characters?
			 */
			char cmd[][8] = {
				"ls",
				"-1",
				"--color",
			};

			/*
			 * Looks like we can add trailing
			 * comma in the array initialization,
			 * but not in the last argument of
			 * function call.
			 *
			 * Reference:
			 * https://stackoverflow.com/a/7043410
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
				(char *)0,
			};

			/*
			 * We look for the executable name
			 * from the environment variable
			 * PATH of the caller's environment,
			 * not from _envp_ argument.
			 */
			int retval_execvpe = execvpe(
				args[0],
				args,
				envp
			);

			if (retval_execvpe == -1) {
				perror("execvpe() failed");
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
