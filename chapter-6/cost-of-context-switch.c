#define _GNU_SOURCE

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "measurement.h"

#define ARRAY_LEN(array) (sizeof(array) / sizeof(*array))

static int pipe_fds[2];

static void cleanup_child(void);
static void cleanup_parent(void);

int main(void)
{
	struct timespec start_time, end_time, result_time;
	cpu_set_t mask;
	int retval_gettime;

	CPU_ZERO(&mask);
	CPU_SET(1, &mask);

	if (sched_setaffinity(0, sizeof(mask), &mask) == -1) {
		perror("sched_setaffinity() failed");
		return 69;
	}

	if (pipe(pipe_fds) == -1) {
		perror("pipe() failed");
		return 69;
	}

	char msg[] = "hello world";

	pid_t retval_fork = fork();

	switch (retval_fork) {
		case -1:
			perror("fork() failed");
			return 69;

		case 0:
			atexit(cleanup_child);

			ssize_t retval_write = write(
				pipe_fds[1],
				msg,
				ARRAY_LEN(msg)
			);

			if (retval_write == -1) {
				perror("write() failed");
				return 69;
			}

			break;

		default:
			atexit(cleanup_parent);

			/*
			 * Not sure if using fork() means
			 * that we are in the "same CPU"
			 * or not, but we can make sure by
			 * using sched_setaffinity()
			 * (specific to glibc).
			 *
			 * Also still not sure what
			 * "same CPU" in the book is about,
			 * is it same thread, or CPU core,
			 * or whole CPU hardware?
			 */
			retval_gettime = clock_gettime(
				CLOCK_REALTIME,
				&start_time
			);

			if (retval_gettime == -1) {
				perror("clock_gettime() failed");
				return 69;
			}

			if (wait(NULL) == -1) {
				perror("wait() failed");
				return 69;
			}

			char result[ARRAY_LEN(msg)];

			ssize_t retval_read = read(
				pipe_fds[0],
				result,
				ARRAY_LEN(result)
			);

			if (retval_read == -1) {
				perror("read() failed");
				return 69;
			} else if (retval_read == 0) {
				puts("End of File");
				return 0;
			}

			retval_gettime = clock_gettime(
				CLOCK_REALTIME,
				&end_time
			);

			if (retval_gettime == -1) {
				perror("clock_gettime() failed");
				return 69;
			}

			sub_timespec(
				start_time,
				end_time,
				&result_time
			);

			printf("msg: %s\n", result);

			printf(
				"time: %ld.%.9ld\n",
				result_time.tv_sec,
				result_time.tv_nsec
			);

			break;
	}

	return 0;
}

static void cleanup_child(void)
{
	if (close(pipe_fds[0]) == -1)
		perror("Child: close() failed (read)");

	if (close(pipe_fds[1]) == -1)
		perror("Child: close() failed (write)");
}

static void cleanup_parent(void)
{
	if (close(pipe_fds[0]) == -1)
		perror("Parent: close() failed (read)");

	if (close(pipe_fds[1]) == -1)
		perror("Parent: close() failed (write)");
}
