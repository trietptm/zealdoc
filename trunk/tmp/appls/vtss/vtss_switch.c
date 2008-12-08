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
 
 $Id: vtss_switch.c,v 1.20 2008-12-08 07:16:50 zhenglv Exp $
 $Revision: 1.20 $

*/

/* API public headers */
#include "vtss_priv.h"

static vtss_rc vtss_update_masks(BOOL src_update,
				 BOOL dest_update, BOOL aggr_update);

/* Global variables for API state information */
vtss_mac_state_t vtss_mac_state;		/* API high level state information */

/* - Reset --------------------------------------------------------- */
static void vtss_init_state(const vtss_init_setup_t * setup)
{
	vtss_port_no_t    port_no, port_end;
	vtss_pgid_no_t    pgid_no;
	vtss_msti_t       msti;
	vtss_vid_t        vid;
	vtss_pgid_entry_t *pgid_entry;
	uint              chip_port;
	int               i;
	
	memset(&vtss_mac_state, 0, sizeof(vtss_mac_state_t));
	
	/* Initialize port map table */
	for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
#ifdef CONFIG_VTSS_ARCH_HEATHROW
		chip_port = port_no-VTSS_PORT_NO_START;
		vtss_mac_state.port_map.chip_port[port_no] = chip_port;
		vtss_mac_state.port_map.vtss_port[chip_port] = port_no;
#endif
		vtss_mac_state.port_map.chip_ports_all[port_no] = chip_port;
	}
	
	/* Initialize aggregations */
	for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
		vtss_mac_state.port_poag_no[port_no] = port_no;
	}
	
	/* Initialize MSTP to VLAN mapping */
	for (vid = 0; vid < VTSS_VIDS; vid++) {
		vtss_mac_state.vlan_table[vid].msti = VTSS_MSTI_START;
	}
	vtss_mac_state.msti_end = VTSS_MSTI_END;
	
	/* Initialize MSTP table */
	for (msti = VTSS_MSTI_START; msti < VTSS_MSTI_END; msti++) {
		for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
			vtss_mac_state.mstp_table[msti].state[port_no] = (msti==VTSS_MSTI_START ? 
VTSS_MSTP_STATE_FORWARDING: 
			VTSS_MSTP_STATE_DISCARDING);
		}
	}
	
	/* Initialize number of MAC addresses */
	vtss_mac_state.mac_addrs = VTSS_MAC_ADDRS;
        
	/* Initialize PGID table */
	vtss_mac_state.pgid_end = VTSS_PGID_END;
	for (pgid_no = VTSS_PGID_START; pgid_no < vtss_mac_state.pgid_end; pgid_no++) {
		pgid_entry = &vtss_mac_state.pgid_table[pgid_no];
		switch (pgid_no) {
		case VTSS_PGID_KILL:
			/* Kill entry */
			pgid_entry->references = 1;
			break;
		case VTSS_PGID_BC:
			/* Broadcast entry */
			for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
				pgid_entry->member[port_no] = MAKEBOOL01(1);
			}
			pgid_entry->references = 1;
			break;
		default:
			if (pgid_no<VTSS_PGID_UNICAST_END) {
				/* Unicast entries */
				pgid_entry->member[pgid_no] = MAKEBOOL01(1);
				pgid_entry->references = 1;
			} else {
				/* Multicast entries */
			}
			break;
		}
	}
	
	/* Initialize STP and Authentication state */
	for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
		vtss_mac_state.stp_state[port_no] = VTSS_STP_STATE_DISABLED;
		vtss_mac_state.auth_state[port_no] = VTSS_AUTH_STATE_BOTH;
	}
	
	/* Initialize PVLAN table */
	for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
		vtss_mac_state.pvlan_table[VTSS_PVLAN_NO_DEFAULT].member[port_no] = 1;
	}
	
	/* Initialize mirror port */
	vtss_mac_state.mirror_port = 0; /* Mirror port disabled */
	
	/* Initialize QoS, all queues used */
	vtss_mac_state.prios = VTSS_PRIOS;
	port_end = VTSS_PORT_NO_END;
	for (port_no = VTSS_PORT_NO_START; port_no < port_end; port_no++) {
		vtss_port_qos_setup_t *qos;
		
		qos = &vtss_mac_state.qos[port_no];
#ifdef VTSS_FEATURE_QCL_PORT
		qos->qcl_id = VTSS_QCL_ID_NONE;
#endif
#if defined(VTSS_FEATURE_QOS_L4_PORT)
		qos->udp_tcp_enable = 0;
#endif
		qos->dscp_enable = 0;
		qos->tag_enable = 0;
#ifdef VTSS_FEATURE_QOS_ETYPE_PORT
		qos->etype_enable = 0;
#endif
#ifdef VTSS_FEATURE_QOS_L4_PORT
		for (i=0; i<10; i++) {
			qos->udp_tcp_val[i] = 0;
		}
#endif
		for (i=0; i<64; i++) {
			qos->dscp_prio[i] = VTSS_PRIO_START;
		}
		for (i=0; i<8; i++) {
			/* Use IEEE user priority mapping adjusted for number of priorities */
			qos->tag_prio[i] = (
				((i == 0 ? 2 : i == 1 ? 0 : i == 2 ? 1 : i)*VTSS_PRIOS/8) + VTSS_PRIO_START);
		}
#ifdef VTSS_FEATURE_QOS_ETYPE_PORT
		qos->etype_val = 0xFFFF;
		qos->etype_prio = VTSS_PRIO_START;
#endif
		qos->default_prio = VTSS_PRIO_START;
#ifdef CONFIG_VTSS_ARCH_HEATHROW
		qos->usr_prio = 0 /*VTSS_TAGPRIO_DEFAULT*/;
#endif
#ifdef VTSS_FEATURE_QOS_POLICER_PORT
		qos->policer_port = VTSS_BITRATE_FEATURE_DISABLED;
#endif
#ifdef VTSS_FEATURE_QOS_POLICER_CIR_PIR_QUEUE
		for (i = VTSS_QUEUE_START; i < VTSS_QUEUE_END; i++) {
			qos->policer_cir_queue[i] = VTSS_BITRATE_FEATURE_DISABLED;
			qos->policer_pir_queue[i] = VTSS_BITRATE_FEATURE_DISABLED;
		}
#endif
#ifdef VTSS_FEATURE_QOS_SHAPER_PORT
		qos->shaper_port = VTSS_BITRATE_FEATURE_DISABLED;
#endif
	}
	
#ifdef VTSS_FEATURE_QOS_POLICER_CPU_SWITCH
	vtss_mac_state.qos_setup.policer_mac = VTSS_PACKET_RATE_DISABLED;
	vtss_mac_state.qos_setup.policer_cat = VTSS_PACKET_RATE_DISABLED;
	vtss_mac_state.qos_setup.policer_learn = VTSS_PACKET_RATE_DISABLED;
#endif
	
#ifdef VTSS_FEATURE_QOS_POLICER_UC_SWITCH
	vtss_mac_state.qos_setup.policer_uc = VTSS_PACKET_RATE_DISABLED;
#endif
	
#ifdef VTSS_FEATURE_QOS_POLICER_MC_SWITCH
	vtss_mac_state.qos_setup.policer_mc = VTSS_PACKET_RATE_DISABLED;
#endif
	
#ifdef VTSS_FEATURE_QOS_POLICER_BC_SWITCH
	vtss_mac_state.qos_setup.policer_bc = VTSS_PACKET_RATE_DISABLED;
#endif
	
	vtss_acl_init_state();
	vtss_mac_state.init_setup = *setup; 
	
#if (CONFIG_VTSS_MAC_NEXT_MAX != 0)
	/* Initialize MAC address table */
	for (i = 0; i < VTSS_MAC_ADDRS; i++) {
		/* Insert first in free list */
		vtss_mac_state.mac_table[i].next = vtss_mac_state.mac_list_free;
		vtss_mac_state.mac_list_free = &vtss_mac_state.mac_table[i];
	}
