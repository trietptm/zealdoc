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

#include "vtss_priv.h"
#include "vtss_grocx.h"

#ifdef CONFIG_VTSS_GROCX

static int vtss_opt_router_wlan_rgmii = 0;
static int vtss_opt_port_count = 0;
static int vtss_opt_router_port_wan = 0;

/* - Star switch specific ------------------------------------------ */
/* device name to do ioctl */
static char vtss_ioctl_dev[] = "eth0";

/* socket file descriptor to do ioctl */
static int vtss_grx_ioctl_fd;

static ui_schema_t vtss_grocx_schema[] = {
	{ UI_TYPE_CLASS, UI_FLAG_SINGLE | UI_FLAG_EXTERNAL,
	  UI_TYPE_STRING, NULL, NULL,
	  ".switch.grocx", "grocx", "Vitesse G-Roc'X chip" },
	{ UI_TYPE_CLASS, UI_FLAG_SINGLE | UI_FLAG_EXTERNAL,
	  UI_TYPE_STRING, NULL, NULL,
	  ".switch.grocx.mib", "mib", "Vitesse G-Roc'X MIB" },
	{ UI_TYPE_CLASS, UI_FLAG_SINGLE | UI_FLAG_EXTERNAL,
	  UI_TYPE_STRING, NULL, NULL,
	  ".switch.grocx.pvlan", "pvlan", "Vitesse G-Roc'X V-LAN" },
	{ UI_TYPE_CLASS, UI_FLAG_SINGLE | UI_FLAG_EXTERNAL,
	  UI_TYPE_STRING, NULL, NULL,
	  ".switch.grocx.vlan", "vlan", "Vitesse G-Roc'X V-LAN" },
	{ UI_TYPE_NONE },
};

vtss_rc vtss_grocx_router_packet_counter_clear(vtss_grx_port_counter_t * ctl)
{
	struct ifreq ifr;
	int err;
	
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, vtss_ioctl_dev);
	
	ifr.ifr_data = (caddr_t)ctl;
	err = ioctl(vtss_grx_ioctl_fd, SIOCDEVPRIVATE, (unsigned long)&ifr);
	if (err < 0) {
		vtss_log(VTSS_LOG_ERR,
			 "GROCX: ioctl(SIOCDEVPRIVATE - port_counter) failed");
		return VTSS_FATAL_ERROR;
	}
	return VTSS_OK;
}

vtss_rc vtss_grocx_router_packet_counter_get(vtss_grx_port_counter_t * ctl)
{
	struct ifreq ifr;
	int err;
	
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, vtss_ioctl_dev);
	
	ifr.ifr_data = (caddr_t)ctl;
	err = ioctl(vtss_grx_ioctl_fd, SIOCDEVPRIVATE, (unsigned long)&ifr);
	if (err < 0) {
		vtss_log(VTSS_LOG_ERR,
			 "GROCX: ioctl(SIOCDEVPRIVATE - port_counter) failed");
		return VTSS_FATAL_ERROR;
	}
	return VTSS_OK;
}

vtss_rc vtss_grocx_router_vlan_control(vtss_grx_vlan_ctl_t *vlan_ctl)
{
	struct ifreq ifr;
	int err;
	
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, vtss_ioctl_dev);
	
	ifr.ifr_data = (caddr_t)vlan_ctl;
	err = ioctl(vtss_grx_ioctl_fd, SIOCDEVPRIVATE, (unsigned long)&ifr);
	if (err < 0) {
		vtss_log(VTSS_LOG_ERR,
			 "GROCX: ioctl(SIOCDEVPRIVATE - vlan) failed");
		return VTSS_FATAL_ERROR;
	}
	return VTSS_OK;
}

vtss_rc vtss_grocx_router_arl_table_control(vtss_grx_arl_table_t *arl_table)
{
	struct ifreq ifr;
	int err;
	
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, vtss_ioctl_dev);
	ifr.ifr_data = (caddr_t)arl_table;
	
	err = ioctl(vtss_grx_ioctl_fd, SIOCDEVPRIVATE, (unsigned long)&ifr);
	if (err < 0) {
		vtss_log(VTSS_LOG_ERR,
			 "GROCX: ioctl(SIOCDEVPRIVATE - arl_table) failed");
		return VTSS_FATAL_ERROR;
	}
	return VTSS_OK;
}

