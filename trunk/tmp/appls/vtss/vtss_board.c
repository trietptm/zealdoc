#include "vtss_priv.h"
#include "vtss_grocx.h"
#include <eloop.h>

/* port status */
static vtss_port_status_t vtss_port_status[VTSS_PORT_ARRAY_SIZE];
/* port configuration */
static vtss_port_conf_t vtss_port_conf[VTSS_PORT_ARRAY_SIZE];
BOOL vtss_port_poll[VTSS_PORT_ARRAY_SIZE];
static BOOL vtss_mac_age_started = FALSE;

static void __vtss_port_setup(vtss_port_no_t port_no, BOOL conf);

/* ================================================================= *
 *  Board specific functions
 * ================================================================= */
/* Board port map */
static vtss_mapped_port_t vtss_port_map[VTSS_PORT_ARRAY_SIZE] = { 
	{ -1, -1, -1 } /*unused*/,
#ifdef CONFIG_VTSS_BOARD_GROCX_REF
	{  5, VTSS_MIIM_CONTROLLER_1, 0 },     /* 5: (R)GMII (WAN) */
	{  0, VTSS_MIIM_CONTROLLER_0, 0 },     /* 0: Internal PHY */
	{  1, VTSS_MIIM_CONTROLLER_0, 1 },     /* 1: Internal PHY */
	{  2, VTSS_MIIM_CONTROLLER_0, 2 },     /* 2: Internal PHY */
	{  3, VTSS_MIIM_CONTROLLER_0, 3 },     /* 3: Internal PHY */
#if (VTSS_PORTS == 7)
	{  6, VTSS_MIIM_CONTROLLER_NONE, -1 }, /* 6: Internal GMII or external RGMII (WiFi) */
#endif /* VTSS_PORTS == 7 */
	{  4, VTSS_MIIM_CONTROLLER_NONE, -1 }, /* 4: Internal GMII */
#endif
};

/* release PHYs from reset */
static void vtss_reset_phy(void) 
{
}

#ifdef CONFIG_VTSS_ROUTER
/* Determine if port is first router port */
static BOOL vtss_router_port_0(vtss_port_no_t port_no)
{
#ifdef CONFIG_VTSS_GROCX
	return (vtss_port_map[port_no].chip_port == 4);
#endif
}

/* Determine if port is second router port */
static BOOL vtss_router_port_1(vtss_port_no_t port_no)
{
#ifdef CONFIG_VTSS_GROCX
	return (vtss_port_map[port_no].chip_port == 6 && 
		vtss_port_map[port_no].miim_controller == VTSS_MIIM_CONTROLLER_NONE);
#endif
}

/* determine if port is router port */
static BOOL vtss_router_port(vtss_port_no_t port_no)
{
	return (vtss_router_port_0(port_no) || vtss_router_port_1(port_no));
}
#endif

#if VTSS_ROUTER_PORT_WAN
/* Setup router VLANs */
static void vtss_router_vlan_setup(void)
{
	vtss_port_no_t        port_no;
	BOOL                  member[VTSS_PORT_ARRAY_SIZE];
	vtss_vlan_port_mode_t mode;
	uint                  count = 0;
	
	/* Count number of router ports */
	for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++)
		if (vtss_router_port(port_no))
			count++;
		
		/* Setup VLAN port mode */
		for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
			mode.aware = vtss_router_port(port_no);
			mode.pvid = (port_no == VTSS_ROUTER_PORT_WAN || vtss_router_port_1(port_no) ? 
						VTSS_ROUTER_VLAN_WAN : VTSS_ROUTER_VLAN_LAN);
			mode.untagged_vid = ((count == 1 && vtss_router_port(port_no)) || VTSS_VID_ALL);
			mode.frame_type = VTSS_VLAN_FRAME_ALL;
			mode.ingress_filter = 0;
			vtss_vlan_port_mode_set(port_no, &mode);
			member[port_no] = (mode.pvid == VTSS_ROUTER_VLAN_LAN);
		}
		
		/* Setup LAN VLAN members */
		vtss_vlan_port_members_set(VTSS_ROUTER_VLAN_LAN, member);
		
		/* Setup WAN VLAN members */
		for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++)
			member[port_no] = (port_no == VTSS_ROUTER_PORT_WAN || 
			(count == 1 && vtss_router_port_0(port_no)) ||
			(count == 2 && vtss_router_port_1(port_no)));
		vtss_vlan_port_members_set(VTSS_ROUTER_VLAN_WAN, member);
}
#endif

