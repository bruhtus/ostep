#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

/*
 * This mechanism is kind of similar to how the shell (like
 * bash, zsh, and so on) execute a program.
 *
 * Here's the simplified illustration of how the shell
 * run a program:
 * prompt> wc p3.c (wait() the program to finish)
 * prompt> wc p3.c (fork() into child process)
 * wc p3.c (exec() in the child process)
 * prompt> (go back to parent process when finish)
 *
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
		 *
		 * execvp() second argument have type
		 * `char *const []`, so does that means
		 * execvp() will change the character that
		 * the pointer points to? Still not sure if
		 * using string literal is a good idea here.
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
