#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

/*
 * Reference:
 * https://pages.cs.wisc.edu/~remzi/OSTEP/cpu-api.pdf
 */
int main(void)
{
	printf("Start (PID: %d)\n", getpid());

	int rc = fork();

	if (rc < 0) {
		fprintf(stderr, "fork failed\n");
		return 1;
	} else if (rc == 0) {
		printf("Child (PID: %d)\n", getpid());

		const char *const my_args[3] = {
			"wc",
			__FILE__,
			NULL
		};

		/*
		 * When we use exec(), we are replacing
		 * the program in the _current_ process
		 * with another program. In this case,
		 * we are replacing the child process
		 * from fork() with `wc` program.
		 *
		 * So we are __not creating a new process__,
		 * but we are __reusing__ the current
		 * process with different instructions.
		 *
		 * That's why the next instruction
		 * (or statement) after exec() is not
		 * executed.
		 */
		int err = execvp(my_args[0], (char **)my_args);

		/*
		 * exec() will only return a value when
		 * an error occured, such as when the
		 * executable does not exist.
		 */
		if (err == -1) {
			perror("exec() error");
		}

		printf("This should not be executed\n");
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
