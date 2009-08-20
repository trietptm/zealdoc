#include <zclib.h>
#include <zc_sysdep.h>
#include "ciphers/tea.h"

struct tea_ctx g_ctx;
u8 gkey[] = "zeal";
u8 g_tmpfile[] = "tmpfile";
int gsrc = 0;
int gdst = 0;

void usage(void)
{
	fprintf(stderr, "Usage: tea <OPTION> <FILE>\n");
	fprintf(stderr, "<OPTION> is en or de\n");
	fprintf(stderr, "<FILE>   is file you need do action\n");

	exit(0);
}

int tea_init(int argc, char **argv)
{
#if 0
	gsrc = zc_open(argv[2], O_RDONLY | O_BINARY);
	if (gsrc == 0)
		return 0;

	gdst = zc_open(g_tmpfile, O_CREATE | O_RDWR);
	if (gdst == 0) {
		zc_close(gsrc);
		return 0;
	}

	tea_setkey(&g_ctx, gkey, 4);
#endif
	return 1;
}

void tea_exit(void)
{
	if (gsrc > 0)
		zc_close(gsrc);
	if (gdst > 0)
		zc_close(gdst);
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
#if 1
		if (type == TEA_OP_EN) {
			tea_encrypt(&g_ctx, dst, src);
			if (-1 == write(gdst, dst, 8))
				break;
		} else {
			tea_decrypt(&g_ctx, dst, src);
			if (-1 == write(gdst, dst, 8))
				break;
		}
#endif
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