#endif
	
#ifdef VTSS_FEATURE_QCL_PORT
	{
		vtss_qcl_id_t    qcl_id;
		vtss_qcl_t       *qcl;
		vtss_qcl_entry_t *qcl_entry;
		
		/* Initialize QCL free/used lists */
		for (qcl_id = VTSS_QCL_ID_START; qcl_id < VTSS_QCL_ID_END; qcl_id++) {
			qcl = &vtss_mac_state.qcl[qcl_id];
			for (i = 0; i < VTSS_QCL_LIST_SIZE; i++) {
				qcl_entry = &qcl->qcl_list[i];
				/* Insert in free list */
				qcl_entry->next = qcl->qcl_list_free;
				qcl->qcl_list_free = qcl_entry;
			}
		}
	}
#endif
}

vtss_rc vtss_api_start(const vtss_init_setup_t * const setup)
{
	vtss_port_no_t port_no;
	vtss_vlan_port_mode_t vlan_mode;
	BOOL member[VTSS_PORT_ARRAY_SIZE];
	
	/* Initialize API state information */
	vtss_init_state(setup);
	
	/* Initialize Low Level Layer */
	VTSS_RC(vtss_ll_start(setup));
    
	if (setup->reset_chip) {
		/* Setup in VLAN unaware mode including all ports in VLAN 1 with PVID 1 */
		vlan_mode.aware = 0;
		vlan_mode.pvid = VTSS_VID_DEFAULT;
		vlan_mode.frame_type = VTSS_VLAN_FRAME_ALL;
		vlan_mode.untagged_vid = VTSS_VID_DEFAULT;
#ifdef VTSS_FEATURE_VLAN_INGR_FILTER_PORT
		vlan_mode.ingress_filter = 0;
#endif
#ifdef VTSS_FEATURE_VLAN_TYPE_STAG
		vlan_mode.stag = 0;
#endif
		for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
			VTSS_RC(vtss_vlan_port_mode_set(port_no, &vlan_mode));
			member[port_no] = 1;
		}
		VTSS_RC(vtss_vlan_port_members_set(VTSS_VID_DEFAULT, member));
		
		/* Setup the broadcast entry in the PGID table */
		VTSS_RC(vtss_pgid_table_write(VTSS_PGID_BC));

		VTSS_RC(vtss_cpu_start());
	}
	
	/* Initialize PHY API */
	VTSS_RC(vtss_phy_start());
	return VTSS_OK;
}

/* - Chip ID and revision ------------------------------------------ */

vtss_rc vtss_chipid_get(vtss_chipid_t * const chipid)
{
	return vtss_ll_chipid_get(chipid);
}

vtss_rc vtss_optimize_1sec(void)
{
	VTSS_RC(vtss_ll_optimize_1sec());
#if (CONFIG_VTSS_MAC_NEXT_MAX != 0)
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	/* No update required for targets supporting GET_NEXT */
#else
	VTSS_RC(vtss_mac_table_optimize());
#endif
#endif
	return VTSS_OK;
}

vtss_rc vtss_optimize_100msec(void)
{
#ifdef CONFIG_VTSS_ARCH_HEATHROW
	return vtss_ll_optimize_100msec();
#endif
	return VTSS_OK;
}

/* - Serial LED ---------------------------------------------------- */
#ifdef VTSS_FEATURE_SERIAL_LED
vtss_rc vtss_serial_led_set(const vtss_led_port_t port, 
                            const vtss_led_mode_t mode[3])
{
	return vtss_ll_serial_led_set(port, mode);
}
#endif

/* - Direct register access (for debugging only) ------------------- */
vtss_rc vtss_register_read(const ulong reg, ulong *const value)
{
	return vtss_ll_register_read(reg,value);
}

vtss_rc vtss_register_write(const ulong reg, const ulong value)
{
	return vtss_ll_register_write(reg, value);
}

vtss_rc vtss_register_writemasked(const ulong reg, const ulong value,
				  const ulong mask)
{
	ulong val;
	
	VTSS_RC(vtss_register_read(reg, &val));
	VTSS_RC(vtss_register_write(reg, (val & ~mask) | (value & mask)));
	return VTSS_OK;
}

/* ================================================================= *
 *  Port and PHY
 * ================================================================= */

/* - Port mapping -------------------------------------------------- */
vtss_rc vtss_port_map_set(const vtss_mapped_port_t mapped_ports[VTSS_PORT_ARRAY_SIZE])
{
	vtss_port_no_t port_no;
	int chip_port;
	int chip_port_count[VTSS_CHIP_PORTS];
	
	/* Clear chip port count array */
	memset(chip_port_count,0,sizeof(chip_port_count));
	
	for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
		chip_port = mapped_ports[port_no].chip_port;
		vtss_mac_state.port_map.chip_port[port_no] = chip_port;
		vtss_mac_state.port_map.vtss_port_unused[port_no] = MAKEBOOL01(chip_port < 0);
		if (chip_port >= 0) {
			vtss_mac_state.port_map.vtss_port[chip_port] = port_no;
			chip_port_count[chip_port]++;
		}
		
		/* MII mapping */
		vtss_mac_state.port_map.phy_addr[port_no] = mapped_ports[port_no].phy_addr;
		vtss_mac_state.port_map.miim_controller[port_no] = mapped_ports[port_no].miim_controller;
		vtss_phy_map_port( port_no,
			VTSS_PHY_BUS(mapped_ports[port_no].miim_controller),
			mapped_ports[port_no].phy_addr );
	}
	
	/* Make sure all ports are mapped */
	for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
		if (vtss_mac_state.port_map.chip_port[port_no] < 0) {
			vtss_port_no_t port;
			
			/* Port is unmapped, look for the first unused chip port */
			for (port = VTSS_PORT_NO_START; port < VTSS_PORT_NO_END; port++) {
				chip_port = vtss_mac_state.port_map.chip_ports_all[port];
				if (chip_port_count[chip_port] == 0) {
					vtss_log(VTSS_LOG_DEBUG,
						 "SWITCH: mapping unused port to chip, port=%d_port, chip_port=%d",
						 port_no, chip_port);
					vtss_mac_state.port_map.chip_port[port_no] = chip_port;
					vtss_mac_state.port_map.vtss_port[chip_port] = port_no;
					chip_port_count[chip_port]++;
					break;
				}
			}
		}
	}
	return VTSS_OK;
}

/* - IEEE 802.3 clause 22 PHY access functions --------------------- */

vtss_rc vtss_register_phy_read(const vtss_miim_controller_t miim_controller,
                               const uint                   phy_addr,
                               const uint                   phy_reg,
                               ushort *const                value)
{
	vtss_phy_map_port(0,VTSS_PHY_BUS(miim_controller),phy_addr);
	return vtss_phy_read(0,phy_reg,value);
}

vtss_rc vtss_register_phy_write(const vtss_miim_controller_t miim_controller,
                                const uint                   phy_addr,
                                const uint                   phy_reg,
                                const ushort                 value)
{
	vtss_phy_map_port(0,VTSS_PHY_BUS(miim_controller),phy_addr);
	return vtss_phy_write(0,phy_reg,value);
}

vtss_rc vtss_register_phy_writemasked(const vtss_miim_controller_t miim_controller,
                                      const uint                   phy_addr,
                                      const uint                   phy_reg,
                                      const ushort                 value,
                                      const ushort                 mask)
{
	vtss_phy_map_port(0,VTSS_PHY_BUS(miim_controller),phy_addr);
	return vtss_phy_writemasked(0,phy_reg,value,mask);
}

