#ifndef __VTSS_BOARD_H_INCLUDE__
#define __VTSS_BOARD_H_INCLUDE__

#include <autoconf.h>

/* ================================================================= *
 *  Features
 * ================================================================= */
/* SparX-28 features */
#ifdef CONFIG_VTSS_GROCX
#define VTSS_ACES 64 
#define VTSS_FEATURE_SERIAL_LED 1             /* Serial LED */
#define VTSS_FEATURE_EXC_COL_CONT 1           /* Excessive collision continuation */
#define VTSS_FEATURE_PORT_CNT_PKT_CAST 1      /* Has separate counters for unicast/multicast */
#define VTSS_FEATURE_PORT_CNT_RMON_ADV 1      /* Advanced RMON counters */
#define VTSS_FEATURE_PORT_CNT_JUMBO 1         /* Jumbo frame (>1518) counters */
#define VTSS_FEATURE_PORT_CNT_QOS 1           /* QoS counters */
#define VTSS_FEATURE_PORT_CNT_BRIDGE 1        /* Bridge counters */
#define VTSS_FEATURE_QOS_WFQ_PORT 1           /* QoS: Weighted Fairness Queueing per port */
#define VTSS_FEATURE_QOS_ETYPE_PORT 1         /* QoS: Ethernet Type per port */
#define VTSS_FEATURE_QOS_IP_TOS_PORT 1        /* QoS: Use 3 MSBit from TOS field per port */
#define VTSS_FEATURE_QOS_L4_PORT 1            /* QoS: Layer 4 per port */
#define VTSS_FEATURE_QOS_DSCP_REMARK 1        /* QoS: DSCP remarking */
#define VTSS_FEATURE_QOS_TAG_REMARK 1         /* QoS: Tag priority remarking */
#define VTSS_FEATURE_QCL_PORT 1               /* QoS: QCLs per port */
#define VTSS_FEATURE_QOS_SHAPER_PORT 1        /* QoS: Shaper per port */
#define VTSS_FEATURE_QOS_POLICER_PORT 1       /* QoS: Policer per port */
#define VTSS_FEATURE_QOS_POLICER_CPU_SWITCH 1 /* QoS: CPU policers per switch */
#define VTSS_FEATURE_QOS_POLICER_UC_SWITCH 1  /* QoS: Unicast policer per switch */
#define VTSS_FEATURE_QOS_POLICER_MC_SWITCH 1  /* QoS: Multicast policer per switch */
#define VTSS_FEATURE_QOS_POLICER_BC_SWITCH 1  /* QoS: Broadcast policer per switch */
#define VTSS_FEATURE_ACL 1                    /* Access Control Lists */
#define VTSS_FEATURE_CPU_RX_LEARN 1           /* Learn flag */
#define VTSS_FEATURE_MAC_AGE_AUTO 1           /* Automatic MAC address ageing */
#define VTSS_FEATURE_VLAN_INGR_FILTER_PORT 1  /* VLAN: Ingr. filter per port */
#define VTSS_FEATURE_VLAN_TYPE_STAG 1         /* VLAN: S-Tag type */
#define VTSS_FEATURE_AGGR_MODE_ADV 1          /* Advanced aggregation mode */
#define VTSS_FEATURE_AGGR_MODE_RANDOM 1       /* Random aggregation mode */
#define VTSS_FEATURE_LEARN_PORT 1             /* Learning per port */
#define VTSS_FEATURE_ISOLATED_PORT 1          /* PVLAN port isolation */
#define VTSS_FEATURE_JUMBO_FRAME_PORT 1       /* Jumbo frames per port */
#endif

#ifdef CONFIG_VTSS_GROCX
/* G-RocX queue system settings */

/* VTSS_<maxsize>_<prios><strict/weighted priority>_<drop/flowcontrol mode>_<register> */
/* define   ::= VTSS_(NORM|9600)_(1|2|4)(S|W)_(DROP|FC)_<register> */
/* register ::= EARLY_TX | FWDP_(START|STOP) | IMIN | IDISC | EMIN | EMAX(0-3) | ZEROPAUSE | PAUSEVALUE */

/* level:    Memory usage in slices */
/* wmaction: 0 = Drop from head, 1 = Drop during reception, 2 = Urgent, 3 = FC */
/* cmpwith:  0 = Total, 1 = Ingress, 2 = Spec, 3 = Egress */
/* qmask:    Which queues to apply, 1=q0, 2=q1, 4=q2, 8=q3, 16=CPU  */
/* chkmin:   Check IMIN WM */
/* lowonly:  Apply only to lowest prio queue with data. */

/* G-RocX strict priority, drop mode */
#define VTSS_NORM_4S_DROP_LEVEL_0    19
#define VTSS_NORM_4S_POLICER_LEVEL_0 9   /* Used if policer enabled */
#define VTSS_NORM_4S_DROP_ACTION_0   2
#define VTSS_NORM_4S_DROP_CMPWITH_0  0
#define VTSS_NORM_4S_DROP_QMASK_0    0x1F
#define VTSS_NORM_4S_DROP_CHKMIN_0   1
#define VTSS_NORM_4S_DROP_LOWONLY_0  0

