#ifndef TEA_H
#define TEA_H

#include <zc_sysdep.h>

struct tea_ctx {
	u32 KEY[4];
};

int tea_setkey(struct tea_ctx *ctx, const u8 *in_key, unsigned int key_len);
void tea_encrypt(struct tea_ctx *ctx, u8 *dst, const u8 *src);
void tea_decrypt(struct tea_ctx *ctx, u8 *dst, const u8 *src);

#endif /* TEA_H */
