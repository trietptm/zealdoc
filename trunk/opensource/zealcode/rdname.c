#include <string.h>
#include <stdio.h>
#include <errno.h>

void readname(const char *argv)
{
	char *path;	
	char cmd[123], buf[123];

	if (argv == NULL)
		path = "/proc/self/exe";
	else
		path = (char *)argv;

	memset(buf, 0, 123);

	if (-1 == readlink(path, buf, 123) && errno == EINVAL)
		sprintf(cmd, "basename %s", path);
	else
		sprintf(cmd, "basename %s", buf);
	system(cmd);
	fprintf(stdout, "\n");
}

int main(int argc, char **argv)
{
	readname(argv[0]);
	return 0;
}
