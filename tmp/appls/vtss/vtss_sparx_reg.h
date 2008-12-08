/*

 Vitesse Switch API software.

 Copyright (c) 2002-2007 Vitesse Semiconductor Corporation "Vitesse". All
 Rights Reserved.
 
 Unpublished rights reserved under the copyright laws of the United States of
 America, other countries and international treaties. Permission to use, copy,
 store and modify, the software and its source code is granted. Permission to
 integrate into other products, disclose, transmit and distribute the software
 in an absolute machine readable format (e.g. HEX file) is also granted.  The
 source code of the software may not be disclosed, transmitted or distributed
 without the written permission of Vitesse. The software and its source code
 may only be used in products utilizing the Vitesse switch products.
 
 This copyright notice must appear in any copy, modification, disclosure,
 transmission or distribution of the software. Vitesse retains all ownership,
 copyright, trade secret and proprietary rights in the software.
 
 THIS SOFTWARE HAS BEEN PROVIDED "AS IS," WITHOUT EXPRESS OR IMPLIED WARRANTY
 INCLUDING, WITHOUT LIMITATION, IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR USE AND NON-INFRINGEMENT.
 
 $Id: vtss_sparx_reg.h,v 1.7 2008-12-08 04:02:55 zhenglv Exp $
 $Revision: 1.7 $

*/

#ifndef _HEATHROW_REG_H_
#define _HEATHROW_REG_H_

/* Naming conventions:
   B_<blk>      : Block <blk>
   S_<sub>      : Sub block <sub>
   R_<blk>_<reg>: Block <blk>, register <reg>
*/

/* ================================================================= *
 * Block IDs
 * ================================================================= */
#define B_PORT     1  /* Port (Sub Blocks: 0-15) */
#define B_ANALYZER 2  /* Analyzer (Sub Block: 0) */
#define B_MIIM     3  /* MII Management (Sub Blocks: 0 or 0-1 depending on chip) */
#define B_MEMINIT  3  /* Memory Initialization (Sub Block: 1 or 2 depending on chip) */
#ifdef CONFIG_VTSS_ARCH_SPARX_28
#define B_ACL      3  /* ACL */
#endif
#define B_CAPTURE  4  /* CPU Capture Buffer (Sub Blocks: 0-3,4,6,7) depending on chip */
#define B_ARBITER  5  /* Arbiter (Sub Block: 0) */
#define B_PORT_HI  6  /* Port 16-31 (Sub Blocks: 0-15) */
#define B_SYSTEM   7  /* System Registers (Sub Block: 0) */

/* Memory initialization sub block */
#define S_MEMINIT 2

#ifdef CONFIG_VTSS_ARCH_SPARX_28
#define S_ACL  3
#endif

/* Capture sub blocks */
#define S_CAPTURE_DATA     0 /* CPU Capture Buffer Data */

#define S_CAPTURE_STATUS   4 /* CPU Capture Buffer Status Sub Block: 4 */

#define S_CAPTURE_RESET    7 /* CPU Capture Buffer Reset Sub Block: 7 */

/* ================================================================= *
 * B_PORT::PORTREG registers
 * ================================================================= */
#define R_PORT_REGMODE 0x01 /* Device Register Mode */

/* ================================================================= *
 * B_PORT::MAC registers
 * ================================================================= */
#define R_PORT_MAC_CFG     0x00 /* MAC Config */
#define R_PORT_MACHDXGAP   0x02 /* Half Duplex Gaps */
#define R_PORT_FCCONF      0x04 /* Flow Control transmit */
#define R_PORT_FCMACHI     0x08 /* Flow Control SMAC High */
#define R_PORT_FCMACLO     0x0C /* Flow Control SMAC Low */
#define R_PORT_MAXLEN      0x10 /* Max Length */
#define R_PORT_ADVPORTM    0x19 /* Advanced Port Mode Setup */
#define R_PORT_TXUPDCFG    0x24 /* Transmit Modify Setup */