vtss_rc vtss_grocx_router_mac1_control(vtss_grx_mac1_control_t * mac1_control)
{
	struct ifreq ifr;
	int err;
	
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, vtss_ioctl_dev);
	
	ifr.ifr_data = (caddr_t)mac1_control;
	
	err = ioctl(vtss_grx_ioctl_fd, SIOCDEVPRIVATE, (unsigned long)&ifr);
	if (err < 0) {
		vtss_log(VTSS_LOG_ERR,
			 "GROCX: ioctl(SIOCDEVPRIVATE - mac1) failed");
		return VTSS_FATAL_ERROR;
	}
	return VTSS_OK;
}

vtss_rc vtss_grocx_router_qos_control(vtss_grx_qos_ctl_t * qos_ctl)
{
	struct ifreq ifr;
	int err;
	
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, vtss_ioctl_dev);
	ifr.ifr_data = (caddr_t)qos_ctl;
	
	err = ioctl(vtss_grx_ioctl_fd, SIOCDEVPRIVATE, (unsigned long)&ifr);
	if (err < 0) {
		vtss_log(VTSS_LOG_ERR,
			 "GROCX: ioctl(SIOCDEVPRIVATE - qos) failed");
		return VTSS_FATAL_ERROR;
	}
	return VTSS_OK;
}

vtss_rc vtss_grocx_router_egress_rate_limit_control(vtss_grx_egress_rate_limit_ctl_t *egress_rate_limit_ctl)
{
	struct ifreq ifr;
	int err;
	
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, vtss_ioctl_dev);
	ifr.ifr_data = (caddr_t)egress_rate_limit_ctl;
	
	err = ioctl(vtss_grx_ioctl_fd, SIOCDEVPRIVATE, (unsigned long)&ifr);
	if (err < 0) {
		vtss_log(VTSS_LOG_ERR,
			 "GROCX: ioctl(SIOCDEVPRIVATE - egress_rate_limit) failed");
		return VTSS_FATAL_ERROR;
	}
	return VTSS_OK;
}

vtss_rc vtss_grocx_router_storm_control(vtss_grx_storm_ctl_t * storm_ctl)
{
	struct ifreq ifr;
	int err;
	
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, vtss_ioctl_dev);
	ifr.ifr_data = (caddr_t)storm_ctl;
	
	err = ioctl(vtss_grx_ioctl_fd, SIOCDEVPRIVATE, (unsigned long)&ifr);
	if (err < 0) {
		vtss_log(VTSS_LOG_ERR,
			 "GROCX: ioctl(SIOCDEVPRIVATE - storm) failed");
		return VTSS_FATAL_ERROR;
	}
	return VTSS_OK;
}

/*  Enable/Disable unknown VLAN trap to CPU port */
vtss_rc vtss_grocx_router_vlan_trap(const BOOL enable)
{
	struct ifreq ifr;
	vtss_grx_vlan_trap_ctl_t trap_ctl;
	int err;
	
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, vtss_ioctl_dev);
	
	trap_ctl.cmd = VTSS_GRX_IOCTL_VLAN_TRAP;
	trap_ctl.enable = enable;
	
	ifr.ifr_data = (caddr_t)&trap_ctl;
	err = ioctl(vtss_grx_ioctl_fd, SIOCDEVPRIVATE, (unsigned long)&ifr);
	if (err < 0) {
		vtss_log(VTSS_LOG_ERR,
			 "GROCX: ioctl(SIOCDEVPRIVATE - vlan_trap) failed");
		return VTSS_FATAL_ERROR;
	}
	return VTSS_OK;
}

/* - wrapper for vtss_grocx_* ------------------------------------------ */

/* create a socket for ioctl */
vtss_rc vtss_grocx_router_ioctl_start(void)
{
	vtss_grx_ioctl_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (vtss_grx_ioctl_fd < 0) {
		vtss_log(VTSS_LOG_ERR,
			 "GROCX: socket(AF_INET, SOCK_DGRAM) failed");
		return VTSS_FATAL_ERROR;
	}
	return VTSS_OK;
}

#if 0
static vtss_rc vtss_grocx_router_ioctl_stop(void)
{
	close(vtss_grx_ioctl_fd);
	return VTSS_OK;
}
#endif

