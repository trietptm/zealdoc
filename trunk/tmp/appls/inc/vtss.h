#ifndef __VTSS_H_INCLUDE__
#define __VTSS_H_INCLUDE__

/* Vitesse G'RocX support */
#include <sysdep.h>
#include <netapl.h>

#include <vtss_board.h>
#include <vtss_conf.h>

#define _PATH_VITGENIO		"/dev/vitgenio"

/* ================================================================= *
 *  Basic types:
 *    uint, ulong, ushort, uchar - unsigned standard types.
 *    longlong, ulonglong - 64 bit integers.
 *    BOOL - The boolean type. false: 0, true: anything but 0.
 * ================================================================= */

/* ================================================================= *
 *  Return Codes
 * ================================================================= */
typedef int vtss_rc;

#define VTSS_OK                  0     /* rc>=0 means OK */
#define VTSS_RC(expr) { vtss_rc rc = (expr); if (rc < VTSS_OK) return rc; }

/* General warnings */
#define VTSS_WARNING            -0x01  /* Error, but fixed by API. */
#define VTSS_INCOMPLETE         -0x02  /* Operation incomplete */

/* General errors */
#define VTSS_UNSPECIFIED_ERROR  -0x03
#define VTSS_NOT_IMPLEMENTED    -0x04
#define VTSS_INVALID_PARAMETER  -0x05
#define VTSS_DATA_NOT_READY     -0x06
#define VTSS_ENTRY_NOT_FOUND    -0x07
#define VTSS_TIMEOUT_RETRYLATER -0x08  /* Timeout, retry later. */
#define VTSS_FATAL_ERROR        -0x09  /* Fatal error. Chip reset required. */

/* PHY errors */
#define VTSS_PHY_NOT_MAPPED     -0x10
#define VTSS_PHY_READ_ERROR     -0x11  /* No PHY read reply. */
#define VTSS_PHY_TIMEOUT        -0x12  /* The PHY did not react within spec'ed time */

/* Packet errors */
#define VTSS_PACKET_BUF_SMALL    -0x30 
#define VTSS_PACKET_PROTOCOL_ERROR -0x31
 
/* Layer 2 errors */
#define VTSS_AGGR_INVALID       -0x40

/* I/O errors for Vitesse implementation */
#define VTSS_IO_READ_ERROR      -0x60  /* I/O Layer read error */
#define VTSS_IO_WRITE_ERROR     -0x61  /* I/O Layer write error */
#define VTSS_IO_DMA             -0x62  /* I/O Layer DMA error */

#define VTSS_IO_NIC_READ_ERROR  -0x63  /* I/O NIC Layer read error */
#define VTSS_IO_NIC_WRITE_ERROR -0x64  /* I/O NIC Layer write error */
#define VTSS_IO_NIC_ERROR       -0x65  /* I/O NIC Layer error */

/* Customer specific errors, use range: -0x1000 to -0x1FFF */

/* ================================================================= *
 * custom types
 * ================================================================= */
/* Big counter type, may be 32 or 64 bits depending on the OS */
/* You may redefine it to ulong or ulonglong */
typedef uint64_t vtss_counter_t;

/* ================================================================= *
 *  Macros:
 *    VTSS_ASSERT(expr) - Call assert(expr).
 *    VTSS_NSLEEP(nsec) - Sleep at least nsec nanoseconds.
 *    VTSS_MSLEEP(msec) - Sleep at least msec milliseconds.
 *  Notes:
 *    VTSS_NSLEEP uses busy waiting, so it should only be used for
 *    very short intervals.
 *    VTSS_MSLEEP should not use busy waiting, but may do so.
 * ================================================================= */
#define VTSS_ASSERT(expr) { assert(expr); }

