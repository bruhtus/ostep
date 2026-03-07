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
 * and signal() to wait the child process
 * finish before continue the parent process.
 *
 * References:
 * - https://stackoverflow.com/a/18476331
 * - https://en.cppreference.com/w/c/program/SIG_ERR
 */
int main(void)
{
	/*
	 * According to `man 2 sigaction` in linux,
	 * POSIX leave it unspecified whether SIGCHLD
	 * signal is generated when a child process
	 * terminates. That means there's no guarantee
	 * that we will have SIGCHLD when the child
	 * process terminate on another system than
	 * linux.
	 *
	 * What happen when we put signal() after calling
	 * fork()?
	 * Currently still not sure what is the difference
	 * between calling signal() before or after fork().
	 */
	if (signal(SIGCHLD, sig_handler) == SIG_ERR) {
		perror("signal() error");
		return 1;
	}

	/*
	 * Is there any difference using `exit()` or `return`
	 * in child process `main()`?
	 */
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
