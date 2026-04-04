#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define ARRAY_LEN(array) (sizeof(array) / sizeof(*array))

static int child_1(int, int);
static int child_2(int, int);

/*
 * Connect the standard output from one child process
 * to standard input from another child process using
 * pipe().
 *
 * Possible scenario:
 * parent: (using this)
 * - child 1:
 *   - child 2
 *
 * parent: (still not sure how to do this)
 * - child 1
 * - child 2
 *
 * References:
 * - https://github.com/veeso/popen-redirections
 * - https://stackoverflow.com/a/50669929
 * - https://stackoverflow.com/a/11636056
 */
int main(void)
{
	const char *err;

	pid_t retval_fork = fork();

	switch (retval_fork) {
		case -1:
			err = strerror(errno);

			printf(
				"fork() failed: %s (on line %d)\n",
				err,
				__LINE__
			);

			return 69;

		case 0:
			int pipe_fds[2];

			if (pipe(pipe_fds) == -1) {
				perror("pipe() error");
				return 69;
			}

			printf("Child (PID: %d)\n", getpid());

			pid_t retval_fork_2 = fork();

			if (retval_fork_2 == -1) {
				err = strerror(errno);

				printf(
					"fork() failed: %s (on line %d)\n",
					err,
					__LINE__
				);

				return 69;
			} else if (retval_fork_2 == 0) {
				int retval = child_2(
					pipe_fds[0],
					pipe_fds[1]
				);

				if (retval != 0)
					return retval;
			} else {
				int retval = child_1(
					pipe_fds[0],
					pipe_fds[1]
				);

				if (retval != 0)
					return retval;
			}

			break;

		default:
			if (wait(NULL) == -1) {
				perror("wait() failed");
				return 69;
			}

			printf("Parent (PID: %d)\n", getpid());

			break;
	}

	return 0;
}

static int child_1(int read_pipe, int write_pipe)
{
	const char *err;
	char msg[12];

	if (close(write_pipe) == -1) {
		err = strerror(errno);

		printf(
			"close() failed: %s (on line %d)\n",
			err,
			__LINE__
		);

		return 69;
	}

	/*
	 * We _connect_ the read end of the pipe's
	 * file _description_ with stdin's file
	 * _description_, which means we can use
	 * stdin's file _descriptor_ to read
	 * content from the pipe.
	 */
	if (dup2(read_pipe, STDIN_FILENO) == -1) {
		err = strerror(errno);

		printf(
			"dup2() failed: %s (on line %d)\n",
			err,
			__LINE__
		);

		return 69;
	}

	/*
	 * Because we already _connect_ the read end of
	 * the pipe's file _description_ with stdin, we
	 * can safely close the read end of the pipe's
	 * file _descriptor_.
	 */
	if (close(read_pipe) == -1) {
		err = strerror(errno);

		printf(
			"close() failed: %s (on line %d)\n",
			err,
			__LINE__
		);

		return 69;
	}

	/*
	 * fgets() stop the reading until
	 * End of File (EoF) or find a newline
	 * character (\n).
	 *
	 * Reference:
	 * https://stackoverflow.com/a/4023921
	 */
	char *retval_fgets = fgets(
		msg,
		ARRAY_LEN(msg),
		stdin
	);

	if (retval_fgets == NULL) {
		printf(
			"fgets() failed (on line %d)\n",
			__LINE__
		);
		return 69;
	}

	printf(
		"Message from second child: %s\n",
		msg
	);

	return 0;
}

static int child_2(int read_pipe, int write_pipe)
{
	const char *err;

	if (close(read_pipe) == -1) {
		err = strerror(errno);

		printf(
			"close() failed: %s (on line %d)\n",
			err,
			__LINE__
		);

		return 69;
	}

	/*
	 * We _connect_ the write end of the pipe's
	 * file _description_ with stdout's file
	 * _description_, which means we can use
	 * stdout's file _descriptor_ to write
	 * content into the pipe.
	 */
	if (dup2(write_pipe, STDOUT_FILENO) == -1) {
		err = strerror(errno);

		printf(
			"dup2() failed: %s (on line %d)\n",
			err,
			__LINE__
		);

		return 69;
	}

	/*
	 * Because we already _connect_ the write end of
	 * the pipe's file _description_ with stdout, we
	 * can safely close the write end of the pipe's
	 * file _descriptor_.
	 */
	if (close(write_pipe) == -1) {
		err = strerror(errno);

		printf(
			"close() failed: %s (on line %d)\n",
			err,
			__LINE__
		);

		return 69;
	}

	/*
	 * Write content to the stdout file _descriptor_.
	 */
	puts("hello world");

	return 0;
}
