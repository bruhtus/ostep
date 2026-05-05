#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	int i;

	/*
	 * If the memory block address (the first argument)
	 * is NULL, then realloc() is equivalent to malloc().
	 */
	int *arr = realloc(NULL, sizeof(*arr) * 2);

	if (arr == NULL) {
		perror("malloc() failed");
		return 69;
	}

	arr[0] = 69;
	arr[1] = 42;

	/*
	 * We need to provide the new _total_ size for
	 * the resized array.
	 */
	int *new_arr = realloc(arr, sizeof(*new_arr) * 4);

	if (new_arr == NULL) {
		perror("malloc() failed");
		return 69;
	}

	arr = new_arr;
	new_arr = NULL; // Prevent dangling pointer.

	arr[2] = 69420;
	arr[3] = 6942;

	for (i = 0; i < 4; ++i)
		printf("arr[%d]: %d\n", i, arr[i]);

	free(arr);

	return 0;
}
