#include <stdio.h>
#include <unistd.h>

/*
 * Reference:
 * https://linuxvox.com/blog/are-child-processes-created-with-fork-automatically-killed-when-the-parent-is-killed/
 */
int main(void)
{
	pid_t pid = fork();

	if (pid < 0) {
		perror("fork() failed");
		return 1;
	} else if (pid == 0) {
		printf(
			"Child: PID = %d, PPID = %d\n",
			getpid(),
			getppid()
		);

		sleep(20);

		printf(
			"Child: PPID = %d\n",
			getppid()
		);

		printf("Child process exiting\n");
	} else {
		printf(
			"Parent: PID = %d, Child PID = %d\n",
			getpid(),
			pid
		);

		printf("Parent process exiting\n");
	}

	return 0;
}
