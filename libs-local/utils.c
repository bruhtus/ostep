#include <stdio.h>
#include <string.h>

void print_err(
	int err_num,
	int line_num,
	const char *func_name
)
{
	const char *err = strerror(err_num);

	printf(
		"%s failed: %s (line: %d)\n",
		func_name,
		err,
		line_num
	);
}