#ifdef CONFIG_VTSS_ARCH_SPARX_28
#define R_PORT_STACKCFG    0x25 /* Stack configuration */
#define R_PORT_SHAPER_CFG  0x28 /* Shaper Setup */
#define R_PORT_POLICER_CFG 0x29 /* Policer Setup */
#endif

/* ================================================================= *
 * B_PORT::Shared FIFO registers
 * ================================================================= */
#define R_PORT_CPUTXDAT    0xC0 /* CPU Transmit DATA */
#define R_PORT_MISCFIFO    0xC4 /* Misc Control Register */
#define R_PORT_MISCSTAT    0xC8 /* Misc Status */

#ifdef CONFIG_VTSS_ARCH_SPARX_28
#define R_PORT_MISCCFG     0xC5 /* Miscellaneous Configuration */
#define R_PORT_CPUTXHDR    0xCC /* CPU Transmit HEADER */
#endif

#define R_PORT_FREEPOOL    0xD8 /* Free RAM Counter */

#ifdef CONFIG_VTSS_ARCH_SPARX
#define R_PORT_Q_MISC_CONF 0xDF /* Misc Pool Control */
#endif /* SPARX */

#ifdef CONFIG_VTSS_ARCH_SPARX_28
#define R_PORT_Q_EGRESS_WM 0xE0 /* Egress Max Watermarks */
#endif /* SPARX_28 */

#ifdef CONFIG_VTSS_ARCH_SPARX_28
#define R_PORT_Q_INGRESS_WM 0xE8 /* 0xE8-0xED: Ingress Watermarks */ 
#endif

/* ================================================================= *
 * B_PORT::Categorizer registers
 * ================================================================= */
#ifdef CONFIG_VTSS_ARCH_SPARX
#define R_PORT_CAT_PR_DSCP_VAL_0_3 0x61 /* Categorizer DSCP Values 0-3 */
#define R_PORT_CAT_DROP            0x6E /* Categorizer Frame Dropping */
#define R_PORT_CAT_PR_MISC_L2      0x6F /* Categorizer Misc L2 QoS */
#define R_PORT_CAT_VLAN_MISC       0x79 /* Categorizer VLAN Misc */
#define R_PORT_CAT_PORT_VLAN       0x7A /* Categorizer Port VLAN */
#define R_PORT_CAT_OTHER_CFG       0x7B /* Categorizer Other Configuration */
#define R_PORT_CAT_GEN_PRIO_REMAP  0x7D /* Categorizer Generic Priority Remap */
#ifdef CONFIG_VTSS_ARCH_SPARX_28
#define R_PORT_CAT_QCE0            0x80 /* 0x80-0x8B: QCE 0-11 */
#define R_PORT_CAT_QCL_RANGE_CFG   0x8C /* QCL Range Configuration */
#define R_PORT_CAT_QCL_DEFAULT_CFG 0x8D /* QCL Default Configuration */
#endif
#endif

/* ================================================================= *
 * B_PORT::Counter registers
 * ================================================================= */
#ifdef CONFIG_VTSS_ARCH_SPARX_28
#define R_PORT_C_CNTDATA    0xDC /* Counter Data */
#define R_PORT_C_CNTADDR    0xDD /* Counter Address */
#define R_PORT_C_CLEAR      0x52 /* Clear Counters */
#else
#define R_PORT_C_RXOCT      0x50 /* Rx Octets */
#define R_PORT_C_TXOCT      0x51 /* Tx Octets */
#endif

/* ================================================================= *
 * B_ANALYZER registers
 * ================================================================= */
