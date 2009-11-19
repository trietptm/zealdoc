#include <stdio.h>
#include <fcntl.h>

static long arr[] = {
	1,
	10,
	100,
	1000,
	10000,
	100000,
	1000000,
	10000000,
	100000000,
	1000000000,
};

#if 0
int bit_cnt(long n)
{
	int i, base = 10;

	for (i = 1; i < 10; i++) {
		if (n < (base))
			return i;
		base *= 10;
	}
	return 0;
}
#endif

long do_pow(long n, int pow)
{
	int residue = 1;

	if (n <= 1 || pow == 1)
		return n;
	while (pow > 1) {
		if (pow & 0x01)
			residue *= n;
		n *= n;
		pow >>= 1;
	}
	return n * residue;
}

int is_daffodil(long n, int bit)
{
	int i;
	long last, tmp, cnt = 0;

	if (bit == 0)
		return 0;

	last = 0;
	for (i = bit - 1; i != -1; i--) {
		tmp = n / arr[i];
		tmp -= last * 10;
		last = n / arr[i];
		tmp = do_pow(tmp, bit);
		cnt += tmp;
	}
	return (n == cnt);
}

void main(void)
{
	long bit = 9;
	long first, end;

	first = arr[bit-1];
	end = arr[bit] - 1;

	for (; first <= end; first++) {
		if (is_daffodil(first, bit))
			printf("%ld\n\n", first);
	}
}
frankwoodscn@g.com
