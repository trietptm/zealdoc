/*

 Vitesse Switch API software.

 Copyright (c) 2002-2008 Vitesse Semiconductor Corporation "Vitesse". All
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
 
 $Id: vtss_sparx_reg.h,v 1.1 2009-01-14 07:49:20 zengjie Exp $
 $Revision: 1.1 $

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
#if defined(VTSS_ARCH_SPARX_28)
#define B_ACL      3  /* ACL */
#if defined(VTSS_FEATURE_VAUI)
#define B_VAUI     3  /* VAUI */
#endif /* VTSS_FEATURE_VAUI */
#endif /* VTSS_ARCH_SPARX_28 */
#define B_CAPTURE  4  /* CPU Capture Buffer (Sub Blocks: 0-3,4,6,7) depending on chip */
#define B_ARBITER  5  /* Arbiter (Sub Block: 0) */
#define B_PORT_HI  6  /* Port 16-31 (Sub Blocks: 0-15) */
#define B_SYSTEM   7  /* System Registers (Sub Block: 0) */

/* Memory initialization sub block */
#if defined(VTSS_ARCH_STANSTED)
#define S_MEMINIT 1
#else
#define S_MEMINIT 2
#endif /* VTSS_ARCH_STANSTED */

#if defined(VTSS_ARCH_SPARX_28)
#define S_ACL  3
#if defined(VTSS_FEATURE_VAUI)
#define S_VAUI 4
#endif /* VTSS_FEATURE_VAUI */
#endif /* VTSS_ARCH_SPARX_28 */

/* Capture sub blocks */
#define S_CAPTURE_DATA     0 /* CPU Capture Buffer Data */

#if defined(VTSS_ARCH_HAWX)
#define S_CAPTURE_STATUS_A 4 /* CPU Capture Buffer Status Sub Block: 4 */
#define S_CAPTURE_STATUS_B 8 /* CPU Capture Buffer Status Sub Block: 8 */
#else
#define S_CAPTURE_STATUS   4 /* CPU Capture Buffer Status Sub Block: 4 */
#endif /* VTSS_ARCH_HAWX */

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

#if defined(VTSS_ARCH_STANSTED) || defined(VTSS_ARCH_STAPLEFORD)
#define R_PORT_SHAPECONF   0x11 /* Shaper Setup */
#define R_PORT_POLICECONF  0x12 /* Policer Setup */
#define R_PORT_MCSTORMCONF 0x13 /* Multicast Storm Setup */
#endif /* VTSS_ARCH_STANSTED/STAPLEFORD */

#if defined(VTSS_ARCH_STAPLEFORD)
#define R_PORT_TBISTAT     0x14 /* TBI Status Register */
#define R_PORT_TBICTRL     0x18 /* TBI Control Register */
#endif /* VTSS_ARCH_STAPLEFORD */

#if defined(VTSS_ARCH_HAWX)
#define R_PORT_POL_CONF    0x12 /* Policer Configuration */
#define R_PORT_POL_LINE    0x13 /* Policer Line Configuration */
#define R_PORT_POL_MCAST   0x14 /* Policer Multicast Configuration */
#define R_PORT_POL_BCAST   0x15 /* Policer Broadcast Configuration */
#define R_PORT_POL_LINECIR 0x16 /* Policer Line CIR Configuration */
#endif /* VTSS_ARCH_HAWX */

#if defined(VTSS_ARCH_HAWX) || defined(VTSS_ARCH_SPARX_G24) || (defined(VTSS_ARCH_SPARX_28) && !defined(G_ROCX))
#define R_PORT_PCSCTRL     0x18 /* PCS Control */
#define R_PORT_SGMII_CFG   0x1A /* SGMII Configuration */
#define R_PORT_PCSSTAT     0x1C /* PCS Status */
#endif /* VTSS_ARCH_HAWX/SPARX_G24/SPARX_28 && !G_ROCX */

#if defined(VTSS_ARCH_HAWX)
#define R_PORT_SGMII_CMSEL 0x1B /* SGMII Input Commonmode Select */
#endif /* VTSS_ARCH_HAWX */

#if defined(VTSS_ARCH_HAWX) || (defined(VTSS_ARCH_SPARX_28) && !defined(G_ROCX))
#define R_PORT_SGMIIOBS    0x1D /* SGMII Observe */
#endif /* VTSS_ARCH_HAWX/SPARX_28 && !G_ROCX */

#if !defined(VTSS_ARCH_SPARX)
#define R_PORT_CFIDROP     0x25 /* CFI Drop Counter */
#endif /* !VTSS_ARCH_SPARX */

#if defined(VTSS_ARCH_HAWX)
#define R_PORT_WS_CONF     0x28 /* Weighting Shaper Configuration */
#define R_PORT_WS_BUCK0    0x29 /* Weighting Shaper Queue-0 */
#define R_PORT_WS_BUCK1    0x2A /* Weighting Shaper Queue-1 */
#define R_PORT_WS_BUCK2    0x2B /* Weighting Shaper Queue-2 */
#define R_PORT_WS_BUCK3    0x2C /* Weighting Shaper Queue-3 */
#define R_PORT_WS_LINE     0x2D /* Weighting Shaper Line */
#define R_PORT_POL_Q0PIR   0x2F /* Policer Queue-0 PIR */
#define R_PORT_POL_Q1PIR   0x30 /* Policer Queue-1 PIR */
#define R_PORT_POL_Q2PIR   0x31 /* Policer Queue-2 PIR */
#define R_PORT_POL_Q3PIR   0x32 /* Policer Queue-3 PIR */
#define R_PORT_POL_Q0CIR   0x33 /* Policer Queue-0 CIR */
#define R_PORT_POL_Q1CIR   0x34 /* Policer Queue-1 CIR */
#define R_PORT_POL_Q2CIR   0x35 /* Policer Queue-2 CIR */
#define R_PORT_POL_Q3CIR   0x36 /* Policer Queue-3 CIR */
#endif /* VTSS_ARCH_HAWX */

#if defined(VTSS_ARCH_SPARX_G24)
#define R_PORT_RATECONF    0x28 /* Rate Configuration */
#endif /* VTSS_ARCH_SPARX_G24 */