#define R_ANALYZER_ADVLEARN     0x03 /* Advanced Learning Setup */
#define R_ANALYZER_IFLODMSK     0x04 /* IP Multicast Flood Mask */
#define R_ANALYZER_VLANMASK     0x05 /* VLAN Source Port Mask */
#define R_ANALYZER_MACHDATA     0x06 /* Mac Address High */
#define R_ANALYZER_MACLDATA     0x07 /* Mac Address Low */
#define R_ANALYZER_ANMOVED      0x08 /* Station Move Logger */
#define R_ANALYZER_ANAGEFIL     0x09 /* Aging Filter */
#define R_ANALYZER_ANEVENTS     0x0A /* Event Sticky Bits */
#define R_ANALYZER_ANCNTMSK     0x0B /* Event Sticky Mask */
#define R_ANALYZER_ANCNTVAL     0x0C /* Event Sticky Counter */
#define R_ANALYZER_LERNMASK     0x0D /* Learn Mask */
#define R_ANALYZER_UFLODMSK     0x0E /* Unicast Flood Mask */
#define R_ANALYZER_MFLODMSK     0x0F /* Multicast Flood Mask */
#define R_ANALYZER_RECVMASK     0x10 /* Receive Mask */
#define R_ANALYZER_AGGRCNTL     0x20 /* Aggregation Mode */
#define R_ANALYZER_AGGRMSKS     0x30 /* 0x30-0x3F Aggregation Masks */
#define R_ANALYZER_DSTMASKS     0x40 /* 0x40-0x7F Destination Port Masks */
#define R_ANALYZER_SRCMASKS     0x80 /* Source Port Masks */

#ifdef CONFIG_VTSS_ARCH_SPARX
#define R_ANALYZER_CAPENAB      0xA0 /* Capture Enable */
#endif

#ifdef CONFIG_VTSS_ARCH_SPARX_28
#define R_ANALYZER_CAPQUEUE     0xA1 /* Capture Queue */
#define R_ANALYZER_LEARNDROP    0xA2 /* Learning Drop */
#define R_ANALYZER_LEARNAUTO    0xA3 /* Learning Auto */
#define R_ANALYZER_LEARNCPU     0xA4 /* Learning CPU */
#define R_ANALYZER_STORMLIMIT   0xAA /* Storm Control */
#define R_ANALYZER_STORMLIMIT_ENA 0xAB  /* Storm Control Enable */
#endif /* SPARX_28 */

#ifdef CONFIG_VTSS_ARCH_SPARX_28
#define R_ANALYZER_CAPQUEUEGARP 0xA8 /* Capture Queue GARP */
#define R_ANALYZER_ACLPOLIDX    0xA9 /* ACL Policer Index */
#define R_ANALYZER_PVLAN_MASK   0xAC /* Private VLAN Mask */
#define R_ANALYZER_I6FLODMSK    0xAD /* IPv6 Multicast Flood Mask */
#define R_ANALYZER_STORMPOLUNIT 0xAE /* Storm Policer Unit */
#define R_ANALYZER_AUTOAGE      0xB1 /* Auto Age Timer */
#define R_ANALYZER_STACKPORTSA  0xF1 /* Stack Ports A */
#define R_ANALYZER_STACKPORTSB  0xF2 /* Stack Ports B */
#define R_ANALYZER_STACKCFG     0xF3 /* Stack Configuration */
#define R_ANALYZER_MIRRORPORTS  0xF4 /* Mirror Ports */
#define R_ANALYZER_DSCPMODELOW  0xF8 /* DSCP Mode Low Ports */
#define R_ANALYZER_DSCPMODEHIGH 0xF9 /* DSCP Mode High Ports */
#define R_ANALYZER_DSCPSELLOW   0xFA /* DSCP Enable Low Ports */
#define R_ANALYZER_DSCPSELHIGH  0xFB /* DSCP Enable High Ports */
#define R_ANALYZER_EMIRRORMASK  0xFC /* Egress Mirror Mask */
#endif

#define R_ANALYZER_MACACCES     0xB0 /* Mac Table Command */

#define R_ANALYZER_MACTINDX     0xC0 /* Mac Table Index */
#define R_ANALYZER_VLANACES     0xD0 /* VLAN Table Command */
#define R_ANALYZER_VLANTINDX    0xE0 /* VLAN Table Index */
#define R_ANALYZER_AGENCNTL     0xF0 /* Analyzer Config Register */

