#include <stdio.h>
#include <unistd.h>

/*
 * Reference:
 * https://github.com/skuhl/sys-prog-examples/blob/master/simple-examples/pipe.c
 */
int main(void)
{
	int fds[2];

	/*
	 * We populate the file descriptors
	 * for read and write with pipe().
	 */
	if (pipe(fds) == -1) {
		perror("pipe() error");
		return 69;
	}

	/*
	 * Index 0 refers to the read end of the pipe.
	 * Index 1 refers to the write end of the pipe.
	 */
	printf("fds[0]: %d\n", fds[0]);
	printf("fds[1]: %d\n", fds[1]);

	char msg[] = "hello";

	/*
	 * Data written to the write end of the pipe is
	 * buffered by the kernel until it's read from
	 * the read end of the pipe.
	 */
	if (write(fds[1], msg, sizeof(msg)) == -1) {
		perror("write() error");
		return 69;
	}

	/*
	 * We need to close the write end of the pipe
	 * so that we don't _hang up_ when reading the
	 * read end of the pipe.
	 */
	if (close(fds[1]) == -1) {
		perror("close() error");
		return 69;
	}

	for (;;) {
		char buf;

		switch (read(fds[0], &buf, 1)) {
			case -1:
				perror("read() error");
				return 69;

			case 0:
				printf("Read end of the file\n");

				if (close(fds[0]) == -1) {
					perror("close() error");
					return 69;
				}

				return 0;

			default:
				if (buf == '\0')
					printf("Read null terminator\n");
				else
					printf("Read character %c\n", buf);

				break;
		}
	}

	return 0;
}
