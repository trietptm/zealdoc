#include <sys/socket.h>
#include <sys/types.h>
#include <linux/in.h>

int sk = -1;

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
	//srv_addr.sin_port = INADDR_ANY;
	srv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	ret = bind(sk, (struct sockaddr *)&srv_addr, sizeof (srv_addr));

	if (ret == -1) {
		printf("bind err");
		close(sk);
		return -1;
	}
	printf("bind succ\n");
	listen(sk, 5);

	for (;;) {
		connfd = accept(sk,
			(struct sockaddr *)&cli_addr, sizeof (cli_addr));
		printf("connect from=%s, port=%d\n", 
			inet_ntop(AF_INET,
			&cli_addr.sin_addr, 
			buff, sizeof (buff)), ntohs(cli_addr.sin_port));
		write(connfd, "haha", strlen("haha"));
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
