#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	int *arr = malloc(sizeof(*arr) * 100);

	if (arr == NULL) {
		perror("malloc() failed");
		return 69;
	}

	arr[100] = 0;

	free(arr);

	return 0;
}