vtss_rc vtss_miim_phy_read(const vtss_miim_controller_t miim_controller,
                           const uint                   addr,
                           const uint                   reg,
                           ushort *const                value)
{
	/* Validate parameters */
	if (miim_controller >= VTSS_MIIM_CONTROLLERS) {
		vtss_log(VTSS_LOG_ERR,
			 "SWITCH: MII controller not valid, mii=%d, addr=%d, reg=%d",
			 miim_controller, addr, reg);
		return VTSS_INVALID_PARAMETER;
	}
	if (addr >= 0x20) {
		vtss_log(VTSS_LOG_ERR,
			 "SWITCH: addr not valid, mii=%d, addr=%d, reg=%d",
			 miim_controller, addr, reg);
		return VTSS_INVALID_PARAMETER;
	}
	if (reg >= 0x20) {
		vtss_log(VTSS_LOG_ERR,
			 "SWITCH: reg not valid, mii=%d, addr=%d, reg=%d",
			 miim_controller, addr, reg);
		return VTSS_INVALID_PARAMETER;
	}
	return vtss_ll_phy_read(miim_controller, addr, reg, value);
}

vtss_rc vtss_miim_phy_write(const vtss_miim_controller_t miim_controller,
                            const uint addr,
                            const uint reg,
                            const ushort value)
{
	/* Validate parameters */
	if (miim_controller >= VTSS_MIIM_CONTROLLERS) {
		vtss_log(VTSS_LOG_ERR,
			 "SWITCH: MII controller not valid, mii=%d, addr=%d, reg=%d, value=0x%x",
			 miim_controller, addr, reg, value);
		return VTSS_INVALID_PARAMETER;
	}
	if (addr >= 0x20) {
		vtss_log(VTSS_LOG_ERR,
			 "SWITCH: addr not valid, mii=%d, addr=%d, reg=%d, value=0x%x",
			 miim_controller, addr, reg, value);
		return VTSS_INVALID_PARAMETER;
	}
	if (reg >= 0x20) {
		vtss_log(VTSS_LOG_ERR,
			 "SWITCH: reg not valid, mii=%d, addr=%d, reg=%d, value=0x%x",
			 miim_controller, addr, reg, value);
		return VTSS_INVALID_PARAMETER;
	}
	
	return vtss_ll_phy_write(miim_controller, addr, reg, value);
}

vtss_rc vtss_miim_phy_writemasked(const vtss_miim_controller_t miim_controller,
				  const uint addr, const uint reg,
				  const ushort value, const ushort mask)
{
	ushort val;
	
	/* Validate parameters */
	if (miim_controller >= VTSS_MIIM_CONTROLLERS) {
		vtss_log(VTSS_LOG_ERR,
			 "SWITCH: MII controller not valid, mii=%d, addr=%d, reg=%d",
			 miim_controller, addr, reg);
		return VTSS_INVALID_PARAMETER;
	}
	if (addr >= 0x20) {
		vtss_log(VTSS_LOG_ERR,
			 "SWITCH: addr not valid, mii=%d, addr=%d, reg=%d",
			 miim_controller, addr, reg);
		return VTSS_INVALID_PARAMETER;
	}
	if (reg >= 0x20) {
		vtss_log(VTSS_LOG_ERR,
			 "SWITCH: reg not valid, mii=%d, addr=%d, reg=%d",
			 miim_controller, addr, reg);
		return VTSS_INVALID_PARAMETER;
	}
	VTSS_RC(vtss_ll_phy_read(miim_controller, addr, reg, &val));
	return vtss_ll_phy_write(miim_controller, addr, reg,
				 (ushort)((val & ~mask) | (value & mask)));
}


/* - Port configuration and reset ---------------------------------- */

vtss_rc vtss_port_setup(const vtss_port_no_t port_no,
                        const vtss_port_setup_t * const setup)
{
	vtss_port_no_t        port;
	vtss_port_interface_t type;
	vtss_speed_t          speed;
	BOOL                  jumbo_old;
	
	type = setup->interface_mode.interface_type;
	speed = setup->interface_mode.speed;
	
	vtss_log(VTSS_LOG_INFO,
		 "SWITCH: setup port, port=%d, %stype+%s%s, speed=%s, fdx=%d",
		 port_no,
		 setup->powerdown ? "POWERDOWN, " : "",
		 type==VTSS_PORT_INTERFACE_NO_CONNECTION ? "NO_CONNECTION" :
		 type==VTSS_PORT_INTERFACE_LOOPBACK ? "LOOPBACK" :
		 type==VTSS_PORT_INTERFACE_INTERNAL ? "INTERNAL" :
		 type==VTSS_PORT_INTERFACE_MII ? "MII" :
		 type==VTSS_PORT_INTERFACE_GMII ? "GMII" :
		 type==VTSS_PORT_INTERFACE_RGMII ? "RGMII" :
		 type==VTSS_PORT_INTERFACE_TBI ? "TBI" :
		 type==VTSS_PORT_INTERFACE_RTBI ? "RTBI" :
		 type==VTSS_PORT_INTERFACE_SGMII ? "SGMII" :
		 type==VTSS_PORT_INTERFACE_SERDES ? "SERDES" :
		 type==VTSS_PORT_INTERFACE_VAUI ? "VAUI" :
		 type==VTSS_PORT_INTERFACE_XGMII ? "XGMII" :
		 "????",
		 "",
		 speed==VTSS_SPEED_UNDEFINED ? "UNDEFINED" :
		 speed==VTSS_SPEED_10M ? "10M" :
		 speed==VTSS_SPEED_100M ? "100M" :
		 speed==VTSS_SPEED_1G ? "1G" :
		 speed==VTSS_SPEED_2500M ? "2.5G" :
		 speed==VTSS_SPEED_5G ? "5G" :
		 speed==VTSS_SPEED_10G ? "10G" :
		 "???",
		 setup->fdx);

	vtss_log(VTSS_LOG_DEBUG,
		 "SWITCH: setup port, pause_obey=%d, pause_generate=%d, max_frame=%d",
		 setup->flowcontrol.obey,
		 setup->flowcontrol.generate,
		 setup->maxframelength);
	
	if (setup->maxframelength > VTSS_MAXFRAMELENGTH_MAX)
		return VTSS_INVALID_PARAMETER;
	
	/* Store the setup and determine if we are in jumbo mode */
	vtss_mac_state.setup[port_no] = *setup;
	vtss_mac_state.port_fc_rx[port_no] = MAKEBOOL01(setup->flowcontrol.obey);
	vtss_mac_state.port_fc_tx[port_no] = MAKEBOOL01(setup->flowcontrol.generate);
	jumbo_old = vtss_mac_state.jumbo;
	vtss_mac_state.jumbo = 0;
	for (port = VTSS_PORT_NO_START; port < VTSS_PORT_NO_END; port++) {
		if (!vtss_mac_state.port_map.vtss_port_unused[port] &&
			vtss_mac_state.setup[port].maxframelength > VTSS_MAXFRAMELENGTH_TAGGED) {
			/* Go to jumbo mode if any active port is above the tagged max length (VMAN) */
			vtss_mac_state.jumbo = 1;
			break;
		}
	}
	
	{
		vtss_chip_counters_t counters;
		
		VTSS_RC(vtss_port_counters_update(port_no));
		VTSS_RC(vtss_ll_port_speed_mode_gmii(port_no, setup));
		
		/* If CRC errors are generated by MAC configuration, all counter changes are ignored */
		vtss_ll_port_counters_get(port_no, &counters);
#ifdef CONFIG_VTSS_ARCH_HEATHROW
		if (counters.rx_crc_align_errors != vtss_mac_state.port_last_counters[port_no].rx_crc_align_errors)
			vtss_mac_state.port_last_counters[port_no] = counters;
#endif
	}
	
	/* Update all other active ports if jumbo mode changed */
	if (vtss_mac_state.jumbo != jumbo_old) {
		for (port = VTSS_PORT_NO_START; port < VTSS_PORT_NO_END; port++) {
			if (!vtss_mac_state.port_map.vtss_port_unused[port] && port != port_no) {
				VTSS_RC(vtss_ll_port_queue_setup(port));
			}
		}
	}
	return VTSS_OK;
}

