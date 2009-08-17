#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

19, 8, 4, 6, 23, 20, 7
19, 25, 23, 20, 7, 8, 6
1, 8, 4, 6, 2, 3, 9,

int openfile(const char *name)
{
	int fd;
	int open_flag = 0;

	if (!name)
		return 0;

	open_flag |= O_RDONLY;
	open_flag |= O_BINARY;

	fd = open(name, open_flag, 0666);

	if (fd < 0) {
		fprintf(stderr, "openfile=%s fail\n", name);
		return 0;
	}
	return fd;
}

void closefile(int fd)
{
	close(fd);
}
int main(void)
{
	int i, fd;
	unsigned char buf[10];

	fd = openfile("G:\\mycodes\\test\\test.mp3");
	if (fd <= 0)
		return 0;

	read(fd, buf, 10);
	for (i = 0; i < 10; i++)
		fprintf(stderr, "%02x \n", buf[i]);

	closefile(fd);
	return 1;
}