static vtss_rc vtss_grocx_router_start(void)
{
	struct ifreq ifr;
	int err;
	vtss_grx_mac1_control_t mac1_control;
	
	vtss_grocx_router_ioctl_start();
	
	mac1_control.cmd = VTSS_GRX_INIT;
	
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, vtss_ioctl_dev);
	
	ifr.ifr_data = (caddr_t)&mac1_control;
	
	err = ioctl(vtss_grx_ioctl_fd, SIOCDEVPRIVATE, (unsigned long)&ifr);
	if (err < 0) {
		vtss_log(VTSS_LOG_ERR,
			 "GROCX: ioctl(SIOCDEVPRIVATE - init) failed");
		return VTSS_FATAL_ERROR;
	}
	return VTSS_OK;
}

static vtss_rc vtss_grocx_router_port_setup(const vtss_grocx_port_conf_t * const setup)
{
	vtss_grx_mac1_control_t ctl;
	
	ctl.an = setup->autoneg;
	switch (setup->speed) {
        case VTSS_SPEED_10M:
		ctl.force_speed = FORCE_10;
		ctl.giga_mode = MODE_10_100;
		break;
	case VTSS_SPEED_100M:
		ctl.force_speed = FORCE_100;
		ctl.giga_mode = MODE_10_100;
		break;
	case VTSS_SPEED_1G:
		ctl.force_speed = FORCE_1000;
		ctl.giga_mode = MODE_10_100_1000;
		break;
	default:
		break;
	}
	
	ctl.force_fc_tx = setup->flow_control;
	ctl.force_fc_rx = setup->flow_control;
	
	
	ctl.cmd = VTSS_GRX_IOCTL_MAC1_CONTROL;
	
	if (setup->fdx == 1)
		ctl.force_duplex = FULL_DUPLEX;
	else
		ctl.force_duplex = HALF_DUPLEX;
	
	vtss_grocx_router_mac1_control(&ctl);
	return VTSS_OK;
}

static vtss_rc vtss_grocx_router_vlan_port_members_set(const vtss_vid_t vid,
						       BOOL member[VTSS_PORT_ARRAY_SIZE])
{
	vtss_grx_vlan_ctl_t ctl;
	uint lan = 0;
	vtss_port_no_t         port_no;
	
	for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
		if (port_no != VTSS_ROUTER_PORT_WAN)
			lan |= member[port_no];
	}
	
	if (lan) {
		ctl.cmd = VTSS_GRX_IOCTL_MODIFY_VLAN;
		ctl.vlan_port_map = VTSS_GRX_MAC0_PMAP | VTSS_GRX_CPU_PMAP;
		ctl.vlan_tag_port_map = VTSS_GRX_MAC0_PMAP | VTSS_GRX_CPU_PMAP;
		ctl.vid = vid;
		
		vtss_grocx_router_vlan_control(&ctl);
	}
	return VTSS_OK;
}

static vtss_rc vtss_grocx_router_vlan_port_members_get(const vtss_vid_t vid,
						       BOOL member[VTSS_PORT_ARRAY_SIZE])
{
	vtss_grx_vlan_ctl_t ctl;
	ctl.cmd = VTSS_GRX_IOCTL_GET_VLAN;
	
	ctl.vid = vid;
	vtss_grocx_router_vlan_control(&ctl);
	member[0] = ctl.vlan_port_map;
	return VTSS_OK;
}

static vtss_rc vtss_grocx_router_poag_counters_get(vtss_poag_counters_t * const counters)
{
	vtss_grx_port_counter_t ctl;
	
	ctl.cmd = VTSS_GRX_IOCTL_COUNTER_GET;
	ctl.port = VTSS_GRX_PORT_MAC1;
	vtss_grocx_router_packet_counter_get(&ctl);
	/* count all drop packet */
	counters->rmon.rx_etherStatsDropEvents = ctl.rx_no_buffer_drop_pkt + ctl.rx_arl_drop_pkt + ctl.rx_vlan_ingress_drop_pkt;
	counters->rmon.rx_etherStatsOctets = ctl.rx_ok_byte;
	counters->rmon.rx_etherStatsPkts = ctl.rx_ok_pkt + ctl.rx_runt_pkt + ctl.rx_over_size_pkt + ctl.rx_crc_err_pkt + ctl.rx_csum_err_pkt; /* + ctl.rx_pause_frame_pkt; need the pause frame */
	counters->rmon.rx_etherStatsCRCAlignErrors = ctl.rx_crc_err_pkt;
	counters->rmon.rx_etherStatsFragments = ctl.rx_runt_pkt;
	counters->rmon.tx_etherStatsOctets = ctl.tx_ok_byte;
	counters->rmon.tx_etherStatsPkts = ctl.tx_ok_pkt;
	
	return VTSS_OK;
}