#if defined(VTSS_ARCH_SPARX_28)
#define R_PORT_STACKCFG    0x25 /* Stack configuration */
#define R_PORT_SHAPER_CFG  0x28 /* Shaper Setup */
#define R_PORT_POLICER_CFG 0x29 /* Policer Setup */
#endif /* VTSS_ARCH_SPARX_28 */

/* ================================================================= *
 * B_PORT::Shared FIFO registers
 * ================================================================= */
#define R_PORT_CPUTXDAT    0xC0 /* CPU Transmit DATA */
#define R_PORT_MISCFIFO    0xC4 /* Misc Control Register */
#define R_PORT_MISCSTAT    0xC8 /* Misc Status */
#if !defined(VTSS_ARCH_SPARX)
#define R_PORT_POOLCFG     0xCC /* Pool Control Register */
#endif /* VTSS_ARCH_SPARX */

#if defined(VTSS_ARCH_SPARX_28)
#define R_PORT_MISCCFG     0xC5 /* Miscellaneous Configuration */
#define R_PORT_CPUTXHDR    0xCC /* CPU Transmit HEADER */
#endif /* VTSS_ARCH_SPARX_28 */

#if defined(VTSS_ARCH_STANSTED) || defined(VTSS_ARCH_STAPLEFORD)
#define R_PORT_DROPCNT     0xD4 /* Drop Counters */
#endif /* VTSS_ARCH_STANSTED/STAPLEFORD */

#define R_PORT_FREEPOOL    0xD8 /* Free RAM Counter */

#if defined(VTSS_ARCH_STANSTED) || defined(VTSS_ARCH_STAPLEFORD)
#define R_PORT_DROPCFG     0xDC /* Drop Control Register */
#define R_PORT_DBQHPRX     0xE8 /* Debug High Priority RX queue pointers (snapshot) */
#define R_PORT_DBQLPRX     0xEC /* Debug Low Priority RX queue pointers (snapshot) */
#define R_PORT_DBQHPTX     0xF0 /* Debug High Priority TX queue pointers (snapshot) */
#define R_PORT_DBQLPTX     0xF4 /* Debug Low Priority TX queue pointers (snapshot) */
#endif /* VTSS_ARCH_STANSTED/STAPLEFORD */

#if defined(VTSS_ARCH_HAWX) || defined(VTSS_ARCH_SPARX)
#if !defined(VTSS_ARCH_SPARX_28)
#define R_PORT_Q_FLOWC_WM  0xDE /* Flow Control Watermarks */
#endif /* VTSS_ARCH_SPARX_28 */
#define R_PORT_Q_MISC_CONF 0xDF /* Misc Pool Control */
#endif /* VTSS_ARCH_HAWX/SPARX */

#if defined(VTSS_ARCH_HAWX)
#define R_PORT_Q_DROP_WM   0xE0 /* Drop Watermarks */
#endif /* HAWX */

#if defined(VTSS_ARCH_SPARX_G8)
#define R_PORT_Q_DROP_WM_0 0xE0 /* Drop Watermarks */
#define R_PORT_Q_DROP_WM_1 0xE1 /* Drop Watermarks */
#define R_PORT_Q_DROP_WM_2 0xE2 /* Drop Watermarks */
#define R_PORT_Q_DROP_WM_3 0xE3 /* Drop Watermarks */
#endif /* VTSS_ARCH_SPARX_G8 */

#if defined(VTSS_ARCH_SPARX_G24) || defined(VTSS_ARCH_SPARX_28)
#define R_PORT_Q_EGRESS_WM 0xE0 /* Egress Max Watermarks */
#endif /* VTSS_ARCH_SPARX_G24/SPARX_28 */

#if defined(VTSS_ARCH_SPARX_28)
#define R_PORT_Q_INGRESS_WM 0xE8 /* 0xE8-0xED: Ingress Watermarks */ 
#endif /* VTSS_ARCH_SPARX_28 */

/* ================================================================= *
 * B_PORT::Categorizer registers
 * ================================================================= */
#if defined(VTSS_ARCH_STANSTED) || defined(VTSS_ARCH_STAPLEFORD)
#define R_PORT_CATCONF      0x60 /* Categorizer Config */
#define R_PORT_CATTAG       0x64 /* Categorizer Map Tag */

#if defined(VTSS_ARCH_STANSTED)
#define R_PORT_CATSUBTAG    0x65 /* Categorizer SubMap Tag */
#endif /* VTSS_ARCH_STANSTED */

#define R_PORT_CATETHT      0x68 /* EtherType Register */
#define R_PORT_CATDSAP      0x6C /* DSAP Register */
#define R_PORT_CATIPPRT     0x70 /* IP Protocol Register */
#define R_PORT_CATPRIO      0x74 /* Categorizer Priorities */

#if defined(VTSS_ARCH_STANSTED)
#define R_PORT_CATSUBPRIO   0x75 /* Categorizer SubPriorities */
#endif /* VTSS_ARCH_STANSTED */

#define R_PORT_CATDSMAPH    0x78 /* DS Mapping Register - high part:63..32 */
#define R_PORT_CATDSMAPL    0x79 /* DS Mapping Register - low part: 31..00 */

#if defined(VTSS_ARCH_STANSTED)
#define R_PORT_CATDSSUBMAPH 0x7A /* DS SubMapping Register - high part:63..32 */
#define R_PORT_CATDSSUBMAPL 0x7B /* DS SubMapping Register - low part: 31..00 */
#endif /* VTSS_ARCH_STANSTED */

#define R_PORT_CATPVID      0x7C /* PVID Register */
#define R_PORT_CATPORT1     0x80 /* IP Port Register 1 */
#define R_PORT_CATPORT2     0x84 /* IP Port Register 2 */
#define R_PORT_CATPORT3     0x88 /* IP Port Register 3 */
#define R_PORT_CATPORT4     0x8C /* IP Port Register 4 */
#define R_PORT_CATPORT5     0x90 /* IP Port Register 5 */
#endif /* VTSS_ARCH_STANSTED/STAPLEFORD */

#if defined(VTSS_ARCH_HAWX) || defined(VTSS_ARCH_SPARX)

