#include <stdio.h>
#include <poll.h>
#include <unistd.h>

#define ARRAY_LEN(array) (sizeof(array) / sizeof(*array))

/*
 * Reference:
 * https://github.com/skuhl/sys-prog-examples/blob/master/simple-examples/poll.c
 */
int main(void)
{
	struct pollfd fds[] = {
		{
			.fd = STDIN_FILENO,
			.events = POLLIN
		}
	};

	/*
	 * Timeout in miliseconds.
	 *
	 * Return a non-negative value, which is a number
	 * of elements in the pollfd array whose `revents`
	 * fields have been set to a non-zero value
	 * (indicating an event or an error).
	 */
	int rc_poll = poll(fds, ARRAY_LEN(fds), 1000);

	if (rc_poll == -1) {
		perror("poll() error");
		return 69;
	} else if (rc_poll == 0) {
		printf("poll() timed out\n");
		return 0;
	} else {
		/*
		 * `revents` is an output parameter filled
		 * by the kernel with the events that
		 * actually occurred.
		 *
		 * To trigger this condition, we can add
		 * `close(STDIN_FILENO)` before calling
		 * poll().
		 */
		if (fds[0].revents & POLLNVAL) {
			printf("Bad file descriptor\n");
			return 69;
		}

		printf("data available now!\n");
	}

	/*
	 * Still not sure _why_ we need to do this but if
	 * we didn't do this, the character we type will
	 * be sent to the shell too.
	 *
	 * If we type more characters than the provided
	 * buffer, we will sent the remaining character to
	 * the shell too.
	 *
	 * We need to press `enter` to terminate the line
	 * for stdin with `\n` (new line character). That's
	 * why we give this 2 characters, instead of
	 * 1 character because the expected input characters
	 * are 2 (any character + new line character).
	 */
	if (1) {
		char buf[2];
		read(STDIN_FILENO, buf, ARRAY_LEN(buf));
	}

	return 0;
}