/* Determine port MAC interface */
static vtss_port_interface_t board_port_mac_interface(vtss_port_no_t port_no)
{
	int chip_port;
	vtss_port_interface_t if_type;
	
	chip_port = vtss_port_map[port_no].chip_port;
	
	/* Determine interface type */
#ifdef CONFIG_VTSS_GROCX
	if_type = VTSS_PORT_INTERFACE_INTERNAL;
	if (chip_port == 5)
		if_type = (VTSS_ROUTER_PORT_5_IF ? VTSS_PORT_INTERFACE_GMII : 
			   VTSS_PORT_INTERFACE_RGMII);
	if (chip_port == 6 && !vtss_router_port_1(port_no))
		if_type = VTSS_PORT_INTERFACE_RGMII;
#endif
	
	return if_type;
}

static vtss_port_no_t vtss_port_no;

#ifdef CONFIG_VTSS_BOARD_GROCX_REF
#define NUM_ACT_STATUS	4
#endif

static void vtss_port_led_update(vtss_port_no_t port_no,
				 vtss_port_status_t *port_status,
				 BOOL act_update)
{   
#ifdef CONFIG_VTSS_BOARD_GROCX_REF
	static BOOL is_init = 0;
	static vtss_counter_t port_counter[4];
	static BOOL act_status[NUM_ACT_STATUS] = {0,0,0,0};
	
	/* BOARD_GROCX_REF LED hardware spec(configuration B).
	 *
	 * Logical port number       LED port number
	 * =======================================
	 * P5/P4/P3/P2 ACT ---> LED0/1/2/3.0
	 * P5/P4/P3/P2 10/100 mode --> LED0/1/2/3.1
	 * P5/P4/P3/P2 GIGA mode --> LED0/1/2/3.2 
	 */   
	vtss_port_no_t port_maps[8] = {0xff,0xff,3,2,1,0,0xff,0xff}; /* Logic port map to LED port(Config B) */
	vtss_led_port_t led_port; 
	vtss_led_mode_t led_mode[3];
	vtss_poag_counters_t counters;
	vtss_counter_t current_counter;
	BOOL is_act = 0;
	
	if (is_init == 0) {
		memset(port_counter, 0, sizeof(port_counter));
		is_init = 1;
	}
	
	led_port = port_maps[port_no];
	if (led_port == 0xff) {
		return;
	}
	
	if (act_status[led_port] == 1 && act_update == 1) {
		return; /* don't change LED status too fast */
	} else if (act_update == 1) {
		vtss_poag_counters_get(port_no, &counters);
		current_counter = (counters.if_group.ifInOctets + counters.if_group.ifOutOctets);
		if (current_counter != port_counter[port_no-2]) {
			is_act = 1;
			act_status[led_port] = 1;
			port_counter[port_no-2] = current_counter;
		}            
	}
        
	if (port_status->link) {
		/* Link up */
		if (!is_act) {
			led_mode[0] = VTSS_LED_MODE_OFF;
		} else {
			led_mode[0] = VTSS_LED_MODE_5;   
		}    
		if (port_status->speed == VTSS_SPEED_1G)
		{   /* Giga mode*/
			led_mode[1] = VTSS_LED_MODE_OFF;
			led_mode[2] = VTSS_LED_MODE_ON;
		} else {
			/* 10/100 mode */
			led_mode[1] = VTSS_LED_MODE_ON;
			led_mode[2] = VTSS_LED_MODE_OFF;
		}
		vtss_serial_led_set(led_port, &led_mode[0]);
	} else {
		/* Link down */
		led_mode[0] = VTSS_LED_MODE_OFF;
		led_mode[1] = VTSS_LED_MODE_OFF;
		led_mode[2] = VTSS_LED_MODE_OFF;
		vtss_serial_led_set(led_port, &led_mode[0]);
	}
#endif
}