static const char *vtss_stp_state_str(vtss_stp_state_t stp_state)
{
	return (stp_state==VTSS_STP_STATE_DISABLED ? "DISABLED" :
		stp_state==VTSS_STP_STATE_BLOCKING ? "BLOCKING" :
		stp_state==VTSS_STP_STATE_LISTENING ? "LISTENING" :
		stp_state==VTSS_STP_STATE_LEARNING ? "LEARNING" :
		stp_state==VTSS_STP_STATE_FORWARDING ? "FORWARDING" :
		stp_state==VTSS_STP_STATE_ENABLED ? "ENABLED" : "???");
}

/* Update source, destination and aggregation masks */
static vtss_rc vtss_update_masks(BOOL src_update, BOOL dest_update, BOOL aggr_update)
{
	vtss_port_no_t   i_port_no, e_port_no;
	vtss_poag_no_t   poag_no;
	vtss_pvlan_no_t  pvlan_no;
	BOOL             member[VTSS_PORT_ARRAY_SIZE];
	vtss_pgid_no_t   pgid_no;
	vtss_ac_no_t     ac_no;
	uint             aggr_count[VTSS_PORT_ARRAY_SIZE], aggr_index[VTSS_PORT_ARRAY_SIZE], n;
	
	/* Update source masks */
	for (i_port_no = VTSS_PORT_NO_START; src_update && i_port_no < VTSS_PORT_NO_END; i_port_no++) {
		/* Avoid setting up unmapped ports */
		if (vtss_mac_state.port_map.vtss_port_unused[i_port_no])
			continue;
		
		/* Exclude all ports by default */
		memset(member, 0, sizeof(member));
		
		if (VTSS_STP_FORWARDING(vtss_mac_state.stp_state[i_port_no]) &&
			vtss_mac_state.auth_state[i_port_no] == VTSS_AUTH_STATE_BOTH) {
			/* STP and Authentication state allow forwarding from port */
			
			/* PVLANs: Include members of the same PVLAN */
			for (pvlan_no = VTSS_PVLAN_NO_START; pvlan_no < VTSS_PVLAN_NO_END; pvlan_no++) {
				if (vtss_mac_state.pvlan_table[pvlan_no].member[i_port_no]) {
					/* The ingress port is a member of this PVLAN */
					for (e_port_no = VTSS_PORT_NO_START; e_port_no < VTSS_PORT_NO_END; e_port_no++) {
						if (vtss_mac_state.pvlan_table[pvlan_no].member[e_port_no]) {
							/* This port is also a member of the PVLAN */
							member[e_port_no] = MAKEBOOL01(1);
						}
					}
				}
			}
			
			/* Aggregations: Exclude members of the same aggregation */
			poag_no = vtss_mac_state.port_poag_no[i_port_no];
			for (e_port_no = VTSS_PORT_NO_START; e_port_no < VTSS_PORT_NO_END; e_port_no++) {
				if (vtss_mac_state.port_map.vtss_port_unused[e_port_no] || 
					vtss_mac_state.port_poag_no[e_port_no] == poag_no) {
					/* Exclude unmapped ports and members of the same aggregation */
					member[e_port_no] = MAKEBOOL01(0);
				}
			}
		}
		
		VTSS_RC(vtss_ll_src_table_write(i_port_no, member));
	} /* src_update */
	
	for (pgid_no = VTSS_PGID_START; dest_update && pgid_no < vtss_mac_state.pgid_end; pgid_no++) {
		if (vtss_mac_state.pgid_table[pgid_no].references > 0) {
			VTSS_RC(vtss_pgid_table_write(pgid_no));
		}
	}
	
	/* update aggregation masks */
	if (aggr_update) {
		/* Count number of operational ports and index of each port */
		for (i_port_no = VTSS_PORT_NO_START;
		     i_port_no < VTSS_PORT_NO_END; i_port_no++) {
			aggr_count[i_port_no] = 0;
			aggr_index[i_port_no] = 0;
			
			/* if port is not forwarding, continue */
			if (!VTSS_STP_FORWARDING(vtss_mac_state.stp_state[i_port_no]) ||
				vtss_mac_state.auth_state[i_port_no] == VTSS_AUTH_STATE_NONE)
				continue;
			
			poag_no = vtss_mac_state.port_poag_no[i_port_no];
			vtss_log(VTSS_LOG_DEBUG,
				 "SWITCH: STP forwarding, port=%d, poag=%d",
				 i_port_no, poag_no);
			for (e_port_no = VTSS_PORT_NO_START; e_port_no < VTSS_PORT_NO_END; e_port_no++) {
				if (vtss_mac_state.port_poag_no[e_port_no] == poag_no &&
					VTSS_STP_FORWARDING(vtss_mac_state.stp_state[e_port_no]) &&
					vtss_mac_state.auth_state[e_port_no] != VTSS_AUTH_STATE_NONE) {
					/* port is forwarding and member of the same aggregation */
					aggr_count[i_port_no]++;
					if (e_port_no < i_port_no)
						aggr_index[i_port_no]++;
				}
			}
		}
		
		for (ac_no = VTSS_AC_START; ac_no < VTSS_AC_END; ac_no++) {
			
			/* include one forwarding port from each aggregation */
			for (i_port_no = VTSS_PORT_NO_START; i_port_no < VTSS_PORT_NO_END; i_port_no++) {
				n = (aggr_index[i_port_no] + ac_no - VTSS_AC_START);
				member[i_port_no] = MAKEBOOL01(aggr_count[i_port_no] != 0 &&
					(n % aggr_count[i_port_no]) == 0);
				
				/* save the first aggregation member list as API state for frame Tx */
				if (ac_no == VTSS_AC_START)
					vtss_mac_state.aggr_member[i_port_no] = member[i_port_no];
			}
			
			/* write to aggregation table */
			VTSS_RC(vtss_ll_aggr_table_write(ac_no, member));
		}
		
#ifdef CONFIG_VTSS_ARCH_SPARX_28
		/* update port map table on aggregation changes */
		for (i_port_no = VTSS_PORT_NO_START;
		     i_port_no < VTSS_PORT_NO_END; i_port_no++) {
			vtss_port_no_t l_port_no = 0;
			
			for (e_port_no = VTSS_PORT_NO_START;
			     e_port_no < VTSS_PORT_NO_END; e_port_no++) {
				if (vtss_mac_state.port_poag_no[i_port_no] == vtss_mac_state.port_poag_no[e_port_no]) {
					/* The logical port is the first operational port in the aggregation */
					if (l_port_no == 0)
						l_port_no = e_port_no;
				}
			}
			/* if port down or no ports in aggregation are up, map to port itself */
			if (l_port_no == 0)
				l_port_no = i_port_no;
			VTSS_RC(vtss_ll_pmap_table_write(i_port_no, l_port_no));
		}
#endif
	}
	return VTSS_OK;
}

/* - Port Link Status ---------------------------------------------- */

/* Gather current status of a port within a generic status structure */
vtss_rc vtss_port_status_get(const vtss_port_no_t port_no,
                             vtss_port_status_t * const status)
{
	uint port_on_chip;
	
	port_on_chip = vtss_mac_state.port_map.chip_port[port_no];
	status->link_down = 0;
	status->link = 0;
	status->speed = VTSS_SPEED_UNDEFINED;
	status->fdx = 0;
	
	/* 1G port without TBI */
	VTSS_RC(vtss_phy_status_get(port_no, status));
	
	return VTSS_OK;
}

/* - Big Counters -------------------------------------------------- */

