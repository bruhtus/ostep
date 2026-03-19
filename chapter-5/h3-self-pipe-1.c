#include <stdio.h>
#include <signal.h>
#include <unistd.h>

/*
 * References:
 * - http://cr.yp.to/docs/selfpipe.html
 * - https://nimfsoft.art/blog/2025/07/23/c-safe-signal-handling-self-pipe/
 * - https://lwn.net/Articles/176911/
 * - https://github.com/skuhl/sys-prog-examples/blob/master/simple-examples/pipe.c
 * - https://github.com/skuhl/sys-prog-examples/blob/master/simple-examples/poll.c
 * - https://shunchild.com/article/will-closing-pipe-ends-in-parent-close-all-ends-c
 * - https://biriukov.dev/docs/fd-pipe-session-terminal/1-file-descriptor-and-open-file-description/
 */
int main(void)
{
	int pipe_fds[2];

	int signo = 0;

	if (pipe(pipe_fds) == -1) {
		perror("pipe() error");
		return 69;
	}

	pid_t rc_fork = fork();

	switch (rc_fork) {
		case -1:
			perror("fork() error");
			return 69;

		/*
		 * Child process.
		 */
		case 0:
			if (close(pipe_fds[0]) == -1) {
				perror("Child process: close() read pipe error");
				return 69;
			}

			printf("hello\n");

			signo = SIGCHLD;

			/*
			 * Close the pipe when write() to the
			 * write end of the pipe error.
			 *
			 * References:
			 * - https://linuxvox.com/blog/waitpid-equivalent-with-timeout/
			 * - https://stackoverflow.com/a/31864860
			 * - https://stackoverflow.com/a/77334357
			 */
			if (write(pipe_fds[1], &signo, sizeof(signo)) == -1) {
				perror("Child process: write() error");

				if (close(pipe_fds[1]) == -1) {
					perror("Child process: close() write pipe error");
					return 69;
				}

				return 69;
			}

			if (close(pipe_fds[1]) == -1) {
				perror("Child process: close() write pipe error");
				return 69;
			}

			break;

		/*
		 * Parent process.
		 */
		default:
			if (close(pipe_fds[1]) == -1) {
				perror("Parent process: close() write pipe error");
				return 69;
			}

			/*
			 * Is there any guarantee that
			 * read() will wait until it got
			 * the file descriptor content?
			 *
			 * Still not sure but when we add
			 * sleep() before write() in the
			 * child process, we are waiting
			 * for the sleep() to finish (?).
			 * Is that a coincidence?
			 */
			ssize_t rc_read = read(
				pipe_fds[0],
				&signo,
				sizeof(signo)
			);

			if (close(pipe_fds[0]) == -1) {
				perror("Parent process: close() read pipe error");
				return 69;
			}

			if (rc_read == -1) {
				perror("Parent process: read() error");
				return 69;
			} else if (rc_read == 0) {
				printf("End of File\n");
				return 69;
			}

			if (signo == SIGCHLD)
				printf("good bye\n");

			break;
	}

	return 0;
}
