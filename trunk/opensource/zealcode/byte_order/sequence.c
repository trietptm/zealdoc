/************************************************************
* Big-endian:  The high addr is corresponding LSB.
*      +---------------------+
*  Low + n | n+1 | n+2 | n+3 + High
*      +---------------------+
*      MSB		    LSB
*	
*	i = 0x01020304
*      +---------------------+
*  Low + 01 | 02 |  03 |  04 + High
*      +---------------------+
*      MSB		    LSB
* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
* 
* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
* Little-endian: The low addr is corresponding LSB.
*      +---------------------+
* High + n+3 | n+2 | n+1 | n + Low
*      +---------------------+
*      MSB		    LSB
*
*	i = 0x01020304
*      +---------------------+
* High + 01 | 02 |  03 |  04 + Low
*      +---------------------+
*      MSB		    LSB
*************************************************************/
#include <stdio.h>
#include "sequence.h"

/* method 1 */
#define IS_BIG_ENDIAN(x)	((*(char *)&(x)) == (x)) ? 1 : 0

/* method 2 */
int is_big_endian()
{
	union w {  
		short a;
		char b;
	}c;
	c.a = 0x0102;
	
	return (c.b == 0x01);
}

#if 1
void main()
{
	
	int b = 0x01000000;
	int c;
/* 	c = IS_BIG_ENDIAN(b);
 */
	c = is_big_endian();
	printf("c=%d\n", c);
}
#else

#endif