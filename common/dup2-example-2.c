#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

/*
 * Trying to emulate the scenario `echo 'hello world' | wc`
 * from the shell.
 *
 * Instead of using _child_ process to use execvp(), we are
 * using _parent_ process to use execvp() and _child_ process
 * to send the data for the command in execvp().
 *
 * References:
 * - https://stackoverflow.com/a/50669929
 * - https://www.cs.uleth.ca/~holzmann/C/system/pipeforkexec.html
 */
int main(void)
{
	int pipe_fds[2];

	pipe(pipe_fds);

	pid_t retval_fork = fork();

	switch (retval_fork) {
		case -1:
			perror("fork() failed");
			return 69;

		case 0:
			if (close(pipe_fds[0]) == -1) {
				perror("close() failed");
				return 69;
			}

			/*
			 * Instead of using the default file _description_
			 * for stdout, use pipe_fds[1] file _description_.
			 *
			 * Keep in mind that file _description_ is different
			 * from file _descriptor_.
			 */
			if (dup2(pipe_fds[1], STDOUT_FILENO) == -1) {
				perror("dup2() failed");
				return 69;
			}

			/*
			 * Because we already _connect_ the pipe_fds[1]
			 * file description to stdout, we can close
			 * the pipe_fds[1] file descriptor (or endpoint)
			 * and use stdout file descriptor instead.
			 */
			if (close(pipe_fds[1]) == -1) {
				perror("close() failed");
				return 69;
			}

			/*
			 * Because we already share the
			 * _file description_ for stdout and
			 * pipe_fds[1], everything put into
			 * stdout will be reflected into
			 * pipe_fds.
			 *
			 * Keep in mind that _file description_
			 * is different from _file descriptor_.
			 * Think of _file descriptor_ like
			 * endpoint to access the data storage
			 * called _file description_.
			 */
			puts("hello world");

			break;

		default:
			if (wait(NULL) == -1) {
				perror("wait() failed");
				return 69;
			}

			if (close(pipe_fds[1]) == -1) {
				perror("close() failed");
				return 69;
			}

			if (dup2(pipe_fds[0], STDIN_FILENO) == -1) {
				perror("dup2() failed");
				return 69;
			}

			if (close(pipe_fds[0]) == -1) {
				perror("close() failed");
				return 69;
			}

			char cmd[] = "wc";

			char *const args[] = {
				cmd,
				NULL
			};

			if (execvp(args[0], args) == -1) {
				perror("execvp() failed");
				return 69;
			}

			break;
	}

	return 0;
}
