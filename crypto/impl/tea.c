#include "tea.h"

#define TEA_KEY_SIZE		16
#define TEA_BLOCK_SIZE		8
#define TEA_ROUNDS		32
#define TEA_DELTA		0x9e3779b9

#define XTEA_KEY_SIZE		16
#define XTEA_BLOCK_SIZE		8
#define XTEA_ROUNDS		32
#define XTEA_DELTA		0x9e3779b9

int tea_setkey(struct tea_ctx *ctx, const u8 *in_key,
	       unsigned int key_len)
{
	const u8 *key = (const u8 *)in_key;

	ctx->KEY[0] = le32_to_cpu(key[0]);
	ctx->KEY[1] = le32_to_cpu(key[1]);
	ctx->KEY[2] = le32_to_cpu(key[2]);
	ctx->KEY[3] = le32_to_cpu(key[3]);

	return 0; 
}

void tea_encrypt(struct tea_ctx *ctx, u8 *dst, const u8 *src)
{
	u32 y, z, n, sum = 0;
	u32 k0, k1, k2, k3;
	const u32 *in = (const u32 *)src;
	u32 *out = (u32 *)dst;

	y = le32_to_cpu(in[0]);
	z = le32_to_cpu(in[1]);

	k0 = ctx->KEY[0];
	k1 = ctx->KEY[1];
	k2 = ctx->KEY[2];
	k3 = ctx->KEY[3];

	fprintf(stderr, "in[0]=%08x\n", in[0]);
	fprintf(stderr, "in[1]=%08x\n", in[1]);

	n = TEA_ROUNDS;

	while (n-- > 0) {
		sum += TEA_DELTA;
		y += ((z << 4) + k0) ^ (z + sum) ^ ((z >> 5) + k1);
		z += ((y << 4) + k2) ^ (y + sum) ^ ((y >> 5) + k3);
	}
	
	out[0] = cpu_to_le32(y);
	out[1] = cpu_to_le32(z);

	fprintf(stderr, "out[0]=%08x\n", out[0]);
	fprintf(stderr, "out[1]=%08x\n", out[1]);
}

void tea_decrypt(struct tea_ctx *ctx, u8 *dst, const u8 *src)
{
	u32 y, z, n, sum;
	u32 k0, k1, k2, k3;
	const u32 *in = (const u32 *)src;
	u32 *out = (u32 *)dst;

	fprintf(stderr, "in[0]=%08x\n", in[0]);
	fprintf(stderr, "in[1]=%08x\n", in[1]);

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
	fprintf(stderr, "out[0]=%08x\n", out[0]);
	fprintf(stderr, "out[1]=%08x\n", out[1]);
}
