#include <sys/socket.h>
#include <sys/types.h>
#include <linux/in.h>

int sock = -1;

/* create socket */
int sock_open(void)
{
	struct sockaddr_in srv_addr;
	int ret;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		printf("socket err\n");
		return -1;
	}

	memset(&srv_addr, 0, sizeof (srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(1234);
	srv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	ret = connect(sock, (struct sockaddr *)&srv_addr, sizeof (srv_addr));
	if (ret != 0) {
		printf("conn fail\n");
		close(sock);
		return -1;
	} else
		printf("conn succ\n");

	printf("send:\n");
	ret = send(sock, "client", strlen("client"), 0);
	if (ret > 0) {
		printf("send succ\n");
	} else
		printf("send %d\n", ret);
	return sock;
}

void sock_close(void)
{
	if (sock > 0)
		close(sock);
	sock = -1;
}

int main(void)
{
	sock_open();
	sock_close();
	return 0;
}