static void vtss_port_led_state(void *eloop_data, void *user_data)
{
	vtss_port_status_t status;
	vtss_port_status_t *ps;
	BOOL link_old;

	if (!vtss_port_poll[vtss_port_no])
		goto next;

	/* get current status */
	vtss_port_status_get(vtss_port_no, &status);
	ps = &vtss_port_status[vtss_port_no];
	link_old = ps->link;
	ps->link = status.link;
	ps->speed = status.speed;
	ps->fdx = status.fdx;
	ps->aneg.obey_pause = status.aneg.obey_pause;
	ps->aneg.generate_pause = status.aneg.generate_pause;
	
	/* detect link down and disable port */
	if ((!status.link || status.link_down) && link_old) {
		vtss_log(VTSS_LOG_INFO,
			 "SWITCH: link down detected, port=%d", vtss_port_no);

		switch_port_link(vtss_port_no, SWITCH_LINK_DOWN);

		vtss_port_stp_state_set(vtss_port_no, VTSS_STP_STATE_DISABLED);
		vtss_mac_table_forget_port(vtss_port_no);

#ifdef CONFIG_VTSS_GROCX
		if (vtss_router_port_0(vtss_port_no)) {
			/* try to bring down WAN port */
		}
#endif
		vtss_port_led_update(vtss_port_no, &status, 0);
	}
	
	/* detect link up and enable port */
	if (status.link && !link_old) { 
		vtss_log(VTSS_LOG_INFO,
			 "SWITCH: link up detected, port=%d", vtss_port_no);

		switch_port_link(vtss_port_no, SWITCH_LINK_UP);

		vtss_port_stp_state_set(vtss_port_no, VTSS_STP_STATE_ENABLED);
		if (vtss_port_conf[vtss_port_no].autoneg)
			__vtss_port_setup(vtss_port_no, 0);
#ifdef CONFIG_VTSS_GROCX
		if (vtss_router_port_0(vtss_port_no)) {
			/* try to bring up WAN port */
		}
#endif
		vtss_port_led_update(vtss_port_no, &status, 0);
	}
	
	/* detect traffic when link is up */
	if (status.link) {    
		vtss_port_led_update(vtss_port_no, &status, 1);
	}

next:
	vtss_port_no++;
	if (vtss_port_no >= VTSS_PORT_NO_END)
		vtss_port_no = VTSS_PORT_NO_START;

	/* 200ms */
	eloop_register_timeout(NULL, 0, 200 * 1000, vtss_port_led_state, NULL, NULL);
}

static void vtss_port_led_start(void)
{
#ifdef CONFIG_VTSS_BOARD_GROCX_REF /* LED init for GROCX_REF Board */
	vtss_led_port_t led_port; 
	vtss_led_mode_t led_mode[3];
	
	led_mode[0] = VTSS_LED_MODE_OFF;
	led_mode[1] = VTSS_LED_MODE_OFF;
	led_mode[2] = VTSS_LED_MODE_OFF;
	/* set the first four ports to "off" mode */
	for (led_port = 0; led_port <= 3; led_port++) {
		vtss_serial_led_set(led_port, &led_mode[0]);
	}    
	
	led_mode[0] = VTSS_LED_MODE_DISABLED;
	led_mode[1] = VTSS_LED_MODE_DISABLED;
	led_mode[2] = VTSS_LED_MODE_DISABLED;
	/* set the rest ports to "disable" mode */
	for (led_port = 4; led_port <= 15; led_port++) {
		vtss_serial_led_set(led_port, &led_mode[0]);
	}
	
	/* use enhanced led mode for VSC8601 */
	vtss_phy_write(1, 31, 0x1);
	vtss_phy_write(1, 17, 0x10);
	vtss_phy_write(1, 16, 0x61a);
	vtss_phy_write(1, 17, 0x14d6);
	vtss_phy_write(1, 31, 0x0);
#endif

	vtss_port_no = VTSS_PORT_NO_START;
	eloop_register_timeout(NULL, 0, 0, vtss_port_led_state, NULL, NULL);
}

