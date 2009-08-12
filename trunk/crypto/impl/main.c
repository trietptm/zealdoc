#include "priv.h"
#include "ciphers/tea.h"

struct tea_ctx g_ctx;
u8 gkey[] = "zeal";
u8 g_tmpfile[] = "tmpfile";
int gsrc = 0;
int gdst = 0;

void usage(void)
{
	fprintf(stderr, "Usage: tea <OPTION> <FILE>\n");
	fprintf(stderr, "<OPTION> is -e or -d\n");
	fprintf(stderr, "<FILE>   is file you need do action\n");

	exit(0);
}

#define OPEN_FLAG_R	1
#define OPEN_FLAG_C	2
#define OPEN_FLAG_W	3

int tea_openfile(const char *name, int flag)
{
	int fd;
	int open_flag = 0;

	if (!name)
		return 0;

	if ((flag & OPEN_FLAG_R) && (flag & OPEN_FLAG_W))
		open_flag |= O_RDWR;
	else if (flag & OPEN_FLAG_R)
		open_flag |= O_RDONLY;
	else if (flag & OPEN_FLAG_W)
		open_flag |= O_WRONLY;

	if (flag & OPEN_FLAG_C)
		open_flag |= O_CREAT;

	fd = open(name, open_flag, 0666);

	if (fd < 0) {
		fprintf(stderr, "openfile=%s fail\n", name);
		return 0;
	}
	return fd;
}

void tea_closefile(int fd)
{
	close(fd);
}

int tea_init(int argc, char **argv)
{
	gsrc = tea_openfile(argv[2], OPEN_FLAG_R);
	if (gsrc == 0)
		return 0;

	gdst = tea_openfile(g_tmpfile, OPEN_FLAG_C | OPEN_FLAG_W);
	if (gdst == 0) {
		tea_closefile(gsrc);
		return 0;
	}

	tea_setkey(&g_ctx, gkey, 4);
	return 1;
}

void tea_exit(void)
{
	if (gsrc > 0)
		tea_closefile(gsrc);
	if (gdst > 0)
		tea_closefile(gdst);
}

#define TEA_OP_EN	1
#define TEA_OP_DE	2

int do_op(int type)
{
	int out = 1;
	unsigned char src[8];
	unsigned char dst[8];

	do {
		int ret;

		ret = read(gsrc, src, 8);

		printf("ret=%d\n", ret);
	
		if (ret < 0)
			break;

		if (ret < 8 || ret == 0)
			out = 0;
		if (ret == 0)
			break;

		if (ret < 8) {
			int i;
			for (i = ret; i < 8; i++)
				src[i] = '\n';
		}
		if (type == TEA_OP_EN) {
			tea_encrypt(&g_ctx, dst, src);
			if (-1 == write(gdst, dst, 8))
				break;
		} else {
			tea_decrypt(&g_ctx, dst, src);
			if (-1 == write(gdst, dst, 8))
				break;
		}
	} while (out);

	/* true if fail */
	return out;
}

int main(int argc, char **argv)
{
	if (argc < 3)
		usage();
	if (tea_init(argc, argv) == 0)
		return 0;

	fprintf(stdout, "start do op, %d/%d\n", gsrc, gdst);

	if (argv[1][0] == 'e') {
		if (do_op(TEA_OP_EN)) {
			/* fail */
		}
	} else {
		if (do_op(TEA_OP_DE)) {
			/* fail */
		}
	}
	tea_exit();
	return 1;
}
