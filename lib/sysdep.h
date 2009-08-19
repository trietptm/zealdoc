#ifndef __SYSDEP_H_INCLUDE__
#define __SYSDEP_H_INCLUDE__

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <stdarg.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <paths.h>

#include <sys/stat.h>
#include <sys/timeb.h>
#include <sys/types.h>

#ifndef __cold
#define __cold
#endif

#ifdef WIN32
#ifndef __cplusplus
#ifndef inline
#define inline __inline
#endif
#endif

#ifndef __FUNCTION__
#define __FUNCTION__		__FILE__
#endif

#define __attribute__(x)
#define __attribute_used__
#define __LITTLE_ENDIAN_BITFIELD	1

/* unreferenced formal parameter */
//#pragma warning (3: 4100)
/* conditional expression is constant */
//#pragma warning (3: 4127)
/* local variable is initialized but not referenced */
#pragma warning (3: 4189)
/* local variable %s may be used without having been initialized */
//#pragma warning (3: 4701)
/* assignment within conditional expression */
//#pragma warning (3: 4706)
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>
#include <io.h>
#include <direct.h>

#ifndef _IOWR
#define _IOWR(x,y,t)     (IOC_IN|IOC_OUT|(((long)sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|(y))
#endif

#define mkdir(x, y)	_mkdir(x)
#define lstat		stat
#define index		strchr
#define rindex		strrchr
#define bzero(a, b)	memset(a, 0, b)
#define bcmp		memcmp
#define bcopy(a, b, c)	memcpy((b), (a), (c))

#ifndef PATH_MAX
#define PATH_MAX	MAX_PATH
#endif

#define SHUT_RD		SD_RECEIVE
#define SHUT_WR		SD_SEND
#define SHUT_RDWR	SD_BOTH

/* sys/param.h macros */
#ifndef MAXPATHLEN
#define MAXPATHLEN	256
#endif

#include <linux/ioctl.h>
#include <linux/termios.h>
#include <linux/uio.h>
#include <linux/in.h>

#define	EIO		 5	/* I/O error */
#define	EADDRNOTAVAIL	49	/* Cannot assign requested address */
#define	EPROTO		71	/* Protocol error */
#define	ENOBUFS		105	/* No buffer space available */
#define	EPFNOSUPPORT	96	/* Protocol family not supported */
#define EOPNOTSUPP	45

#ifndef caddr_t
#define caddr_t		char *
#endif
#define ssize_t		size_t
#define sockfd_t	SOCKET

#undef timezone
struct timezone {
	int tz_minuteswest;
	int tz_dsttime;
};

struct timespec {
	time_t tv_sec;			/* Seconds.  */
	long int tv_nsec;		/* Nanoseconds.  */
};

typedef int32_t __fsblkcnt_t;		/* Type to count file system blocks. */
typedef struct {
	int __val[2];
} __fsid_t;				/* Type of file system IDs.  */

struct statfs {
	long int f_type;
#define f_fstyp f_type
	long int f_bsize;
	long int f_frsize;	/* Fragment size - unsupported */
	__fsblkcnt_t f_blocks;
	__fsblkcnt_t f_bfree;
	__fsblkcnt_t f_files;
	__fsblkcnt_t f_ffree;
	__fsblkcnt_t f_bavail;
	
	/* Linux specials */
	__fsid_t f_fsid;
	long int f_namelen;
	long int f_spare[6];
};

#define fsync		_commit
#define vsnprintf	_vsnprintf
#define snprintf	_snprintf
#define strcasecmp	_stricmp
#define strncasecmp	_strnicmp
#define random		rand
#define srandom		srand

#define ftruncate(f,s)	chsize(f,s)
#define msleep(x)	Sleep(x);
#define sleep(x)	Sleep(x*1000)

#define S_ISREG(x)	(x & _S_IFREG)
#define S_ISDIR(x)	(x & _S_IFDIR)

#define CHAR_BIT	8
#define F_OK		0

int gettimeofday(struct timeval *tp, struct timezone *tz);
int settimeofday(const struct timeval *tp, const struct timezone *tz);
int statfs(const char *path, struct statfs *buf);

#include <missing.h>

typedef unsigned int		uint;
typedef unsigned long		ulong;
typedef unsigned short		ushort;
typedef unsigned char		uchar;
typedef signed char		schar;
typedef __int64			longlong;
typedef unsigned __int64	ulonglong;

#else	/* WIN32 */

#include <unistd.h>
#include <sys/mman.h>
#include <fnmatch.h>
#include <pwd.h>
#include <grp.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/mount.h>

#define _dirent		dirent
#define sockfd_t	int
typedef long long		longlong;
typedef unsigned long long	ulonglong;
typedef int			BOOL;

#define msleep(x)	usleep(x*1000);
#define closesocket(x)	close(x)
#define INVALID_SOCKET	-1

#endif

/* Standard file descriptors.  */
#define	STDIN_FILENO	0	/* Standard input.  */
#define	STDOUT_FILENO	1	/* Standard output.  */
#define	STDERR_FILENO	2	/* Standard error output.  */

#ifndef FALSE
#define FALSE		0
#define TRUE		!FALSE
#endif

typedef int bool;
typedef unsigned char		uchar;

#include <dirent.h>
#include <netdb.h>
#include <atomic.h>
#include <linux/types.h>
#include <linux/netlink.h>

#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b))?(a):(b))
#endif

#ifndef MAX
#define MAX(a, b) (((a) > (b))?(a):(b))
#endif

/* And this pointer trick too */
#ifndef offsetof
#define offsetof(s,m)   (size_t)&(((s *)0)->m)
#endif
#define container_of(ptr, type, member) (type *)( (char *)ptr - offsetof(type,member) )

#define swap_pointer(x, y)		\
	do {			\
		*(x) ^= *(y);	\
		*(y) ^= *(x);	\
		*(x) ^= *(y);	\
	} while (0)

#define ALIGN(x,a)		__ALIGN_MASK(x,(a)-1)
#define __ALIGN_MASK(x,mask)	(((x)+(mask))&~(mask))

#ifdef WIN32
#define KBUILD_NO_NLS 1
#endif
#ifndef KBUILD_NO_NLS
#include <libintl.h>
#define ENABLE_NLS	1
#else
#define ENABLE_NLS	0
#endif
#define YYENABLE_NLS		ENABLE_NLS
#define YYLTYPE_IS_TRIVIAL	0

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

#endif /* __SYSDEP_H_INCLUDE__ */