/* Commands for Mac Table Command register */
#define MAC_CMD_IDLE        0  /* Idle */
#define MAC_CMD_LEARN       1  /* Insert (Learn) 1 entry */
#define MAC_CMD_FORGET      2  /* Delete (Forget) 1 entry */
#define MAC_CMD_TABLE_AGE   3  /* Age entire table */
#ifdef CONFIG_VTSS_ARCH_SPARX_28
#define MAC_CMD_GET_NEXT    4  /* Get next entry */
#else
#define MAC_CMD_TABLE_FLUSH 4  /* Delete all non-locked entries in table */
#endif /* SPARX_28 */
#define MAC_CMD_TABLE_CLEAR 5  /* Delete all entries in table */
#define MAC_CMD_READ        6  /* Read entry at Mac Table Index */
#define MAC_CMD_WRITE       7  /* Write entry at Mac Table Index */

/* Commands for VLAN Table Command register */
#define VLAN_CMD_IDLE        0  /* Idle */
#define VLAN_CMD_READ        1  /* Read entry at VLAN Table Index */
#define VLAN_CMD_WRITE       2  /* Write entry at VLAN Table Index */
#define VLAN_CMD_TABLE_CLEAR 3  /* Reset all entries in table */

/* Modes for Aggregation Mode register */
#define AGGRCNTL_MODE_SMAC         1   /* SMAC only */
#define AGGRCNTL_MODE_DMAC         2   /* DMAC only */
#define AGGRCNTL_MODE_SMAC_DMAC    3   /* SMAC xor DMAC */
#ifdef CONFIG_VTSS_ARCH_SPARX
#define AGGRCNTL_MODE_SMAC_DMAC_IP 0   /* SMAC xor DMAC xor IP-info */
#define AGGRCNTL_MODE_PSEUDORANDOM 4   /* Pseudo randomized */
#define AGGRCNTL_MODE_IP           5   /* IP-info only */
#endif /* SPARX */

/* ================================================================= *
 * B_MIIM registers
 * ================================================================= */
#define R_MIIM_MIIMSTAT 0x00 /* MII-M Status */
#define R_MIIM_MIIMCMD  0x01 /* MII-M Command */
#define R_MIIM_MIIMDATA 0x02 /* MII-M Return data */
#define R_MIIM_MIIMPRES 0x03 /* MII-M Prescaler */
#define R_MIIM_MIIMSCAN 0x04 /* MII-M Scan setup */
#define R_MIIM_MIIMSRES 0x05 /* MII-M Scan Results */

/* ================================================================= *
 * B_MEMINIT registers
 * ================================================================= */
#define R_MEMINIT_MEMINIT 0x00 /* Memory Initialization */
#define R_MEMINIT_MEMRES  0x01 /* Memory Initialization Result */

/* Commands for Memory Initialization register */
#ifdef CONFIG_VTSS_ARCH_SPARX_28
#define MEMINIT_CMD_INIT  0x10101 /* Initialize Memory */
#else
#define MEMINIT_CMD_INIT  0x10104 /* Initialize Memory */
#endif
#define MEMINIT_CMD_READ  0x00200 /* Read Result */

/* Results for Memory Initialization Result register */
#define MEMRES_OK         0x3  /* Result OK */

/* Maximum Memory ID and skipped memory IDs */
#ifdef CONFIG_VTSS_ARCH_SPARX_28
#ifdef CONFIG_VTSS_GROCX
#define MEMINIT_MAXID   15
#else
#define MEMINIT_MAXID   36
#endif
#define MEMID_SKIP_MIN  7
#define MEMID_SKIP_MAX  8
#endif

#ifdef CONFIG_VTSS_ARCH_SPARX_28
/* ================================================================= *
 * B_ACL registers
 * ================================================================= */
#define R_ACL_ACL_CFG             0x00 /* ACL General Configuration */
#define R_ACL_PAG_CFG             0x01 /* Port to PAG mapping */

