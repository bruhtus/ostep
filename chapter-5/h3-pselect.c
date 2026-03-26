#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>

static void sig_handler(int);

/*
 * Reference:
 * https://lwn.net/Articles/176911/
 */
int main(void)
{
	sigset_t empty_set;

	if (sigemptyset(&empty_set) == -1) {
		perror("sigemptyset() failed");
		printf("On line: %d\n", __LINE__);
		return 69;
	}

	struct sigaction act = {
		.sa_handler = sig_handler,
		.sa_flags = 0,
		.sa_mask = empty_set
	};

	if (sigaction(SIGCHLD, &act, NULL) == -1) {
		perror("sigaction() failed");
		printf("On line: %d\n", __LINE__);
		return 69;
	}

	int retval_fork = fork();

	if (retval_fork == -1) {
		fprintf(stderr, "fork failed\n");
		return 69;
	} else if (retval_fork == 0) {
		printf("hello\n");
	} else {
		struct timespec timeout = {10, 0};

		/*
		 * Currently still not sure how to
		 * use the signal mask in pselect()
		 * correctly.
		 */
		int retval_pselect = pselect(
			0,
			NULL,
			NULL,
			NULL,
			&timeout,
			&empty_set
		);

		switch (retval_pselect) {
			case -1:
				/*
				* Reference:
				* https://stackoverflow.com/a/46014661
				*/
				if (errno != EINTR)
					perror("pselect() error");

				break;

			case 0:
				printf("pselect() timeout\n");
				break;

			default:
				printf("pselect() ready\n");
				break;
		}
	}

	return 0;
}

static void sig_handler(int signum)
{
	char msg[] = "good bye\n";

	write(STDOUT_FILENO, msg, sizeof(msg));
}
