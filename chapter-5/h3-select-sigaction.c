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
	/*
	 * The sa_mask is a temporary signal mask used
	 * during the signal handler execution, which means
	 * when we encounter those signal during
	 * the execution of the signal handler, we will
	 * _ignore_ those signal (?).
	 *
	 * In the example below, we will ignore SIGINT signal
	 * (usually using `ctrl-c`) when we are executing
	 * the signal handler. And after we finish the signal
	 * handler, the SIGINT signal will be evaluated in
	 * the main program.
	 *
	 * References:
	 * - https://csresources.github.io/SystemProgrammingWiki/SystemProgramming/Signals,-Part-2:-Pending-Signals-and-Signal-Masks/
	 * - https://cs341.cs.illinois.edu/coursebook/Signals
	 * - https://sourceware.org/glibc/manual/latest/html_node/Process-Signal-Mask.html
	 * - https://www.man7.org/tlpi/code/online/dist/signals/signal_functions.c.html
	 * - https://stackoverflow.com/a/17572787
	 */
	sigset_t sig_mask;
	sigemptyset(&sig_mask);
	sigaddset(&sig_mask, SIGINT);

	struct sigaction act = {
		.sa_handler = sig_handler,
		.sa_mask = sig_mask,
		.sa_flags = 0
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
 * a signal handler. Reenterant in here means that the function
 * can be frozen at any point and executed again.
 *
 * References:
 * - `man signal-safety`
 * - https://cs341.cs.illinois.edu/coursebook/Signals#handling-signals
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

			sleep(5);

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
