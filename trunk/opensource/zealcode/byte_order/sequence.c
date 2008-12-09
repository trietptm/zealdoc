/************************************************************
* Big-endian:  The high addr is corresponding LSB.
*	+---------------------+
*  Low + n | n+1 | n+2 | n+3 + High
*      +---------------------+
*      MSB		    LSB
*	
*	i = 0x01020304
*	+---------------------+
*  Low + 01 | 02 |  03 |  04 + High
*      +---------------------+
*      MSB		    LSB
* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
* 
* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
* Little-endian: The low addr is corresponding LSB.
*	+---------------------+
* High + n+3 | n+2 | n+1 | n + Low
*      +---------------------+
*      MSB		    LSB
*
*	i = 0x01020304
*	+---------------------+
* High + 01 | 02 |  03 |  04 + Low
*      +---------------------+
*      MSB		    LSB
*************************************************************/
#include <stdio.h>
#include "sequence.h"

/* method 1 */
#define IS(x)	((*(char *)&(x)) == (x)) ? 1 : 0

/* method 2 */
int is_big_endian()
{
	union w {  
		int a;
		char b;
	}c;
	c.a = 1;
	
	return (c.b == 1);
}


void main()
{
	
	int b = 1;
	int c;
	c = IS(b);
	printf("b %d\n", c);
}