/* This is called regularly to update the big counters */
vtss_rc vtss_port_counters_update(const vtss_port_no_t port_no)
{
	vtss_port_no_t       port;
	int                  i;
	vtss_chip_counters_t counters;
	vtss_counter_t       *big, incr;
	vtss_chip_counter_t  *new, *old, n;
	uint                 ctr;
	
	/* Check that the port number is valid */
	if (!VTSS_PORT_IS_PORT(port_no)) {
		vtss_log(VTSS_LOG_ERR,
			 "SWITCH: illegal port, port=%d", port_no);
		return VTSS_INVALID_PARAMETER;
	}
	
	port = port_no;
	for (i = 0; i < 2; i++) {
		
		/* Get updated chip counters */
		VTSS_RC(vtss_ll_port_counters_get(port, &counters));
		
		new = (vtss_chip_counter_t *)&counters;
		old = (vtss_chip_counter_t *)&vtss_mac_state.port_last_counters[port];
		for (ctr = 0; ctr < sizeof(vtss_chip_counters_t) /
		     sizeof(vtss_chip_counter_t); ctr++) {
			/* Calculate increment taking wrapping into account */
#ifdef CONFIG_VTSS_ARCH_SPARX_28
			n = 32;
#else
			n = sig_bits[ctr];
#endif
			incr = (new[ctr] + (new[ctr] >= old[ctr] ? 0 : ((1 << n) - 1)) - old[ctr]);
			
			/* Store latest chip counter */
			old[ctr] = new[ctr];
			
			/* Update port counter */
			big = (vtss_counter_t *)&vtss_mac_state.poag_big_counters[port_no];
			big[ctr] += incr;
			
			/* If port is aggregated, update aggregation counter */
			if (VTSS_POAG_IS_AGGR(vtss_mac_state.port_poag_no[port_no])) {
				big = (vtss_counter_t *)&vtss_mac_state.poag_big_counters[vtss_mac_state.port_poag_no[port_no]];
				big[ctr] += incr;
			}
		}
		
		break;
	}
	return VTSS_OK;
}

vtss_rc vtss_poag_counters_clear(const vtss_poag_no_t poag_no)
{
	vtss_port_no_t port_no;
	uint           ctr;
	
	/* Check that the poag number is valid */
	if (!(VTSS_POAG_IS_POAG(poag_no))) {
		vtss_log(VTSS_LOG_ERR, "SWITCH: illegal poag, poag=%d", poag_no);
		return VTSS_INVALID_PARAMETER;
	}
	
	/* Update counters for poag and clear counter variables */
	for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
		if (port_no == poag_no || vtss_mac_state.port_poag_no[port_no] == poag_no) {
			VTSS_RC(vtss_port_counters_update(port_no));
			for (ctr = 0; ctr < sizeof(vtss_port_big_counters_t) / sizeof(vtss_counter_t); ctr++) {
				((vtss_counter_t *)(&(vtss_mac_state.poag_big_counters[poag_no])))[ctr] = 0;
			}
		}
	}
	
	return VTSS_OK;
}

/* This is used to retrieve the counters. */
vtss_rc vtss_poag_counters_get(const vtss_poag_no_t poag_no,
                               vtss_poag_counters_t *const big_counters)
{
	vtss_port_no_t port_no;
	vtss_port_big_counters_t *counters;
	
	/* Check that the poag number is valid */
	if (!(VTSS_POAG_IS_POAG(poag_no))) {
		vtss_log(VTSS_LOG_ERR, "SWITCH: illegal poag, poag=%d", poag_no);
		return VTSS_INVALID_PARAMETER;
	}
	
	/* Update counters for poag first */
	for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
		if (port_no == poag_no || vtss_mac_state.port_poag_no[port_no] == poag_no) {
			VTSS_RC(vtss_port_counters_update(port_no));
		}
	}
	
	counters = &vtss_mac_state.poag_big_counters[poag_no];
	
	/* Convert from chip specific counters to standard counters */
#ifdef CONFIG_VTSS_ARCH_HEATHROW
	/* RMON Rx counters */
	big_counters->rmon.rx_etherStatsDropEvents = counters->rx_drops;
	big_counters->rmon.rx_etherStatsOctets = counters->rx_octets;
	big_counters->rmon.rx_etherStatsPkts = counters->rx_packets;
#ifdef VTSS_FEATURE_PORT_CNT_PKT_CAST
	big_counters->rmon.rx_etherStatsBroadcastPkts = counters->rx_broadcasts;
	big_counters->rmon.rx_etherStatsMulticastPkts = counters->rx_multicasts;
#endif
#ifdef VTSS_FEATURE_PORT_CNT_RMON_ADV
	big_counters->rmon.rx_etherStatsCRCAlignErrors = counters->rx_crc_align_errors;
	big_counters->rmon.rx_etherStatsUndersizePkts = counters->rx_shorts;
	big_counters->rmon.rx_etherStatsOversizePkts = counters->rx_longs;
	big_counters->rmon.rx_etherStatsFragments = counters->rx_fragments;
	big_counters->rmon.rx_etherStatsJabbers = counters->rx_jabbers;
	big_counters->rmon.rx_etherStatsPkts64Octets = counters->rx_64;
	big_counters->rmon.rx_etherStatsPkts65to127Octets = counters->rx_65_127;
	big_counters->rmon.rx_etherStatsPkts128to255Octets = counters->rx_128_255;
	big_counters->rmon.rx_etherStatsPkts256to511Octets = counters->rx_256_511;
	big_counters->rmon.rx_etherStatsPkts512to1023Octets = counters->rx_512_1023;
#endif
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	big_counters->rmon.rx_etherStatsPkts1024to1518Octets = counters->rx_1024_1526;
	big_counters->rmon.rx_etherStatsPkts1519toMaxOctets = counters->rx_1527_max;
#endif /* SPARX_28 */
	
	/* RMON Tx counters */
#ifdef CONFIG_VTSS_ARCH_SPARX
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	big_counters->rmon.tx_etherStatsDropEvents = (counters->tx_fifo_drops + counters->tx_aging);
#else
	big_counters->rmon.tx_etherStatsDropEvents = counters->tx_drops;
#endif
#else
	big_counters->rmon.tx_etherStatsDropEvents = counters->tx_fifo_drops;
#endif
	big_counters->rmon.tx_etherStatsOctets = counters->tx_octets;
	big_counters->rmon.tx_etherStatsPkts = counters->tx_packets ;
#ifdef VTSS_FEATURE_PORT_CNT_PKT_CAST
	big_counters->rmon.tx_etherStatsBroadcastPkts = counters->tx_broadcasts;
	big_counters->rmon.tx_etherStatsMulticastPkts = counters->tx_multicasts;
#endif
#ifdef VTSS_FEATURE_PORT_CNT_RMON_ADV
	big_counters->rmon.tx_etherStatsCollisions = counters->tx_collisions;
	big_counters->rmon.tx_etherStatsPkts64Octets = counters->tx_64;
	big_counters->rmon.tx_etherStatsPkts65to127Octets = counters->tx_65_127;
	big_counters->rmon.tx_etherStatsPkts128to255Octets = counters->tx_128_255;
	big_counters->rmon.tx_etherStatsPkts256to511Octets = counters->tx_256_511;
	big_counters->rmon.tx_etherStatsPkts512to1023Octets = counters->tx_512_1023;
#endif
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	big_counters->rmon.tx_etherStatsPkts1024to1518Octets = counters->tx_1024_1526;
	big_counters->rmon.tx_etherStatsPkts1519toMaxOctets = counters->tx_1527_max;
#endif /* SPARX_28 */
	
	/* Interfaces Group Rx counters */
	/* ifInOctets includes bytes in bad frames */
	big_counters->if_group.ifInOctets = counters->rx_octets;
#ifdef VTSS_FEATURE_PORT_CNT_PKT_CAST
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	big_counters->if_group.ifInUcastPkts = counters->rx_unicast;
#endif
	big_counters->if_group.ifInMulticastPkts = counters->rx_multicasts;
	big_counters->if_group.ifInBroadcastPkts = counters->rx_broadcasts;
	big_counters->if_group.ifInNUcastPkts = (counters->rx_multicasts + 
						 counters->rx_broadcasts);
