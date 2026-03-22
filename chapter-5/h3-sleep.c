#include <stdio.h>
#include <unistd.h>

int main(void)
{
	pid_t rc_fork = fork();

	switch (rc_fork) {
		case -1:
			perror("fork() error");
			break;

		case 0:
			printf("hello\n");
			break;

		/*
		 * Using sleep() to wait for child process
		 * to finish might cause some race condition
		 * because most likely we can not predict how
		 * much time the child process need to finish.
		 */
		default:
			sleep(1);
			printf("good bye\n");
			break;
	}

	return 0;
}
