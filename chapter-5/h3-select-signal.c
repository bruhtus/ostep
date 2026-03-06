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

static void sig_handler(int signum)
{
	printf("good bye\n");
}