static vtss_rc vtss_grocx_router_poag_counters_clear(void)
{
	vtss_grx_port_counter_t ctl;

	ctl.cmd = VTSS_GRX_IOCTL_COUNTER_CLEAR;
	ctl.port = VTSS_GRX_PORT_MAC1;
	vtss_grocx_router_packet_counter_clear(&ctl);
	return VTSS_OK;
}

/* - vitesse grocx API ------------------------------------------ */
vtss_rc vtss_grocx_start(void)
{
#if (VTSS_ROUTER_PORT_WAN == 0) /* config C or config D*/
	return vtss_grocx_router_start();
#else /* config A or config B */
	return vtss_grocx_router_start();
#endif
}

vtss_rc vtss_grocx_port_setup(const vtss_port_no_t port_no,
			      const vtss_grocx_port_conf_t * const setup)
{
	vtss_port_setup_t port_setup, *ps;
	
#if (VTSS_ROUTER_PORT_WAN == 0) /* config C or config D */
	if(port_no == 1)
		return vtss_grocx_router_port_setup(setup);
#else /* config A or config B */
	
#endif
	ps = &port_setup;
	memset(ps, 0, sizeof(*ps));
	ps->interface_mode.interface_type = VTSS_PORT_INTERFACE_INTERNAL;
	ps->powerdown = (setup->enable ? 0 : 1);
	ps->maxframelength = setup->maxframelength;
	ps->frame_gaps.hdx_gap_1 = VTSS_FRAME_GAP_DEFAULT;
	ps->frame_gaps.hdx_gap_2 = VTSS_FRAME_GAP_DEFAULT;
	ps->frame_gaps.fdx_gap = VTSS_FRAME_GAP_DEFAULT;
	ps->interface_mode.speed = setup->speed;
	ps->fdx = setup->fdx;
	ps->flowcontrol.obey = setup->flow_control;
	ps->flowcontrol.generate = setup->flow_control; 
	
	return vtss_port_setup(port_no, ps);
}

vtss_rc vtss_grocx_poag_counters_clear(const vtss_poag_no_t poag_no)
{
#if (VTSS_ROUTER_PORT_WAN == 0) /* config C or config D */
	return ((poag_no == 1) ? vtss_grocx_router_poag_counters_clear() : vtss_poag_counters_clear(poag_no));
#else /* config A or config B */
	return vtss_poag_counters_clear(poag_no);
#endif
}

vtss_rc vtss_grocx_poag_counters_get(const vtss_poag_no_t poag_no,
				     vtss_poag_counters_t * const counters)
{
#if (VTSS_ROUTER_PORT_WAN == 0) /* config C or config D */
	return ((poag_no == 1) ? vtss_grocx_router_poag_counters_get(counters) : vtss_poag_counters_get(poag_no, counters));
#else /* config A or config B */
	vtss_poag_counters_get(poag_no, counters);
#endif
	return VTSS_OK;
}

vtss_rc vtss_grocx_vlan_port_members_get(const vtss_vid_t vid,
					 BOOL member[VTSS_PORT_ARRAY_SIZE])
{
	/* vtss_grocx_router_vlan_port_members_get(vid, member); */
	vtss_vlan_port_members_get(vid, member);
	return VTSS_OK;
}

vtss_rc vtss_grocx_vlan_port_members_set(const vtss_vid_t vid,
					 BOOL member[VTSS_PORT_ARRAY_SIZE])
{
	vtss_grocx_router_vlan_port_members_set(vid, member);
	vtss_vlan_port_members_set(vid, member);
	return VTSS_OK;
}

void vtss_grocx_port_mode_start(void)
{
	int type = 4;

	if (type < 1 || type > 4) {
		log_kern(LOG_CRIT,
			 "VTSS: uncorrect G-Roc'X port mode, mode=%c",
			 type + 'A');
		panic();
	}

	if (type == 1) {
		vtss_opt_router_wlan_rgmii = 1;
		vtss_opt_port_count = 7;
		vtss_opt_router_port_wan =1;
	} else if (type == 4) {
		vtss_opt_router_wlan_rgmii = 1;
	} else {
		vtss_opt_router_port_wan = 1;
	}
}

