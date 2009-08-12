
#include <linux/types.h>

#ifndef _VTSS_STATE_H_
#define _VTSS_STATE_H_

/* Number of ports on chip */
#if defined(VTSS_ARCH_GATWICK)
#define VTSS_CHIP_PORTS 32
#endif /* VTSS_ARCH_GATWICK */

#if defined(VTSS_ARCH_HEATHROW)

#if defined(HEATHROW3)
#define VTSS_CHIP_PORTS 24
#define VTSS_CHIP_PORTMASK ((1<<VTSS_PORTS)-1)<<4
#endif /* HEATHROW3 */

#if defined(SPARX_G5)
#define VTSS_CHIP_PORTS    7
#define VTSS_CHIP_PORTMASK 0x5f /* Chip port 0-4 and 6 */
#endif /* SPARX_G5 */

#if defined(VTSS_ARCH_SPARX_28)
#if defined(G_ROCX)
#define VTSS_CHIP_PORTS    VTSS_PORT_COUNT
#else
#define VTSS_CHIP_PORTS    28
#endif /* G_ROCX */
#define VTSS_CHIP_PORT_CPU VTSS_CHIP_PORTS
#define VTSS_CHIP_PORTMASK ((1<<VTSS_CHIP_PORTS)-1)
#endif /* VTSS_ARCH_SPARX_28 */

#if defined(ELSTREE)
#define VTSS_CHIP_PORTS 12
#endif

#if defined(VTSS_ARCH_HAWX)
#define VTSS_CHIP_PORTS 26
#endif /* VTSS_ARCH_HAWX */

#if !defined(VTSS_CHIP_PORTS)
#define VTSS_CHIP_PORTS VTSS_PORT_COUNT
#endif

#if !defined(VTSS_CHIP_PORTMASK)
#define VTSS_CHIP_PORTMASK ((1<<VTSS_PORT_COUNT)-1)
#endif
#endif /* VTSS_ARCH_HEATHROW */

#define VTSS_CHIP_PORT_AGGR_0 24 /* Chip port 24 and 25 are paired */
#define VTSS_CHIP_PORT_AGGR_1 26 /* Chip port 26 and 27 are paired */
#define VTSS_PORT_NO_AGGR_0   (VTSS_PORTS+1) /* Pseudo port mapping to chip port 25 */
#define VTSS_PORT_NO_AGGR_1   (VTSS_PORTS+2) /* Pseudo port mapping to chip port 27 */

#if defined(VTSS_ARCH_SPARX_28)
#define VTSS_CHIP_PORT_VAUI_START 24
#endif /* VTSS_ARCH_SPARX_28 */

/* Number of internally aggregated ports */
#if VTSS_OPT_INT_AGGR
#define VTSS_PORTS_INT_AGGR 2
#else
#define VTSS_PORTS_INT_AGGR 0
#endif /* VTSS_OPT_INT_AGGR */

/* Port array including internally aggregated ports */
#define VTSS_PORT_EXT_ARRAY_SIZE (VTSS_PORT_ARRAY_SIZE+VTSS_PORTS_INT_AGGR)

/* Bit field macros */
#define VTSS_BF_SIZE(n)      ((n+7)/8)
#define VTSS_BF_GET(a, n)    ((a[(n)/8] & (1<<((n)%8))) ? 1 : 0)
#define VTSS_BF_SET(a, n, v) { if (v) { a[(n)/8] |= (1<<((n)%8)); } else { a[(n)/8] &= ~(1<<((n)%8)); }}
#define VTSS_BF_CLR(a, n)    (memset(a, 0, VTSS_BF_SIZE(n)))

/* Port member bit field macros */
#define VTSS_PORT_BF_SIZE                VTSS_BF_SIZE(VTSS_PORTS)
#define VTSS_PORT_BF_GET(a, port_no)     VTSS_BF_GET(a, port_no - VTSS_PORT_NO_START)
#define VTSS_PORT_BF_SET(a, port_no, v)  VTSS_BF_SET(a, port_no - VTSS_PORT_NO_START, v)
#define VTSS_PORT_BF_CLR(a)              VTSS_BF_CLR(a, VTSS_PORTS)

/* - Port state definitions ---------------------------------------- */

/* - Packet state definitions -------------------------------------- */