#endif
	big_counters->if_group.ifInDiscards = big_counters->rmon.rx_etherStatsDropEvents;
	big_counters->if_group.ifInErrors = (counters->rx_crc_align_errors + 
		counters->rx_shorts + 
		counters->rx_longs +
		counters->rx_fragments +
		counters->rx_jabbers);
	
	/* Interfaces Group Tx counters */
	big_counters->if_group.ifOutOctets = counters->tx_octets;
#ifdef VTSS_FEATURE_PORT_CNT_PKT_CAST
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	big_counters->if_group.ifOutUcastPkts = counters->tx_unicast;
#else
	big_counters->if_group.ifOutUcastPkts = subtract_floor0(
		counters->tx_packets, /*-*/
		(counters->tx_multicasts +
		counters->tx_broadcasts) );
#endif
	big_counters->if_group.ifOutMulticastPkts = counters->tx_multicasts;
	big_counters->if_group.ifOutBroadcastPkts = counters->tx_broadcasts;
	big_counters->if_group.ifOutNUcastPkts = (counters->tx_multicasts + 
		counters->tx_broadcasts);
#endif
	big_counters->if_group.ifOutDiscards = big_counters->rmon.tx_etherStatsDropEvents;
	/* Late/excessive collisions and aged frames */
	big_counters->if_group.ifOutErrors = counters->tx_drops; 
#endif
	
#ifdef VTSS_FEATURE_PORT_CNT_BRIDGE
	/* Bridge counters */
	big_counters->bridge.dot1dTpPortInDiscards = counters->rx_local_drops;
#endif
#ifdef VTSS_FEATURE_PORT_CNT_QOS
	/* Prioprietary counters */
	{
		uint i;
		
		for (i = 0; i < VTSS_PRIOS; i++) {
			big_counters->prop.rx_prio[i] = counters->rx_class[i];
			big_counters->prop.tx_prio[i] = counters->tx_class[i];
		}
	}
#endif
	return VTSS_OK;
}

/* - Quality of Service -------------------------------------------- */
vtss_rc vtss_qos_prios_set(const vtss_prio_t prios)
{
	vtss_rc        ex = VTSS_OK;
	BOOL           warning = 0;
	vtss_port_no_t port_no;
	
	if (prios < VTSS_PRIO_START ||
		prios>=VTSS_PRIO_END || prios==3 || (prios>=5 && prios<=7)) {
		vtss_log(VTSS_LOG_ERR, "SWITCH: illegal prios, prios=%d", prios);
		return VTSS_UNSPECIFIED_ERROR;
	}
	
	if (prios != vtss_mac_state.prios) {
		vtss_mac_state.prios = prios;
		
		for (port_no = VTSS_PORT_NO_START;
		     port_no < VTSS_PORT_NO_END; port_no++) {
			if (!vtss_mac_state.port_map.vtss_port_unused[port_no]) {
				if ((ex = vtss_ll_port_qos_setup_set(port_no, &vtss_mac_state.qos[port_no])) < VTSS_WARNING)
					return ex;
				if (ex < 0) warning = 1;
				
				if ((ex = vtss_ll_port_queue_setup(port_no)) < VTSS_WARNING)
					return ex;
				if (ex < 0) warning = 1;
				
				if ((ex = vtss_ll_port_queue_enable(port_no, 1)) < VTSS_WARNING)
					return ex;
				if (ex < 0) warning = 1;
			}
		}
		VTSS_RC(vtss_ll_qos_setup_set(&vtss_mac_state.qos_setup));
	}
	if (warning)
		ex = VTSS_WARNING;
	
	return ex;
}

vtss_rc vtss_port_qos_get(const vtss_port_no_t port_no,
                          vtss_port_qos_setup_t * const qos)
{
	/* Check that the port number is valid */
	if (!VTSS_PORT_IS_PORT(port_no)) {
		vtss_log(VTSS_LOG_ERR, "SWITCH: illegal port, port=%d", port_no);
		return VTSS_INVALID_PARAMETER;
	}
	
	*qos = vtss_mac_state.qos[port_no];
	return VTSS_OK;
}

vtss_rc vtss_port_qos_set(const vtss_port_no_t port_no,
                          const vtss_port_qos_setup_t * const qos)
{
	/* Check that the port number is valid */
	if (!VTSS_PORT_IS_PORT(port_no)) {
		vtss_log(VTSS_LOG_ERR, "SWITCH: illegal port, port=%d", port_no);
		return VTSS_INVALID_PARAMETER;
	}
	
	vtss_mac_state.qos[port_no] = *qos;
#ifdef VTSS_FEATURE_QOS_WFQ_PORT
	/* Use WFQ water marks on all ports if any port has WFQ enabled */
	{
		vtss_port_no_t port;
		BOOL           wfq_old;
		
		wfq_old = vtss_mac_state.wfq;
		vtss_mac_state.wfq = 0;
		for (port = VTSS_PORT_NO_START; port < VTSS_PORT_NO_END; port++) {
			if (!vtss_mac_state.port_map.vtss_port_unused[port] && vtss_mac_state.qos[port].wfq_enable)
				vtss_mac_state.wfq = 1;
		}
		if (vtss_mac_state.wfq != wfq_old) {
			for (port = VTSS_PORT_NO_START; port < VTSS_PORT_NO_END; port++) {
				if (!vtss_mac_state.port_map.vtss_port_unused[port]) {
					VTSS_RC(vtss_ll_port_queue_setup(port));
				}
			}
		}
	}
#endif
	return vtss_ll_port_qos_setup_set(port_no, qos);
}

/* Get QoS setup for switch */
vtss_rc vtss_qos_setup_get(vtss_qos_setup_t * const qos)
{
	*qos = vtss_mac_state.qos_setup;
	return VTSS_OK;
}

/* Set QoS setup for switch */
vtss_rc vtss_qos_setup_set(const vtss_qos_setup_t * const qos)
{
	vtss_mac_state.qos_setup = *qos;
	return vtss_ll_qos_setup_set(qos);
}

/* - QoS Control Lists --------------------------------------------- */
#ifdef VTSS_FEATURE_QCL_PORT
vtss_rc vtss_qce_add(const vtss_qcl_id_t         qcl_id,
                     const vtss_qce_id_t         qce_id,
                     const vtss_qce_t    * const qce)
{
	return vtss_ll_qce_add(qcl_id, qce_id, qce);
}

vtss_rc vtss_qce_del(const vtss_qcl_id_t  qcl_id,
                     const vtss_qce_id_t  qce_id)
{
	return vtss_ll_qce_del(qcl_id, qce_id);
}
#endif

/* ================================================================= *
 *  Security
 * ================================================================= */

/* - Port Based Network Access Control, 802.1X --------------------- */
vtss_rc vtss_port_auth_state_set(const vtss_port_no_t    port_no,
                                 const vtss_auth_state_t auth_state)
{
	BOOL dest_update = 0;
	
	/* Check that the port number is valid */
	if (!(VTSS_PORT_IS_PORT(port_no))) {
		vtss_log(VTSS_LOG_ERR,
			 "SWITCH: illegal port, port=%d", port_no);
		return VTSS_INVALID_PARAMETER;
	}
	
	/* Check that the Authentication state is valid */
	switch (auth_state) {
	case VTSS_AUTH_STATE_NONE:
	case VTSS_AUTH_STATE_EGRESS:
	case VTSS_AUTH_STATE_BOTH:
		break;
	default:
		vtss_log(VTSS_LOG_ERR,
			 "SWITCH: illegal auth state, state=%d", auth_state);
		return VTSS_INVALID_PARAMETER;
		break;
	}
	
	vtss_mac_state.auth_state[port_no] = auth_state; 
	return vtss_update_masks(1, dest_update, 1);
}




/* ================================================================= *
 *  Layer 2
 * ================================================================= */

/* - Port STP State ------------------------------------------------ */

