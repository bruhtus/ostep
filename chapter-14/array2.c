#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	int *arr = malloc(sizeof(*arr) * 100);
	arr[0] = 0; // Prevent uninitialized read.

	free(arr);

	/*
	 * This is usually called "use after free",
	 * and this is one of those _undefined behavior_.
	 * Anything can happen when we do "use after free".
	 */
	printf("value: %d\n", arr[0]);

	return 0;
}
