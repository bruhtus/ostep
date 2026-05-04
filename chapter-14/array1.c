#include <stdlib.h>

int main(void)
{
	int *arr = malloc(sizeof(*arr) * 100);
	arr[100] = 0;

	free(arr);

	return 0;
}
