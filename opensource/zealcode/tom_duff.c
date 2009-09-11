#include <stdio.h>
#include <stdlib.h>

/* http://www.lysator.liu.se/c/duffs-device.html */

#define COPY_COUNT	5

void foo(void)
{
	int a = 10;
	int round = ( a + (COPY_COUNT - 1) ) / COPY_COUNT;

	switch ( a % COPY_COUNT ) {
	case 0: do {
		putchar ( '*' );
	case 4: putchar ( '*' );
	case 3: putchar ( '*' );
	case 2: putchar ( '*' );
	case 1: putchar ( '*' );
		} while ( --round );
	} 

	printf ( "\n" );
}

int main(void)
{
	foo();

	return 0;
}
