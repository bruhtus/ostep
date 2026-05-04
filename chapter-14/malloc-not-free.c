#include <stdlib.h>

/*
 * Try using valgrind with this command:
 * valgrind --leak-check=yes ./chapter-14/malloc-not-free
 */
int main(void)
{
	int *p1 = malloc(sizeof(*p1) * 1);
	int *p2 = malloc(sizeof(*p2) * 1);

	return 0;
}
