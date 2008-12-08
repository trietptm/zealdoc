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
 
 $Id: vtss_cil.h,v 1.8 2008-12-08 07:16:50 zhenglv Exp $
 $Revision: 1.8 $

*/

#ifndef __VTSS_CIL_H_INCLUDE__
#define __VTSS_CIL_H_INCLUDE__

/* Read from the phy register */
vtss_rc vtss_ll_phy_read(uint miim_controller, uint phy_addr, uint phy_reg, ushort *value);

/* Write to the phy register */
vtss_rc vtss_ll_phy_write(uint miim_controller, uint phy_addr, uint phy_reg, ushort value);

/* Configure port into one of the gmii modes or tbi mode */
vtss_rc vtss_ll_port_speed_mode_gmii(vtss_port_no_t port_no,
                                     const vtss_port_setup_t * setup);

/* Enable or disable and flush port queue systems */
vtss_rc vtss_ll_port_queue_enable(vtss_port_no_t port_no, BOOL enable);

/* Setup water marks, drop levels, etc for port */
vtss_rc vtss_ll_port_queue_setup(vtss_port_no_t port_no);

/* Get VAUI port status */
vtss_rc vtss_ll_vaui_status_get(vtss_port_no_t port_no, vtss_port_status_t * const status);

/* Determine if 10G port is up */
vtss_rc vtss_ll_port_up(vtss_port_no_t port_no, BOOL *port_up);

/* Set port QoS setup */
vtss_rc vtss_ll_port_qos_setup_set(vtss_port_no_t port_no, const vtss_port_qos_setup_t *qos);

/* Set switch QoS setup */
vtss_rc vtss_ll_qos_setup_set(const vtss_qos_setup_t *qos);

/* Set SIP filter */
vtss_rc vtss_ll_filter_sip_set(const vtss_port_no_t port_no,
                               const vtss_ip_t      sip,
                               const vtss_ip_t      mask);

/* Read port counters */
vtss_rc vtss_ll_port_counters_get(uint port_no, vtss_chip_counters_t * counters);

#ifdef VTSS_FEATURE_QCL_PORT
/* Add QCE */
vtss_rc vtss_ll_qce_add(vtss_qcl_id_t    qcl_id,
                        vtss_qce_id_t    qce_id,
                        const vtss_qce_t *qce);

/* Delete QCE */
vtss_rc vtss_ll_qce_del(vtss_qcl_id_t  qcl_id,
                        vtss_qce_id_t  qce_id);
#endif

/* Set aggregation mode */
vtss_rc vtss_ll_aggr_mode_set(const vtss_aggr_mode_t *mode);

/* set learn port mode */
#ifdef VTSS_FEATURE_LEARN_PORT
vtss_rc vtss_ll_learn_port_mode_set(const vtss_port_no_t port_no,
                                    vtss_learn_mode_t * const learn_mode);
#endif

#ifdef VTSS_FEATURE_LEARN_SWITCH
/* set learn mode */
vtss_rc vtss_ll_learn_mode_set(vtss_learn_mode_t * const learn_mode);
#endif

/* set logical port mapping */
vtss_rc vtss_ll_pmap_table_write(vtss_port_no_t physical_port, vtss_port_no_t logical_port);
/* set VLAN port mode */
vtss_rc vtss_ll_vlan_port_mode_set(vtss_port_no_t port_no, const vtss_vlan_port_mode_t *vlan_mode);
/* set VLAN port members */
vtss_rc vtss_ll_vlan_table_write(vtss_vid_t vid, BOOL member[VTSS_PORT_ARRAY_SIZE]);
/* set isolated ports */
vtss_rc vtss_ll_isolated_ports_set(const BOOL member[VTSS_PORT_ARRAY_SIZE]);
/* set VLAN MSTP instance */
vtss_rc vtss_ll_vlan_table_mstp_set(vtss_vid_t vid, uint msti);
/* set MSTP state for port and mstp instance */
vtss_rc vtss_ll_mstp_table_write(vtss_port_no_t port_no, vtss_msti_t msti, vtss_mstp_state_t state);

