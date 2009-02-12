#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>

/* only change alpha */

int rot13_main(int c)
{
	return (isalpha(c) ? tolower(c) < 'n' ? c + 13 : c-13 : c);
}

void rot13_write(int fd, int new_fd)
{
	char ch1, ch2;

	while (1 == read(fd, &ch1, 1)) {
		ch2 = rot13_main(ch1);
		write(new_fd, &ch2, 1);
	}
	return;
}


void main(void)
{
	int fd1 = open("g:\\rot13.txt", O_RDWR);
	int fd2 = open("g:\\rot13.txt.new", O_RDWR | O_CREAT);

	if (fd1 < 0 || fd2 < 0) {
		printf("open err\n");
		return;
	}
	rot13_write(fd1, fd2);
	close(fd1);
	close(fd2);
}

