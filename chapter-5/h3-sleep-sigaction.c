#include <stdio.h>
#include <signal.h>
#include <unistd.h>

static void sig_handler(int);

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

	switch (retval_fork) {
		case -1:
			fprintf(stderr, "fork failed\n");
			return 69;

		case 0:
			printf("hello\n");
			break;

		default:
			/*
			 * Because we didn't wait for any
			 * file descriptor to be ready, we
			 * can use sleep() instead of select()
			 * or pselect().
			 */
			sleep(2);
			break;
	}

	return 0;
}

static void sig_handler(int signum)
{
	char msg[] = "good bye\n";

	write(STDOUT_FILENO, msg, sizeof(msg));
}
