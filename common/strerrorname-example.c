/*
 * Because strerrorname_np() is specific to
 * glibc (GNU C Library), we need to add
 * _GNU_SOURCE to use it.
 *
 * Reference:
 * https://stackoverflow.com/a/76625962
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

static void exit_before_open(void);
static void exit_after_open(void);

/*
 * Reference:
 * https://www.codegenes.net/blog/how-can-i-print-the-symbolic-name-of-an-errno-in-c/
 */
int main(void)
{
	atexit(exit_before_open);

	int fd = open("anu.txt", 0);

	if (fd == -1) {
		const char *errno_name = strerrorname_np(errno);
		const char *errno_desc = strerrordesc_np(errno);

		printf(
			"errno_name: %s\n",
			errno_name ? errno_name : "Unknown errno"
		);

		printf(
			"errno_desc: %s\n",
			errno_desc ? errno_desc : "Unknown errno"
		);

		/*
		 * If we didn't want to trigger the handler in
		 * atexit(), we can use _exit().
		 *
		 * _exit() and exit() is a different function.
		 */
		return 69;
	}

	/*
	 * This atexit() handler is not triggered when we
	 * return early.
	 *
	 * The execution order of atexit() is
	 * _last in, first out_.
	 */
	atexit(exit_after_open);

	return 0;
}

/*
 * Currently still not sure if we need to use async-safe
 * functions like in signal handler for atexit() handler
 * or not.
 */
static void exit_before_open(void)
{
	char msg[] = "before open()\n";
	char err_msg[] = "stdout is not available\n";

	/*
	 * We can trigger this condition by closing the
	 * stdout like this:
	 * close(STDOUT_FILENO);
	 */
	if (write(STDOUT_FILENO, msg, sizeof(msg)) == -1) {
		write(STDERR_FILENO, err_msg, sizeof(err_msg));
	}
}

static void exit_after_open(void)
{
	char msg[] = "after open()\n";
	write(STDOUT_FILENO, msg, sizeof(msg));
}
