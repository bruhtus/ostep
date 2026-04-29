#include <stdio.h>
#include <stdlib.h>

void func(void);
void func2(void);

/*
 * Run the compiled executable with this:
 * setarch "$(uname -m)" -R bash -c './chapter-13/va'
 *
 * `setarch -R` disable the randomization of virtual
 * address space.
 */
int main(void)
{
	int x = 0;
	int *m = malloc(sizeof(*m));

	printf("location of main : %p\n", main);
	printf("location of func : %p\n", func);
	printf("location of func2: %p\n", func2);
	printf("location of heap : %p\n", (void *)m);
	printf("location of stack: %p\n", (void *)&x);

	free(m);

	return 0;
}

void func(void)
{
	puts("something");
}

void func2(void)
{
	puts("something");
}
