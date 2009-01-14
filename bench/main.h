#ifndef _MAIN_H_
#define _MAIN_H_

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <sys/times.h>
#include <sys/types.h>		/* For pid_t. */
#include <sys/wait.h>
#include <sys/param.h>		/* For getpagesize, maybe.  */

#include "vtss/vtss.h"
/* main.c */
/*
 * Error code definitions
 */
#define ERR_SUC  0 /* no error */
#define ERR_CMD  1 /* command lines error */
#define ERR_PIP  2 /* pipe created error */
#define ERR_FRK  3 /* fork error */
#define ERR_WRI  4 /* write error */
#define ERR_RED  5 /* read error */
#define ERR_FOP  6 /* fopen error */
#define ERR_DIR  7 /* dir error */
#define ERR_TIO  8 /* timeout error */
#define ERR_SYN  9 /* sync error */
#define ERR_SIG  10 /* kill error */
#define ERR_CLK  11 /* clock error */

/* --time.c-- */
#define TV_MSEC tv_usec / 1000
#include <sys/resource.h>

/* Information on the resources used by a child process.  */
typedef struct {
	int waitstatus;
	struct rusage ru;
	struct timeval start, elapsed; /* Wallclock time of process.  */
} resource_t;

typedef int (*test_func)(void);
int time_main (test_func foo);
/* --time.c-- */

#endif	/* _MAIN_H_ */