vtss_rc vtss_port_stp_state_get(const vtss_port_no_t     port_no,
                                vtss_stp_state_t * const stp_state)
{
	/* Check that the port number is valid */
	if (!(VTSS_PORT_IS_PORT(port_no))) {
		vtss_log(VTSS_LOG_ERR,
			 "SWITCH: illegal port, port=%d", port_no);
		return VTSS_INVALID_PARAMETER;
	}
	
	*stp_state = vtss_mac_state.stp_state[port_no];
	vtss_log(VTSS_LOG_DEBUG,
		 "SWITCH: port_no=%d, stp_state=%s",
		 port_no, vtss_stp_state_str(*stp_state));
	return VTSS_OK;
}

vtss_rc vtss_port_stp_state_set(const vtss_port_no_t port_no,
                                const vtss_stp_state_t stp_state)
{
	BOOL dest_update = 0;
	
	/* Check that the port number is valid */
	if (!(VTSS_PORT_IS_PORT(port_no))) {
		vtss_log(VTSS_LOG_ERR,
			"SWITCH: illegal port, port=%d", port_no);
		return VTSS_INVALID_PARAMETER;
	}
	
	/* Check that the STP state is valid */
	switch (stp_state) {
	case VTSS_STP_STATE_DISABLED:
	case VTSS_STP_STATE_BLOCKING:
	case VTSS_STP_STATE_LISTENING:
	case VTSS_STP_STATE_LEARNING:
	case VTSS_STP_STATE_FORWARDING:
	case VTSS_STP_STATE_ENABLED:
		break;
	default:
		vtss_log(VTSS_LOG_ERR,
			 "SWITCH: illegal stp state, stp_state=%d", stp_state);
		return VTSS_INVALID_PARAMETER;
		break;
	}
	
	vtss_mac_state.stp_state[port_no] = stp_state; 
	VTSS_RC(vtss_ll_port_queue_enable(port_no, 1));
	return vtss_update_masks(1, dest_update, 1);
}

vtss_rc vtss_port_mstp_state_set(const vtss_port_no_t port_no, 
                                 const vtss_msti_t msti,
                                 const vtss_mstp_state_t mstp_state)
{
	/* Check that the poag number is valid */
	if (!(VTSS_PORT_IS_PORT(port_no))) {
		vtss_log(VTSS_LOG_ERR,
			 "SWITCH: illegal port, port=%d", port_no);
		return VTSS_INVALID_PARAMETER;
	}
	
	/* Check that the MSTP instance is valid */
	if (msti < VTSS_MSTI_START || msti >= vtss_mac_state.msti_end) {
		vtss_log(VTSS_LOG_ERR,
			 "SWITCH: illegal msti, msti=%d", msti);
		return VTSS_INVALID_PARAMETER;
	}
	
	/* Check that the MSTP state is valid */
	if (mstp_state!=VTSS_MSTP_STATE_DISCARDING && 
	    mstp_state!=VTSS_MSTP_STATE_LEARNING && 
	    mstp_state!=VTSS_MSTP_STATE_FORWARDING) {
		vtss_log(VTSS_LOG_ERR,
			 "SWITCH: illegal state, state=%d", mstp_state);
		return VTSS_INVALID_PARAMETER;
	}
	
	vtss_mac_state.mstp_table[msti].state[port_no] = mstp_state;
	
	return vtss_ll_mstp_table_write(port_no, msti, mstp_state);
}

vtss_rc vtss_mstp_vlan_set(const vtss_vid_t vid, const vtss_msti_t msti)
{
	/* Check that the MSTP instance is valid */
	if (msti < VTSS_MSTI_START || msti >= vtss_mac_state.msti_end) {
		vtss_log(VTSS_LOG_ERR,
			"SWITCH: illegal msti, msti=%d", msti);
		return VTSS_INVALID_PARAMETER;
	}
	
	vtss_mac_state.vlan_table[vid].msti = msti;
	return vtss_ll_vlan_table_mstp_set(vid, msti);
}

/* - Learning options ---------------------------------------------- */

#ifdef VTSS_FEATURE_LEARN_PORT
vtss_rc vtss_learn_port_mode_set(const vtss_port_no_t      port_no,
                                 vtss_learn_mode_t * const learn_mode)
{
	/* Check that the poag number is valid */
	if (!(VTSS_POAG_IS_PORT(port_no))) {
		vtss_log(VTSS_LOG_ERR,
			 "SWITCH: illegal port, port=%d", port_no);
		return VTSS_INVALID_PARAMETER;
	}
	return vtss_ll_learn_port_mode_set(port_no, learn_mode);
}
#endif

#ifdef VTSS_FEATURE_LEARN_SWITCH
vtss_rc vtss_learn_mode_set(vtss_learn_mode_t * const learn_mode)
{
	return vtss_ll_learn_mode_set(learn_mode);
}
#endif

/* - VLAN Port Mode ------------------------------------------------ */
/* Get VLAN mode for port or aggregation */
vtss_rc vtss_vlan_port_mode_get(const vtss_port_no_t port_no,
                                vtss_vlan_port_mode_t * const vlan_mode)
{
	/* Check that the poag number is valid */
	if (!(VTSS_POAG_IS_PORT(port_no))) {
		vtss_log(VTSS_LOG_ERR,
			 "SWITCH: illegal port, port=%d", port_no);
		return VTSS_INVALID_PARAMETER;
	}
	
	*vlan_mode = vtss_mac_state.vlan_port_table[port_no];
	return VTSS_OK;
}

vtss_rc vtss_vlan_port_mode_set(const vtss_port_no_t port_no,
                                const vtss_vlan_port_mode_t * const vlan_mode)
{
	/* Check that the poag number is valid */
	if (!(VTSS_PORT_IS_PORT(port_no))) {
		vtss_log(VTSS_LOG_ERR,
			 "SWITCH: illegal port, port=%d", port_no);
		return VTSS_INVALID_PARAMETER;
	}
	
	vtss_mac_state.vlan_port_table[port_no] = *vlan_mode;
	return vtss_ll_vlan_port_mode_set(port_no, vlan_mode);
}

/* - VLAN Table ---------------------------------------------------- */

vtss_rc vtss_vlan_port_members_get(const vtss_vid_t vid,
                                   BOOL member[VTSS_PORT_ARRAY_SIZE])
{
	/* Return internal state information */
	memcpy(member, vtss_mac_state.vlan_table[vid].member,
	       sizeof(vtss_mac_state.vlan_table[vid].member));
	return VTSS_OK;
}

vtss_rc vtss_vlan_port_members_set(const vtss_vid_t vid, BOOL member[VTSS_PORT_ARRAY_SIZE])
{
	vtss_port_no_t port_no;
	
	VTSS_RC(vtss_ll_vlan_table_write(vid, member));
	
	memcpy(vtss_mac_state.vlan_table[vid].member, member, VTSS_PORT_ARRAY_SIZE*sizeof(BOOL)); 
	for (port_no = VTSS_PORT_NO_START;
	     port_no < VTSS_PORT_NO_END; port_no++) {
		if (member[port_no])
			break;
	}
	vtss_mac_state.vlan_table[vid].enabled = (port_no != VTSS_PORT_NO_END);
	return VTSS_OK;
}

#ifdef VTSS_FEATURE_ISOLATED_PORT

/* - Port Isolation------------------------------------------------- */

vtss_rc vtss_isolated_vlan_set(const vtss_vid_t vid,
                               const BOOL       isolated)
{
	vtss_vlan_entry_t *vlan_entry;
	
	vlan_entry = &vtss_mac_state.vlan_table[vid];
	vlan_entry->isolated = isolated;
	return vtss_ll_vlan_table_write(vid, vlan_entry->member);
}

vtss_rc vtss_isolated_ports_set(const BOOL member[VTSS_PORT_ARRAY_SIZE])
{
	return vtss_ll_isolated_ports_set(member);
}
#endif


/* - Private VLAN (PVLAN) ------------------------------------------ */

