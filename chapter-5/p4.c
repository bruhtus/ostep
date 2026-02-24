#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

/*
 * File descriptor will be kept open across the exec() system
 * call because file descriptor is attached to the process
 * instead of the program. So even though we replaced the
 * program instructions in the child process, we are still
 * using the same file descriptor.
 *
 * Possible scenario under the hood of shell redirection:
 * prompt> wc p4.c > file.txt (wait() the program to finish)
 * prompt> wc p4.c > file.txt (fork() to the child process, including stdin, stdout, stderr)
 * prompt> wc p4.c > file.txt (close() stdout file descriptor and open() to open or create file.txt)
 * wc p4.c (exec() in the child process with file descriptor from file.txt instead of stdout)
 * prompt> (return to parent process when finish)
 *
 * Why open() using stdout file descriptor by default? Is it
 * because stdin already _opened_ and the lowest-numbered
 * file descriptor that have not opened yet after stdin is
 * stdout?
 *
 * References:
 * - https://unix.stackexchange.com/a/757197
 * - https://www.gnu.org/software/bash/manual/html_node/Redirections.html
 * - https://pubs.opengroup.org/onlinepubs/007904975/functions/stdin.html
 */
int main(void)
{
	int rc = fork();

	if (rc < 0) {
		fprintf(stderr, "fork failed\n");
		return 1;
	} else if (rc == 0) {
		int return_value = 0;

		return_value = close(STDOUT_FILENO);

		if (return_value == -1)
			perror("close() error");

		printf("After stdout closed, we can't show this\n");

		return_value = open(
			"./p4.txt",
			O_CREAT | O_WRONLY | O_TRUNC,
			00644
		);

		/*
		 * Looks like we are using the same file
		 * descriptor number as stdout, which is 1 (?).
		 */
		if (return_value == -1)
			perror("open() error");
		else
			dprintf(
				return_value,
				"Add info in file descriptor %d\n",
				return_value
			);

		/*
		 * We use the array of characters instead
		 * of string literal directly to prevent
		 * accidental modification of the string
		 * literal in another function.
		 */
		char cmd[] = "wc";
		char cmd_args[] = __FILE__;

		/*
		 * We also need to provide the executable
		 * name as the first element of the array
		 * for execvp().
		 *
		 * If we didn't provide the executable
		 * name as the first element of the array,
		 * the execvp() will hang and we need to
		 * terminate the process using ctrl-c.
		 *
		 * Still not sure why we need to provide
		 * the executable name in the array of
		 * arguments when we already provide
		 * that as the first argument of execvp().
		 */
		char *const args[] = {
			cmd,
			cmd_args,
			NULL
		};

		return_value = execvp(args[0], args);

		if (return_value == -1)
			perror("exec() error");
	} else {
		int wstatus;

		int rc_wait = waitpid(rc, &wstatus, 0);

		printf(
			"Parent of %d (rc_wait: %d, PID: %d, wstatus: %d, child_exit_code: %d)\n",
			rc,
			rc_wait,
			getpid(),
			wstatus,
			WEXITSTATUS(wstatus)
		);
	}

	return 0;
}