#define VTSS_NORM_4S_DROP_LEVEL_1    0
#define VTSS_NORM_4S_DROP_ACTION_1   2
#define VTSS_NORM_4S_DROP_CMPWITH_1  2
#define VTSS_NORM_4S_DROP_QMASK_1    0x01
#define VTSS_NORM_4S_DROP_CHKMIN_1   0
#define VTSS_NORM_4S_DROP_LOWONLY_1  0

#define VTSS_NORM_4S_DROP_LEVEL_2    0
#define VTSS_NORM_4S_DROP_ACTION_2   2
#define VTSS_NORM_4S_DROP_CMPWITH_2  2
#define VTSS_NORM_4S_DROP_QMASK_2    0x03
#define VTSS_NORM_4S_DROP_CHKMIN_2   0
#define VTSS_NORM_4S_DROP_LOWONLY_2  0

#define VTSS_NORM_4S_DROP_LEVEL_3    0
#define VTSS_NORM_4S_DROP_ACTION_3   2
#define VTSS_NORM_4S_DROP_CMPWITH_3  2
#define VTSS_NORM_4S_DROP_QMASK_3    0x07
#define VTSS_NORM_4S_DROP_CHKMIN_3   0
#define VTSS_NORM_4S_DROP_LOWONLY_3  0

#define VTSS_NORM_4S_DROP_LEVEL_4    0
#define VTSS_NORM_4S_DROP_ACTION_4   2
#define VTSS_NORM_4S_DROP_CMPWITH_4  2
#define VTSS_NORM_4S_DROP_QMASK_4    0x7
#define VTSS_NORM_4S_DROP_CHKMIN_4   0
#define VTSS_NORM_4S_DROP_LOWONLY_4  0

#define VTSS_NORM_4S_DROP_LEVEL_5    21
#define VTSS_NORM_4S_POLICER_LEVEL_5 11   /* Used if policer enabled */
#define VTSS_NORM_4S_DROP_ACTION_5   1
#define VTSS_NORM_4S_DROP_CMPWITH_5  1
#define VTSS_NORM_4S_DROP_QMASK_5    0xf
#define VTSS_NORM_4S_DROP_CHKMIN_5   1
#define VTSS_NORM_4S_DROP_LOWONLY_5  0

#define VTSS_NORM_4S_DROP_IMIN          0
#define VTSS_NORM_4S_DROP_EARLY_TX      0
#define VTSS_NORM_4S_DROP_EMIN          3
#define VTSS_NORM_4S_DROP_EMAX0         14
#define VTSS_NORM_4S_DROP_EMAX1         14
#define VTSS_NORM_4S_DROP_EMAX2         14
#define VTSS_NORM_4S_DROP_EMAX3         14
#define VTSS_NORM_4S_DROP_ZEROPAUSE     0
#define VTSS_NORM_4S_DROP_PAUSEVALUE    0 /*Don't care*/


/* G-RocX weighted priority, drop mode */
#define VTSS_NORM_4W_DROP_LEVEL_0    17
#define VTSS_NORM_4W_POLICER_LEVEL_0 9   /* Used if policer enabled */
#define VTSS_NORM_4W_DROP_ACTION_0   2
#define VTSS_NORM_4W_DROP_CMPWITH_0  0
#define VTSS_NORM_4W_DROP_QMASK_0    0x1F
#define VTSS_NORM_4W_DROP_CHKMIN_0   1
#define VTSS_NORM_4W_DROP_LOWONLY_0  0

#define VTSS_NORM_4W_DROP_LEVEL_1    31
#define VTSS_NORM_4W_DROP_ACTION_1   0
#define VTSS_NORM_4W_DROP_CMPWITH_1  0
#define VTSS_NORM_4W_DROP_QMASK_1    0
#define VTSS_NORM_4W_DROP_CHKMIN_1   0
#define VTSS_NORM_4W_DROP_LOWONLY_1  0

#define VTSS_NORM_4W_DROP_LEVEL_2    31
#define VTSS_NORM_4W_DROP_ACTION_2   0
#define VTSS_NORM_4W_DROP_CMPWITH_2  0
#define VTSS_NORM_4W_DROP_QMASK_2    0
#define VTSS_NORM_4W_DROP_CHKMIN_2   0
#define VTSS_NORM_4W_DROP_LOWONLY_2  0

#define VTSS_NORM_4W_DROP_LEVEL_3    31
#define VTSS_NORM_4W_DROP_ACTION_3   0
#define VTSS_NORM_4W_DROP_CMPWITH_3  0
#define VTSS_NORM_4W_DROP_QMASK_3    0
#define VTSS_NORM_4W_DROP_CHKMIN_3   0
#define VTSS_NORM_4W_DROP_LOWONLY_3  0

