#include <stdio.h>
#include <string.h>

struct crypto_encrypt {
	unsigned char *input;
	unsigned char *output;
	size_t inlen;
	size_t *outlen;
};

#define crypto_create(x)	1
#define crypto_free(x)
#define CIPHER
#define ASE
#define MAX_STR	1
#define IOCTL_CRYPTO_ENCRYT 1
#define hex_dump(x, y)

void test_encrypt(void)
{
	int num = crypto_create(CIPHER | ASE);
	int ret, fd;
	char dev[MAX_STR];
	char test_in[MAX_STR], test_out[MAX_STR];
	struct crypto_encrypt ens;

	if (fd < 0)
		return;

	sprintf(dev, "/dev/crypto%d", fd);

	fd = open(dev);

	if (fd < 0)
		goto fail;

	ens.input = test_in;
	ens.output = test_out;
	ens.inlen = MAX_STR;

	ret = ioctl(fd, IOCTL_CRYPTO_ENCRYT, (unsigned long)&ens);
	if (ret >= 0) {
		printf("en success\n");
		hex_dump(ens.input, ens.inlen);
		hex_dump(ens.output, *ens.outlen);
	} else {
		printf("en failure\n");
	}

	memset(test_in, 0, MAX_STR);
	memset(test_out, 0, MAX_STR);
fail:
	if (num >= 0)
		crypto_free(num);
	if (fd > 0)
		close(fd);
	return;
}

int main(void)
{
	return 0;
}
