#include <stdio.h>
#include <signal.h>
#include <unistd.h>

/*
 * Using the type sig_atomic_t give guarantee
 * that the variable will be modified
 * atomically (?). This is to prevent race
 * condition when using async signal handler.
 *
 * Global or static variable have default value
 * of 0.
 *
 * References:
 * - https://linuxvox.com/blog/why-can-only-async-signal-safe-functions-be-called-from-signal-handlers-safely/#pitfall-3-ignoring-volatile-sig_atomic_t
 * - https://www.sanfoundry.com/c-tutorials-importance-volatile-data-context-signals/
 */
static volatile sig_atomic_t is_child_finish;

static void sigchld_handler(int);

int main(void)
{
	sigset_t empty_set;

	if (sigemptyset(&empty_set) == -1) {
		perror("sigemptyset() failed");
		printf("On line: %d\n", __LINE__);
		return 69;
	}

	struct sigaction act = {
		.sa_handler = sigchld_handler,
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
			puts("hello");
			break;

		default:
			/*
			 * Because we didn't wait for any
			 * file descriptor to be ready, we
			 * can use sleep() instead of select()
			 * or pselect().
			 */
			sleep(2);

			if (is_child_finish == 1)
				puts("good bye");
			else if (is_child_finish == -1)
				puts("unknown signal");

			break;
	}

	return 0;
}

static void sigchld_handler(int signum)
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