#define VTSS_NORM_4W_DROP_LEVEL_4    31
#define VTSS_NORM_4W_DROP_ACTION_4   0
#define VTSS_NORM_4W_DROP_CMPWITH_4  0
#define VTSS_NORM_4W_DROP_QMASK_4    0
#define VTSS_NORM_4W_DROP_CHKMIN_4   0
#define VTSS_NORM_4W_DROP_LOWONLY_4  0

#define VTSS_NORM_4W_DROP_LEVEL_5    31
#define VTSS_NORM_4W_POLICER_LEVEL_5 11   /* Used if policer enabled */
#define VTSS_NORM_4W_DROP_ACTION_5   1
#define VTSS_NORM_4W_DROP_CMPWITH_5  1
#define VTSS_NORM_4W_DROP_QMASK_5    0xf
#define VTSS_NORM_4W_DROP_CHKMIN_5   1
#define VTSS_NORM_4W_DROP_LOWONLY_5  0

#define VTSS_NORM_4W_DROP_IMIN          0
#define VTSS_NORM_4W_DROP_EARLY_TX      0
#define VTSS_NORM_4W_DROP_EMIN          3
#define VTSS_NORM_4W_DROP_EMAX0         14
#define VTSS_NORM_4W_DROP_EMAX1         14
#define VTSS_NORM_4W_DROP_EMAX2         14
#define VTSS_NORM_4W_DROP_EMAX3         14
#define VTSS_NORM_4W_DROP_ZEROPAUSE     0
#define VTSS_NORM_4W_DROP_PAUSEVALUE    0 /*Don't care*/

/* G-RocX flow control mode */
/* Note! */
/* 2.5GB Stack ports need modified WM: */
/* #define VTSS_NORM_4S_FC_LEVEL_2    4 */


/* G-RocX flow control mode */
#define VTSS_NORM_4S_FC_LEVEL_0    31
#define VTSS_NORM_4S_FC_ACTION_0   0
#define VTSS_NORM_4S_FC_CMPWITH_0  1
#define VTSS_NORM_4S_FC_QMASK_0    0x1F
#define VTSS_NORM_4S_FC_CHKMIN_0   1
#define VTSS_NORM_4S_FC_LOWONLY_0  1

#define VTSS_NORM_4S_FC_LEVEL_1    15
#define VTSS_NORM_4S_FC_ACTION_1   2
#define VTSS_NORM_4S_FC_CMPWITH_1  1
#define VTSS_NORM_4S_FC_QMASK_1    0x1F
#define VTSS_NORM_4S_FC_CHKMIN_1   1
#define VTSS_NORM_4S_FC_LOWONLY_1  1

#define VTSS_NORM_4S_FC_LEVEL_2    6
#define VTSS_NORM_4S_FC_ACTION_2   3
#define VTSS_NORM_4S_FC_CMPWITH_2  1
#define VTSS_NORM_4S_FC_QMASK_2    0x1F
#define VTSS_NORM_4S_FC_CHKMIN_2   0
#define VTSS_NORM_4S_FC_LOWONLY_2  0		   

#define VTSS_NORM_4S_FC_LEVEL_3    4
#define VTSS_NORM_4S_FC_ACTION_3   3
#define VTSS_NORM_4S_FC_CMPWITH_3  1
#define VTSS_NORM_4S_FC_QMASK_3    0x1F
#define VTSS_NORM_4S_FC_CHKMIN_3   0
#define VTSS_NORM_4S_FC_LOWONLY_3  0		   

#define VTSS_NORM_4S_FC_LEVEL_4    31
#define VTSS_NORM_4S_FC_ACTION_4   0
#define VTSS_NORM_4S_FC_CMPWITH_4  0
#define VTSS_NORM_4S_FC_QMASK_4    0
#define VTSS_NORM_4S_FC_CHKMIN_4   0
#define VTSS_NORM_4S_FC_LOWONLY_4  0		

#define VTSS_NORM_4S_FC_LEVEL_5    31
#define VTSS_NORM_4S_FC_ACTION_5   0
#define VTSS_NORM_4S_FC_CMPWITH_5  0
#define VTSS_NORM_4S_FC_QMASK_5    0
#define VTSS_NORM_4S_FC_CHKMIN_5   0
#define VTSS_NORM_4S_FC_LOWONLY_5  0		

#define VTSS_NORM_4S_FC_IMIN          0
#define VTSS_NORM_4S_FC_EARLY_TX      0		   
#define VTSS_NORM_4S_FC_EMIN          0
#define VTSS_NORM_4S_FC_EMAX0         6
#define VTSS_NORM_4S_FC_EMAX1         6
#define VTSS_NORM_4S_FC_EMAX2         6
#define VTSS_NORM_4S_FC_EMAX3         6
#define VTSS_NORM_4S_FC_ZEROPAUSE     1
#define VTSS_NORM_4S_FC_PAUSEVALUE    0xFF

#endif

#endif /* __VTSS_BOARD_H_INCLUDE__ */
