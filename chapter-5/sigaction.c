#include <signal.h>
#include <stdio.h>
#include <unistd.h>

static void sig_handler(int);

/*
 * Reference:
 * https://stackoverflow.com/a/17572787
 */
int main(void)
{
	struct sigaction new_act, old_act;
	int rc_sigaction;
	sigset_t sig_mask;

	sigemptyset(&sig_mask);

	/*
	 * This means that while _executing_
	 * the signal handler, the SIGTERM
	 * signal will be ignored. But after
	 * the signal handler _finish_, the
	 * SIGTERM will be evaluated in
	 * the main program.
	 *
	 * We can check this by using
	 * `ctrl-c` (to trigger SIGINT),
	 * and using
	 * `pkill --signal SIGTERM -f ./chapter-5/sigaction`
	 * immediately after hitting `ctrl-c`.
	 */
	sigaddset(&sig_mask, SIGTERM);

	new_act.sa_handler = sig_handler;
	new_act.sa_mask = sig_mask;
	new_act.sa_flags = 0;

	rc_sigaction = sigaction(
		SIGINT,
		NULL,
		&old_act
	);

	if (rc_sigaction < 0) {
		perror("sigaction() error");
		return 1;
	}

	/*
	 * Currently still not sure __when__
	 * the `old_act.sa_handler` will get
	 * SIG_IGN.
	 */
	if (old_act.sa_handler != SIG_IGN) {
		rc_sigaction = sigaction(
			SIGINT,
			&new_act,
			NULL
		);

		if (rc_sigaction < 0) {
			perror("sigaction() error");
			return 1;
		}
	}

	/*
	 * References:
	 * - https://www.programmersought.com/article/47703515150/
	 * - https://www.programmersought.com/article/45777882305/
	 */
	for (;;) {
		printf("in the loop\n");
		sleep(10);
	}

	return 0;
}

static void sig_handler(int signum)
{
	char msg[] = "hello from handler\n";

	write(
		STDOUT_FILENO,
		msg,
		sizeof(msg)
	);

	/*
	 * When we are inside the signal handler,
	 * the signal that we put in the sa_mask
	 * will be ignored until we finish the
	 * signal handler function (?).
	 */
	sleep(5);
}
