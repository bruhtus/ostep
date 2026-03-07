#include <signal.h>
#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>

static void sig_handler(int);

/*
 * Trying to make sure child process
 * finish first before parent process
 * without wait() or waitpid().
 *
 * In this case, we trying to use select()
 * and sigaction() to wait the child process
 * finish before continue the parent process.
 *
 * Reference:
 * https://github.com/skuhl/sys-prog-examples/blob/master/simple-examples/sigaction.c
 */
int main(void)
{
	struct sigaction act = {
		.sa_handler = sig_handler
	};

	/*
	 * According to `man 2 sigaction` in linux,
	 * POSIX leave it unspecified whether SIGCHLD
	 * signal is generated when a child process
	 * terminates. That means there's no guarantee
	 * that we will have SIGCHLD when the child
	 * process terminate on another system than
	 * linux.
	 *
	 * What happen when we put sigaction() after calling
	 * fork()?
	 * Currently still not sure what is the difference
	 * between calling sigaction() before or after fork().
	 */
	int rc_sigaction = sigaction(
		SIGCHLD,
		&act,
		NULL
	);

	if (rc_sigaction < 0) {
		perror("sigaction() error");
		return 1;
	}

	int rc_fork = fork();

	if (rc_fork < 0) {
		fprintf(stderr, "fork failed\n");
		return 1;
	} else if (rc_fork == 0) {
		printf("hello\n");
	} else {
		struct timeval timeout = {10, 0};

		int rc_delay = select(
			0,
			NULL,
			NULL,
			NULL,
			&timeout
		);

		if (rc_delay == 0)
			printf("child process timeout\n");
	}

	return 0;
}

static void sig_handler(int signum)
{
	switch (signum) {
		case SIGCHLD:
			printf("good bye\n");
			break;

		default:
			printf("Unrecognized signal\n");
			break;
	}
}