/* Setup port based on configuration and auto negotiation result */
static void __vtss_port_setup(vtss_port_no_t vtss_port_no, BOOL conf)
{
	vtss_port_status_t *ps;
	vtss_port_conf_t *pc;
	vtss_port_setup_t setup;
	vtss_phy_setup_t phy;
	
	pc = &vtss_port_conf[vtss_port_no];
	memset(&setup, 0, sizeof(setup));
	setup.interface_mode.interface_type = board_port_mac_interface(vtss_port_no);
	setup.powerdown = (pc->enable ? 0 : 1);
	setup.flowcontrol.smac.addr[5] = vtss_port_no;
	setup.maxframelength = pc->max_length;
	setup.frame_gaps.hdx_gap_1 = VTSS_FRAME_GAP_DEFAULT;
	setup.frame_gaps.hdx_gap_2 = VTSS_FRAME_GAP_DEFAULT;
	setup.frame_gaps.fdx_gap = VTSS_FRAME_GAP_DEFAULT;
	
	if (conf) {
		/* Configure port */
		if (vtss_port_map[vtss_port_no].miim_controller != VTSS_MIIM_CONTROLLER_NONE) {
			/* Setup PHY */
			phy.mode = (pc->enable ? 
				    (pc->autoneg || pc->speed == VTSS_SPEED_1G ? 
				     VTSS_PHY_MODE_ANEG : VTSS_PHY_MODE_FORCED) : 
				     VTSS_PHY_MODE_POWER_DOWN);
			phy.aneg.speed_10m_hdx = 1;
			phy.aneg.speed_10m_fdx = 1;
			phy.aneg.speed_100m_hdx = 1;
			phy.aneg.speed_100m_fdx = 1;
			phy.aneg.speed_1g_fdx = 1;
			phy.aneg.symmetric_pause = pc->flow_control;
			phy.aneg.asymmetric_pause = pc->flow_control;
			phy.forced.speed = pc->speed;
			phy.forced.fdx = pc->fdx;
			vtss_phy_setup(vtss_port_no, &phy);
		}
		/* Use configured values */
		setup.interface_mode.speed = pc->speed;
		setup.fdx = pc->fdx;
		setup.flowcontrol.obey = pc->flow_control;
		setup.flowcontrol.generate = pc->flow_control;
	} else {
		/* Setup port based on auto negotiation status */
		ps = &vtss_port_status[vtss_port_no];
		setup.interface_mode.speed = ps->speed;
		setup.fdx = ps->fdx;
		setup.flowcontrol.obey = ps->aneg.obey_pause;
		setup.flowcontrol.generate = ps->aneg.generate_pause;
	}
	
#if defined(CONFIG_VTSS_GROCX) && (VTSS_ROUTER_PORT_WAN == 0)
	if (vtss_port_no == 1) {
		vtss_grocx_port_conf_t grocx_port_setup;
		
		grocx_port_setup.enable = 1;
		grocx_port_setup.speed = setup.interface_mode.speed;
		grocx_port_setup.fdx = setup.fdx;
		grocx_port_setup.flow_control = setup.flowcontrol.obey;
		
		vtss_grocx_port_setup(vtss_port_no, &grocx_port_setup);
		return;
	}
#endif
	
	vtss_port_setup(vtss_port_no, &setup);
}

#ifndef VTSS_FEATURE_MAC_AGE_AUTO
static void vtss_mac_age_timeout(void *eloop_data, void *user_data)
{
	if (vtss_main_config.mac_aging) {
		if (vtss_mac_age_started) {
			/* manually do MAC table aging */
			vtss_mac_table_age();
			eloop_register_timeout(NULL, VTSS_MAC_AGE_DEFAULT/2,
					       0, vtss_mac_age_timeout, NULL, NULL);
		}
	}
}
#endif

static int vtss_chip_start(void)
{
	vtss_init_setup_t init_setup;
	int rc;

	/* initialize switch chip */
	init_setup.reset_chip = 1;
	init_setup.use_cpu_si = 0;
#ifdef CONFIG_VTSS_USE_CPU_SI
	init_setup.use_cpu_si = 1;
#endif
	rc = vtss_api_start(&init_setup);
	if (rc == 0) {
		vtss_chipid_t chipid;
		
		if (vtss_chipid_get(&chipid)<0) {
			vtss_log(VTSS_LOG_ERR, "CHIP: vtss_chipid_get failed");
		} else {
			vtss_log(VTSS_LOG_INFO,
				 "CHIP, vitesse chip initialized, part_number=0x%04x, revision=%d",
				 chipid.part_number, chipid.revision);
		}
	}
	return rc;
}

