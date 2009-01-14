/* Standard headers */
#ifndef VTSS_H
#define VTSS_H

#include <stdio.h>
#include <string.h>
#ifdef ARCH_ARM
 #include <linux/vitgenio.h>
#endif
#include <sys/ioctl.h> /* ioctl() */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

/* API private headers */
#include "vtss_state.h"
#include "vtss_sparx_reg.h"
//#include "vtss_switch_api.h"

typedef int vtss_rc;
#define VTSS_OK		0
#define VTSS_CHIP_PORTS	7
#define VTSS_CHIP_PORTMASK 	((1 << VTSS_CHIP_PORTS) - 1)
#define VTSS_CHIP_PORT_CPU	VTSS_CHIP_PORTS

#ifdef ARCH_ARM
/* vtss_io */
extern int vtss_vit_init(void);
extern void vtss_vit_exit(void);
/* vtss_regop */
extern int ht_rd(uint blk, uint sub, uint reg, ulong *value);
extern int ht_cpurx_readbuffer(int queue_no, const uint addr, ulong *value);

#define HT_RD(blk, sub, reg, value) \
{ \
    vtss_rc rc; \
    if ((rc = ht_rd_wr(B_##blk, sub, R_##blk##_##reg, value, 0)) < 0) \
        return rc; \
}
#else
static inline int vtss_vit_init(void) {return 0;}
static inline void vtss_vit_exit(void) {}
static inline int ht_rd(uint blk, uint sub, uint reg, ulong *value){sleep(1);return 0;}
static inline int ht_cpurx_readbuffer(int queue_no, const uint addr, ulong *value){int i=0;i++;i++;return 0;}
#endif /* ARCH_ARM */
#endif	/* VTSS_H */