#if !defined(VTSS_ARCH_SPARX_28)
#define R_PORT_CAT_PR_DSCP_QOS     0x60 /* Categorizer DSCP QoS */
#endif /* VTSS_ARCH_SPARX_28 */

#if defined(VTSS_ARCH_SPARX)
#define R_PORT_CAT_PR_DSCP_VAL_0_3 0x61 /* Categorizer DSCP Values 0-3 */
#endif /* VTSS_ARCH_SPARX */
#if defined(VTSS_ARCH_SPARX_G8) || defined(VTSS_ARCH_SPARX_G24)
#define R_PORT_CAT_PR_DSCP_VAL_4_6 0x62 /* Categorizer DSCP Values 4-6 */
#endif /* VTSS_ARCH_SPARX_G8/G24 */
#if defined(VTSS_ARCH_SPARX_G24)
#define R_PORT_SIPCONF             0x63 /* Source IP Filter Config */
#define R_PORT_SIPADDR             0x64 /* Source IP Filter Address */
#endif /* VTSS_ARCH_SPARX_G24 */

#if defined(VTSS_ARCH_HAWX)
#define R_PORT_CAT_FA_TAG_ANALYSIS 0x68 /* Categorizer Tag Analysis */
#define R_PORT_CAT_CPU_8021D_DMAC  0x69 /* Categorizer 802.1D Capture Enable */
#define R_PORT_CAT_CPU_MISC        0x6D /* Categorizer CPU Misc */
#endif /* VTSS_ARCH_HAWX */

#define R_PORT_CAT_DROP            0x6E /* Categorizer Frame Dropping */
#define R_PORT_CAT_PR_MISC_L2      0x6F /* Categorizer Misc L2 QoS */

#if defined(VTSS_ARCH_HAWX)
#define R_PORT_CAT_PR_TCPUDP_PORTS 0x70 /* 0x70-0x74: Categorizer TCP/UDP Ports */
#endif /* VTSS_ARCH HAWX */

#if !defined(VTSS_ARCH_SPARX_28)
#define R_PORT_CAT_PR_USR_PRIO     0x75 /* Categorizer User Priority */
#endif /* VTSS_ARCH_SPARX_28 */

#if defined(VTSS_ARCH_HAWX)
#define R_PORT_CAT_PR_ETYPE        0x76 /* Categorizer Ethernet Type QoS */
#endif /* VTSS_ARCH_HAWX */

#if !defined(VTSS_ARCH_SPARX_28)
#define R_PORT_CAT_PR_MISC_L3      0x77 /* Categorizer Misc L3 QoS */
#endif /* VTSS_ARCH_SPARX_28 */

#if defined(VTSS_ARCH_HAWX)
#define R_PORT_CAT_PR_TCPUDP_QOS   0x78 /* Categorizer TCP/UDP QoS */
#endif /* VTSS_ARCH_HAWX */

#define R_PORT_CAT_VLAN_MISC       0x79 /* Categorizer VLAN Misc */

#if defined(VTSS_ARCH_HAWX)
#define R_PORT_CAT_VLAN_AB         0x7A /* Categorizer VLAN A+B */
#endif /* VTSS_ARCH_HAWX */

#if defined(VTSS_ARCH_SPARX)
#define R_PORT_CAT_PORT_VLAN       0x7A /* Categorizer Port VLAN */
#endif /* VTSS_ARCH_SPARX */

#define R_PORT_CAT_OTHER_CFG       0x7B /* Categorizer Other Configuration */
#define R_PORT_CAT_GEN_PRIO_REMAP  0x7D /* Categorizer Generic Priority Remap */

#if defined(VTSS_ARCH_HAWX)
#define R_PORT_CAT_CPU_QUEUE       0x7E /* Categorizer CPU Queue */
#endif /* VTSS_ARCH_HAWX */

#if defined(VTSS_ARCH_SPARX_28)
#define R_PORT_CAT_QCE0            0x80 /* 0x80-0x8B: QCE 0-11 */
#define R_PORT_CAT_QCL_RANGE_CFG   0x8C /* QCL Range Configuration */
#define R_PORT_CAT_QCL_DEFAULT_CFG 0x8D /* QCL Default Configuration */
#endif /* VTSS_ARCH_SPARX_28 */

#endif /* VTSS_ARCH_HAWX/SPARX */

/* ================================================================= *
 * B_PORT::Counter registers
 * ================================================================= */
#if defined(VTSS_ARCH_SPARX_28)
#define R_PORT_C_CNTDATA    0xDC /* Counter Data */
#define R_PORT_C_CNTADDR    0xDD /* Counter Address */
#define R_PORT_C_CLEAR      0x52 /* Clear Counters */
#else
#if defined(VTSS_ARCH_STANSTED) || defined(VTSS_ARCH_STAPLEFORD)
#define R_PORT_RXBADPKT     0x37 /* Rx Total Bad Packets */
#define R_PORT_RXCTRL       0x39 /* Rx Control Packets */
#if defined(VTSS_ARCH_STANSTED)
#define R_PORT_C_RXPAUSE    0x3D /* Rx Pause Frames */
#define R_PORT_C_TXPAUSE    0x3E /* Tx Pause Frames */
#define R_PORT_C_RXCATDROP  0x3F /* Rx Classified Drops */
#endif /* VTSS_ARCH_STANSTED */
#define R_PORT_TXERR        0x44 /* Tx Total Error Packets */
#endif /* VTSS_ARCH_STANSTED/STAPLEFORD */

#if defined(VTSS_ARCH_HAWX)
#define R_PORT_RXCTRL       0x3F /* Rx Control Packets */
#define R_PORT_TXERR        0x40 /* Tx Total Error Packets */
#define R_PORT_C_RXJUMBO    0x41 /* Rx 1527-MAX bytes */
#define R_PORT_C_TXJUMBO    0x42 /* Tx 1527-MAX bytes */
#endif /* VTSS_ARCH_HAWX */

#define R_PORT_C_RXOCT      0x50 /* Rx Octets */
#define R_PORT_C_TXOCT      0x51 /* Tx Octets */

