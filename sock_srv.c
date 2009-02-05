#include <sys/socket.h>
#include <sys/types.h>
#include <linux/in.h>

int sk = -1;

void print_sname(int sk)
{
	struct sockaddr_in tmp_addr;

	memset(&tmp_addr, 0, sizeof (tmp_addr));
	getsockname(sk, (struct sockaddr *)&tmp_addr, sizeof (tmp_addr));
	
	printf("tmp_addr=%s, port=%d\n", inet_ntoa(tmp_addr), ntohs(tmp_addr.sin_port));
}

/* create socket */
int sock_open(void)
{
	struct sockaddr_in srv_addr;
	struct sockaddr_in cli_addr;
	char buff[100];
	int connfd, loop = 3;
	int ret;

	sk = socket(AF_INET, SOCK_STREAM, 0);
	if (sk < 0) {
		printf("socket err\n");
		return -1;
	}

	memset(&srv_addr, 0, sizeof (srv_addr));
	memset(&cli_addr, 0, sizeof (cli_addr));

	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(1234);
	srv_addr.sin_addr.s_addr = inet_addr("192.168.100.111");
//	srv_addr.sin_addr.s_addr = INADDR_ANY;

#if 1
	ret = bind(sk, (struct sockaddr *)&srv_addr, sizeof (srv_addr));
	if (ret == -1) {
		printf("bind err");
		close(sk);
		return -1;
	}
#endif

	print_sname(sk);

	listen(sk, 5);

	for (;;) {
		connfd = accept(sk,
			(struct sockaddr *)&cli_addr, sizeof (cli_addr));
		printf("connect from=%s, port=%d\n", inet_ntoa(cli_addr), ntohs(cli_addr.sin_port));
		write(connfd, "haha", strlen("haha"));
		print_sname(sk);
		sleep(2);
		if (loop-- == 0)
		break;
	}

	return sk;
}

void sock_close(void)
{
	if (sk > 0)
		close(sk);
}

int main(void)
{
	sock_open();
	sock_close();
	return 0;
}
