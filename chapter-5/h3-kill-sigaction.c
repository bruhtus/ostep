#include <stdio.h>
#include <signal.h>
#include <unistd.h>

static volatile sig_atomic_t is_child_finish;

static void sig_handler(int);

int main(void)
{
	sigset_t signal_set;

	pid_t parent_pid = getpid();

	if (sigemptyset(&signal_set) == -1) {
		perror("sigemptyset() failed");
		return 69;
	}

	/*
	 * Looks like we can also add trailing
	 * comma in struct initialization.
	 */
	const struct sigaction act = {
		.sa_handler = sig_handler,
		.sa_flags = 0,
		.sa_mask = signal_set,
	};

	if (sigaction(SIGCHLD, &act, NULL) == -1) {
		perror("sigaction() failed");
		return 69;
	}

	pid_t retval_fork = fork();

	switch (retval_fork) {
		case -1:
			fprintf(stderr, "fork failed\n");
			return 69;

		case 0:
			puts("hello");

			/*
			 * It looks like we can send the
			 * SIGCHLD signal using kill().
			 * Maybe we can use this when
			 * the system didn't automatically
			 * send SIGCHLD signal when the
			 * child process terminate (?).
			 *
			 * Currently still not sure how to
			 * replicate the scenario when the
			 * _system didn't send_ SIGCHLD signal
			 * when child process terminate. So
			 * can't test if this is true or not.
			 */
			if (kill(parent_pid, SIGCHLD) == -1) {
				perror("kill() failed");
				return 69;
			}

			break;

		default:
			sleep(1);

			if (is_child_finish == 1)
				puts("good bye");
			else if (is_child_finish == -1)
				puts("unknown signal");

			break;
	}

	return 0;
}

static void sig_handler(int signum)
{
	switch (signum) {
		case SIGCHLD:
			is_child_finish = 1;
			break;

		default:
			is_child_finish = -1;
			break;
	}
}