#if defined(VTSS_ARCH_SPARX_G8)
#define R_PORT_C_RX0        0x52 /* Rx Counter 0 */
#define R_PORT_C_RX1        0x53 /* Rx Counter 1 */
#define R_PORT_C_RX2        0x54 /* Rx Counter 2 */
#define R_PORT_C_TX0        0x55 /* Tx Counter 0 */
#define R_PORT_C_TX1        0x56 /* Tx Counter 1 */
#define R_PORT_C_TX2        0x57 /* Tx Counter 2 */
#define R_PORT_CNT_CTRL_CFG 0x58 /* Counter Control Configuration */
#endif /* VTSS_ARCH_SPARX_G8 */

#if defined(VTSS_ARCH_SPARX_G24)
#define R_PORT_C_RX0        0x52 /* Rx Counter 0 */
#define R_PORT_C_RX1        0x53 /* Rx Counter 1 */
#define R_PORT_C_RX2        0x54 /* Rx Counter 2 */
#define R_PORT_C_RX3        0x55 /* Rx Counter 3 */
#define R_PORT_C_RX4        0x56 /* Rx Counter 4 */
#define R_PORT_C_TX0        0x57 /* Tx Counter 0 */
#define R_PORT_C_TX1        0x58 /* Tx Counter 1 */
#define R_PORT_C_TX2        0x59 /* Tx Counter 2 */
#define R_PORT_C_TX3        0x5A /* Tx Counter 3 */
#define R_PORT_C_TX4        0x5B /* Tx Counter 4 */
#define R_PORT_CNT_CTRL_CFG 0x5C /* Counter Control Configuration */
#define R_PORT_CNT_CTRL_CFG2 0x5D /* Counter Control Configuration Extra */
#endif /* VTSS_ARCH_SPARX_G24 */

#if defined(VTSS_ARCH_STANSTED) || defined(VTSS_ARCH_STAPLEFORD) || defined(VTSS_ARCH_HAWX)
#define R_PORT_C_RXDROP     0xA0 /* Rx Drops */
#define R_PORT_C_RXPKT      0xA1 /* Rx Packets */
#define R_PORT_C_RXBC       0xA2 /* Rx Broadcasts */
#define R_PORT_C_RXMC       0xA3 /* Rx Multicasts */
#define R_PORT_C_RXCRC      0xA4 /* Rx CRC/ALIGN */
#define R_PORT_C_RXSHT      0xA5 /* Rx Undersize */
#define R_PORT_C_RXLONG     0xA6 /* Rx Oversize */
#define R_PORT_C_RXFRAG     0xA7 /* Rx Fragments */
#define R_PORT_C_RXJAB      0xA8 /* Rx Jabbers */
#define R_PORT_C_RX64       0xA9 /* Rx 64 bytes */
#define R_PORT_C_RX65       0xAA /* Rx 65-127 bytes */
#define R_PORT_C_RX128      0xAB /* Rx 128-255 bytes */
#define R_PORT_C_RX256      0xAC /* Rx 256-511 bytes */
#define R_PORT_C_RX512      0xAD /* Rx 512-1023 bytes */
#define R_PORT_C_RX1024     0xAE /* Rx 1024-long bytes (HawX: Rx 1024-1526 bytes) */
#define R_PORT_C_TXDROP     0xAF /* Tx Drops */
#define R_PORT_C_TXPKT      0xB0 /* Tx Packets */
#define R_PORT_C_TXBC       0xB1 /* Tx Broadcasts */
#define R_PORT_C_TXMC       0xB2 /* Tx Multicasts */
#define R_PORT_C_TXCOL      0xB3 /* Tx Collisions */
#define R_PORT_C_TX64       0xB4 /* Tx 64 bytes */
#define R_PORT_C_TX65       0xB5 /* Tx 65-127 bytes */
#define R_PORT_C_TX128      0xB6 /* Tx 128-255 bytes */
#define R_PORT_C_TX256      0xB7 /* Tx 256-511 bytes */
#define R_PORT_C_TX512      0xB8 /* Tx 512-1023 bytes */
#define R_PORT_C_TX1024     0xB9 /* Tx 1024-long bytes (HawX: Tx 1024-1526 bytes) */
#define R_PORT_C_TXOVFL     0xBA /* Tx FIFO Drops */
#endif /* VTSS_ARCH_STANSTED/STAPLEFORD/HAWX */

#if defined(VTSS_ARCH_STANSTED) || defined(VTSS_ARCH_STAPLEFORD)
#define R_PORT_C_RXHP       0xBB /* Rx High Priority Frames */
#define R_PORT_C_RXLP       0xBC /* Rx Low Priority Frames */
#define R_PORT_C_TXHP       0xBD /* Tx High Priority Frames */
#define R_PORT_C_TXLP       0xBE /* Tx Low Priority Frames */
#endif /* VTSS_ARCH_STANSTED/STAPLEFORD */

#if defined(VTSS_ARCH_HAWX)
#define R_PORT_C_RXPAUSE    0xBB /* Rx Pause Frames */
#define R_PORT_C_TXPAUSE    0xBC /* Tx Pause Frames */
#define R_PORT_C_RXCATDROP  0xBD /* Rx Classified Drops */
#define R_PORT_C_RXBWDROP   0xBE /* Rx Backward Drops */
#define R_PORT_RXBADPKT     0xBF /* Rx Total Bad Packets */
#define R_PORT_C_RXCLASS0   0xE8 /* Rx Class-0 Packets */
#define R_PORT_C_RXCLASS1   0xE9 /* Rx Class-1 Packets */
#define R_PORT_C_RXCLASS2   0xEA /* Rx Class-2 Packets */
#define R_PORT_C_RXCLASS3   0xEB /* Rx Class-3 Packets */
#define R_PORT_C_RXCLASS4   0xEC /* Rx Class-4 Packets */
#define R_PORT_C_RXCLASS5   0xED /* Rx Class-5 Packets */
#define R_PORT_C_RXCLASS6   0xEE /* Rx Class-6 Packets */
#define R_PORT_C_RXCLASS7   0xEF /* Rx Class-7 Packets */
#define R_PORT_C_TXCLASS0   0xF0 /* Tx Class-0 Packets */
#define R_PORT_C_TXCLASS1   0xF1 /* Tx Class-1 Packets */
#define R_PORT_C_TXCLASS2   0xF2 /* Tx Class-2 Packets */
#define R_PORT_C_TXCLASS3   0xF3 /* Tx Class-3 Packets */
#define R_PORT_C_TXCLASS4   0xF4 /* Tx Class-4 Packets */
#define R_PORT_C_TXCLASS5   0xF5 /* Tx Class-5 Packets */
#define R_PORT_C_TXCLASS6   0xF6 /* Tx Class-6 Packets */
#define R_PORT_C_TXCLASS7   0xF7 /* Tx Class-7 Packets */
#define R_PORT_C_RXPOLDROP0 0xF8 /* Rx Class-0 PolDrops */
#define R_PORT_C_RXPOLDROP1 0xF9 /* Rx Class-1 PolDrops */
#define R_PORT_C_RXPOLDROP2 0xFA /* Rx Class-2 PolDrops */
#define R_PORT_C_RXPOLDROP3 0xFB /* Rx Class-3 PolDrops */
#endif /* VTSS_ARCH_HAWX */
#endif /* VTSS_ARCH_SPARX_28 */