#ifdef VTSS_FEATURE_MAC_AGE_AUTO
/* set MAC address age time */
vtss_rc vtss_ll_mac_table_age_time_set(vtss_mac_age_time_t age_time);
#endif

/* age MAC address table */
vtss_rc vtss_ll_mac_table_age(BOOL pgid_age, vtss_pgid_no_t pgid_no, BOOL vid_age, vtss_vid_t vid);
/* flush MAC address table */
vtss_rc vtss_ll_mac_table_flush(BOOL pgid_age, vtss_pgid_no_t pgid_no, 
                                BOOL vid_age, vtss_vid_t vid);
/* learn (VID, MAC) */
vtss_rc vtss_ll_mac_table_learn(const vtss_mac_table_entry_t *entry, vtss_pgid_no_t pgid_no);
/* unlearn (VID, MAC) */
vtss_rc vtss_ll_mac_table_unlearn(const vtss_vid_mac_t *vid_mac);
/* lookup (VID, MAC) */
vtss_rc vtss_ll_mac_table_lookup(vtss_mac_table_entry_t *entry, vtss_pgid_no_t *pgid_no);
/* direct MAC read */
vtss_rc vtss_ll_mac_table_read(uint index, vtss_mac_table_entry_t *entry, vtss_pgid_no_t *pgid_no);
/* get next */
vtss_rc vtss_ll_mac_table_get_next(const vtss_vid_mac_t *vid_mac, vtss_mac_table_entry_t *entry, vtss_pgid_no_t *pgid_no);
/* get MAC table status */
vtss_rc vtss_ll_mac_table_status_get(vtss_mac_table_status_t *status);
/* write PGID entry */
vtss_rc vtss_ll_pgid_table_write(vtss_pgid_no_t pgid_no, BOOL member[VTSS_PORT_ARRAY_SIZE]);
/* write source port entry */
vtss_rc vtss_ll_src_table_write(vtss_port_no_t port_no, BOOL member[VTSS_PORT_ARRAY_SIZE]);
/* set monitor port for mirroring */
vtss_rc vtss_ll_mirror_port_set(vtss_port_no_t port_no);
/* enable/disable egress mirroring of port */
vtss_rc vtss_ll_dst_mirror_set(vtss_port_no_t port_no, BOOL enable);
/* enable/disable ingress mirroring of port */
vtss_rc vtss_ll_src_mirror_set(vtss_port_no_t port_no, BOOL enable);
/* write aggregation entry */
vtss_rc vtss_ll_aggr_table_write(vtss_ac_no_t ac, BOOL member[VTSS_PORT_ARRAY_SIZE]);
/* set IP Multicast flooding ports */
vtss_rc vtss_ll_ipmc_flood_mask_set(const BOOL member[VTSS_PORT_ARRAY_SIZE]);

/* PI/SI register read */
vtss_rc vtss_ll_register_read(ulong reg, ulong *value);
/* PI/SI register write */
vtss_rc vtss_ll_register_write(ulong reg, ulong value);
/* get chip ID and revision */
vtss_rc vtss_ll_chipid_get(vtss_chipid_t *chipid);
/* optimization function called every second */
vtss_rc vtss_ll_optimize_1sec(void);
/* optimization function called every 100th millisecond */
vtss_rc vtss_ll_optimize_100msec(void);

#ifdef VTSS_FEATURE_SERIAL_LED
/* setup serial LED mode */
vtss_rc vtss_ll_serial_led_set(const vtss_led_port_t port, 
                               const vtss_led_mode_t mode[3]);
#endif

/* initialize low level layer */
vtss_rc vtss_ll_start(const vtss_init_setup_t *setup);

#endif /* __VTSS_CIL_H_INCLUDE__ */