#ifdef CONFIG_VTSS_GROCX
#define R_ACL_TCP_RNG_ENA_CFG_0   0x09 /* 0x09-0x18: UDP/TCP range */
#define R_ACL_TCP_RNG_VALUE_CFG_0 0x0A

#define ACE_DATA_OFFS             0x19 /* 0x19-0x50: ACE data */
#define ACE_MASK_OFFS             0x51 /* 0x51-0x88: ACE mask */

/* ACE Ingress and Egress Actions */
#define R_ACL_IN_VLD              0x89
#define R_ACL_IN_MISC_CFG         0x8A
#define R_ACL_IN_REDIR_CFG        0x8B
#define R_ACL_IN_CNT              0x8C
#define R_ACL_EG_VLD              0x8D
#define R_ACL_EG_MISC             0x8E
#define R_ACL_EG_PORT_MASK        0x8F
#define R_ACL_EG_CNT              0x90

/* ACE Control */
#define R_ACL_UPDATE_CTRL         0x91
#define R_ACL_MV_CFG              0x92
#define R_ACL_STATUS              0x93
#define R_ACL_STICKY              0x94

#else

#define R_ACL_TCP_RNG_ENA_CFG_0   0x1D /* 0x1D-0x2B: UDP/TCP range */
#define R_ACL_TCP_RNG_VALUE_CFG_0 0x1E

#define ACE_DATA_OFFS             0x2D /* 0x2D-0x64: ACE data */
#define ACE_MASK_OFFS             0x65 /* 0x65-0x9C: ACE mask */

/* ACE Ingress and Egress Actions */
#define R_ACL_IN_VLD              0x9D
#define R_ACL_IN_MISC_CFG         0x9E
#define R_ACL_IN_REDIR_CFG        0x9F
#define R_ACL_IN_CNT              0xA0
#define R_ACL_EG_VLD              0xA1
#define R_ACL_EG_PORT_MASK        0xA2
#define R_ACL_EG_CNT              0xA3

/* ACE Control */
#define R_ACL_UPDATE_CTRL         0xA5
#define R_ACL_MV_CFG              0xA6
#define R_ACL_STATUS              0xA7
#define R_ACL_STICKY              0xA8

#endif

/* MAC_ETYPE ACE */
#define R_ACL_ETYPE_TYPE          0x00
#define R_ACL_ETYPE_L2_SMAC_HIGH  0x01
#define R_ACL_ETYPE_L2_SMAC_LOW   0x02
#define R_ACL_ETYPE_L2_DMAC_HIGH  0x03
#define R_ACL_ETYPE_L2_DMAC_LOW   0x04
#define R_ACL_ETYPE_L2_ETYPE      0x05

/* MAC_LLC ACE */
#define R_ACL_LLC_TYPE            0x08
#define R_ACL_LLC_L2_SMAC_HIGH    0x09
#define R_ACL_LLC_L2_SMAC_LOW     0x0A
#define R_ACL_LLC_L2_DMAC_HIGH    0x0B
#define R_ACL_LLC_L2_DMAC_LOW     0x0C
#define R_ACL_LLC_L2_LLC          0x0E

/* MAC_SNAP ACE */
#define R_ACL_SNAP_TYPE           0x10
#define R_ACL_SNAP_L2_SMAC_HIGH   0x11
#define R_ACL_SNAP_L2_SMAC_LOW    0x12
#define R_ACL_SNAP_L2_DMAC_HIGH   0x13
#define R_ACL_SNAP_L2_DMAC_LOW    0x14
#define R_ACL_SNAP_L2_SNAP_LOW    0x15
#define R_ACL_SNAP_L2_SNAP_HIGH   0x16