/* ================================================================= *
 * B_ANALYZER registers
 * ================================================================= */
#if defined(VTSS_ARCH_SPARX_G8)
#define R_ANALYZER_STORMLIMIT 0x02 /* Flooding storm control */
#endif /* VTSS_ARCH_SPARX_G8 */

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

#if defined(VTSS_ARCH_SPARX)
#define R_ANALYZER_CAPENAB      0xA0 /* Capture Enable */
#endif /* VTSS_ARCH_SPARX */

#if defined(SPARX_G5)
#define R_ANALYZER_I6FLODMSK    0x11 /* IPv6 Multicast Flood Mask */
#endif /* SPARX_G5 */

#if defined(VTSS_ARCH_SPARX_G24) || defined(VTSS_ARCH_SPARX_28)
#define R_ANALYZER_CAPQUEUE     0xA1 /* Capture Queue */
#define R_ANALYZER_LEARNDROP    0xA2 /* Learning Drop */
#define R_ANALYZER_LEARNAUTO    0xA3 /* Learning Auto */
#define R_ANALYZER_LEARNCPU     0xA4 /* Learning CPU */
#define R_ANALYZER_STORMLIMIT   0xAA /* Storm Control */
#define R_ANALYZER_STORMLIMIT_ENA 0xAB  /* Storm Control Enable */
#endif /* VTSS_ARCH_SPARX_G24/SPARX_28 */
#if defined(VTSS_ARCH_SPARX_G24)
#define R_ANALYZER_ARP_IP0      0xA5 /* ARP IP0 */
#define R_ANALYZER_ARP_IP1      0xA6 /* ARP IP1 */
#define R_ANALYZER_TCPUDP_PORTS 0xA7 /* TCP/UDP Port Numbers */
#endif /* VTSS_ARCH_SPARX_G24 */

#if defined(VTSS_ARCH_SPARX_28)
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
#endif /* VTSS_ARCH_SPARX_28 */

#if defined(VTSS_ARCH_HAWX)
#define R_ANALYZER_LEARNDROP    0xA0 /* Learning Drop */
#define R_ANALYZER_LEARNAUTO    0xA1 /* Learning Auto */
#define R_ANALYZER_LEARNCPU     0xA2 /* Learning CPU */
#endif /* VTSS_ARCH_HAWX */

#define R_ANALYZER_MACACCES     0xB0 /* Mac Table Command */

#if defined(VTSS_ARCH_HAWX) || defined(VTSS_ARCH_SPARX_G8)
#define R_ANALYZER_IPMCACCESS   0xB1 /* IPMC Access */
#endif /* VTSS_ARCH_HAWX/SPARX_G8 */

#define R_ANALYZER_MACTINDX     0xC0 /* Mac Table Index */
#define R_ANALYZER_VLANACES     0xD0 /* VLAN Table Command */
#define R_ANALYZER_VLANTINDX    0xE0 /* VLAN Table Index */
#define R_ANALYZER_AGENCNTL     0xF0 /* Analyzer Config Register */

/* Commands for Mac Table Command register */
#define MAC_CMD_IDLE        0  /* Idle */
#define MAC_CMD_LEARN       1  /* Insert (Learn) 1 entry */
#define MAC_CMD_FORGET      2  /* Delete (Forget) 1 entry */
#define MAC_CMD_TABLE_AGE   3  /* Age entire table */
#if defined(VTSS_ARCH_HAWX) || defined(VTSS_ARCH_SPARX_28)
#define MAC_CMD_GET_NEXT    4  /* Get next entry */
#else
#define MAC_CMD_TABLE_FLUSH 4  /* Delete all non-locked entries in table */
#endif /* VTSS_ARCH_HAWX/SPARX_28 */
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
#if defined(VTSS_ARCH_HAWX) || defined(VTSS_ARCH_SPARX)
#define AGGRCNTL_MODE_SMAC_DMAC_IP 0   /* SMAC xor DMAC xor IP-info */
#define AGGRCNTL_MODE_PSEUDORANDOM 4   /* Pseudo randomized */
#define AGGRCNTL_MODE_IP           5   /* IP-info only */
#endif /* VTSS_ARCH_HAWX/SPARX */

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
#if defined(VTSS_ARCH_SPARX_28)
#define MEMINIT_CMD_INIT  0x10101 /* Initialize Memory */
#else
#define MEMINIT_CMD_INIT  0x10104 /* Initialize Memory */
#endif /* VTSS_ARCH_SPARX_28 */
#define MEMINIT_CMD_READ  0x00200 /* Read Result */

/* Results for Memory Initialization Result register */
#define MEMRES_OK         0x3  /* Result OK */

