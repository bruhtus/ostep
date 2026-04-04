#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

/*
 * If we use _parent_ process to write to the pipe and using
 * execvp() in the _child_ process, the shell prompt will not
 * appear after finishing the process in child process.
 *
 * Is this related to how the child process become
 * _orphaned_ process?
 *
 * References:
 * - https://stackoverflow.com/a/50669929
 * - https://stackoverflow.com/a/11636056
 * - https://www.cs.uleth.ca/~holzmann/C/system/pipeforkexec.html
 */
int main(void)
{
	int pipe_fds[2];

	if (pipe(pipe_fds) == -1) {
		perror("pipe() failed");
		return 69;
	}

	pid_t retval_fork = fork();

	switch (retval_fork) {
		case -1:
			perror("fork() failed");
			return 69;

		case 0:
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

		default:
			pid_t retval_waitpid = waitpid(
				-1,
				NULL,
				WNOHANG
			);

			if (retval_waitpid == -1) {
				perror("waitpid() failed");
				return 69;
			}

			if (close(pipe_fds[0]) == -1) {
				perror("close() failed");
				return 69;
			}

			/*
			 * Because we already _connect_
			 * the pipe_fds[1] file _description_
			 * to stdout file _descriptor_, we
			 * can use stdout file _descriptor_
			 * to write data into pipe_fds[1].
			 */
			if (dup2(pipe_fds[1], STDOUT_FILENO) == -1) {
				perror("dup2() failed");
				return 69;
			}

			if (close(pipe_fds[1]) == -1) {
				perror("close() failed");
				return 69;
			}

			puts("hello world");

			break;
	}

	return 0;
}
