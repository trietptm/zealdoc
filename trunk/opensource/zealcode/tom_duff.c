#include <stdio.h>

/* http://www.lysator.liu.se/c/duffs-device.html */

#define COPY_COUNT	5

void foo(const char *from, char *to, int copy_bytes)
{
	int round = ( copy_bytes + (COPY_COUNT - 1) ) / COPY_COUNT;

	switch ( copy_bytes % COPY_COUNT ) {
	case 0: do {
		*to++ = *from++;
	case 4: *to++ = *from++;
	case 3: *to++ = *from++;
	case 2: *to++ = *from++;
	case 1: *to++ = *from++;
		} while ( --round );
	}

	printf ( "\n" );
}

void foo2(const char *from, char *to, int copy_bytes)
{
	int i;
	for (i = 0; i < copy_bytes; i++)
		*to++ = *from++;
}

void foo3(const char *from, char *to, int copy_bytes)
{
	do {
		*to++ = *from++;
	} while (--copy_bytes);
}

int main(void)
{
	char ba[12], bb[12];
	foo(ba, bb, 12);

	return 0;
}