#ifdef WIN32
#define VTSS_NSLEEP(nsec) { Sleep((nsec+999999)/1000000); }
#define VTSS_MSLEEP(msec) { Sleep(msec); }
#else
#define VTSS_NSLEEP(nsec) { \
	struct timespec ts = { 0, nsec }; \
	while (nanosleep(&ts,&ts) == -1 && errno == EINTR); \
}
#define VTSS_MSLEEP(msec) { \
	struct timespec ts; \
	ts.tv_sec = msec/1000; \
	ts.tv_nsec = (msec%1000)*1000000; \
	while (nanosleep(&ts,&ts) == -1 && errno == EINTR); \
}
#if 0
#define VTSS_NSLEEP(nsec) { \
	struct timeval tve, tv; \
	gettimeofday(&tve, NULL); \
	tve.tv_usec += (nsec+999)/1000;\
	if (tve.tv_usec >= 1000000) { \
		tve.tv_sec += tve.tv_usec/1000000; \
		tve.tv_usec %= 1000000; } \
	do { gettimeofday(&tv, NULL); } \
	while ( timercmp(&tv, &tve, <) ); \
}
#endif
#endif

/* ================================================================= *
 *  Timers
 * ================================================================= *
 *  Type declarations:
 *    vtss_mtimer_t - Millisecond timer type.
 *  Macros:
 *    VTSS_MTIMER_START(&timer,msec) - Start timer of msec milliseconds.
 *    VTSS_MTIMER_TIMEOUT(&timer) - Returns TRUE if timer timed out.
 *    VTSS_MTIMER_CANCEL(&timer) - Cancel the timer.
 *  Usage Example:
 *    vtss_mtimer_t t;
 *    int i = 1;
 *    VTSS_MTIMER_START(&t,1000);
 *    while (!VTSS_MTIMER_TIMEOUT(&t)) {
 *        VTSS_MSLEEP(200);
 *        printf("loop %d\n",i);
 *        if (i++>=10) {
 *            printf("break\n");
 *            VTSS_MTIMER_CANCEL(&t);
 *            return;
 *        }
 *    }
 *    printf("done\n");
 *  Notes:
 *    Before a running timer is deallocated, VTSS_MTIMER_CANCEL()
 *    must be called, to make sure that no pointers in the O/S are
 *    pointing to the timer being deallocated.
 *    It is harmless calling VTSS_MTIMER_CANCEL() with a timer which
 *    has already timed out.
 * ================================================================= */
/* The function "gettimeofday" is available, so use it. */
typedef struct _vtss_mtimer_t {
	struct timeval  timeout;
	struct timeval  now;
} vtss_mtimer_t;

#define VTSS_MTIMER_START(timer,msec) { \
	gettimeofday(&((timer)->timeout), NULL); \
	(timer)->timeout.tv_usec += msec*1000; \
	if ((timer)->timeout.tv_usec >= 1000000) { \
		(timer)->timeout.tv_sec += (timer)->timeout.tv_usec/1000000; \
		(timer)->timeout.tv_usec%=1000000; \
	} \
}
#define VTSS_MTIMER_TIMEOUT(timer) (gettimeofday(&((timer)->now),NULL)==0 && timercmp(&((timer)->now),&((timer)->timeout),>))
#define VTSS_MTIMER_CANCEL(timer) /* No action in this implementation. */

/* ================================================================= *
 *  Various definitions and macros
 * ================================================================= */
/* MAKEBOOL01(value): Convert BOOL value to 0 (false) or 1 (true). */
/* Use this to ensure BOOL values returned are always 1 or 0. */
#ifndef MAKEBOOL01
#define MAKEBOOL01(value) ((value)?1:0)
#endif

/* Event type. When a variable of this type is used as an input parameter,
   the API will set the variable if the event has occured. The API will 
   never clear the variable. If is up to the application to clear the
   variable, when the event has been handled. */
typedef BOOL vtss_event_t;

#ifndef NULL
#define NULL ((void *)0)
#endif

typedef struct _vtss_config_t {
	int mac_aging;
#define VTSS_MAC_AGE_DEFAULT	300 /* 300 seconds MAC age timer by default */
} vtss_config_t;

#endif /* __VTSS_H_INCLUDE__ */
