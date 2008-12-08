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

*/


#ifndef __VTSS_GROCX_H_INCLUDE__
#define __VTSS_GROCX_H_INCLUDE__

#ifdef CONFIG_VTSS_GROCX

#include <linux/sockios.h>
#include <linux/star_switch_api.h>

/******************************************************************************
 * Description: Initialize chip and API.
 *
 * \param setup (input): Pointer to initialization setup.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_grocx_start(void);


/******************************************************************************
 * Description: Setup port.
 *
 * \param port_no (input): Port number.
 * \param setup (input)  : Port setup structure.
 *
 * \return : Return code.
 ******************************************************************************/
 /* Port configuration */
typedef struct {
	BOOL         enable;          /* Admin enable/disable */
	BOOL         autoneg;         /* Auto negotiation */
	vtss_speed_t speed;           /* Forced port speed */
	BOOL         fdx;             /* Forced duplex mode */
	BOOL         flow_control;    /* Flow control */
	uint         maxframelength;  /* Maximum frame length */
} vtss_grocx_port_conf_t;

vtss_rc vtss_grocx_port_setup(const vtss_port_no_t port_no,
			      const vtss_grocx_port_conf_t * const setup);

/******************************************************************************
 * Description: Clear counters for port.
 *
 * \param poag_no (input): Port/aggregation number.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_grocx_poag_counters_clear(const vtss_poag_no_t poag_no);


/******************************************************************************
 * Description: Get counters for port.
 *
 * \param poag_no (input)  : Port/aggregation number.
 * \param counters (output): Counter structure.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_grocx_poag_counters_get(const vtss_poag_no_t poag_no,
				     vtss_poag_counters_t * const counters);

/******************************************************************************
 * Description: Get VLAN membership.
 *
 * \param vid (input)    : VLAN ID.
 * \param member (output): VLAN port member array.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_grocx_vlan_port_members_get(const vtss_vid_t vid,
					 BOOL member[VTSS_PORT_ARRAY_SIZE]);


/******************************************************************************
 * Description: Set VLAN membership.
 *
 * \param vid (input)   : VLAN ID.
 * \param member (input): VLAN port member array.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_grocx_vlan_port_members_set(const vtss_vid_t vid,
					 BOOL member[VTSS_PORT_ARRAY_SIZE]);


/* - Star switch specific ------------------------------------------ */
/******************************************************************************
 * Description: create a socket for ioctl.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_grocx_router_ioctl_start(void);

/******************************************************************************
 * Description: Configure mac port 1
 *
 * \param mac1_control (input): mac port control
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_grocx_router_mac1_control(vtss_grx_mac1_control_t * mac1_control);


/******************************************************************************
 * Description: Enable/Disable unknown VLAN trap to CPU port.
 *
 * \param port (input): which port will involve with this function.
 * \param enable (input): TRUE if want to enable, FALSE if want to disable.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_grocx_router_vlan_trap(const BOOL enable);


/******************************************************************************
 * Description: Enable/Disable Storm Control.
 *
 * \param storm_ctl (input): storm control
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_grocx_router_storm_control(vtss_grx_storm_ctl_t * storm_ctl);


/******************************************************************************
 * Description: Enable/Disable Egress Rate Limit Control
 *
 * \param egress_rate_limit_ctl (input): egress rate limit control
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_grocx_router_egress_rate_limit_control(vtss_grx_egress_rate_limit_ctl_t *egress_rate_limit_ctl);


/******************************************************************************
 * Description:  Add VLAN / Remove VLAN / Modify VLAN
 *
 * \param vlan_ctl (input) : vlan control
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_grocx_router_vlan_control(vtss_grx_vlan_ctl_t *vlan_ctl);


/******************************************************************************
 * Description: Enable/Disable QoS(only check VLAN priority)
 *
 * \param vtss_grx_qos_ctl_t (input) : QOS control
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_grocx_router_qos_control(vtss_grx_qos_ctl_t * qos_ctl);


/******************************************************************************
 * Description: ARL table control
 *
 * \param vtss_grx_arl_table_t (input) : ARL table control
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_grocx_router_arl_table_control(vtss_grx_arl_table_t *arl_table);


/******************************************************************************
 * Description: counter get
 *
 * \param vtss_grx_port_counter_t (input) : counter struct
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_grocx_router_packet_counter_get(vtss_grx_port_counter_t * ctl);


/******************************************************************************
 * Description: counter clear
 *
 * \param vtss_grx_port_counter_t (input) : counter struct
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_grocx_router_packet_counter_clear(vtss_grx_port_counter_t * ctl);

void vtss_grocx_port_mode_start(void);

#endif

#endif /* __VTSS_GROCX_H_INCLUDE__ */
