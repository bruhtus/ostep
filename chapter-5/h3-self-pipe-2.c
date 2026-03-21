#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define ARRAY_LEN(array) (sizeof(array) / sizeof(*array))

/*
 * Set this to global static for atexit() handler.
 */
static int pipe_fds[2];
static int signo;

static void sigchld_exit(void);

/*
 * References:
 * - https://nimfsoft.art/blog/2025/07/23/c-safe-signal-handling-self-pipe/
 * - https://gist.github.com/jelford/80367bc790ad1a46a3aa25e413e4eaf2
 * - https://iifx.dev/en/articles/460140665/the-self-pipe-trick-waking-poll-for-server-shutdown
 */
int main(void)
{
	unsigned i;

	if (pipe(pipe_fds) == -1) {
		perror("pipe() error");
		return 69;
	}

	struct pollfd poll_fds[] = {
		{
			.fd = pipe_fds[0],
			.events = POLLIN
		}
	};

	pid_t rc_fork = fork();

	switch (rc_fork) {
		case -1:
			perror("fork() error");
			return 69;

		case 0:
			if (close(pipe_fds[0]) == -1) {
				perror("Child process: close() read pipe error");
				return 69;
			}

			printf("hello\n");

			/*
			 * When we change the variable in
			 * the child process, the variable
			 * value in the parent process is
			 * not effected. That's why we
			 * need pipe() to communicate
			 * between parent and child process.
			 */
			signo = SIGCHLD;
			atexit(sigchld_exit);

			break;

		default:
			if (close(pipe_fds[1]) == -1) {
				perror("Parent process: close() write pipe error");
				return 69;
			}

			for (i = 0; i < ARRAY_LEN(poll_fds); ++i) {
				int rc_poll = poll(
					poll_fds,
					ARRAY_LEN(poll_fds),
					1000
				);

				if (rc_poll == -1) {
					perror("Parent process: poll() error");
					return 69;
				} else if (rc_poll == 0) {
					printf("Parent process: poll() timed out\n");
					return 42;
				}

				/*
				 * We can have multiple bits in the
				 * `revents`, that's why we check the
				 * `revents` condition with bitwise
				 * operation to check if one of the bits
				 * provided exists in the `revents`, like
				 * POLLIN bits exist in `revents` bits or
				 * not.
				 *
				 * Reference:
				 * https://stackoverflow.com/a/78489980
				 */
				if (
					rc_poll > 0 &&
					poll_fds[i].revents & POLLIN &&
					poll_fds[i].fd == pipe_fds[0]
				) {
					ssize_t rc_read = read(
						poll_fds[i].fd,
						&signo,
						sizeof(signo)
					);

					if (close(poll_fds[i].fd) == -1) {
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

					continue;
				}
			}

			break;
	}

	return 0;
}

static void sigchld_exit(void)
{
	if (write(pipe_fds[1], &signo, sizeof(signo)) == -1) {
		perror("Child process: write() error");
	}

	if (close(pipe_fds[1]) == -1) {
		perror("Child process: close() write pipe error");
	}
}