/* ARP ACE */
#define R_ACL_ARP_TYPE            0x18
#define R_ACL_ARP_L2_SMAC_HIGH    0x19
#define R_ACL_ARP_L2_SMAC_LOW     0x1A
#define R_ACL_ARP_L3_ARP          0x1B
#define R_ACL_ARP_L3_IPV4_DIP     0x1C
#define R_ACL_ARP_L3_IPV4_SIP     0x1D

/* IPV4_TCP_UDP ACE */
#define R_ACL_UDP_TCP_TYPE        0x20
#define R_ACL_UDP_TCP_L3_MISC_CFG 0x21
#define R_ACL_UDP_TCP_L3_IPV4_DIP 0x22
#define R_ACL_UDP_TCP_L3_IPV4_SIP 0x23
#define R_ACL_UDP_TCP_L4_PORT     0x24
#define R_ACL_UDP_TCP_L4_MISC     0x25

/* IPV4_OTHER ACE */
#define R_ACL_IPV4_TYPE           0x28
#define R_ACL_IPV4_L3_MISC_CFG    0x29
#define R_ACL_IPV4_L3_IPV4_DIP    0x2A
#define R_ACL_IPV4_L3_IPV4_SIP    0x2B
#define R_ACL_IPV4_DATA_0         0x2C
#define R_ACL_IPV4_DATA_1         0x2D

/* IPV6 ACE */
#define R_ACL_IPV6_TYPE           0x30
#define R_ACL_IPV6_L3_MISC_CFG    0x31
#define R_ACL_IPV6_L3_IPV6_SIP_0  0x32
#define R_ACL_IPV6_L3_IPV6_SIP_1  0x33
#define R_ACL_IPV6_L3_IPV6_SIP_2  0x34
#define R_ACL_IPV6_L3_IPV6_SIP_3  0x35

/* ACL update commands */
#define ACL_CMD_WRITE             0
#define ACL_CMD_READ              1
#define ACL_CMD_MOVE_UP           2
#define ACL_CMD_MOVE_DOWN         3
#define ACL_CMD_INIT              4
#define ACL_CMD_RESET_CNT         5

/* ACL types */
#define ACL_TYPE_ETYPE            0
#define ACL_TYPE_LLC              1
#define ACL_TYPE_SNAP             2
#define ACL_TYPE_ARP              3
#define ACL_TYPE_UDP_TCP          4
#define ACL_TYPE_IPV4             5
#define ACL_TYPE_IPV6             6

/* ACL PAG width */
#ifdef CONFIG_VTSS_GROCX
#define ACL_PAG_WIDTH 6
#else
#define ACL_PAG_WIDTH 8
#endif

#endif

/* ================================================================= *
 * B_CAPTURE registers
 * ================================================================= */
#define R_CAPTURE_FRAME_DATA 0x00 /* Frame Data (Sub Block depends on chip and CPU queue) */

/* Internal Frame Header, IFH0 (bit 63-32) and IFH1 (bit 31-0) */

#define O_IFH_LENGTH   48     /* 61-48: Length */ 
#define M_IFH_LENGTH   0x3fff 

#ifdef CONFIG_VTSS_ARCH_SPARX_28
#define O_IFH_PORT     42     /* 46-42: Port */
#define M_IFH_PORT     0x1f
#endif /* SPARX_28 */

#ifdef CONFIG_VTSS_ARCH_SPARX_28
#define O_IFH_TAGGED   5      /* 5    : Tagged */ 
#define M_IFH_TAGGED   0x1
#endif

#ifdef CONFIG_VTSS_ARCH_SPARX
#define O_IFH_UPRIO    29     /* 31-29: Tag priority */
#define M_IFH_UPRIO    0x7
#define O_IFH_CFI      28     /* 28   : CFI */
#define M_IFH_CFI      0x1
#define O_IFH_VID      16     /* 27-16: VID */
#define M_IFH_VID      0xfff   
#else
#define O_IFH_UPRIO    28     /* 30-28: Tag priority */
#define M_IFH_UPRIO    0x7
#define O_IFH_CFI      27     /* 27   : CFI */
#define M_IFH_CFI      0x1
#define O_IFH_VID      15     /* 26-15: VID */
#define M_IFH_VID      0xfff   
#endif

