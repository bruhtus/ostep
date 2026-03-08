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

		/*
		 * We block the parent process with
		 * select() for 10 seconds, and then
		 * when the child process finish,
		 * we execute the sig_handler().
		 *
		 * So if the child process take longer
		 * than the timeout, the sig_handler()
		 * might not be executed.
		 */
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

/*
 * When performing buferred I/O on a file, the stdio
 * functions must maintain the allocated data buffer along
 * with associated counters and indexes (or pointers) that
 * record the amount of data and the current position in the
 * buffer.
 *
 * Let's say the main program is in the middle of calling
 * printf(), where the buffer and associated variables have
 * been partially updated. During that printf() process in
 * main program, if the program interrupted by a signal
 * handler that also call printf(), then the printf() in
 * signal handler will operate on inconsistent data with
 * unpredictable result.
 *
 * In general, a function is async-signal-safe either because
 * it is reenterant or its execution can't be interupted by
 * a signal handler.
 *
 * Reference:
 * `man signal-safety`
 */
static void sig_handler(int signum)
{
	switch (signum) {
		case SIGCHLD:
			char msg[] = "good bye\n";

			write(
				STDOUT_FILENO,
				msg,
				sizeof(msg)
			);

			break;

		default:
			char default_msg[] = "Unrecognized signal\n";

			write(
				STDOUT_FILENO,
				default_msg,
				sizeof(default_msg)
			);

			break;
	}
}