#define VTSS_FCS_SIZE 4 /* MAC frame CRC size */

/* - L2 state definitions ------------------------------------------ */

/* Port Group ID */
typedef uint vtss_pgid_no_t;

#if defined(VTSS_ARCH_GATWICK)
#define VTSS_CHIP_PGIDS_GW   256 /* Gatwick PGIDs */
#define VTSS_CHIP_PGIDS      512
#define VTSS_PGID_END_GW     (VTSS_PGID_START+VTSS_CHIP_PGIDS_GW-(VTSS_CHIP_PORTS-VTSS_PORTS))
#endif /* VTSS_ARCH_GATWICK */

#if defined(VTSS_ARCH_HEATHROW)
#define VTSS_CHIP_PGIDS      64
#endif /* VTSS_ARCH_HEATHROW */

#define VTSS_PGID_NONE            (0)
#define VTSS_PGID_START           (1)
#if defined(VTSS_ARCH_GATWICK) && defined(VTSS_CHIPS)
#define VTSS_PGID_SIZE            (VTSS_CHIP_PGIDS)
#define VTSS_PGID_GLAG_START      257
#else
#define VTSS_PGID_SIZE            (VTSS_CHIP_PGIDS-(VTSS_CHIP_PORTS-VTSS_PORTS))
#endif /* VTSS_ARCH_GATWICK && VTSS_CHIPS */
#define VTSS_PGID_END             (VTSS_PGID_START+VTSS_PGID_SIZE)
#define VTSS_PGID_UNICAST_START   (VTSS_PGID_START)
#define VTSS_PGID_UNICAST_END     (VTSS_PGID_START+VTSS_PORTS)
#define VTSS_PGID_KILL            (VTSS_PGID_UNICAST_END)
#define VTSS_PGID_BC              (VTSS_PGID_KILL+1)
#define VTSS_PGID_MULTICAST_START (VTSS_PGID_BC+1)
#define VTSS_PGID_MULTICAST_END   (VTSS_PGID_END)
#define VTSS_PGID_ARRAY_SIZE      VTSS_PGID_END

/* Masks reserved for GLAG support */
#if defined(VTSS_FEATURE_AGGR_GLAG)
#define VTSS_PGID_GLAG_START      (VTSS_PGID_START+30+VTSS_PORTS-VTSS_CHIP_PORTS)
#define VTSS_PGID_GLAGS           8
#define VTSS_PGID_GLAG_END        (VTSS_PGID_GLAG_START+VTSS_PGID_GLAGS)
#define VTSS_PGID_GLAG_DEST       (VTSS_PGID_GLAG_START+0) /* 30-31: Dest. masks */
#define VTSS_PGID_GLAG_SRC        (VTSS_PGID_GLAG_START+2) /* 32-33: Src. masks */
#define VTSS_PGID_GLAG_AGGR_0     (VTSS_PGID_GLAG_START+4) /* 34-35: Aggr. A masks */
#define VTSS_PGID_GLAG_AGGR_1     (VTSS_PGID_GLAG_START+6) /* 36-37: Aggr. B masks */
#endif /* VTSS_FEATURE_AGGR_GLAG */

/* Mapping between high level PGID and chip PGID is done in the low level layer */

#if defined(VTSS_ARCH_HEATHROW)
#if defined(HEATHROW2)
#define VTSS_ACS      ((vtss_ac_no_t)8)
#else
#define VTSS_ACS      ((vtss_ac_no_t)16)
#endif /* HEATHROW2 */
#endif /* VTSS_ARCH_HEATHROW */

#if defined(VTSS_ARCH_GATWICK)
#define VTSS_ACS      ((vtss_ac_no_t)64)
#endif /* VTSS_ARCH_GATWICK */

#define VTSS_AC_START ((vtss_ac_no_t)1)
#define VTSS_AC_END   (VTSS_AC_START+VTSS_ACS)

#define VTSS_STP_FORWARDING(stp_state) (stp_state==VTSS_STP_STATE_FORWARDING || stp_state==VTSS_STP_STATE_ENABLED)
#define VTSS_STP_UP(stp_state)         (stp_state!=VTSS_STP_STATE_DISABLED)

