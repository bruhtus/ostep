#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	int *arr = malloc(sizeof(*arr) * 100);
	arr[0] = 0; // Prevent uninitialized read.

	/*
	 * Causing a compiler warning:
	 * "calling free() with nonzero offset".
	 *
	 * And also, causing an IOT instruction error
	 * when we run the program.
	 */
	free(arr + 5);
	printf("value: %d\n", arr[0]);

	return 0;
}