static void vtss_reset_all_ports(void)
{
	vtss_port_no_t port_no;
	vtss_phy_reset_setup_t phy_reset;
	vtss_port_conf_t *pc;

	/* reset all ports */
	for (port_no = VTSS_PORT_NO_START;
	     port_no < VTSS_PORT_NO_END; port_no++) {
		vtss_port_status[port_no].link = 0;
		
		/* default port configuration */
		pc = &vtss_port_conf[port_no];
		pc->enable = 1;
		pc->autoneg = 0;
		pc->speed = VTSS_SPEED_1G;
		pc->fdx = 1;
		pc->flow_control = 0;
		pc->max_length = VTSS_MAXFRAMELENGTH_STANDARD;
		vtss_port_poll[port_no] = 1;

		switch_register_port(port_no);
		
		if (vtss_port_map[port_no].miim_controller == VTSS_MIIM_CONTROLLER_NONE) {
			/* no PHY */
			/* setup for 1G, full duplex,
			 * flow control disabled
			 */
			vtss_port_stp_state_set(port_no, VTSS_STP_STATE_ENABLED);
			vtss_port_poll[port_no] = 0;
			switch_unregister_port(port_no);
		} else {
			/* reset PHY */
			pc->autoneg = 1;
			pc->flow_control = 1;
			phy_reset.mac_if = board_port_mac_interface(port_no);
			phy_reset.rgmii.rx_clk_skew_ps = 1800;
			phy_reset.rgmii.tx_clk_skew_ps = 1800;
			phy_reset.media_if = VTSS_PHY_MEDIA_INTERFACE_COPPER;
			vtss_phy_reset(port_no, &phy_reset);
		}

#ifdef CONFIG_VTSS_GROCX
		/* set WAN port phy 8601 RGMII skew value to 2.0ns */
		/* vtss_phy_page_ext(1); */
		vtss_phy_write(1, 31, 0x1);
		vtss_phy_writemasked(1, 28, 0xf000, 0xf000);    
		vtss_phy_write(1, 31, 0x0);
		/* vtss_phy_page_std(1); */       
#endif
#ifdef CONFIG_VTSS_ROUTER
		if (vtss_router_port(port_no)) {
			/* setup for 1G, full duplex, always up */
#ifdef CONFIG_VTSS_ROUTER_PORT_FLOW_CONTROL
			pc->flow_control = 1;
#endif
			vtss_port_stp_state_set(port_no, VTSS_STP_STATE_ENABLED);
			vtss_port_poll[port_no] = 0;
			switch_unregister_port(port_no);

			/* disable ACL on router port */
			vtss_acl_reset_port(port_no);
		}
#endif
		__vtss_port_setup(port_no, 1);
	}
}

static void vtss_reset_qce(void)
{
#ifdef VTSS_FEATURE_QOS_TAG_REMARK
        vtss_qce_t  qce;
        qce.id = 1;
        qce.type = VTSS_QCE_TYPE_TAG;
        qce.frame.tag_prio[0]= 1;
        qce.frame.tag_prio[1]= 1;
        qce.frame.tag_prio[2]= 2;
        qce.frame.tag_prio[3]= 2;
        qce.frame.tag_prio[4]= 3;
        qce.frame.tag_prio[5]= 3;
        qce.frame.tag_prio[6]= 4;
        qce.frame.tag_prio[7]= 4;
        vtss_qce_add(6,0,&qce);
#endif
}

void vtss_mac_age_start(void)
{
	if (vtss_main_config.mac_aging) {
#ifdef VTSS_FEATURE_MAC_AGE_AUTO
		/* automatic aging used */
		vtss_mac_table_age_time_set(vtss_main_config.mac_aging);
		vtss_mac_age_started = 0;
#else
		/* manual aging used */
		eloop_register_timeout(NULL, 0, 0, vtss_mac_age_timeout, NULL, NULL);
		vtss_mac_age_started = 1;
#endif
	}
}

void vtss_mac_age_stop(void)
{
	if (!vtss_main_config.mac_aging) {
#ifdef VTSS_FEATURE_MAC_AGE_AUTO
		/* automatic aging used */
		vtss_mac_table_age_time_set(0);
#else
		/* manual aging used */
		eloop_cancel_timeout(NULL, vtss_mac_age_timeout, NULL, NULL);
#endif
		vtss_mac_age_started = 0;
	}
}

static int vtss_board_start(void)
{
	/* setup port map for board */
	vtss_port_map_set(vtss_port_map);
#if VTSS_ROUTER_PORT_WAN
	/* setup router VLANs */
	vtss_router_vlan_setup();
#endif
	/* release PHYs from reset */
	vtss_reset_phy();
#ifdef CONFIG_VTSS_GROCX
	/* initialize grocx router-core control interface */
	vtss_grocx_router_ioctl_start();
#endif
	vtss_reset_all_ports();
	vtss_reset_qce();
	return 0;
}

int vtss_appl_start(void)
{
	int rc = VTSS_OK;

#ifdef CONFIG_VTSS_GROCX
	vtss_grocx_port_mode_start();
#endif
	rc = vtss_chip_start();
	if (rc != VTSS_OK) return -1;
	
	rc = vtss_board_start();
	if (rc != VTSS_OK) return -1;

	vtss_mac_age_start();
	/* initialize Board LED */
	vtss_port_led_start();
	return 0;
}
