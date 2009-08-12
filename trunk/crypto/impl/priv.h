#ifndef PRIV_H
#define PRIV_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#define le32_to_cpu(x)	(x)
#define cpu_to_le32(x)	(x)
#define __le32

typedef unsigned char u8;
typedef unsigned long u32;

#endif /* PRIV_H */
