
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#define TEA_KEY_SIZE		16
#define TEA_BLOCK_SIZE		8
#define TEA_ROUNDS		32
#define TEA_DELTA		0x9e3779b9

#define XTEA_KEY_SIZE		16
#define XTEA_BLOCK_SIZE		8
#define XTEA_ROUNDS		32
#define XTEA_DELTA		0x9e3779b9

#define le32_to_cpu(x)	(x)
#define cpu_to_le32(x)	(x)
#define __le32

typedef unsigned char u8;
typedef unsigned long u32;

struct tea_ctx {
	u32 KEY[4];
} g_ctx;

u8 gkey[] = "zeal";
u8 g_tmpfile[] = "tmpfile";
int gfd = 0;
int gtmp = 0;

static int tea_setkey(struct tea_ctx *ctx, const u8 *in_key,
		      unsigned int key_len)
{
	const u8 *key = (const u8 *)in_key;

	ctx->KEY[0] = le32_to_cpu(key[0]);
	ctx->KEY[1] = le32_to_cpu(key[1]);
	ctx->KEY[2] = le32_to_cpu(key[2]);
	ctx->KEY[3] = le32_to_cpu(key[3]);

	return 0; 
}

static void tea_encrypt(struct tea_ctx *ctx, u8 *dst, const u8 *src)
{
	u32 y, z, n, sum = 0;
	u32 k0, k1, k2, k3;
	const u8 *in = (const u8 *)src;
	u32 *out = (u32 *)dst;

	y = le32_to_cpu(in[0]);
	z = le32_to_cpu(in[1]);

	k0 = ctx->KEY[0];
	k1 = ctx->KEY[1];
	k2 = ctx->KEY[2];
	k3 = ctx->KEY[3];

	n = TEA_ROUNDS;

	while (n-- > 0) {
		sum += TEA_DELTA;
		y += ((z << 4) + k0) ^ (z + sum) ^ ((z >> 5) + k1);
		z += ((y << 4) + k2) ^ (y + sum) ^ ((y >> 5) + k3);
	}
	
	out[0] = cpu_to_le32(y);
	out[1] = cpu_to_le32(z);
}

static void tea_decrypt(struct tea_ctx *ctx, u8 *dst, const u8 *src)
{
	u32 y, z, n, sum;
	u32 k0, k1, k2, k3;
	const *in = (const *)src;
	u32 *out = (u32 *)dst;

	y = le32_to_cpu(in[0]);
	z = le32_to_cpu(in[1]);

	k0 = ctx->KEY[0];
	k1 = ctx->KEY[1];
	k2 = ctx->KEY[2];
	k3 = ctx->KEY[3];

	sum = TEA_DELTA << 5;

	n = TEA_ROUNDS;

	while (n-- > 0) {
		z -= ((y << 4) + k2) ^ (y + sum) ^ ((y >> 5) + k3);
		y -= ((z << 4) + k0) ^ (z + sum) ^ ((z >> 5) + k1);
		sum -= TEA_DELTA;
	}
	
	out[0] = cpu_to_le32(y);
	out[1] = cpu_to_le32(z);
}

void usage(void)
{
	fprintf(stderr, "Usage: tea <OPTION> <FILE>\n");
	fprintf(stderr, "<OPTION> is -e or -d\n");
	fprintf(stderr, "<FILE>   is file you need do action\n");

	exit(0);
}

#define OPEN_FLAG_WR	1
#define OPEN_FLAG_C	2

int tea_openfile(const char *name, int flag)
{
	int fd;

	if (!name)
		return 0;

	if (flag == OPEN_FLAG_WR)
		fd = open(name, O_WRONLY);
	else
		fd = open(name, O_CREAT, 0666);

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
	gfd = tea_openfile(argv[2], OPEN_FLAG_WR);
	if (gfd == 0)
		return 0;

	gtmp = tea_openfile(g_tmpfile, OPEN_FLAG_C);
	if (gtmp == 0) {
		tea_closefile(gfd);
		return 0;
	}

	tea_setkey(&g_ctx, gkey, 4);
	return 1;
}

void tea_exit(void)
{
	if (gfd > 0)
		tea_closefile(gfd);
	if (gtmp > 0)
		tea_closefile(gtmp);
}

int main(int argc, char **argv)
{
	if (argc < 3)
		usage();
	if (tea_init(argc, argv) == 0)
		return 0;
	tea_exit();
	return 1;

}