/* Port forwarding state */
#define VTSS_PORT_RX_FORWARDING(fwd_state) (fwd_state == VTSS_PORT_FORWARD_ENABLED || fwd_state == VTSS_PORT_FORWARD_INGRESS)
#define VTSS_PORT_TX_FORWARDING(fwd_state) (fwd_state == VTSS_PORT_FORWARD_ENABLED || fwd_state == VTSS_PORT_FORWARD_EGRESS)

/* Size of lookup page and pointer array */
#define VTSS_MAC_PAGE_SIZE 128 
#define VTSS_MAC_PTR_SIZE  (VTSS_MAC_ADDRS/VTSS_MAC_PAGE_SIZE)

/* MAC address table for get next operations */
typedef struct vtss_mac_entry_t {
    struct vtss_mac_entry_t *next;  /* Next in list */
    ulong                   mach;  /* VID and 16 MSB of MAC */
    ulong                   macl;  /* 32 LSB of MAC */
#if defined(VTSS_ARCH_HAWX) || defined(VTSS_ARCH_SPARX_28)
    uchar                   member[VTSS_PORT_BF_SIZE];
#else
    ulong                   block; /* Table block index */
#endif /* VTSS_ARCH_HAWX/SPARX_28 */
} vtss_mac_entry_t;

/* IPv4 and IPv6 multicast address */
#define VTSS_MAC_IPV4_MC(mac) (mac[0] == 0x01 && mac[1] == 0x00 && mac[2] == 0x5e && (mac[3] & 0x80) == 0x00)
#define VTSS_MAC_IPV6_MC(mac) (mac[0] == 0x33 && mac[1] == 0x33)

/* - L3 state definitions ------------------------------------------ */

#if defined(VTSS_FEATURE_LAYER3)

/* Port Router Leg ID: VTSS_PRLID_START..(VTSS_PRLID_END-1) */
typedef uchar vtss_prlid_t;
#define VTSS_PRLIDS           ((vtss_prlid_t)4)
#define VTSS_PRLID_L2         ((vtss_prlid_t)0) /* Pseudo PRLID for L2 */
#define VTSS_PRLID_START      ((vtss_prlid_t)1)
#define VTSS_PRLID_END        (VTSS_PRLID_START+VTSS_PRLIDS)
#define VTSS_PRLID_ARRAY_SIZE VTSS_PRLID_END

/* Egress Router Leg ID */
typedef uint vtss_erlgid_t;
#define VTSS_ERLGIDS           ((vtss_erlgid_t)31) /* ERLGID zero reserved for L2 */
#define VTSS_ERLGID_START      ((vtss_erlgid_t)1)
#define VTSS_ERLGID_END        (VTSS_ERLGID_START+VTSS_ERLGIDS)
#define VTSS_ERLGID_ARRAY_SIZE VTSS_ERLGID_END

/* Unicast forwarding table index */
typedef uint vtss_ucid_t;
#define VTSS_UCIDS           ((vtss_ucid_t)64)
#define VTSS_UCID_START      ((vtss_ucid_t)1)
#define VTSS_UCID_END        (VTSS_UCID_START+VTSS_UCIDS)
#define VTSS_UCID_ARRAY_SIZE VTSS_UCID_END

/* ARP table index */
typedef uint vtss_arpid_t;
#define VTSS_ARPIDS           ((vtss_arpid_t)8192)
#define VTSS_ARPID_START      ((vtss_arpid_t)1)  /* First entry */
#define VTSS_ARPID_FIRST      ((vtss_arpid_t)2)  /* First entry available for allocation */
#define VTSS_ARPID_END        (VTSS_ARPID_START+VTSS_ARPIDS)
#define VTSS_ARPID_ARRAY_SIZE VTSS_ARPID_END

#endif /* VTSS_FEATURE_LAYER3 */

#if defined(VTSS_ARCH_SPARX_G8)
typedef enum _vtss_luton_version_t {
    VTSS_LUTON_TYPE_BASIC,      /* SparX-G5 or SparX-G8 */
    VTSS_LUTON_TYPE_EXT,        /* SparX-G5e or SparX-G8e */
    VTSS_LUTON_TYPE_R2          /* SparX-G5m */
} vtss_luton_version_t;
#endif /* VTSS_ARCH_SPARX_G8 */
#endif /* _VTSS_STATE_H_ */
/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