static void vtss_grocx_add_counter(ui_table_t *table, int i,
				   const char *mib_class,
				   const char *name,
				   vtss_counter_t counter)
{
	char buf[25];
	ui_add_value(table, "class", i, mib_class);
	ui_add_value(table, "counter", i, name);
	snprintf(buf, sizeof(buf), "%d", counter);
	ui_add_value(table, "value", i, buf);
}

static int vtss_grocx_cmd_mib(ui_session_t *sess, ui_entry_t *inst,
			      void *ctx, int argc, char **argv)
{
	int i = 0;
	int port_no = atoi(argv[0]);
	ui_table_t *table = ui_table_by_name(sess, "grocx_counters");
	vtss_poag_counters_t counters;
	vtss_rc rc;

	if (table) {
		ui_table_delete(table);
	}
	table = ui_table_create(sess, "grocx_counters");
	if (!table)
		return -1;

	ui_add_title(table, 0, "class");
	ui_add_title(table, 1, "counter");
	ui_add_title(table, 2, "value");

	rc = vtss_grocx_poag_counters_get(port_no, &counters);
	if (rc != VTSS_OK) {
		vtss_log(VTSS_LOG_ERR, "GROCX: failed to get MIB counters, port=%d",
			 port_no);
		goto end;
	}

	/* TODO: MIB Variables Mapping
	 * if we have RMON, IF-GROUP, ETHER-LIKE MIB and MIB parser
	 */
	vtss_grocx_add_counter(table, i, "EtherStatsEntry",
			       "rx_etherStatsPkts",
			       counters.rmon.rx_etherStatsPkts);
	i++;
	vtss_grocx_add_counter(table, i, "EtherStatsEntry",
			       "tx_etherStatsPkts",
			       counters.rmon.tx_etherStatsPkts);
	i++;
	vtss_grocx_add_counter(table, i, "EtherStatsEntry",
			       "rx_etherStatsOctets",
			       counters.rmon.rx_etherStatsOctets);
	i++;
	vtss_grocx_add_counter(table, i, "EtherStatsEntry",
			       "tx_etherStatsOctets",
			       counters.rmon.tx_etherStatsOctets);
	i++;
	vtss_grocx_add_counter(table, i, "EtherStatsEntry",
			       "rx_etherStatsDropEvents",
			       counters.rmon.rx_etherStatsDropEvents);
	i++;
	vtss_grocx_add_counter(table, i, "EtherStatsEntry",
			       "tx_etherStatsDropEvents",
			       counters.rmon.tx_etherStatsDropEvents);
	i++;
	vtss_grocx_add_counter(table, i, "IfEntry",
			       "ifInErrors",
			       counters.if_group.ifInErrors);
	i++;
	vtss_grocx_add_counter(table, i, "IfEntry",
			       "ifOutErrors",
			       counters.if_group.ifOutErrors);
	i++;

#ifdef VTSS_FEATURE_PORT_CNT_QOS
	fprintf(stdout, "|%d/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu", 
		7,
		counters.prop.rx_prio[0],
		counters.prop.tx_prio[0],
		counters.prop.rx_prio[1],
		counters.prop.tx_prio[1],
		counters.prop.rx_prio[2],
		counters.prop.tx_prio[2],
		counters.prop.rx_prio[3],
		counters.prop.tx_prio[3]);
#endif

	sess->result_table = table;
end:
	return 0;
}

static int vtss_grocx_cmd_pvlans(ui_session_t *sess, ui_entry_t *inst,
				 void *ctx, int argc, char **argv)
{
	int i = 0, vid;
	int port_no;
	ui_table_t *table = ui_table_by_name(sess, "grocx_pvlans");
	BOOL member[VTSS_PORT_ARRAY_SIZE];
	char buf[25];

	if (table) {
		ui_table_delete(table);
	}
	table = ui_table_create(sess, "grocx_pvlans");
	if (!table)
		return -1;

	ui_add_title(table, 0, "pvlan");
	i = 1;
	for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
		snprintf(buf, sizeof(buf), "port-%d", port_no);
		ui_add_title(table, i, buf);
		i++;
	}

	i = 0;
        for (vid = VTSS_PVLAN_NO_START; vid < VTSS_PORT_ARRAY_SIZE; vid++) {
		if (vtss_pvlan_port_members_get(vid, member) != VTSS_OK) {
			continue;
		}
		snprintf(buf, sizeof(buf), "%d", vid);
		ui_add_value(table, "pvlan", i, buf);
		for (port_no = VTSS_PORT_NO_START;
		     port_no < VTSS_PORT_NO_END; port_no++) {
			snprintf(buf, sizeof(buf), "port-%d", port_no);
			if (member[port_no])
				ui_add_value(table, buf, i, "o");
			else
				ui_add_value(table, buf, i, "x");
		}
		i++;
	}

	sess->result_table = table;
	return 0;
}

