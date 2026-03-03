#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(void)
{
	int fd = open(
		"./h2.txt",
		O_CREAT | O_WRONLY | O_TRUNC,
		00644
	);

	if (fd == -1) {
		perror("open() error");
		return 1;
	}

	/*
	 * Looks like there's a difference between
	 * opening the file before fork() and after
	 * fork().
	 *
	 * When we open the file before fork(), it
	 * looks like the behavior is like _combining_
	 * the parent and child output and write that
	 * in the file. But if we open the file after
	 * fork(), it looks like the behavior is write
	 * the last output to the file, only parent
	 * or child output (?).
	 */
	int rc = fork();

	if (rc < 0) {
		fprintf(stderr, "fork failed\n");
		return 1;
	} else if (rc == 0) {
		printf("Child PID: %d\n", getpid());

		dprintf(
			fd,
			"Child PID: %d\n",
			getpid()
		);
	} else {
		/*
		 * How to make the child and parent process
		 * write to the same file at the same time?
		 *
		 * At this moment, still have no idea.
		 */
		int rc_wait = wait(NULL);

		if (rc_wait < 1) {
			fprintf(stderr, "wait() failed\n");
			return 1;
		}

		/*
		 * There's an interesting scenario where
		 * the content in the file have empty
		 * line after the child process content.
		 *
		 * Here's how to reproduce that scenario:
		 * - Open() file after fork().
		 * - Not using wait().
		 * - Add new line character in printf()
		 *   below.
		 * - The parent process must be executed
		 *   before the child process.
		 *
		 * Still have no idea why that happen.
		 */
		printf("Parent PID: %d\n", getpid());

		dprintf(
			fd,
			"Parent PID: %d\n",
			getpid()
		);
	}

	/*
	 * Looks like when we close the file descriptor
	 * in child process, it did not effect the
	 * file descriptor opened in parent process (?).
	 */
	int rc_close = close(fd);

	if (rc_close == -1)
		perror("close() error");

	return 0;
}