#ifdef CONFIG_VTSS_ARCH_SPARX_28
#define O_IFH_LEARN    4      /* 4    : Learn */
#define M_IFH_LEARN    0x1
#endif

/* Get field from IFH0/IFH1 */
#define IFH_GET(ifh0, ifh1, field) ((O_IFH_##field > 31 ? (ifh0 >> (O_IFH_##field - 32)) : (ifh1 >> O_IFH_##field)) & M_IFH_##field)

/* Put field to IFH0/IFH1 */
#define IFH_PUT(ifh0, ifh1, field, val) { ifh0 |= (O_IFH_##field > 31 ? ((val)<<(O_IFH_##field - 32)) : 0); ifh1 |= (O_IFH_##field > 31 ? 0 : (val)<<O_IFH_##field); }

/* ================================================================= *
 * B_ARBITER registers
 * ================================================================= */
#define R_ARBITER_ARBEMPTY     0x0C /* Arbiter Empty */
#define R_ARBITER_ARBDISC      0x0E /* Arbiter Discard */

#ifdef CONFIG_VTSS_ARCH_SPARX
#define R_ARBITER_ARBBURSTPROB 0x15 /* Burst Probability */
#endif

#ifdef CONFIG_VTSS_ARCH_SPARX_28
#define R_ARBITER_ARBHOLBPENAB 0x17 /* Head of Line Blocking Protection */
#endif

/* ================================================================= *
 * B_SYSTEM registers
 * ================================================================= */
#define R_SYSTEM_CPUMODE      0x00 /* CPU Transfer Mode */
#define R_SYSTEM_SIPAD        0x01 /* SI Padding */
#define R_SYSTEM_PICONF       0x02 /* PI Config */

#ifdef CONFIG_VTSS_ARCH_SPARX_28
#define R_SYSTEM_MACRO_CTRL   0x08 /* Hardware semaphore */
#endif

#define R_SYSTEM_HWSEM        0x13 /* Hardware semaphore */
#define R_SYSTEM_GLORESET     0x14 /* Global Reset */
#define R_SYSTEM_CHIPID       0x18 /* Chip Identification */
#define R_SYSTEM_TIMECMP      0x24 /* Time Compare Value */
#define R_SYSTEM_SLOWDATA     0x2C /* SlowData */
#define R_SYSTEM_CPUCTRL      0x30 /* CPU/interrupt Control */

#ifdef CONFIG_VTSS_ARCH_SPARX
#define R_SYSTEM_CAPCTRL      0x31 /* Capture Control */
#endif /* SPARX */

#ifdef CONFIG_VTSS_ARCH_SPARX_28
#define R_SYSTEM_GPIOCTRL     0x33 /* GPIO Control */
#endif
#define R_SYSTEM_GPIO         0x34 /* General Purpose IO */

#ifdef CONFIG_VTSS_ARCH_SPARX
#define R_SYSTEM_SIMASTER     0x35 /* SI Master Interface */
#ifdef CONFIG_VTSS_ARCH_SPARX_28
#define R_SYSTEM_RATEUNIT     0x36 /* Policer/shaper rate unit */
#define R_SYSTEM_LEDTIMER     0x3C /* LED timer */
#define R_SYSTEM_LEDMODES     0x3D /* LED modes */
#endif
#define R_SYSTEM_SGMII_TR_DBG 0x08 /* SGMII debug */
#define R_SYSTEM_ICPU_CTRL    0x10 /* Internal CPU Control */
#define R_SYSTEM_ICPU_ADDR    0x11 /* Internal CPU On-Chip RAM Address */
#define R_SYSTEM_ICPU_DATA    0x12 /* Internal CPU On-Chip RAM Data */
#endif

#define VTSS_T_RESET      125000  /* Waiting time (nanoseconds) after reset command */

#endif /* _HEATHROWII_REG_H */