/* Maximum Memory ID and skipped memory IDs */
#if defined(ELSTREE)
#define MEMINIT_MAXID   15     
#define MEMID_SKIP_MIN  6
#define MEMID_SKIP_MAX  7
#endif /* ELSTREE */
#if defined(STANSTED)
#define MEMINIT_MAXID   19
#define MEMID_SKIP_MIN  6
#define MEMID_SKIP_MAX  7
#endif /* STANSTED */
#if defined(HEATHROW3)
#define MEMINIT_MAXID   25
#define MEMID_SKIP_MIN  6
#define MEMID_SKIP_MAX  9
#endif /* HEATHROW3 */
#if defined(STAPLEFORD)
#define MEMINIT_MAXID   29
#define MEMID_SKIP_MIN  (MEMINIT_MAXID+1) /* None skipped */
#define MEMID_SKIP_MAX  (MEMINIT_MAXID+1) /* None skipped */
#endif /* STAPLEFORD */
#if defined(VTSS_ARCH_HAWX)
#define MEMINIT_MAXID   33
#define MEMID_SKIP_MIN  6
#define MEMID_SKIP_MAX  7
#endif /* VTSS_ARCH_HAWX */
#if defined(VTSS_ARCH_SPARX_G8)
#define MEMINIT_MAXID   15
#define MEMID_SKIP_MIN  6
#define MEMID_SKIP_MAX  7
#endif /* VTSS_ARCH_SPARX_G8 */
#if defined(VTSS_ARCH_SPARX_G24)
#define MEMINIT_MAXID   32
#define MEMID_SKIP_MIN  1
#define MEMID_SKIP_MAX  2
#endif /* VTSS_ARCH_SPARX_G24 */
#if defined(VTSS_ARCH_SPARX_28)
#if defined(G_ROCX)
#define MEMINIT_MAXID   15
#else
#define MEMINIT_MAXID   36
#endif /* G_ROCX */
#define MEMID_SKIP_MIN  7
#define MEMID_SKIP_MAX  8
#endif /* VTSS_ARCH_SPARX_28 */

#if defined(VTSS_ARCH_SPARX_28)
/* ================================================================= *
 * B_ACL registers
 * ================================================================= */
#define R_ACL_ACL_CFG             0x00 /* ACL General Configuration */
#define R_ACL_PAG_CFG             0x01 /* Port to PAG mapping */

#if defined(G_ROCX)
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

#endif /* G_ROCX */

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
#if defined(G_ROCX)
#define ACL_PAG_WIDTH 6
#else
#define ACL_PAG_WIDTH 8
#endif /* G_ROCX */

/* ================================================================= *
 * B_VAUI registers
 * ================================================================= */

#if defined(VTSS_FEATURE_VAUI)
#define R_VAUI_LCPLL_CFG             0x00
#define R_VAUI_LCPLL_TEST_CFG        0x01
#define R_VAUI_LCPLL_STATUS          0x02
#define R_VAUI_RCOMP_CFG             0x03
#define R_VAUI_LANE_CFG              0x04
#define R_VAUI_LANE_TEST_CFG         0x05
#define R_VAUI_IB_CFG                0x06
#define R_VAUI_OB_CFG                0x07
#define R_VAUI_LANE_STATUS           0x14
#define R_VAUI_IB_STATUS             0x15
#define R_VAUI_LANE_TESTPATTERN_CFG  0x1C
#define R_VAUI_SIGDET_CFG            0x1D
#define R_VAUI_SIGDET_TEST           0x1E
#define R_VAUI_ANEG_CFG              0x1F
#define R_VAUI_ANEG_TEST             0x20
#define R_VAUI_ANEG_ADV_ABILITY_0    0x21
#define R_VAUI_ANEG_ADV_ABILITY_1    0x22
#define R_VAUI_ANEG_NEXT_PAGE_0      0x23
#define R_VAUI_ANEG_NEXT_PAGE_1      0x24
#define R_VAUI_ANEG_LP_ADV_ABILITY_0 0x37
#define R_VAUI_ANEG_LP_ADV_ABILITY_1 0x38
#define R_VAUI_ANEG_STATUS           0x39
#define R_VAUI_ANEG_DEBUG            0x3A
#endif /* VTSS_FEATURE_VAUI */

#endif /* VTSS_ARCH_SPARX_28 */

/* ================================================================= *
 * B_CAPTURE registers
 * ================================================================= */
#define R_CAPTURE_FRAME_DATA 0x00 /* Frame Data (Sub Block depends on chip and CPU queue) */

#if !defined(VTSS_ARCH_SPARX_28)
#define R_CAPTURE_CAPREADP   0x00 /* Read Pointer (Sub Block 4) */
#define R_CAPTURE_CAPWRP     0x03 /* Write Pointer (Sub Block 4) */
#if defined(VTSS_ARCH_SPARX_G24)
#define R_CAPTURE_CAPDROPCNT 0x04 /* Drop Counter (Sub Block 4) */
#endif /* VTSS_ARCH_SPARX_G24 */
#define R_CAPTURE_CAPRST     0xFF /* Full Reset (Sub Block 7) */
#endif /* VTSS_ARCH_SPARX_28 */

/* Internal Frame Header, IFH0 (bit 63-32) and IFH1 (bit 31-0) */

#define O_IFH_LENGTH   48     /* 61-48: Length */ 
#define M_IFH_LENGTH   0x3fff 

#if defined(VTSS_ARCH_STANSTED) || defined(VTSS_ARCH_STAPLEFORD) || defined(VTSS_ARCH_HAWX)
#define O_IFH_PORT     32     /* 36-32: Port */
#define M_IFH_PORT     0x1f
#endif /* VTSS_ARCH_STANSTED/STAPLEFORD/HAWX */
#if defined(VTSS_ARCH_SPARX_G24) || defined(VTSS_ARCH_SPARX_28)
#define O_IFH_PORT     42     /* 46-42: Port */
#define M_IFH_PORT     0x1f
#endif /* VTSS_ARCH_SPARX_G24/SPARX_28 */
#if defined(VTSS_ARCH_SPARX_G8)
#define O_IFH_PORT     32     /* 34-32: Port */
#define M_IFH_PORT     0x7
#endif /* VTSS_ARCH_SPARX_G8 */

