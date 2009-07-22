#include <stdio.h>
#include <string.h>

/* 
 *	byte order
 *	----------
 *	big
 * -----------------------
 * | n | n+1 | n+2 | n+3 |
 * -----------------------
 * MSB                 LSB
 *
 *	little
 * -----------------------
 * | n+3 | n+2 | n+1 | n |
 * -----------------------
 * MSB                 LSB
 *
 *	v
 *	v
 * long a = 0x04030201;
 *
 *	big
 * ---------------------
 * | 04 | 03 | 02 | 01 |
 * ---------------------
 * MSB		     LSB
 *
 *	little
 * ---------------------
 * | 04 | 03 | 02 | 01 |
 * ---------------------
 * MSB	             LSB
 *
 *	v
 *	v
 * long *cp = (long *)a;
 *	big:
 *	  cp[0] = 04
 *	little:
 *	  cp[0] = 01
 *
 *
 *
 *	bit order
 *	---------
 *
 *	1010 1010
 *
 * http://www.linuxjournal.com/article/6788
 * http://en.wikipedia.org/wiki/Least_significant_bit
 */

typedef unsigned long uint32_t;
typedef unsigned char uint8_t;

#define BE	0x01
#define LE	0x02
uint8_t endianness = LE;

struct bit_field {
	char a0:1;
	char a1:1;
	char a2:1;
	char a3:1;
	char a4:1;
	char a5:1;
	char a6:1;
	char a7:1;
};

union endian_tester {
	uint32_t my_int;
	uint8_t  my_bytes[4];
};

void test_byte_field(void)
{
	
	struct t{
		uint8_t a;
		uint8_t b;
	} bf;
	struct t *tmp;
	bf.a = 0x01;
	bf.b = 0x00;
	tmp = &bf;
	printf("tmp=%d\n", *(short *)tmp);
}
void test_bit_field(void)
{
	struct bit_field bf;
	
	memset(&bf, 0, sizeof (struct bit_field));
	/* ANSI C: the later member at high address */
	bf.a7 = 1;
	/* In LE, bf=128
	 * In BE, bf=1
	 */
	if (endianness == LE) {
		printf("Little endian system will show 128, check it!\n");
	} else {
		printf("Big endian system will show 1, check it!\n");
	
	}
	printf("[%d] is it your expect?\n", bf);
}


void test_endianness(void)
{
	union endian_tester et;

	et.my_int = 0x0a0b0c0d;
	if(et.my_bytes[0] == 0x0a) {
		endianness = BE;
		printf( "I'm on a big-endian system\n" );
	} else {
		endianness = LE;
		printf( "I'm on a little-endian system\n" );

	}
}

int main(void)
{
	test_endianness();
	test_bit_field();
	test_byte_field();

	return 0;
}