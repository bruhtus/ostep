#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define CONVERT_SIGLIST(signal) {signal, #signal}

struct siglist {
	const int signum;
	const char *signame;
};

/*
 * To list all the signal:
 * kill -l | tr ' ' '\n' | sed 's/^/SIG/'
 *
 * References:
 * - https://github.com/skuhl/sys-prog-examples/blob/master/simple-examples/sigprocmask.c
 * - https://pubs.opengroup.org/onlinepubs/009695399/basedefs/signal.h.html
 */
static void print_signal_set(const sigset_t *);

/*
 * References:
 * - https://linuxvox.com/blog/about-the-ambiguous-description-of-sigwait/
 * - https://pubs.opengroup.org/onlinepubs/009695399/basedefs/xbd_chap11.html
 * - https://sourceware.org/glibc/manual/latest/html_node/Termination-Signals.html
 */
int main(void)
{
	int rc_fork = fork();

	if (rc_fork < 0) {
		fprintf(stderr, "fork failed\n");
		return 1;
	} else if (rc_fork == 0) {
		printf("hello\n");
	} else {
		int signum_sigwait;

		sigset_t signal_set, old_signal_set;
		sigemptyset(&signal_set);
		sigaddset(&signal_set, SIGCHLD);

		/*
		 * Before we use sigwait(), we need
		 * to block the signal with sigprocmask().
		 * If we didn't do that, the sigwait() will
		 * suspend the execution until the signal
		 * set provided is in the _pending_ state.
		 *
		 * Pending state in here means that the
		 * signal already generated but have not
		 * delivered yet.
		 *
		 * If we didn't block the signal with
		 * sigprocmask(), the signal will be
		 * delivered immediately without going
		 * through the _pending_ state.
		 */
		int rc_sigprocmask = sigprocmask(
			SIG_SETMASK,
			&signal_set,
			&old_signal_set
		);

		if (rc_sigprocmask < 0) {
			perror("sigprocmask() failed");
			return 69;
		}

		int rc_sigwait = sigwait(
			&signal_set,
			&signum_sigwait
		);

		if (rc_sigwait != 0) {
			perror("sigwait() failed");
			return 69;
		}

		printf("good bye\n");

		/*
		 * Try to see if there's any signal set
		 * before sigprocmask(), and it looks like
		 * there's none.
		 */
		print_signal_set(&old_signal_set);
	}

	return 0;
}

static void print_signal_set(const sigset_t *signal_set)
{
	unsigned i;

	/*
	 * This is the available signals from
	 * command `kill -l`, except for
	 * `SIGKILL` and `SIGSTOP` because
	 * we can't catch those signals.
	 *
	 * Check "ERRORS" section from
	 * `man 2 sigaction` for reference.
	 */
	const struct siglist signal_list[] = {
		CONVERT_SIGLIST(SIGHUP),
		CONVERT_SIGLIST(SIGINT),
		CONVERT_SIGLIST(SIGQUIT),
		CONVERT_SIGLIST(SIGILL),
		CONVERT_SIGLIST(SIGTRAP),
		CONVERT_SIGLIST(SIGIOT),
		CONVERT_SIGLIST(SIGBUS),
		CONVERT_SIGLIST(SIGFPE),
		CONVERT_SIGLIST(SIGUSR1),
		CONVERT_SIGLIST(SIGSEGV),
		CONVERT_SIGLIST(SIGUSR2),
		CONVERT_SIGLIST(SIGPIPE),
		CONVERT_SIGLIST(SIGALRM),
		CONVERT_SIGLIST(SIGTERM),
		CONVERT_SIGLIST(SIGSTKFLT),
		CONVERT_SIGLIST(SIGCHLD),
		CONVERT_SIGLIST(SIGCONT),
		CONVERT_SIGLIST(SIGTSTP),
		CONVERT_SIGLIST(SIGTTIN),
		CONVERT_SIGLIST(SIGTTOU),
		CONVERT_SIGLIST(SIGURG),
		CONVERT_SIGLIST(SIGXCPU),
		CONVERT_SIGLIST(SIGXFSZ),
		CONVERT_SIGLIST(SIGVTALRM),
		CONVERT_SIGLIST(SIGPROF),
		CONVERT_SIGLIST(SIGWINCH),
		CONVERT_SIGLIST(SIGPOLL),
		CONVERT_SIGLIST(SIGPWR),
		CONVERT_SIGLIST(SIGSYS)
	};

	const unsigned signal_len = sizeof(signal_list) / sizeof(*signal_list);

	for (i = 0; i < signal_len; ++i) {
		int rc = sigismember(
			signal_set,
			signal_list[i].signum
		);

		switch (rc) {
			case -1:
				perror("sigismember() error");
				exit(69);

			case 1:
				printf(
					"Signal %s in set\n",
					signal_list[i].signame
				);
				break;

			default:
				break;
		}
	}
}