#if defined(VTSS_ARCH_STANSTED) || defined(VTSS_ARCH_STAPLEFORD)
#define O_IFH_TAGGED   12     /* 12   : Tagged */
#define M_IFH_TAGGED   0x1
#endif /* VTSS_ARCH_STANSTED/STAPLEFORD */
#if defined(VTSS_ARCH_HAWX) || defined(VTSS_ARCH_SPARX_G8) || defined(VTSS_ARCH_SPARX_G24)
#define O_IFH_TAGGED   63     /* 63   : Tagged */ 
#define M_IFH_TAGGED   0x1
#endif /* VTSS_ARCH_HAWX/SPARX_G8/SPARX_G24 */
#if defined(VTSS_ARCH_SPARX_28)
#define O_IFH_TAGGED   5      /* 5    : Tagged */ 
#define M_IFH_TAGGED   0x1
#endif /* VTSS_ARCH_SPARX_28 */

#if defined(VTSS_ARCH_SPARX)
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
#endif /* VTSS_ARCH_SPARX */

#if defined(VTSS_ARCH_HAWX)
#define O_IFH_LEARN    31     /* 31   : Learn */
#define M_IFH_LEARN    0x1
#endif /* VTSS_ARCH_HAWX */
#if defined(VTSS_ARCH_STANSTED) || defined(VTSS_ARCH_STAPLEFORD) || defined(VTSS_ARCH_SPARX_G8) || defined(VTSS_ARCH_SPARX_G24)
#define O_IFH_LEARN    13     /* 13   : Learn */
#define M_IFH_LEARN    0x1
#endif /* VTSS_ARCH_STANSTED/STAPLEFORD/SPARX_G8/SPARX_G24 */
#if defined(VTSS_ARCH_SPARX_28)
#define O_IFH_LEARN    4      /* 4    : Learn */
#define M_IFH_LEARN    0x1
#endif /* VTSS_ARCH_SPARX_28 */

#if defined(VTSS_ARCH_STANSTED) || defined(VTSS_ARCH_STAPLEFORD)
#define O_IFH_COOKIE   0      /* 10-0 : Cookie */
#define M_IFH_COOKIE   0x7ff 
#endif /* VTSS_ARCH_STANSTED/STAPLEFORD */
#if defined(VTSS_ARCH_HAWX)
#define O_IFH_COOKIE_A 4      /* 12-4 : Cookie, HawX-A */
#define M_IFH_COOKIE_A 0x1ff 
#define O_IFH_COOKIE_B 5      /* 12-5 : Cookie, HawX-B */
#define M_IFH_COOKIE_B 0xff 
#endif /* VTSS_ARCH_HAWX */
#if defined(VTSS_ARCH_SPARX_G8) || defined(VTSS_ARCH_SPARX_G24)
#define O_IFH_COOKIE   4      /* 12-4 : Cookie */
#define M_IFH_COOKIE   0x1ff 
#endif /* VTSS_ARCH_SPARX_G8/G24 */

/* Get field from IFH0/IFH1 */
#define IFH_GET(ifh0, ifh1, field) ((O_IFH_##field > 31 ? (ifh0 >> (O_IFH_##field - 32)) : (ifh1 >> O_IFH_##field)) & M_IFH_##field)

/* Put field to IFH0/IFH1 */
#define IFH_PUT(ifh0, ifh1, field, val) { ifh0 |= (O_IFH_##field > 31 ? ((val)<<(O_IFH_##field - 32)) : 0); ifh1 |= (O_IFH_##field > 31 ? 0 : (val)<<O_IFH_##field); }

#if defined(VTSS_ARCH_STANSTED) || defined(VTSS_ARCH_STAPLEFORD)
#define CAP_COOKIE 0x254 /* The cookie in the IFH */
#endif /* VTSS_ARCH_STANSTED/STAPLEFORD */

#if defined(VTSS_ARCH_HAWX) || defined(VTSS_ARCH_SPARX_G8) || defined(VTSS_ARCH_SPARX_G24)
#define CAP_COOKIE 0x052 /* The cookie in the IFH */
#endif /* VTSS_ARCH_HAWX/SPARX_G8/SPARX_G24 */

#if defined(VTSS_FEATURE_VSTAX)

#define O_IFH_VSTAX   63  /* 63: IFH VStaX header present flag */
#define M_IFH_VSTAX   0x1
#define O_IFH_STACK_B 14  /* 14: Send to stack link B */
#define M_IFH_STACK_B 0x1
#define O_IFH_STACK_A 13  /* 13: Send to stack link A */
#define M_IFH_STACK_A 0x1

#define ETYPE_VTSS      0x8880 /* Vitesse Ethernet Type */
#define EPID_VSTAX      0x8000 /* Vitesse Ethernet Protocol ID for VStaX */

/* VStaX header format */
#define O_VSH_ETYPE     80      /* 95-80: Ethernet Type */
#define M_VSH_ETYPE     0xffff
#define O_VSH_EPID      64      /* 79-64: Ethernet Protocol ID */
#define M_VSH_EPID      0xffff
#define O_VSH_FORMAT    63      /* 63: Format (0) */
#define M_VSH_FORMAT    0x1
#define O_VSH_DP        61      /* 66-61: Drop Precedence */
#define M_VSH_DP        0x3
#define O_VSH_SPRIO     59      /* 59: Super Priority */
#define M_VSH_SPRIO     0x1
#define O_VSH_IPRIO     56      /* 58-56: Internal priority */
#define M_VSH_IPRIO     0x7
#define O_VSH_IDM       55      /* 55: Ingress Drop Mode */
#define M_VSH_IDM       0x1
#define O_VSH_DMAC_UNKN 53      /* 53: DMAC Unknown */
#define M_VSH_DMAC_UNKN 0x1
#define O_VSH_TTL       48      /* 52-48: TTL */
#define M_VSH_TTL       0x1f
#define O_VSH_LRN_SKIP  47      /* 47: Learning Skip */
#define M_VSH_LRN_SKIP  0x1
#define O_VSH_FWD_MODE  44      /* 46-44: Forward Mode */
#define M_VSH_FWD_MODE  0x7
#define O_VSH_DST_UID   37      /* 41-37: Destination Unit ID */
#define M_VSH_DST_UID   0x1f
#define O_VSH_DST_PORT  32      /* 36-32: Destination Port/Queue */
#define M_VSH_DST_PORT  0x1f
#define O_VSH_UPRIO     29      /* 31-29: User Priority */
#define M_VSH_UPRIO     0x7
#define O_VSH_CFI       28      /* 28: CFI */
#define M_VSH_CFI       0x1
#define O_VSH_VID       16      /* 27-16: VID */
#define M_VSH_VID       0xfff
#define O_VSH_TAGGED    15      /* 15: Tagged flag */
#define M_VSH_TAGGED    0x1
#define O_VSH_STAG      14      /* 14: S-Tag Type */
#define M_VSH_STAG      0x1
#define O_VSH_ISOLATED  13      /* 13: Isolated Port */
#define M_VSH_ISOLATED  0x1
#define O_VSH_SRC_MODE  10      /* 10: Source address mode */
#define M_VSH_SRC_MODE  0x1
#define O_VSH_SRC_UID   5       /* 9-5: Source Unit ID */
#define M_VSH_SRC_UID   0x1f
#define O_VSH_SRC_PORT  0       /* 4-0: Source Port */
#define M_VSH_SRC_PORT  0x1f

