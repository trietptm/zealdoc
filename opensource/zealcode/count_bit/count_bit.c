#include <stdio.h>
#define DEBUG
/*
 * man lrint for help
 * 		2009.01.04
 * */
/*
 * This lib collect the method that can return how many 1 bit in N.
 *
 * But may refer to kernel bitmap or else
 */

/*
 * Count every bit.
 * return count
 */
int bit_count(unsigned long N)
{
	int count;
	
	for (count = 0; N; N >>= 1) {
		count += N & 1;
	}
	return count;
}

/*
 * N & N-1 => 100100 & 100011 => 100000
 * Do one time, and remove the last one.
 * How many times we need and how many one bit we get. 
 */
int bit_count2(unsigned int N)
{
	int count;
	
	for (count = 0; N; N &= N - 1) {
		count++;
	}
	return count;
}

/*
 *	MASK(0) = 55555555 h = 01010101010101010101010101010101 b
 *	MASK(1) = 33333333 h = 00110011001100110011001100110011 b
 *	MASK(2) = 0f0f0f0f h = 00001111000011110000111100001111 b
 *	MASK(3) = 00ff00ff h = 00000000111111110000000011111111 b
 *	MASK(4) = 0000ffff h = 00000000000000001111111111111111 b
 *	...
 *	... not only 32 bits
 *	Example:    N = (10)d => (N & 10 >> 1) + (N & 01) => 01 + 01 = (10)b = (2)d
 *	Count the 1's number in High pow and Low pow, and then add them.
 */
#define POW(c) (1<<(c))	
#define MASK(c) (((unsigned int)-1) / (POW(POW(c)) + 1))
#define ROUND(n, c) (((n) & MASK(c)) + ((n) >> POW(c) & MASK(c)))

int bit_count3(unsigned int n)	/* the type must match the MACRO */	
{
	n = ROUND(n, 0);
	n = ROUND(n, 1);
	n = ROUND(n, 2);
	n = ROUND(n, 3);
	n = ROUND(n, 4); /* for 32 bits number */
	return n;
}

void main()
{
//	int n = bit_count3((unsigned int)114);
//	printf("is %d\n", bit_count((unsigned long)1544445555554444));
//	printf("is %d\n", bit_count2((unsigned long)55));
	
	printf("is %d\n", ((2 & 0xF0) >> 4) + (2 & 0x0F));

}