vtss_rc vtss_pvlan_port_members_get(const vtss_pvlan_no_t pvlan_no,
                                    BOOL member[VTSS_PORT_ARRAY_SIZE])
{
	/* Check PVLAN number */
	if (pvlan_no < VTSS_PVLAN_NO_START || pvlan_no >= VTSS_PVLAN_NO_END) {
		vtss_log(VTSS_LOG_ERR,
			 "SWITCH: illegal pvlan, pvlan=%d", pvlan_no);
		return VTSS_INVALID_PARAMETER;
	}
	
	/* Return internal state information */
	memcpy(member, vtss_mac_state.pvlan_table[pvlan_no].member, sizeof(vtss_mac_state.pvlan_table[pvlan_no].member));
	return VTSS_OK;
}

vtss_rc vtss_pvlan_port_members_set(const vtss_pvlan_no_t pvlan_no, 
                                     const BOOL member[VTSS_PORT_ARRAY_SIZE])
{
	/* Check PVLAN number */
	if (pvlan_no < VTSS_PVLAN_NO_START || pvlan_no >= VTSS_PVLAN_NO_END) {
		vtss_log(VTSS_LOG_ERR,
			 "SWITCH: illegal pvlan, pvlan=%d", pvlan_no);
		return VTSS_INVALID_PARAMETER;
	}
	
	memcpy(vtss_mac_state.pvlan_table[pvlan_no].member, member, VTSS_PORT_ARRAY_SIZE*sizeof(BOOL));
	return vtss_update_masks(1, 0, 0);
}


/* - Aggregation --------------------------------------------------- */

vtss_rc vtss_aggr_port_members_get(const vtss_poag_no_t poag_no,
                                   BOOL member[VTSS_PORT_ARRAY_SIZE])
{
	vtss_port_no_t port_no;
	
	/* Check that the poag number is valid */
	if (!(VTSS_POAG_IS_POAG(poag_no))) {
		vtss_log(VTSS_LOG_ERR,
			 "SWITCH: illegal poag, poag=%d", poag_no);
		return VTSS_INVALID_PARAMETER;
	}
	
	for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
		member[port_no] = MAKEBOOL01(vtss_mac_state.port_poag_no[port_no] == poag_no);
	}
	return VTSS_OK;
}

vtss_rc vtss_aggr_port_members_set(const vtss_poag_no_t poag_no,
                                   const BOOL member[VTSS_PORT_ARRAY_SIZE])
{
	vtss_port_no_t port_no;
	
	/* Check that the poag number is valid */
	if (!(VTSS_POAG_IS_POAG(poag_no))) {
		vtss_log(VTSS_LOG_ERR,
			 "SWITCH: illegal poag, poag=%d", poag_no);
		return VTSS_INVALID_PARAMETER;
	}
	
	if (VTSS_POAG_IS_PORT(poag_no)) {
		/* When the PoAg is a Port, the Aggregation must be itself only. */
		for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
			if (port_no == poag_no) {
				if (!member[port_no]) {
					vtss_log(VTSS_LOG_ERR,
						 "SWITCH: poag is port, but not enabled in member set, poag_no=%d",
						 poag_no);
					return VTSS_AGGR_INVALID;
				}
			} else {
				if (member[port_no]) {
					vtss_log(VTSS_LOG_ERR,
						 "SWITCH: port is enabled in member set, port=%d",
						 poag_no);
					return VTSS_AGGR_INVALID;
				}
			}
		}
	}
	
	for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
		if (member[port_no]) {
			if (vtss_mac_state.port_poag_no[port_no] != poag_no)  {
				/* The port is now member of this aggregation */
				vtss_mac_state.port_poag_no[port_no] = poag_no;
				/* Note that a port can be moved directly from one aggregation to another */
			}
		} else {
			if (vtss_mac_state.port_poag_no[port_no] == poag_no) {
				/* The port is no longer a member of this aggregation */
				vtss_mac_state.port_poag_no[port_no] = port_no;
			}
		}
	}
#if (CONFIG_VTSS_MAC_NEXT_MAX != 0)
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	vtss_mac_table_update();
#endif
#endif
	
	return vtss_update_masks(1, 1, 1);
}

vtss_poag_no_t vtss_aggr_port_member_get(const vtss_port_no_t port_no)
{
	/* Check that the port number is valid */
	if (!VTSS_PORT_IS_PORT(port_no)) {
		vtss_log(VTSS_LOG_ERR,
			 "SWITCH: illegal port, port=%d", port_no);
		return VTSS_INVALID_PARAMETER;
	}
	return vtss_mac_state.port_poag_no[port_no];
}

vtss_rc vtss_aggr_port_member_set(const vtss_port_no_t port_no,
                                  const vtss_poag_no_t poag_no)
{

	/* Check that the port number is valid */
	if (!VTSS_PORT_IS_PORT(port_no)) {
		vtss_log(VTSS_LOG_ERR,
			 "SWITCH: illegal port, port=%d", port_no);
		return VTSS_INVALID_PARAMETER;
	}
	
	/* Check that the poag number is valid */
	if (!VTSS_POAG_IS_POAG(poag_no)) {
		vtss_log(VTSS_LOG_ERR,
			 "SWITCH: illegal port, port=%d", port_no);
		return VTSS_INVALID_PARAMETER;
	}
	
	/* If the poag number is a port number, it must be itself */
	if (VTSS_POAG_IS_PORT(poag_no) && poag_no != port_no) {
		vtss_log(VTSS_LOG_ERR,
			 "SWITCH: port is port and poag is different, port_no=%d, poag=%d",
			 port_no, poag_no);
		return VTSS_INVALID_PARAMETER;
	}
	
	if (vtss_mac_state.port_poag_no[port_no] == poag_no) 
		return VTSS_OK; /* No change */
	
	/* Note that a port can be moved directly from one aggregation to another */
	vtss_mac_state.port_poag_no[port_no] = poag_no;
#if (CONFIG_VTSS_MAC_NEXT_MAX != 0)
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	vtss_mac_table_update();
#endif
#endif
	return vtss_update_masks(1, 1, 1);
}

vtss_rc vtss_aggr_mode_set(const vtss_aggr_mode_t * const mode)
{
	return vtss_ll_aggr_mode_set(mode);
}

/* - Mirroring ----------------------------------------------------- */
vtss_rc vtss_mirror_monitor_port_set(const vtss_port_no_t port_no)
{
	/* Check that the port number is valid */
	if (port_no != 0 && !VTSS_PORT_IS_PORT(port_no)) {
		vtss_log(VTSS_LOG_ERR,
			 "SWITCH: illegal port, port=%d", port_no);
		return VTSS_INVALID_PARAMETER;
	}
	vtss_mac_state.mirror_port = port_no;
	return vtss_ll_mirror_port_set(port_no);
}

vtss_rc vtss_mirror_ingress_ports_set(const BOOL member[VTSS_PORT_ARRAY_SIZE])
{
	vtss_port_no_t port_no;
	
	for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
		VTSS_RC(vtss_ll_src_mirror_set(port_no, member[port_no]));
		vtss_mac_state.mirror_ingress[port_no] = member[port_no];
	}
	return VTSS_OK;
}

vtss_rc vtss_mirror_egress_ports_set(const BOOL member[VTSS_PORT_ARRAY_SIZE])
{
	vtss_port_no_t port_no;
	
	/* Enable mirroring in egress mirror mask */
	for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
#ifdef CONFIG_VTSS_ARCH_SPARX_28
		VTSS_RC(vtss_ll_dst_mirror_set(port_no, member[port_no]));
#endif /* SPARX_28 */
		vtss_mac_state.mirror_egress[port_no] = member[port_no];
	}
	return VTSS_OK;
}

vtss_rc vtss_ipv4_mc_flood_mask_set(const BOOL member[VTSS_PORT_ARRAY_SIZE])
{
	return vtss_ll_ipmc_flood_mask_set(member);
}