/* Get field from VSH1/VSH2 */
#define VSH_GET(vsh1, vsh2, field) ((O_VSH_##field > 31 ? (vsh1 >> (O_VSH_##field - 32)) : (vsh2 >> O_VSH_##field)) & M_VSH_##field)

/* Put field to VSH1/VSH2 */
#define VSH_PUT(vsh1, vsh2, field, val) { vsh1 |= (O_VSH_##field > 31 ? ((val)<<(O_VSH_##field - 32)) : 0); vsh2 |= (O_VSH_##field > 31 ? 0 : (val)<<O_VSH_##field); }

#endif /* VTSS_FEATURE_VSTAX */

/* ================================================================= *
 * B_ARBITER registers
 * ================================================================= */
#define R_ARBITER_ARBEMPTY     0x0C /* Arbiter Empty */
#define R_ARBITER_ARBDISC      0x0E /* Arbiter Discard */

#if defined(VTSS_ARCH_HAWX) || defined(VTSS_ARCH_SPARX_G8)
#define R_ARBITER_SBACKWDROP   0x12 /* Backward for Source */
#define R_ARBITER_DBACKWDROP   0x13 /* Backward for Destination */
#endif /* VTSS_ARCH_HAWX/SPARX_G8 */

#if defined(VTSS_ARCH_HAWX) || defined(VTSS_ARCH_SPARX)
#define R_ARBITER_ARBBURSTPROB 0x15 /* Burst Probability */
#endif /* VTSS_ARCH_HAWX/SPARX */

#if defined(VTSS_ARCH_HAWX) || defined(VTSS_ARCH_SPARX_G24)
#define R_ARBITER_ARBDDPROB    0x16 /* Deliberate Drop Probability */
#define R_ARBITER_ARBDDENAB    0x17 /* Deliberate Drop Enabled */
#endif /* VTSS_ARCH_HAWX/SPARX_G24 */

#if defined(VTSS_ARCH_SPARX_28)
#define R_ARBITER_ARBHOLBPENAB 0x17 /* Head of Line Blocking Protection */
#endif /* VTSS_ARCH_SPARX_28 */

#if defined(VTSS_ARCH_SPARX_G24)
#define R_ARBITER_RATEUNIT     0x18 /* Traffic Rate Unit */
#endif /* VTSS_ARCH_SPARX_G24 */

/* ================================================================= *
 * B_SYSTEM registers
 * ================================================================= */
#define R_SYSTEM_CPUMODE      0x00 /* CPU Transfer Mode */
#define R_SYSTEM_SIPAD        0x01 /* SI Padding */
#define R_SYSTEM_PICONF       0x02 /* PI Config */

#if defined(VTSS_ARCH_SPARX_G8)
#define R_SYSTEM_GMIIDELAY    0x05 /* GMII clock delay */
#endif /* VTSS_ARCH_SPARX_G8 */

#if defined(VTSS_ARCH_SPARX_28)
#define R_SYSTEM_MACRO_CTRL   0x08 /* Hardware semaphore */
#endif /* VTSS_ARCH_SPARX_28 */

#define R_SYSTEM_HWSEM        0x13 /* Hardware semaphore */
#define R_SYSTEM_GLORESET     0x14 /* Global Reset */
#define R_SYSTEM_CHIPID       0x18 /* Chip Identification */
#define R_SYSTEM_TIMECMP      0x24 /* Time Compare Value */
#define R_SYSTEM_SLOWDATA     0x2C /* SlowData */
#define R_SYSTEM_CPUCTRL      0x30 /* CPU/interrupt Control */

#if defined(VTSS_ARCH_HAWX) || defined(VTSS_ARCH_SPARX)
#define R_SYSTEM_CAPCTRL      0x31 /* Capture Control */
#endif /* VTSS_ARCH_HAWX/SPARX */

#if defined(VTSS_ARCH_SPARX_28)
#define R_SYSTEM_GPIOCTRL     0x33 /* GPIO Control */
#endif /* VTSS_ARCH_SPARX_28 */
#define R_SYSTEM_GPIO         0x34 /* General Purpose IO */

#if defined(VTSS_ARCH_SPARX)
#define R_SYSTEM_SIMASTER     0x35 /* SI Master Interface */
#if defined(VTSS_ARCH_SPARX_28)
#define R_SYSTEM_RATEUNIT     0x36 /* Policer/shaper rate unit */
#define R_SYSTEM_LEDTIMER     0x3C /* LED timer */
#define R_SYSTEM_LEDMODES     0x3D /* LED modes */
#endif /* VTSS_ARCH_SPARX_28 */
#define R_SYSTEM_SGMII_TR_DBG 0x08 /* SGMII debug */
#define R_SYSTEM_ICPU_CTRL    0x10 /* Internal CPU Control */
#define R_SYSTEM_ICPU_ADDR    0x11 /* Internal CPU On-Chip RAM Address */
#define R_SYSTEM_ICPU_DATA    0x12 /* Internal CPU On-Chip RAM Data */
#endif /* VTSS_ARCH_SPARX */

#define VTSS_T_RESET      125000  /* Waiting time (nanoseconds) after reset command */

#endif /* _HEATHROWII_REG_H */

