/* This is zealcook lib(zclib) file. */

#include "sysdep.h"

/* open a file.
 * return fd(>0) as success.
 * return 0 for failure.
 */
int zclib_open(const char *name, int flag)
{
	int fd;

	if (!name)
		return 0;

	fd = open(name, flag, 0666);

	if (fd < 0) {
		fprintf(stderr, "zclib_open=%s fail\n", name);
		return 0;
	}
	return fd;
}

void zclib_close(int fd)
{
	if (fd > 0)
		close(fd);
}
