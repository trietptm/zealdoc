#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>

void usage(void);

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
	int fd1;
	int fd2;
	
	fd1 = open("g:\\rot13.txt", O_RDONLY);

	if (fd1 < 0) {
		usage();
		return;
	}
	fd2 = open("g:\\rot13.txt.new", O_RDWR | O_CREAT);

	if (fd2 < 0) {
		usage();
		return;
	}
	rot13_write(fd1, fd2);
	close(fd1);
	close(fd2);
	
	printf("finish...\r\n");
	getch();
}

void usage(void)
{
	printf("usage: \r\n");
	printf("1) put original file into g:\\rot13.txt\r\n");
	printf("2) run this program\r\n");
	printf("3) will generate rot13.txt.new\r\n");
	exit(1);
}