string_map_t vtss_vlan_frame_types[] = {
	{ "all", VTSS_VLAN_FRAME_ALL, "Accept all frames" },
	{ "tagged", VTSS_VLAN_FRAME_TAGGED, "Accept tagged frames only" },
	{ "untagged", VTSS_VLAN_FRAME_UNTAGGED, "Accept untagged frames only" },
};

static const char *vtss_vlan_frametypes2name(vtss_vlan_frame_t type)
{
	return int2name(vtss_vlan_frame_types, type, "all");
}

static int vtss_grocx_cmd_vlans(ui_session_t *sess, ui_entry_t *inst,
				void *ctx, int argc, char **argv)
{
	int i = 0;
	int port_no;
	ui_table_t *table = ui_table_by_name(sess, "grocx_vlans");
	vtss_vlan_port_mode_t vlan_mode;
	char buf[25];
	static int once = 0;

	if (table) {
		ui_table_delete(table);
	}
	table = ui_table_create(sess, "grocx_vlans");
	if (!table)
		return -1;

	ui_add_title(table, 0, "port");
	ui_add_title(table, 1, "aware");
	ui_add_title(table, 2, "vid");
	ui_add_title(table, 3, "egress_tag");
	ui_add_title(table, 4, "frame_type");

	for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
		if (vtss_vlan_port_mode_get(port_no, &vlan_mode) != VTSS_OK)
			continue;

		if (!once) {
			if (port_no == 2 || port_no == 7) {
				vlan_mode.pvid = vlan_mode.untagged_vid = 3;
				vtss_vlan_port_mode_set(port_no, &vlan_mode);
			}
		}
		snprintf(buf, sizeof(buf), "%d", port_no);
		ui_add_value(table, "port", i, buf);
		ui_add_value(table, "aware", i, vlan_mode.aware ? "o" : "x");
		snprintf(buf, sizeof(buf), "%d", vlan_mode.pvid);
		ui_add_value(table, "vid", i, buf);
		snprintf(buf, sizeof(buf), "%d", vlan_mode.untagged_vid);
		ui_add_value(table, "egress_tag", i, buf);
		ui_add_value(table, "frame_type", i, vtss_vlan_frametypes2name(vlan_mode.frame_type));
		i++;
	}

	once = 1;
	sess->result_table = table;
	return 0;
}

ui_argument_t vtss_grocx_port_args[] = {
	{ "port", "Port number", "switch_ports", UI_TYPE_CHOICE, },
};

ui_command_t vtss_grocx_mib_command = {
	"dump",
	"Dump G-Roc'X MIB",
	".switch.grocx.mib",
	UI_CMD_SINGLE_INST,
	vtss_grocx_port_args,
	1,
	LIST_HEAD_INIT(vtss_grocx_mib_command.link),
	vtss_grocx_cmd_mib,
};

ui_command_t vtss_grocx_pvlan_command = {
	"dump",
	"Dump G-Roc'X private V-LANs",
	".switch.grocx.pvlan",
	UI_CMD_SINGLE_INST,
	NULL,
	0,
	LIST_HEAD_INIT(vtss_grocx_pvlan_command.link),
	vtss_grocx_cmd_pvlans,
};

ui_command_t vtss_grocx_vlan_command = {
	"dump",
	"Dump G-Roc'X V-LANs",
	".switch.grocx.vlan",
	UI_CMD_SINGLE_INST,
	NULL,
	0,
	LIST_HEAD_INIT(vtss_grocx_vlan_command.link),
	vtss_grocx_cmd_vlans,
};

int __init vtss_grocx_init(void)
{
	ui_register_schema(vtss_grocx_schema);
	ui_register_command(&vtss_grocx_mib_command);
	ui_register_command(&vtss_grocx_pvlan_command);
	ui_register_command(&vtss_grocx_vlan_command);
	return 0;
}

void __exit vtss_grocx_exit(void)
{
	ui_unregister_command(&vtss_grocx_vlan_command);
	ui_unregister_command(&vtss_grocx_pvlan_command);
	ui_unregister_command(&vtss_grocx_mib_command);
	ui_unregister_schema(vtss_grocx_schema);
}
#endif
