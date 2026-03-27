#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

/*
 * Reference:
 * https://shunchild.com/article/will-closing-pipe-ends-in-parent-close-all-ends-c
 */
int main(void)
{
	int fds[2];

	if (pipe(fds) == -1) {
		perror("pipe() error");
		return 69;
	}

	pid_t rc_fork = fork();

	switch (rc_fork) {
		case -1:
			perror("fork() error");
			return 69;

		case 0:
			printf("fds[0] child: %d\n", fds[0]);
			printf("fds[1] child: %d\n", fds[1]);

			/*
			 * While the parent and child process
			 * share the same file descriptors
			 * initially, closing the file
			 * descriptors in the parent process
			 * does not effect the file
			 * descriptors in the child process,
			 * and vice versa.
			 *
			 * So we need to explicitly close
			 * the file descriptors if it's not
			 * needed.
			 */
			if (close(fds[0]) == -1) {
				perror("close() read pipe error");
				return 69;
			}

			int sigchld = SIGCHLD;

			if (write(fds[1], &sigchld, sizeof(sigchld)) == -1) {
				perror("write() error");
				return 69;
			}

			if (close(fds[1]) == -1) {
				perror("close() write pipe error");
				return 69;
			}

			break;

		default:
			/*
			 * Looks like in this case,
			 * the parent and child process using
			 * the same file descriptor number but
			 * on different process.
			 */
			printf("fds[0] parent: %d\n", fds[0]);
			printf("fds[1] parent: %d\n", fds[1]);

			if (close(fds[1]) == -1) {
				perror("close() write pipe error");
				return 69;
			}

			int signo;

			if (read(fds[0], &signo, sizeof(signo)) == -1) {
				perror("read() error");
				return 69;
			}

			if (close(fds[0]) == -1) {
				perror("close() read pipe error");
				return 69;
			}

			printf("signo: %d\n", signo);

			if (wait(NULL) == -1) {
				perror("wait() error");
				return 69;
			}

			break;
	}

	return 0;
}
