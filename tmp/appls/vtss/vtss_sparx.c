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
 
 $Id: vtss_sparx.c,v 1.31 2008-12-08 09:00:01 zhenglv Exp $
 $Revision: 1.31 $

*/

#include "vtss_priv.h"
#include "vtss_heathrow.h"

/* ================================================================= *
 *  Port MAC constants and types
 * ================================================================= */

/* Convert from chip port bitfield to vtss_port_no bitfield. */
ulong vtss_chip2portmask(const ulong chip_port_bitfield)
{
	vtss_port_no_t port_no;
	ulong bitfield = 0;
	
	for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
		if (chip_port_bitfield & (1<<HT_CHIP_PORT(port_no))) {
			bitfield |= 1<<port_no;
		}
	}
	return bitfield;
}

/* Convert from vtss_port_no list to chip port bitfield, */
ulong vtss_portmask2chip(const BOOL member[VTSS_PORT_ARRAY_SIZE])
{
	vtss_port_no_t port_no;
	uint port_on_chip;
	ulong bitfield = 0;
	
	for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
		if (member[port_no]) {
			if (vtss_mac_state.port_map.vtss_port_unused[port_no]) {
				vtss_log(VTSS_LOG_ERR,
					 "SPARX: unmapped port enabled, port=%d",
					 port_no);
			} else {
				port_on_chip = HT_CHIP_PORT(port_no);
				bitfield |= (1 << port_on_chip);
			}
		}
	}
	return bitfield;
}

/* Convert from vtss_pgid_no to destination index on chip */
uint vtss_pgid2chip(const vtss_pgid_no_t pgid_no)
{
	if (pgid_no<VTSS_PGID_UNICAST_END) {
		return HT_CHIP_PORT(pgid_no);
	} else {
		return pgid_no-VTSS_PGID_UNICAST_END+VTSS_CHIP_PORTS;
	}
}

/* Convert from destination index on chip to vtss_pgid_no */
vtss_pgid_no_t vtss_chip2pgid(const uint chip_pgid)
{
	if (chip_pgid<VTSS_CHIP_PORTS) {
		return vtss_mac_state.port_map.vtss_port[chip_pgid];
	} else {
		return VTSS_PGID_UNICAST_END+chip_pgid-VTSS_CHIP_PORTS;
	}
}

/* Read target register using current CPU interface */
vtss_rc ht_rd_wr(uint blk, uint sub, uint reg, ulong *value, BOOL do_write)
{
	vtss_rc rc;
	BOOL    error;
	
	switch (blk) {
	case B_PORT:
		/* By default, it is an error if the sub block is not included in chip port mask */
		error = ((sub > VTSS_CHIP_PORTS) ||
			(VTSS_CHIP_PORTMASK & (1<<sub)) == 0);
#ifdef CONFIG_VTSS_ARCH_SPARX_28
		if (sub == VTSS_CHIP_PORT_CPU)
			error = 0;
#endif
		if (error)
			break;
		if (sub >= 16) {
			blk = B_PORT_HI;
			sub -= 16;
		}
		break;
	case B_MIIM: /* B_MEMINIT/B_ACL/B_VAUI */
#ifdef CONFIG_VTSS_ARCH_SPARX_28
		error = (sub > S_ACL);
#else
		error = (sub > S_MEMINIT);
#endif
		break;
	case B_CAPTURE:
		error = 0;
		break;
	case B_ANALYZER:
	case B_ARBITER:
	case B_SYSTEM:
		error = (sub != 0);
		break;
	case B_PORT_HI: /* vtss_ll_register_read/write may access this directly */
		error = 0;
		break;
	default:
		error = 1;
		break;
	}
	
	if (error) {
		vtss_log(VTSS_LOG_ERR,
			 "SPARX: illegal access, blk=0x%02x, sub=0x%02x, reg=0x%02x",
			 blk, sub, reg);
		return VTSS_INVALID_PARAMETER;
	}
	
	if (vtss_mac_state.init_setup.use_cpu_si)
		rc = (do_write ? vtss_io_si_wr(blk, sub, reg, *value) : 
	vtss_io_si_rd(blk, sub, reg, value));
	else 
		rc = (do_write ? vtss_io_pi_wr(blk, sub, reg, *value) : 
	vtss_io_pi_rd(blk, sub, reg, value));
	
	if (vtss_mac_state.debug_read_write) {
		vtss_log(VTSS_LOG_DEBUG,
			 "SPARX: %s access, blk=%d, sub=%2d, reg=0x%02x, value=0x%08lx",
			 do_write ? "write" : "read ", blk, sub, reg, *value);
	}
	return rc;
}

/* Read target register using current CPU interface */
vtss_rc ht_rd(uint blk, uint sub, uint reg, ulong *value)
{
	return ht_rd_wr(blk, sub, reg, value, 0);
}

/* Write target register using current CPU interface */
vtss_rc ht_wr(uint blk, uint sub, uint reg, ulong value)
{
	return ht_rd_wr(blk, sub, reg, &value, 1);
}

/* Read-modify-write target register using current CPU interface */
vtss_rc ht_wrm(uint blk, uint sub, uint reg, ulong value, ulong mask)
{
	vtss_rc rc;
	ulong val;
	
	if ((rc = ht_rd_wr(blk, sub, reg, &val, 0))>=0) {
		val = ((val & ~mask) | (value & mask));
		rc = ht_rd_wr(blk, sub, reg, &val, 1);
	}
	return rc;
}

/* Read target register field using current CPU interface */
vtss_rc ht_rdf(uint blk, uint sub, uint reg, uint offset, ulong mask, ulong *value)
{
	vtss_rc rc;
	
	rc = ht_rd_wr(blk, sub, reg, value, 0);
	*value = ((*value >> offset) & mask);
	return rc;
}

/* Read-modify-write target register field using current CPU interface */
vtss_rc ht_wrf(uint blk, uint sub, uint reg, uint offset, ulong mask, ulong value)
{
	return ht_wrm(blk, sub, reg, value<<offset, mask<<offset);
}

/* ================================================================= *
 * MII management
 * ================================================================= */
/* returns (ushort) result or (long) <0 */
static vtss_rc ht_phy_readwrite(BOOL do_write, uint miim_controller,
				uint phy_addr, uint phy_reg, ushort *value)
{
	ulong data;
	
	/* Enqueue MIIM operation to be executed */
	HT_WR(MIIM, miim_controller, MIIMCMD,
	      ((do_write ? 0 : 1) << 26) | (phy_addr << 21) | (phy_reg << 16) | *value);
	
	/* Wait for MIIM operation to finish */
	while (1) {
		HT_RD(MIIM, miim_controller, MIIMSTAT, &data);
		if (!(data & 0xF)) /* Include undocumented "pending" bit */
			break;
	}
	if (!do_write) {
		HT_RD(MIIM, miim_controller, MIIMDATA, &data);
		if (data & (1<<16))
			return VTSS_PHY_READ_ERROR;
		*value = (ushort)(data & 0xFFFF);
	}
	return VTSS_OK;
}

/* Read from the phy register */
vtss_rc vtss_ll_phy_read(uint miim_controller, uint phy_addr, uint phy_reg, ushort *value)
{
	return ht_phy_readwrite(0, miim_controller, phy_addr, phy_reg, value);
}

/* Write to the phy register */
vtss_rc vtss_ll_phy_write(uint miim_controller, uint phy_addr, uint phy_reg, ushort value)
{
	return ht_phy_readwrite(1, miim_controller, phy_addr, phy_reg, &value);
}


/* ================================================================= *
 * frame arbiter
 * ================================================================= */
static vtss_rc ht_arbiter_empty_allports(BOOL *empty)
{
	ulong value;
	
	HT_RD(ARBITER, 0, ARBEMPTY, &value);
	*empty = (vtss_chip2portmask(value) == ((1<<VTSS_PORTS)-1)<<1);
	return VTSS_OK; 
}

/* ================================================================= *
 * port queues
 * ================================================================= */
static vtss_rc ht_port_queue_enable(vtss_port_no_t port_no, BOOL enable) 
{
	uint port_on_chip = HT_CHIP_PORT(port_no);
	
	vtss_log(VTSS_LOG_DEBUG,
		 "SPARX: port on chip enable, port_on_chip=%d, enable=%d",
		 port_on_chip, enable);
	/* Enable/disable Arbiter discard */
	HT_WRF(ARBITER, 0, ARBDISC, port_on_chip, 0x1, enable ? 0 : 1);
	/* Enable/disable MAC Rx */
	HT_WRF(PORT, port_on_chip, MAC_CFG, 16, 0x1, enable ? 1 : 0);
	/* Enable/disable MAC Tx */
	HT_WRF(PORT, port_on_chip, MAC_CFG, 28, 0x1, enable ? 1 : 0);
	return VTSS_OK;
}

/* Set STP learn mode */
static vtss_rc ht_port_learn_set(vtss_port_no_t port_no)
{
	uint port_on_chip = HT_CHIP_PORT(port_no);
	vtss_stp_state_t stp_state = vtss_mac_state.stp_state[port_no];
	BOOL learn;
	
	/* Setup STP learning state */
	learn = (stp_state == VTSS_STP_STATE_LEARNING ||
		 stp_state == VTSS_STP_STATE_FORWARDING ||
		 stp_state == VTSS_STP_STATE_ENABLED);
	
	HT_WRF(ANALYZER, 0, LERNMASK, port_on_chip, 0x1, learn ? 1 : 0);
	return VTSS_OK;
}

/* Enable or disable and flush port queue systems */
vtss_rc vtss_ll_port_queue_enable(vtss_port_no_t port_no, BOOL enable)
{
	/* Enable/disable queues */
	VTSS_RC(ht_port_queue_enable(port_no, enable));
	return ht_port_learn_set(port_no);
}

/* Setup water marks, drop levels, etc for port */
vtss_rc vtss_ll_port_queue_setup(vtss_port_no_t port_no)
{
	vtss_port_setup_t *setup = &vtss_mac_state.setup[port_no];
	uint              port_on_chip = HT_CHIP_PORT(port_no);
	uchar             *mac;
	BOOL              zero_pause_enable;
	ushort            pause_value;
#ifdef CONFIG_VTSS_ARCH_SPARX
	BOOL              wfq = 0;
	enum {
		VTSS_NORM_SQ_DROP,
		VTSS_NORM_WFQ_DROP,
		VTSS_JUMBO_SQ_DROP,
		VTSS_JUMBO_WFQ_DROP,
		VTSS_NORM_SQ_FC,
		/* not used: VTSS_NORM_WFQ_FC, */
		VTSS_JUMBO_SQ_FC
		/* not used: VTSS_JUMBO_WFQ_FC */
	} mode;
	
#ifdef VTSS_FEATURE_QOS_WFQ_PORT
	wfq = vtss_mac_state.wfq;
#endif
	
	if (setup->flowcontrol.generate) {
		/* Flow control mode */
		if (setup->maxframelength<=VTSS_MAXFRAMELENGTH_TAGGED) {
			mode = VTSS_NORM_SQ_FC;
		} else {
			mode = VTSS_JUMBO_SQ_FC;
		}
	} else {
		/* Drop mode */
		if (setup->maxframelength<=VTSS_MAXFRAMELENGTH_TAGGED) {
			mode = (wfq ? VTSS_NORM_WFQ_DROP : VTSS_NORM_SQ_DROP);
		} else {
			mode = (wfq ? VTSS_JUMBO_WFQ_DROP : VTSS_JUMBO_SQ_DROP);
		}
	}
#endif /* SPARX */
	
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	switch (mode) {
	case VTSS_NORM_SQ_DROP: 
	case VTSS_JUMBO_SQ_DROP: 
		zero_pause_enable = VTSS_NORM_4S_DROP_ZEROPAUSE; 
		pause_value = VTSS_NORM_4S_DROP_PAUSEVALUE; 
		break;
	case VTSS_NORM_WFQ_DROP: 
	case VTSS_JUMBO_WFQ_DROP: 
		zero_pause_enable = VTSS_NORM_4W_DROP_ZEROPAUSE; 
		pause_value = VTSS_NORM_4W_DROP_PAUSEVALUE; 
		break;
	case VTSS_NORM_SQ_FC:   
	case VTSS_JUMBO_SQ_FC:   
		zero_pause_enable = VTSS_NORM_4S_FC_ZEROPAUSE;   
		pause_value = VTSS_NORM_4S_FC_PAUSEVALUE;   
		break;
	default:
		vtss_log(VTSS_LOG_ERR,
			 "SPARX: unknown mode, mode=%d", mode);
		zero_pause_enable = 0;
		pause_value = 0;
		break;
	}
#endif
	
	/* Setup flow control registers */
	mac = &setup->flowcontrol.smac.addr[0];
	HT_WR(PORT, port_on_chip, FCMACHI, (mac[0]<<16) | (mac[1]<<8) | mac[2]);
	HT_WR(PORT, port_on_chip, FCMACLO, (mac[3]<<16) | (mac[4]<<8) | mac[5]);
	HT_WR(PORT, port_on_chip, FCCONF,
	      ((zero_pause_enable ? 1 : 0)<<17) | 
	      ((setup->flowcontrol.obey ? 1 : 0)<<16) | 
	      (pause_value<<0));
	
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	{
		ulong mcnf = 0, ewm = 0, iwm[6];
		uint  i;

#define MCNF(mode) \
	((VTSS_NORM_##mode##_IMIN<<16) | \
	 (VTSS_NORM_##mode##_EMIN<<8) | \
	 (VTSS_NORM_##mode##_EARLY_TX<<1))

#define EWM(mode) \
	((VTSS_NORM_##mode##_EMAX3<<24) | \
	 (VTSS_NORM_##mode##_EMAX2<<16) | \
	 (VTSS_NORM_##mode##_EMAX1<<8) | \
	 (VTSS_NORM_##mode##_EMAX0<<0))

#define IWM(mode, i) \
	((VTSS_NORM_##mode##_LOWONLY_##i<<18) | \
	 (VTSS_NORM_##mode##_QMASK_##i<<13) | \
	 (VTSS_NORM_##mode##_ACTION_##i<<11) | \
	 (VTSS_NORM_##mode##_CHKMIN_##i<<10) | \
	 (VTSS_NORM_##mode##_CMPWITH_##i<<8) | \
	 (VTSS_NORM_##mode##_LEVEL_##i<<0))
		
		switch (mode) {
		case VTSS_NORM_SQ_DROP: 
		case VTSS_JUMBO_SQ_DROP: 
			mcnf = MCNF(4S_DROP);
			ewm = EWM(4S_DROP);
			iwm[0] = IWM(4S_DROP, 0);
			iwm[1] = IWM(4S_DROP, 1);
			iwm[2] = IWM(4S_DROP, 2);
			iwm[3] = IWM(4S_DROP, 3);
			iwm[4] = IWM(4S_DROP, 4);
			iwm[5] = IWM(4S_DROP, 5);
			if (vtss_mac_state.qos[port_no].policer_port != VTSS_BITRATE_FEATURE_DISABLED) {
				/* Special settings if policer enabled */
				iwm[0] = ((iwm[0] & 0xffffffe0) | (VTSS_NORM_4S_POLICER_LEVEL_0<<0));
				iwm[5] = ((iwm[5] & 0xffffffe0) | (VTSS_NORM_4S_POLICER_LEVEL_5<<0));  
			}
			break;
		case VTSS_NORM_WFQ_DROP: 
		case VTSS_JUMBO_WFQ_DROP: 
			mcnf = MCNF(4W_DROP);
			ewm = EWM(4W_DROP);
			iwm[0] = IWM(4W_DROP, 0);
			iwm[1] = IWM(4W_DROP, 1);
			iwm[2] = IWM(4W_DROP, 2);
			iwm[3] = IWM(4W_DROP, 3);
			iwm[4] = IWM(4W_DROP, 4);
			iwm[5] = IWM(4W_DROP, 5);
			if (vtss_mac_state.qos[port_no].policer_port != VTSS_BITRATE_FEATURE_DISABLED) {
				/* Special settings if policer enabled */
				iwm[0] = ((iwm[0] & 0xffffffe0) | (VTSS_NORM_4W_POLICER_LEVEL_0<<0));
				iwm[5] = ((iwm[5] & 0xffffffe0) | (VTSS_NORM_4W_POLICER_LEVEL_5<<0));  
			}
			break;
		case VTSS_NORM_SQ_FC:   
		case VTSS_JUMBO_SQ_FC:   
			mcnf = MCNF(4S_FC);
			ewm = EWM(4S_FC);
			iwm[0] = IWM(4S_FC, 0);
			iwm[1] = IWM(4S_FC, 1);
			iwm[2] = IWM(4S_FC, 2);
			iwm[3] = IWM(4S_FC, 3);
			iwm[4] = IWM(4S_FC, 4);
			iwm[5] = IWM(4S_FC, 5);
			break;
		default:
			vtss_log(VTSS_LOG_ERR,
				 "SPARX: unknown mode, mode=%d", mode);
			return VTSS_UNSPECIFIED_ERROR;
		}
		
#ifndef CONFIG_VTSS_GROCX
		if (setup->interface_mode.speed == VTSS_SPEED_10M && setup->fdx == 0) {
			/* Special settings for 10 Mbps half duplex operation */
			mcnf = ((mcnf & ((0x1f<<16) | (0xf<<1))) | (VTSS_NORM_10_HDX_EMIN<<8));
			ewm = EWM(10_HDX);
		}
#endif
		
		/* Setup queue system drop levels and water marks */
		HT_WR(PORT, port_on_chip, Q_MISC_CONF, mcnf);
		HT_WR(PORT, port_on_chip, Q_EGRESS_WM, ewm);
		
		/* Ingress watermarks */
		for (i = 0; i < 6; i++) {
			HT_WR(PORT, port_on_chip, Q_INGRESS_WM + i, iwm[i]);
		}
	}
#endif
	return VTSS_OK;
}

/* ================================================================= *
 * port MAC
 * ================================================================= */
#define VTSS_ARBITER_WAIT_MAXRETRIES 1000000

/* Reset the MAC and setup the port, watermarks etc. */
static vtss_rc ht_port_setup(vtss_port_no_t port_no,
			     const vtss_port_setup_t * setup)
{
	uint port_on_chip = HT_CHIP_PORT(port_no);
	ulong macconf, value;
	ulong macconf_speedbits;
	ht_tx_clock_select_t tx_clock_select;
	vtss_port_interface_t if_type = setup->interface_mode.interface_type;
	BOOL if_loopback = (if_type == VTSS_PORT_INTERFACE_LOOPBACK);
	vtss_frame_gaps_t gaps;
	uint hdx_late_col_pos;
	BOOL powerdown = MAKEBOOL01(setup->powerdown);
	vtss_speed_t speed = setup->interface_mode.speed;
	ulong aggr_masks[VTSS_ACS];
	vtss_ac_no_t ac;
	BOOL fifo_not_empty = 1;
	uint fifo_empty_retries = 0;
	
	/* Determine interframe gaps and default clock selection based on speed only */
	gaps.hdx_gap_1   = 0;
	gaps.hdx_gap_2   = 0;
	gaps.fdx_gap     = 0;
	hdx_late_col_pos = 0;
	
	switch (speed) {
	case VTSS_SPEED_10M:
	case VTSS_SPEED_100M:
		tx_clock_select = (speed == VTSS_SPEED_10M ? HT_TX_CLOCK_10M : HT_TX_CLOCK_100M);
#ifdef CONFIG_VTSS_ARCH_SPARX
		gaps.hdx_gap_1   = 6;
		gaps.hdx_gap_2   = 8;
		gaps.fdx_gap     = 17;
		hdx_late_col_pos = 2;
#endif /* SPARX */
		break;
	case VTSS_SPEED_1G:
		tx_clock_select = HT_TX_CLOCK_GIGA;
#ifdef CONFIG_VTSS_ARCH_SPARX
		gaps.fdx_gap     = 6;
#endif /* SPARX */
		break;
	default:
		vtss_log(VTSS_LOG_ERR,
			 "SPARX: illegal speed, port=%d", port_no);
		return VTSS_INVALID_PARAMETER;
	}
	
	/* Check interface type and determine clock selection */
	switch (if_type) {
	case VTSS_PORT_INTERFACE_NO_CONNECTION:
		tx_clock_select = HT_TX_CLOCK_OFF;
		break;
#ifdef CONFIG_VTSS_GROCX
	case VTSS_PORT_INTERFACE_MII:
		if (speed == VTSS_SPEED_1G) {
			vtss_log(VTSS_LOG_ERR,
				 "SPARX: illegal speed, port=%d", port_no);
			return VTSS_INVALID_PARAMETER;
		}
		if (port_on_chip != 5) /* MII on port 5 only */
			return VTSS_INVALID_PARAMETER;
		tx_clock_select = HT_TX_CLOCK_MISC;
		break;
	case VTSS_PORT_INTERFACE_GMII:
		if (port_on_chip != 5) /* GMII on port 5 only */
			return VTSS_INVALID_PARAMETER;
		if (speed != VTSS_SPEED_1G)
			tx_clock_select = HT_TX_CLOCK_MISC;
		break;
	case VTSS_PORT_INTERFACE_RGMII:
		if (port_on_chip < 5) /* RGMII on port 5 and 6 only */
			return VTSS_INVALID_PARAMETER;
		break;
#endif
#ifdef CONFIG_VTSS_ARCH_SPARX
	case VTSS_PORT_INTERFACE_INTERNAL:
		tx_clock_select = HT_TX_CLOCK_MISC;
#ifdef CONFIG_VTSS_ARCH_SPARX_28
#ifdef CONFIG_VTSS_GROCX
		if (port_on_chip == 4 || port_on_chip == 6)
			tx_clock_select = HT_TX_CLOCK_GIGA;
#else
		return VTSS_INVALID_PARAMETER;
#endif
#endif
		break;
	case VTSS_PORT_INTERFACE_LOOPBACK:
		if (tx_clock_select == HT_TX_CLOCK_OFF) { /* Not 10/100/1000 */
			vtss_log(VTSS_LOG_ERR,
				 "SPARX: illegal speed, port=%d", port_no);
			return VTSS_INVALID_PARAMETER;
		}
		break;
#endif
	default:
		vtss_log(VTSS_LOG_ERR,
			 "SPARX: illegal speed, port=%d", port_no);
		return VTSS_INVALID_PARAMETER;
	}
	
	/* Read and remove port from aggregation masks to avoid forwarding to port */
	for (ac = VTSS_AC_START; ac < VTSS_AC_END; ac++) {
		HT_RD(ANALYZER, 0, AGGRMSKS + ac - VTSS_AC_START, &value);
		aggr_masks[ac - VTSS_AC_START] = value;
		HT_WR(ANALYZER, 0, AGGRMSKS + ac - VTSS_AC_START,
		      value & ~(1<<port_on_chip));
	}
	
	/* Setup half duplex gaps */
	if (setup->frame_gaps.hdx_gap_1 != VTSS_FRAME_GAP_DEFAULT)
		gaps.hdx_gap_1 = setup->frame_gaps.hdx_gap_1;
	if (setup->frame_gaps.hdx_gap_2 != VTSS_FRAME_GAP_DEFAULT)
		gaps.hdx_gap_2 = setup->frame_gaps.hdx_gap_2;
	if (setup->frame_gaps.fdx_gap != VTSS_FRAME_GAP_DEFAULT)
		gaps.fdx_gap = setup->frame_gaps.fdx_gap;
	HT_WR(PORT, port_on_chip, MACHDXGAP,
	      (7<<16) |
	      (8<<12) | 
	      (hdx_late_col_pos<<8) |
	      (gaps.hdx_gap_2<<4) |
	      (gaps.hdx_gap_1<<0));
	
	macconf = (((setup->fdx ? 1 : 0)<<18) |
		  (gaps.fdx_gap<<6));
	
	macconf |= ((setup->flowcontrol.smac.addr[5]<<19) |
#ifdef CONFIG_VTSS_ARCH_SPARX
                   ((setup->length_check == VTSS_LENGTH_TAG_DOUBLE ? 1 : 0)<<15) | 
#endif /* SPARX */
                   ((setup->length_check == VTSS_LENGTH_TAG_NONE ? 0 : 1)<<14));
	macconf_speedbits = (((speed == VTSS_SPEED_10M || speed == VTSS_SPEED_100M ? 0 : 1)<<17) |
		            (tx_clock_select<<0));
	
		/* We need to make sure that the FIFO is not receiving frames
	(from the bus) when it's coming out of reset  */
	
	while(fifo_not_empty) {
		/* Hold MAC Reset */
		value = (macconf | (1<<29) | (1<<27) | (1<<5) | (1<<4));
#ifdef CONFIG_VTSS_ARCH_SPARX
		/* Don't reset PCS, because that would mess up autonegotiation, 
		if this function is called because autoneg completed */
		value |= ((0<<3) | (0<<2));
		value |= macconf_speedbits;
#endif /* SPARX */
		HT_WR(PORT, port_on_chip, MAC_CFG, value);
		
		/* Set Arbiter to not discard frames from the port */
		HT_WRF(ARBITER, 0, ARBDISC, port_on_chip, 0x1, 0);
		
		/* Release MAC from Reset and set Seed_Load */
#ifdef CONFIG_VTSS_ARCH_SPARX
		/* (MAC here meaning both MAC and PCS) */
#endif
		value = (macconf | (0<<29) | (1<<27) | (0<<5) | (0<<4) | macconf_speedbits);
#ifdef CONFIG_VTSS_ARCH_SPARX
		value |= ((0<<3) | (0<<2));
#endif        
		
		HT_WR(PORT, port_on_chip, MAC_CFG, value);
		
		VTSS_NSLEEP(1000);
		/* Clear Seed_Load bit */
		HT_WR(PORT, port_on_chip, MAC_CFG, value & ~(1<<27));
		
		HT_RD(PORT, port_on_chip, FREEPOOL, &value);
		fifo_not_empty = 0;
		if ((value & 0xffff) != 0) {
			/*       vtss_log(VTSS_LOG_ERR, "SPARX: rreepool is not empty after reset:0x%x.",value));  */
			fifo_not_empty = 1;
			fifo_empty_retries++;
			if (fifo_empty_retries > 10)
				break;            
			VTSS_MSLEEP(100);
		} 
	}
	
	/* Restore aggregation masks */
	for (ac = VTSS_AC_START; ac < VTSS_AC_END; ac++) {
		HT_WR(ANALYZER, 0, AGGRMSKS + ac - VTSS_AC_START, aggr_masks[ac - VTSS_AC_START]);
	}
	
	/* Set Advanced Port Mode */
#ifdef CONFIG_VTSS_GROCX
	value = (((setup->exc_col_cont ? 1 : 0)<<6) | /* EXC_COL_CONT */
		((port_on_chip == 4 || if_type == VTSS_PORT_INTERFACE_GMII ? 1 : 0)<<4) | /* INVERT_GTX */
		((port_on_chip > 3 ? 1 : 0)<<3) |    /* ENABLE_GTX */
		((if_type == VTSS_PORT_INTERFACE_RGMII ? 1 : 0)<<2) |    /* DDR_MODE */
		((port_on_chip > 3 ? 1 : 0)<<1) |    /* INV_RXCLK */
		((if_loopback ? 1 : 0)<<0));         /* HOST_LOOPBACK */
#endif
	HT_WR(PORT, port_on_chip, ADVPORTM, value);
	
	if (powerdown) {
		/* Power down MAC */
		value = ((1<<5) | (1<<4));
#ifdef CONFIG_VTSS_ARCH_SPARX_28
		value |= ((1<<3) | (1<<2));
#endif /* SPARX_28 */
		HT_WRM(PORT, port_on_chip, MAC_CFG, value, value);
	}
	
	{
		uint maxlen = setup->maxframelength;
		HT_WR(PORT, port_on_chip, MAXLEN, maxlen);
	}
	return vtss_ll_port_queue_setup(port_no);
}

/* Configure port into gmii modes */
vtss_rc vtss_ll_port_speed_mode_gmii(vtss_port_no_t port_no,
                                     const vtss_port_setup_t * setup)
{
	uint  wait_attempt;
	ulong value;
	BOOL  empty;
	
#ifdef VTSS_FEATURE_EXC_COL_CONT
	{
		vtss_port_no_t port;
		vtss_port_setup_t *ps;
		
		value = HT_TIMECMP_DEFAULT;
		/* Count number of half duplex ports with excessive
		 * collision continuation enabled and setup frame aging
		 * accordingly
		 */
		for (port = VTSS_PORT_NO_START;
		     port < VTSS_PORT_NO_END; port++) {
			ps = &vtss_mac_state.setup[port];
			if (ps->exc_col_cont && !ps->fdx) {
				value = 0; /* Frame aging disabled */
				break; 
			}
		}
		HT_WR(SYSTEM, 0, TIMECMP, value);
	}
#endif
	/* Discard frames from the port, Disable RX and Drop
	 * everything in the FIFOs
	 */
	VTSS_RC(ht_port_queue_enable(port_no, 0));
	
	/* Wait until Arbiter port empty */
	for (wait_attempt = 0;
	     wait_attempt < VTSS_ARBITER_WAIT_MAXRETRIES; wait_attempt++) {
		HT_RD(ARBITER, 0, ARBEMPTY, &value);
		if (value & (1<<HT_CHIP_PORT(port_no)))
			break;
	}
	if (wait_attempt>=VTSS_ARBITER_WAIT_MAXRETRIES) {
		/* Timeout */
		vtss_port_no_t port; /* Other ports */
		
		vtss_log(VTSS_LOG_DEBUG,
			 "SPARX: timeout - arbiter does not empty port, reset and setup all ports, port=%d",
			 port_no);
		
		/* Discard frames from all ports, Disable RX and Drop everything in the FIFOs */
		for (port = VTSS_PORT_NO_START; port < VTSS_PORT_NO_END; port++) {
			if (port == port_no) continue;
			VTSS_RC(ht_port_queue_enable(port, 0));
		}
		/* Wait until Arbiter empty for all ports */
		for (wait_attempt=0; wait_attempt<VTSS_ARBITER_WAIT_MAXRETRIES; wait_attempt++) {
			VTSS_RC(ht_arbiter_empty_allports(&empty));
			if (empty)
				break;
		}
		if (wait_attempt>=VTSS_ARBITER_WAIT_MAXRETRIES) {
			/* Timeout */
			vtss_log(VTSS_LOG_ERR,
				 "SPARX: timeout - Arbiter does not empty all ports");
			return VTSS_FATAL_ERROR;
		}
		
		/* Reset and setup all ports, watermarks etc. */
		for (port = VTSS_PORT_NO_START; port < VTSS_PORT_NO_END; port++) {
			if (port == port_no) continue;
			VTSS_RC(ht_port_setup(port, &vtss_mac_state.setup[port]));
			VTSS_RC(ht_port_queue_enable(port, 1));
		}
	}
	
	/* Reset and setup the port, watermarks etc. */
	VTSS_RC(ht_port_setup(port_no, setup));
	
	VTSS_RC(ht_port_queue_enable(port_no, 1));
	
#ifdef CONFIG_VTSS_ARCH_SPARX_28    
	/* Count number of ports in flow control and setup HOLB */
	value = 0;
	for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
		if (vtss_mac_state.setup[port_no].flowcontrol.obey ||
			vtss_mac_state.setup[port_no].flowcontrol.generate)
			value++;
	}
	HT_WR(ARBITER, 0, ARBHOLBPENAB, value ? 0 : VTSS_CHIP_PORTMASK);
#endif
	return VTSS_OK;
}

/* ================================================================= *
 * port QoS (Categorizer/Policer/Shaper)
 * ================================================================= */
static int chip_prio(const vtss_prio_t prio)
{
#if (VTSS_PRIOS==2)
	switch (vtss_mac_state.prios) {
        case 1: return 1;
        case 2: return (prio-VTSS_PRIO_START);
	}
#endif /* (VTSS_PRIOS==2) */
#if (VTSS_PRIOS==4)
	switch (vtss_mac_state.prios) {
        case 1: return 3;
        case 2: return (prio==1 ? 1 : 3);
        case 4: return (prio-VTSS_PRIO_START);
	}
#endif /* (VTSS_PRIOS==4) */
#if (VTSS_PRIOS==8)
	switch (vtss_mac_state.prios) {
        case 1: return 7;
        case 2: return (prio==1 ? 5 : 7);
        case 4: return (prio==1 ? 1 : prio==2 ? 3 : prio==3 ? 5 : 7);
        case 8: return (prio-VTSS_PRIO_START);
	}
#endif /* (VTSS_PRIOS==8) */
	return 0; /* Never comes here, but avoid the compiler warning. */
}

#ifdef CONFIG_VTSS_ARCH_SPARX_28

#ifdef CONFIG_VTSS_ARCH_SPARX_28
#ifdef CONFIG_VTSS_GROCX
#define RATE_FACTOR 1176471 /* 8000000/6.8 */
#define RATE_MAX    4595    /* 0xFFF*RATE_FACTOR/0xFFFFF */
#else
#define RATE_FACTOR 1666667 /* 8000000/4.8 */
#define RATE_MAX    6510    /* 0xFFF*RATE_FACTOR/0xFFFFF */
#endif
#endif

/* Setup policers and shapers for all ports */
static vtss_rc sparx_policer_shaper_setup(void)
{
	uint i, port_on_chip, j, weight, w[VTSS_QUEUES];
	vtss_port_no_t port_no;
	ulong max_rate = 0, policer_rate, shaper_rate;
	vtss_port_qos_setup_t *qos;
	
	for (i = 0; i < 2; i++) {
		for (port_no = VTSS_PORT_NO_START;
		     port_no < VTSS_PORT_NO_END; port_no++) {
			if (vtss_mac_state.port_map.vtss_port_unused[port_no])
				continue;
			
			qos = &vtss_mac_state.qos[port_no];
			policer_rate = qos->policer_port;
			if (policer_rate == VTSS_BITRATE_FEATURE_DISABLED)
				policer_rate = 0;
			
			shaper_rate = qos->shaper_port;
			if (shaper_rate == VTSS_BITRATE_FEATURE_DISABLED)
				shaper_rate = 0;
			
			if (i == 0) {
				/* First round: calculate maximum rate */
				if (policer_rate > max_rate)
					max_rate = policer_rate;
				
				if (shaper_rate > max_rate)
					max_rate = shaper_rate;
			} else {
				/* Second round: Setup policer and shaper */
				port_on_chip = HT_CHIP_PORT(port_no);
				for (j = 0; j < VTSS_QUEUES; j++) {
					weight = qos->weight[j + VTSS_QUEUE_START];
					w[j] = (qos->wfq_enable ? 
						(weight == VTSS_WEIGHT_1 ? 3 :
					weight == VTSS_WEIGHT_2 ? 2 :
					weight == VTSS_WEIGHT_4 ? 1 :
					weight == VTSS_WEIGHT_8 ? 0 : 4) : 0);
					if (w[j] > 3) {
						vtss_log(VTSS_LOG_ERR,
							 "SPARX: illegal weight, weight=%d",
							 weight);
						w[j] = 0;
					}
				}
#ifdef CONFIG_VTSS_ARCH_SPARX_28
				HT_WR(PORT, port_on_chip, SHAPER_CFG,
				      (20<<24) | /* GAP_VALUE */
				      (w[0]<<22) | (w[1]<<20) | (w[2]<<18) | (w[3]<<16) | 
				      ((qos->wfq_enable ? 1 : 0)<<15) |
				      (1<<12) |  /* EGRESS_BURST: 4 kB */
				      (shaper_rate*0xFFF/max_rate)<<0); /* EGRESS_RATE */
				HT_WR(PORT, port_on_chip, POLICER_CFG,
				      (1<<15) | /* POLICE_FWD */
				      (0<<14) | /* POLICE_FC */
				      (1<<12) | /* INGRESS_BURST: 4 kB */
				      (policer_rate*0xFFF/max_rate)<<0); /* INGRESS RATE */
#endif
			}
		}
		if (i == 0) {
			/* Calculate maximum rate */
			if (max_rate > 1000000 || max_rate == 0) /* Maximum 1G */
				max_rate = 1000000;
#ifdef CONFIG_VTSS_ARCH_SPARX_28
			/* Setup rate unit */
			if (max_rate < RATE_MAX)
				max_rate = RATE_MAX; 
			HT_WR(SYSTEM, 0, RATEUNIT,
			      5*(((0xFFF/5)*RATE_FACTOR)/max_rate));
#endif
		}
	}
	return VTSS_OK;
}
#endif /* SPARX_28 */

#ifdef CONFIG_VTSS_ARCH_SPARX_28
#define QCL_PORT_MAX 12 /* Number of QCLs per port */
#define QCE_TYPE_FREE      (0<<24)
#define QCE_TYPE_ETYPE     (1<<24)
#define QCE_TYPE_VID       (2<<24)
#define QCE_TYPE_UDP_TCP   (3<<24)
#define QCE_TYPE_DSCP      (4<<24)
#define QCE_TYPE_TOS       (5<<24)
#define QCE_TYPE_TAG       (6<<24)

static BOOL ht_qcl_full(const vtss_qcl_id_t qcl_id)
{
	int i;
	vtss_qcl_entry_t *entry;
	
	for (i = 0, entry = vtss_mac_state.qcl[qcl_id].qcl_list_used;
	entry != NULL; i++, entry = entry->next) {
		
		switch (entry->qce.type) {
		case VTSS_QCE_TYPE_ETYPE:
		case VTSS_QCE_TYPE_VLAN:
		case VTSS_QCE_TYPE_TOS:
		case VTSS_QCE_TYPE_TAG:
			/* These entries take up a single QCL entry */
			break;
		case VTSS_QCE_TYPE_UDP_TCP:
			if (entry->qce.frame.udp_tcp.low != entry->qce.frame.udp_tcp.high) {
				/* Range, at least two entries required */
				if (i & 1) /* Odd address, three entries requried */
					i++;
				i++;
			}
			break;
		case VTSS_QCE_TYPE_DSCP:
			if (entry->next != NULL && entry->next->qce.type == VTSS_QCE_TYPE_DSCP) {
				/* If next entry is DSCP, merge */
				entry = entry->next;
			}
			break;
		default: 
			vtss_log(VTSS_LOG_ERR,
				 "SPARX: unknown QCE type, qce=%d",
				 entry->qce.type); 
			return 0;
		}
	}
	return (i > QCL_PORT_MAX);
}

static vtss_rc ht_qcl_port_setup_set(const vtss_port_no_t port_no, const vtss_qcl_id_t qcl_id)
{
	uint port_on_chip = HT_CHIP_PORT(port_no);
	uint i, j;
	ulong value;
	vtss_qcl_entry_t *entry;
	
	/* Clear range table */
	HT_WR(PORT, port_on_chip, CAT_QCL_RANGE_CFG, 0);
	
	for (i = 0, entry = vtss_mac_state.qcl[qcl_id].qcl_list_used;
	     i < QCL_PORT_MAX && entry != NULL; i++, entry = entry->next) {
		switch (entry->qce.type) {
		case VTSS_QCE_TYPE_ETYPE:
			value = (QCE_TYPE_ETYPE | 
				(chip_prio(entry->qce.frame.etype.prio)<<16) | 
				(entry->qce.frame.etype.val<<0)); 
			vtss_log(VTSS_LOG_DEBUG,
				 "SPARX: adding ETYPE QCE[%d]: value=0x%08lx",
				 i, value); 
			HT_WR(PORT, port_on_chip, CAT_QCE0 + i, value);
			break;
		case VTSS_QCE_TYPE_VLAN:
			value = (QCE_TYPE_VID | 
				(chip_prio(entry->qce.frame.vlan.prio)<<16) | 
				(entry->qce.frame.vlan.vid<<0));
			vtss_log(VTSS_LOG_DEBUG,
				 "SPARX: adding VLAN QCE[%d]: value=0x%08lx",
				 i, value); 
			HT_WR(PORT, port_on_chip, CAT_QCE0 + i, value);
			break;
		case VTSS_QCE_TYPE_UDP_TCP:
			if (entry->qce.frame.udp_tcp.low != entry->qce.frame.udp_tcp.high) {
				if (i & 1) {
					vtss_log(VTSS_LOG_DEBUG,
						 "SPARX: adding free QCE[%d] for UDP/TCP range",
						 i);
					HT_WR(PORT, port_on_chip, CAT_QCE0 + i, QCE_TYPE_FREE);
					i++;
					if (i >= QCL_PORT_MAX)
						break;
				}
				vtss_log(VTSS_LOG_DEBUG,
					 "SPARX: setting bit #%d of QCL range cfg",
					 i/2);
				HT_WRF(PORT, port_on_chip, CAT_QCL_RANGE_CFG, i/2, 0x1, 1);
			}
			value = (QCE_TYPE_UDP_TCP | 
				(chip_prio(entry->qce.frame.udp_tcp.prio)<<16) | 
				(entry->qce.frame.udp_tcp.low<<0)); 
			vtss_log(VTSS_LOG_DEBUG,
				 "SPARX: adding UDP/TCP QCE[%d], value=0x%08lx",
				 i, value);
			HT_WR(PORT, port_on_chip, CAT_QCE0 + i, value);
			if (entry->qce.frame.udp_tcp.low != entry->qce.frame.udp_tcp.high) {
				i++;
				if (i >= QCL_PORT_MAX)
					break;
				value = (QCE_TYPE_UDP_TCP | 
					(chip_prio(entry->qce.frame.udp_tcp.prio)<<16) | 
					(entry->qce.frame.udp_tcp.high<<0)); 
				vtss_log(VTSS_LOG_DEBUG,
					 "SPARX: adding UDP/TCP QCE[%d], value=0x%08lx",
					 i, value);
				HT_WR(PORT, port_on_chip, CAT_QCE0 + i, value);
			}
			break;
		case VTSS_QCE_TYPE_DSCP:
			value = (QCE_TYPE_DSCP | 
				(entry->qce.frame.dscp.dscp_val <<  2) | 
				(chip_prio(entry->qce.frame.dscp.prio) << 0));  /* DSCP0 */ 
			if (entry->next != NULL && entry->next->qce.type == VTSS_QCE_TYPE_DSCP) {
				/* If next entry is DSCP, merge */
				entry = entry->next;
			}
			value |= ((entry->qce.frame.dscp.dscp_val << 10) | 
				(chip_prio(entry->qce.frame.dscp.prio) << 8)); /* DSCP1 */ 
			vtss_log(VTSS_LOG_DEBUG,
				 "SPARX: adding DSCP QCE[%d], value=0x%08lx",
				 i, value);
			HT_WR(PORT, port_on_chip, CAT_QCE0 + i, value);
			break;
		case VTSS_QCE_TYPE_TOS:
			value = QCE_TYPE_TOS;
			for (j = 0; j < 8; j++) {
				value |= (chip_prio(entry->qce.frame.tos_prio[j])<<(j*2));
			}
			vtss_log(VTSS_LOG_DEBUG,
				 "SPARX: adding TOS QCE[%d], value=0x%08lx",
				 i, value);
			HT_WR(PORT, port_on_chip, CAT_QCE0 + i, value);
			break;
		case VTSS_QCE_TYPE_TAG:
			value = QCE_TYPE_TAG;
			for (j = 0; j < 8; j++) {
				value |= (chip_prio(entry->qce.frame.tag_prio[j])<<(j*2));
			}
			vtss_log(VTSS_LOG_DEBUG,
				 "SPARX: adding TAG QCE[%d], value=0x%08lx",
				 i, value);
			HT_WR(PORT, port_on_chip, CAT_QCE0 + i, value);
			break;
		default: 
			vtss_log(VTSS_LOG_ERR,
				 "SPARX: unknown QCE type, qce=%d",
				 entry->qce.type); 
			return VTSS_UNSPECIFIED_ERROR;
		}
	}
	
	/* clear unused entries */
	while (i < QCL_PORT_MAX) {
		HT_WR(PORT, port_on_chip, CAT_QCE0 + i, QCE_TYPE_FREE);
		i++;
	}
	return VTSS_OK;
}

/* Set port QoS setup */
vtss_rc vtss_ll_port_qos_setup_set(vtss_port_no_t port_no,
				   const vtss_port_qos_setup_t *qos)
{
	uint port_on_chip = HT_CHIP_PORT(port_no);
	uint i = 0, j, k, max, prio_count[VTSS_PRIO_ARRAY_SIZE];
	vtss_prio_t prio;
	ulong tos = QCE_TYPE_TOS, dscp = 0, tag = QCE_TYPE_TAG;
	
	if (qos->qcl_id != VTSS_QCL_ID_NONE) {
		/* setup QCL */
		ht_qcl_port_setup_set(port_no, qos->qcl_id);
		
		/* skip old-style setup below by setting i to QCL_PORT_MAX */
		i = QCL_PORT_MAX;
	} else {
		/* Disable range checkers */
		HT_WR(PORT, port_on_chip, CAT_QCL_RANGE_CFG, 0);
	}
	
	/* UDP/TCP port numbers */
	if (qos->udp_tcp_enable) {
		for (j = 0; j < 10; j++) {
			if (i < QCL_PORT_MAX && qos->udp_tcp_val[j] != 0) {
				HT_WR(PORT, port_on_chip, CAT_QCE0 + i,
					QCE_TYPE_UDP_TCP | 
					(chip_prio(qos->udp_tcp_prio[j])<<16) | 
					(qos->udp_tcp_val[j]<<0));
				i++;
			}
		}
	}
	
	/* DSCP */
	if (qos->dscp_enable) {
		for (j = 0; j < 8; j++) {
			/* Find most frequent DSCP priority for each ToS value */
			memset(prio_count, 0, sizeof(prio_count));
			for (k = 0; k < 8; k++)
				prio_count[qos->dscp_prio[j*8+k]]++;
			max = VTSS_PRIO_START;
			for (k = VTSS_PRIO_START; k < VTSS_PRIO_END; k++)
				if (prio_count[k] > prio_count[max])
					max = k;
				
				/* Update ToS register value */
				tos |= (chip_prio(max)<<(j*2));
				
				/* Add entries for DSCP priorities different from the most frequent one */
				for (k = 0; k < 8; k++) {
					prio = qos->dscp_prio[j*8+k];
					if (prio != max) {
						if (dscp == 0) {
							dscp = QCE_TYPE_DSCP | ((j*8+k)<<10) | (chip_prio(prio)<<8);
						} else {
							dscp |= (((j*8+k)<<2) | (chip_prio(prio)<<0));
							if (i < (QCL_PORT_MAX - 1)) {
								HT_WR(PORT, port_on_chip, CAT_QCE0 + i, dscp);
								i++;
							}
							dscp = 0;
						}
					}
				}
		} 
		
		/* If odd number of DSCP exceptions, write one more */
		if (dscp != 0 && i < (QCL_PORT_MAX - 1)) {
			HT_WR(PORT, port_on_chip, CAT_QCE0 + i, dscp | ((dscp>>8) & 0xFF));
			i++;
		}
		
		/* Add ToS entry with most frequent priorities */
		if (i < QCL_PORT_MAX) {
			HT_WR(PORT, port_on_chip, CAT_QCE0 + i, tos);
			i++;
		}
	} else if (qos->tos_enable && i < QCL_PORT_MAX) {
		for (j = 0; j < 8; j++) {
			tos |= (chip_prio(qos->dscp_prio[j*8])<<(j*2));
		}
		HT_WR(PORT, port_on_chip, CAT_QCE0 + i, tos);
		i++;
	}
	
	/* Tag priority */
	if (qos->tag_enable && i < QCL_PORT_MAX) {
		for (j = 0; j < 8; j++) {
			tag |= (chip_prio(qos->tag_prio[j])<<(j*2));
		}
		HT_WR(PORT, port_on_chip, CAT_QCE0 + i, tag);
		i++;
	}
	
	/* Ethernet Type */
	if (qos->etype_enable && i < QCL_PORT_MAX) {
		HT_WR(PORT, port_on_chip, CAT_QCE0 + i, 
			QCE_TYPE_ETYPE | (chip_prio(qos->etype_prio)<<16) | (qos->etype_val<<0));
		i++;
	}
	
	/* Skip remaining QCEs */
	while (i < QCL_PORT_MAX) {
		HT_WR(PORT, port_on_chip, CAT_QCE0 + i, QCE_TYPE_FREE);
		i++;
	}
	
	/* Default ingress VLAN tag priority */
	HT_WRF(PORT, port_on_chip, CAT_PORT_VLAN, 12, 0x7, qos->usr_prio);
	
	/* Default priority */
	HT_WR(PORT, port_on_chip, CAT_QCL_DEFAULT_CFG, chip_prio(qos->default_prio));
	
#ifdef VTSS_FEATURE_QOS_DSCP_REMARK
	/* DSCP remarking */
	dscp = qos->dscp_mode;
	if (port_on_chip < 16) {
		HT_WRF(ANALYZER, 0, DSCPMODELOW, port_on_chip*2, 0x3, dscp);
	} else {
		HT_WRF(ANALYZER, 0, DSCPMODEHIGH, (port_on_chip - 16)*2, 0x3, dscp);
	}
	
	HT_WRF(PORT, port_on_chip, TXUPDCFG, 18, 0x1, qos->dscp_remark ? 1 : 0);
	
	dscp = 0;
	for (prio = VTSS_PRIO_START; prio < VTSS_PRIO_END; prio++)
		dscp |= (qos->dscp_map[prio]<<((prio - VTSS_PRIO_START)*8));
	HT_WR(PORT, port_on_chip, CAT_PR_DSCP_VAL_0_3, dscp);
#endif
	
#ifdef VTSS_FEATURE_QOS_TAG_REMARK
	/* Tag remarking */
	HT_WRF(PORT, port_on_chip, TXUPDCFG, 17, 0x1, qos->tag_remark ? 1 : 0);
	
	tag = 0;
	for (prio = VTSS_PRIO_START; prio < VTSS_PRIO_END; prio++)
		tag |= (qos->tag_map[prio]<<((prio - VTSS_PRIO_START)*4));
	HT_WR(PORT, port_on_chip, CAT_GEN_PRIO_REMAP, tag);
#endif
	
	/* Policers and shapers */
	VTSS_RC(sparx_policer_shaper_setup());
	
	/* Setup queues as water marks depend on policer setup */
	VTSS_RC(vtss_ll_port_queue_setup(port_no));
	return VTSS_OK;
}
#endif

#ifdef VTSS_FEATURE_QCL_PORT
static vtss_rc ht_qcl_commit(const vtss_qcl_id_t qcl_id)
{
	vtss_port_no_t port_no;
	vtss_rc qcl_rc = VTSS_OK;
	
	if (ht_qcl_full(qcl_id)) {
		vtss_log(VTSS_LOG_WARN,
			 "SPARX: QCL will be truncated when applied, qcl=%d",
			 qcl_id);
		qcl_rc = VTSS_WARNING;
	}
	
	for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
		if (vtss_mac_state.qos[port_no].qcl_id == qcl_id) {
			/* Reapply QoS settings */
			VTSS_RC(vtss_ll_port_qos_setup_set(port_no, &vtss_mac_state.qos[port_no]));
		}
	}
	return qcl_rc;
}

/* Add QCE */
vtss_rc vtss_ll_qce_add(vtss_qcl_id_t qcl_id,
                        vtss_qce_id_t qce_id,
                        const vtss_qce_t *qce)
{
	vtss_qcl_t *qcl;
	vtss_qcl_entry_t *cur, *prev = NULL;
	vtss_qcl_entry_t *new = NULL, *new_prev = NULL, *ins = NULL, *ins_prev = NULL;
	
	/* QCE ID 0 is reserved for insertion at end. Also check for same id */
	if (qce->id == VTSS_QCE_ID_LAST || qce->id == qce_id) {
		vtss_log(VTSS_LOG_ERR,
			 "SPARX: illegal qce id, qce=%lu", qce->id);        
		return VTSS_INVALID_PARAMETER;
	}
	
	/* Check that the QCL ID is valid */
	if (qcl_id < VTSS_QCL_ID_START || qcl_id >= VTSS_QCL_ID_END) {
		vtss_log(VTSS_LOG_ERR,
			 "SPARX: illegal qcl id, qcl=%lu", qcl_id);        
		return VTSS_INVALID_PARAMETER;
	}
	
	/* Search for existing entry and place to add */
	qcl = &vtss_mac_state.qcl[qcl_id];
	for (cur = qcl->qcl_list_used; cur != NULL; prev = cur, cur = cur->next) {
		if (cur->qce.id == qce->id) {
			/* Entry already exists */
			new_prev = prev;
			new = cur;
		}
		
		if (cur->qce.id == qce_id) {
			/* Found insertion point */
			ins_prev = prev;
			ins = cur;
		}
	}
	
	if (qce_id == VTSS_QCE_ID_LAST)
		ins_prev = prev;
	
	/* Check if the place to insert was found */
	if (ins == NULL && qce_id != VTSS_QCE_ID_LAST) {
		vtss_log(VTSS_LOG_ERR,
			 "SPARX: could not find qce, qce=%ld", qce_id);        
		return VTSS_ENTRY_NOT_FOUND;
	}
	
	if (new == NULL) {
		/* Take new entry from free list */
		if ((new = qcl->qcl_list_free) == NULL) {
			vtss_log(VTSS_LOG_ERR, "SPARX: QCL is full");
			return VTSS_UNSPECIFIED_ERROR;
		}
		qcl->qcl_list_free = new->next;
	} else {
		/* Take existing entry out of list */
		if (ins_prev == new)
			ins_prev = new_prev;
		if (new_prev == NULL)
			qcl->qcl_list_used = new->next;
		else
			new_prev->next = new->next;
	}
	
	/* Insert new entry in list */
	if (ins_prev == NULL) {
		new->next = qcl->qcl_list_used;
		qcl->qcl_list_used = new;
	} else {
		new->next = ins_prev->next;
		ins_prev->next = new;
	}
	new->qce = *qce;
	
	return ht_qcl_commit(qcl_id);
}

/* Delete QCE */
vtss_rc vtss_ll_qce_del(vtss_qcl_id_t  qcl_id,
                        vtss_qce_id_t  qce_id)
{
	vtss_qcl_t *qcl;
	vtss_qcl_entry_t *cur, *prev;
	
	/* Check that the QCL ID is valid */
	if (qcl_id < VTSS_QCL_ID_START || qcl_id >= VTSS_QCL_ID_END) {
		vtss_log(VTSS_LOG_ERR, "SPARX: illegal QCL ID, qcl=%d", qcl_id);
		return VTSS_INVALID_PARAMETER;
	}
	
	qcl = &vtss_mac_state.qcl[qcl_id];
	
	for (cur = qcl->qcl_list_used, prev = NULL; cur != NULL;
	     prev = cur, cur = cur->next) {
		if (cur->qce.id == qce_id) {
			vtss_log(VTSS_LOG_DEBUG,
				 "SPARX: found existing ID, qce=%lu",
				 qce_id);
			if (prev == NULL)
				qcl->qcl_list_used = cur->next;
			else
				prev->next = cur->next;
			cur->next = qcl->qcl_list_free;
			qcl->qcl_list_free = cur;
			break;
		}
	}
	return (cur == NULL ? VTSS_ENTRY_NOT_FOUND : ht_qcl_commit(qcl_id));
}
#endif

#if defined(CONFIG_VTSS_ARCH_SPARX) && \
    defined(VTSS_FEATURE_QOS_POLICER_MC_SWITCH) && \
    defined(VTSS_FEATURE_QOS_POLICER_BC_SWITCH)
/* Calculate packet rate register field */
ulong calc_packet_rate(vtss_packet_rate_t rate, ulong *unit)
{
	ulong value;
	
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	ulong max;
	/* If the rate is greater than 1000 pps, the unit is kpps */
	max = (rate >= 1000 ? (rate/1000) : rate);
	*unit = (rate >= 1000 ? 0 : 1);
	for (value = 15; value != 0; value--) {
		if (max >= (ulong)(1<<value))
			break;
	}
#endif
	return value;
}
#endif

/* Set switch QoS setup */
vtss_rc vtss_ll_qos_setup_set(const vtss_qos_setup_t *qos)
{
#ifdef VTSS_FEATURE_QOS_DSCP_REMARK
	{
		ulong i, low = 0, high = 0;
		
		for (i = 0; i < 64; i++) {
			if (qos->dscp_remark[i]) {
				if (i < 32)
					low |= (1<<i);
				else
					high |= (1<<(i-32));
			}
		}
		HT_WR(ANALYZER, 0, DSCPSELLOW, low);
		HT_WR(ANALYZER, 0, DSCPSELHIGH, high);
	}
#endif
	
#if defined(VTSS_FEATURE_QOS_POLICER_MC_SWITCH) && \
    defined(VTSS_FEATURE_QOS_POLICER_BC_SWITCH)
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	{
		ulong unit_bc, unit_mc, unit_uc;
		
		HT_WRF(ANALYZER, 0, STORMLIMIT, 28, 0xf, 6); /* Burst of 64 frames allowed */
		HT_WRF(ANALYZER, 0, STORMLIMIT, 8, 0xf, calc_packet_rate(qos->policer_bc, &unit_bc));
		HT_WRF(ANALYZER, 0, STORMLIMIT, 4, 0xf, calc_packet_rate(qos->policer_mc, &unit_mc));
		HT_WRF(ANALYZER, 0, STORMLIMIT, 0, 0xf, calc_packet_rate(qos->policer_uc, &unit_uc));
		HT_WRF(ANALYZER, 0, STORMLIMIT_ENA, 2, 0x1,
		       MAKEBOOL01(qos->policer_bc != VTSS_PACKET_RATE_DISABLED));
		HT_WRF(ANALYZER, 0, STORMLIMIT_ENA, 1, 0x1,
		       MAKEBOOL01(qos->policer_mc != VTSS_PACKET_RATE_DISABLED));
		HT_WRF(ANALYZER, 0, STORMLIMIT_ENA, 0, 0x1,
		       MAKEBOOL01(qos->policer_uc != VTSS_PACKET_RATE_DISABLED));
#ifdef CONFIG_VTSS_ARCH_SPARX_28
		HT_WRF(ANALYZER, 0, STORMPOLUNIT, 2, 0x1, unit_bc);
		HT_WRF(ANALYZER, 0, STORMPOLUNIT, 1, 0x1, unit_mc);
		HT_WRF(ANALYZER, 0, STORMPOLUNIT, 0, 0x1, unit_uc);
#endif
	}
#endif
#endif
	
#ifdef VTSS_FEATURE_QOS_POLICER_CPU_SWITCH
	{
		ulong              i, unit;
		vtss_packet_rate_t rate;
		
		for (i = 3; i < 6; i++) {
			rate = (i == 3 ? qos->policer_learn : 
		i == 4 ? qos->policer_cat : qos->policer_mac);
		HT_WRF(ANALYZER, 0, STORMLIMIT, 4*i, 0xf, calc_packet_rate(rate, &unit));
		HT_WRF(ANALYZER, 0, STORMPOLUNIT, i, 0x1, unit);
		HT_WRF(ANALYZER, 0, STORMLIMIT_ENA, i, 0x1, 
			rate == VTSS_PACKET_RATE_DISABLED ? 0 : 1);
		}
	}
#endif
	
	return VTSS_OK;
}

/* Read port counters */
vtss_rc vtss_ll_port_counters_get(uint port_no, vtss_chip_counters_t * counters)
{
	uint port_on_chip = HT_CHIP_PORT(port_no);
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	
	/* Counter base address */
#ifdef CONFIG_VTSS_GROCX
#define COUNTER_BASE 0x1800
#else
#define COUNTER_BASE 0x3000
#endif
	
	ulong value;
	
	/* Read Rx counters in 32-bit mode */
	HT_WR(PORT, port_on_chip, C_CNTADDR, (1<<31) | (COUNTER_BASE<<0));
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->rx_octets);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->rx_packets);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &value); /* c_rxbcmc */
	HT_RD(PORT, port_on_chip, C_CNTDATA, &value); /* c_rxbadpkt */
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->rx_broadcasts);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->rx_multicasts);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->rx_64);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->rx_65_127);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->rx_128_255);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->rx_256_511);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->rx_512_1023);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->rx_1024_1526);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->rx_1527_max);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->rx_pauses);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->rx_drops);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->rx_local_drops);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->rx_classified_drops);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->rx_crc_align_errors);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->rx_shorts);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->rx_longs);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->rx_fragments);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->rx_jabbers);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &value); /* c_rxctrl */
	HT_RD(PORT, port_on_chip, C_CNTDATA, &value); /* c_rxgoodpkt */
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->rx_class[0]);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->rx_class[1]);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->rx_class[2]);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->rx_class[3]);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &value); /* c_rxtotdrop */
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->rx_unicast);
	
	/* Read Tx counters in 32-bit mode */
	HT_WR(PORT, port_on_chip, C_CNTADDR, (1<<31) | ((COUNTER_BASE+0x40)<<0));
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->tx_octets);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->tx_packets);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &value); /* c_txbcmc */
	HT_RD(PORT, port_on_chip, C_CNTDATA, &value); /* c_txbadpkt */
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->tx_broadcasts);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->tx_multicasts);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->tx_64);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->tx_65_127);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->tx_128_255);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->tx_256_511);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->tx_512_1023);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->tx_1024_1526);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->tx_1527_max);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->tx_pauses);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->tx_fifo_drops);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->tx_drops);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->tx_collisions);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &value); /* c_txcfidrop */
	HT_RD(PORT, port_on_chip, C_CNTDATA, &value); /* c_txgoodpkt */
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->tx_class[0]);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->tx_class[1]);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->tx_class[2]);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->tx_class[3]);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &value); /* c_txtotdrop */
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->tx_unicast);
	HT_RD(PORT, port_on_chip, C_CNTDATA, &counters->tx_aging);
#else
	HT_RD(PORT, port_on_chip, C_RXOCT, &counters->rx_octets);
	HT_RD(PORT, port_on_chip, C_TXOCT, &counters->tx_octets);
#ifdef CONFIG_VTSS_ARCH_SPARX
	HT_RD(PORT, port_on_chip, C_RX0, &counters->rx_packets);
	HT_RD(PORT, port_on_chip, C_TX0,&counters->tx_packets);
#endif
#endif
	return VTSS_OK;
}

/* Set aggregation mode */
vtss_rc vtss_ll_aggr_mode_set(const vtss_aggr_mode_t *mode)
{
#ifdef VTSS_FEATURE_AGGR_MODE_ADV
	ulong          value;
	vtss_port_no_t port_no;
	uint           port_on_chip;
	
	/* Setup analyzer */
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	value = (((mode->random ? 1 : 0)<<3) |
		((mode->sip_dip_enable || mode->sport_dport_enable ? 1 : 0)<<2) |
		((mode->dmac_enable ? 1 : 0)<<1) |
		((mode->smac_enable ? 1 : 0)<<0));
#else
	if (mode->sip_dip_enable || mode->sport_dport_enable) {
		if (mode->smac_enable || mode->dmac_enable) {
			/* SMAC XOR DMAC XOR IPINFO selected */
			value = AGGRCNTL_MODE_SMAC_DMAC_IP;
		} else {
			/* IPINFO selected */
			value = AGGRCNTL_MODE_IP;
		}
	} else {
		value = 0;
		if (mode->smac_enable) {
			/* SMAC included */
			value |= AGGRCNTL_MODE_SMAC;
		}
		if (mode->dmac_enable) {
			/* DMAC included */
			value |= AGGRCNTL_MODE_DMAC;
		}
		if (value==0) {
			/* All fields disabled, select IPINFO */
			value = AGGRCNTL_MODE_IP;
		}
	}
#ifdef VTSS_FEATURE_AGGR_MODE_RANDOM
	/* Random mode takes precedence, if enabled */
	if (mode->random) {
		value = AGGRCNTL_MODE_PSEUDORANDOM;
	}
#endif
#endif
	HT_WR(ANALYZER, 0, AGGRCNTL, value);
	/* Setup categorizer */
	value = 0;
	if (mode->sip_dip_enable) {
		/* SIP/DIP included */
		value |= (1<<1);
	}
	if (mode->sport_dport_enable) {
		/* SPORT/DPORT included */
		value |= (1<<0);
	}
	for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
		port_on_chip = HT_CHIP_PORT(port_no);
		HT_WRF(PORT, port_on_chip, CAT_OTHER_CFG, 0, 0x3, value);
	}
#else
	HT_WR(ANALYZER, 0, AGGRCNTL, 
	      ((mode->smac_enable ? 1 : 0)<<0) |
	      ((mode->dmac_enable ? 1 : 0)<<1));
#endif
	
	return VTSS_OK;
}

#ifdef VTSS_FEATURE_LEARN_PORT
/* Set learn port mode */
vtss_rc vtss_ll_learn_port_mode_set(const vtss_port_no_t      port_no,
                                    vtss_learn_mode_t * const learn_mode)
{
	uint  port_on_chip = HT_CHIP_PORT(port_no);
	
	HT_WRF(ANALYZER, 0, LEARNDROP, port_on_chip, 0x1, learn_mode->discard);
	HT_WRF(ANALYZER, 0, LEARNAUTO, port_on_chip, 0x1, learn_mode->automatic);
	HT_WRF(ANALYZER, 0, LEARNCPU, port_on_chip, 0x1, learn_mode->cpu);
	
	if (!learn_mode->automatic) {
		/* Flush entries previously learned on port to avoid continous refreshing */
		HT_WRF(ANALYZER, 0, LERNMASK, port_on_chip, 0x1, 0);
		VTSS_RC(vtss_ll_mac_table_flush(1, port_no, 0, 0));
		VTSS_RC(ht_port_learn_set(port_no));
	}
	
	return VTSS_OK;
}
#endif

#ifdef VTSS_FEATURE_LEARN_SWITCH
/* Set learn mode */
vtss_rc vtss_ll_learn_mode_set(vtss_learn_mode_t * const learn_mode)
{
	uint cpu_bit;
	
	cpu_bit = 24;
	
	HT_WRM(ANALYZER, 0, ADVLEARN,
	       ((learn_mode->automatic ? 1 : 0)<<31) |
	       ((learn_mode->discard ? 1 : 0)<<30) |
	       ((learn_mode->cpu ? 1 : 0)<<cpu_bit),
	       (1<<31) | (1<<30) | (1<<cpu_bit));
	return VTSS_OK;
}
#endif

/* Set logical port mapping */
vtss_rc vtss_ll_pmap_table_write(vtss_port_no_t physical_port, vtss_port_no_t logical_port)
{
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	uint port_on_chip, lport;
	
	port_on_chip = HT_CHIP_PORT(physical_port);
	lport = HT_CHIP_PORT(logical_port);
	HT_WRF(PORT, port_on_chip, STACKCFG, 0, 0x1F, lport);
#endif
	return VTSS_OK;
}

/* Set VLAN port mode */
vtss_rc vtss_ll_vlan_port_mode_set(vtss_port_no_t port_no, const vtss_vlan_port_mode_t *vlan_mode)
{
	uint port_on_chip = HT_CHIP_PORT(port_no);
	ulong value, mask;
	vtss_vid_t uvid;
	
	/* Ingress */
#ifdef CONFIG_VTSS_ARCH_SPARX
	value = vlan_mode->pvid;
	HT_WRF(PORT, port_on_chip, CAT_PORT_VLAN, 0, 0xFFF, value);
	value = (((vlan_mode->aware ? 0 : 1)<<8) |
		((vlan_mode->aware ? 0 : 1)<<7));
#ifdef VTSS_FEATURE_VLAN_TYPE_STAG
	value |= ((vlan_mode->stag ? 1 : 0)<<5);
#endif
	mask = ((1<<8) | (1<<7) | (1<<5));
	HT_WRM(PORT, port_on_chip, CAT_VLAN_MISC, value, mask);
	HT_WRM(PORT, port_on_chip, CAT_DROP, 
	       ((vlan_mode->frame_type == VTSS_VLAN_FRAME_TAGGED ? 1 : 0)<<2) |
	       ((vlan_mode->frame_type == VTSS_VLAN_FRAME_UNTAGGED ? 1 : 0)<<1),
	       (1<<2) | (1<<1));
#endif /* SPARX */
	
#ifdef VTSS_FEATURE_VLAN_INGR_FILTER_PORT
	HT_WRF(ANALYZER, 0, VLANMASK, port_on_chip, 0x1, vlan_mode->ingress_filter ? 1 : 0);
#endif
	
	/* Egress */
	uvid = vlan_mode->untagged_vid;
	value = (((vlan_mode->untagged_vid & 0xFFF)<<4) |
		((uvid != VTSS_VID_ALL && uvid != VTSS_VID_NULL ? 1 : 0)<<3) |
		(uvid != VTSS_VID_ALL ? 1 : 0)<<0);
	mask = ((0xFFF<<4) | (1<<3) | (1<<0));
	HT_WRM(PORT, port_on_chip, TXUPDCFG, value, mask);
	
	return VTSS_OK;
}

/* ================================================================= *
 * ================================================================= *
 * = VLAN Table ==================================================== *
 * ================================================================= *
 * ================================================================= */

static vtss_rc ht_vlan_table_idle(void)
{
	ulong cmd;
	
	while (1) {
		HT_RDF(ANALYZER, 0, VLANACES, 0, 0x3, &cmd);
		if (cmd == VLAN_CMD_IDLE)
			break;
	}
	return VTSS_OK;
}

/* Set VLAN port members */
vtss_rc vtss_ll_vlan_table_write(vtss_vid_t vid, BOOL member[VTSS_PORT_ARRAY_SIZE])
{
	vtss_port_no_t    port_no;
	BOOL              port_member[VTSS_PORT_ARRAY_SIZE], mirror = 0;
	vtss_mstp_entry_t *mstp_entry;
	ulong             value;
	
	/* Lookup MSTP entry */
	mstp_entry = &vtss_mac_state.mstp_table[vtss_mac_state.vlan_table[vid].msti];
	
	/* Enable mirror port if egress mirroring enabled on a member port */
	for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
		port_member[port_no] = (member[port_no] && 
			mstp_entry->state[port_no] == VTSS_MSTP_STATE_FORWARDING);
		if (port_member[port_no] && vtss_mac_state.mirror_egress[port_no])
			mirror = 1;
	}
	value = (vid<<0);
#ifdef VTSS_FEATURE_ISOLATED_PORT
	value |= ((vtss_mac_state.vlan_table[vid].isolated ? 1 : 0)<<15);
#endif
	HT_WR(ANALYZER, 0, VLANTINDX, value);
	
	HT_WR(ANALYZER, 0, VLANACES, 
	      (vtss_portmask2chip(port_member)<<2) |
	      (VLAN_CMD_WRITE<<0));
	
	return ht_vlan_table_idle();
}

/* Clear VLAN table (by default all ports are members of all VLANs) */
static vtss_rc ht_vlan_table_clear(void)
{
	vtss_vid_t vid;
	
	for (vid = VTSS_VID_NULL; vid < VTSS_VIDS; vid++) {
		HT_WR(ANALYZER, 0, VLANTINDX, vid<<0);
		HT_WR(ANALYZER, 0, VLANACES, VLAN_CMD_WRITE<<0);
		VTSS_RC(ht_vlan_table_idle());
	}
	return VTSS_OK;
}

/* Set VLAN MSTP instance */
vtss_rc vtss_ll_vlan_table_mstp_set(vtss_vid_t vid, uint msti)
{
	/* Update VLAN entry */
	return vtss_ll_vlan_table_write(vid,
					vtss_mac_state.vlan_table[vid].member);
}

/* Set MSTP state for port and mstp instance */
vtss_rc vtss_ll_mstp_table_write(vtss_port_no_t port_no, vtss_msti_t msti, 
                                 vtss_mstp_state_t state)
{
	vtss_vid_t        vid;
	vtss_vlan_entry_t *vlan_entry;
	
	/* Update all VLANs mapping to MSTI */
	for (vid = VTSS_VID_NULL; vid < VTSS_VIDS; vid++) {
		vlan_entry = &vtss_mac_state.vlan_table[vid];
		if (vlan_entry->enabled && vlan_entry->msti == msti)
			VTSS_RC(vtss_ll_vlan_table_write(vid, vlan_entry->member));
	}
	return VTSS_OK;
}

#ifdef VTSS_FEATURE_ISOLATED_PORT
/* Set isolated ports */
vtss_rc vtss_ll_isolated_ports_set(const BOOL member[VTSS_PORT_ARRAY_SIZE])
{
	vtss_port_no_t port_no;
	BOOL           port_member[VTSS_PORT_ARRAY_SIZE];
	
	for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++)
		port_member[port_no] = (member[port_no] ? 0 : 1); 
	
	HT_WR(ANALYZER, 0, PVLAN_MASK, 
	      vtss_portmask2chip(port_member) | (1<<VTSS_CHIP_PORT_CPU));
	
	return VTSS_OK;
}
#endif

/* ================================================================= *
 * ================================================================= *
 * = MAC Table ===================================================== *
 * ================================================================= *
 * ================================================================= */

static vtss_rc ht_mac_table_idle(void)
{
	ulong cmd;
	
	while (1) {
		HT_RDF(ANALYZER, 0, MACACCES, 0, 0x7, &cmd);
		if (cmd == MAC_CMD_IDLE)
			break;
	}
	return VTSS_OK;
}

#ifdef VTSS_FEATURE_MAC_AGE_AUTO
/* Set MAC address age time */
vtss_rc vtss_ll_mac_table_age_time_set(vtss_mac_age_time_t age_time)
{
	vtss_mac_age_time_t atime;
	
	/* Scan two times per age time */
	atime = age_time/2;
	if (atime > 0xfffff)
		atime = 0xfffff;
	HT_WR(ANALYZER, 0, AUTOAGE, atime);
	return VTSS_OK;
}
#endif

/* Age MAC address table */
vtss_rc vtss_ll_mac_table_age(BOOL pgid_age, vtss_pgid_no_t pgid_no, 
                              BOOL vid_age, vtss_vid_t vid)
{
	uint pgid = (pgid_age ? vtss_pgid2chip(pgid_no) : 0);
	
	HT_WR(ANALYZER, 0, ANAGEFIL,
	      ((pgid_age ? 1 : 0)<<31) | (pgid<<16) |
	      ((vid_age ? 1 : 0)<<15) | (vid<<0));
	HT_WR(ANALYZER, 0, MACACCES, MAC_CMD_TABLE_AGE<<0);
	VTSS_RC(ht_mac_table_idle());
	
	return VTSS_OK;
}

/* Flush MAC address table */
vtss_rc vtss_ll_mac_table_flush(BOOL pgid_age, vtss_pgid_no_t pgid_no, 
                                BOOL vid_age, vtss_vid_t vid)
{
	/* Age twice instead of flushing. 
	 * This ensures that sticky bit for aged entries is updated
	 */ 
	VTSS_RC(vtss_ll_mac_table_age(pgid_age, pgid_no, vid_age, vid));
	return vtss_ll_mac_table_age(pgid_age, pgid_no, vid_age, vid);
}

/* Learn (VID, MAC) */
/* Note: Uses pgid_no, doesn't use entry->destination list. */
vtss_rc vtss_ll_mac_table_learn(const vtss_mac_table_entry_t *entry, vtss_pgid_no_t pgid_no)
{
	ulong mach,macl;
	uint  pgid = 0, locked, aged, fwd_kill = 0, ipv6_mask = 0;
	
	/* Calculate MACH/MACL registers */
	mach = ((entry->vid_mac.vid<<16) |
		(entry->vid_mac.mac.addr[0]<<8) |
		(entry->vid_mac.mac.addr[1]<<0));
	macl = ((entry->vid_mac.mac.addr[2]<<24) |
		(entry->vid_mac.mac.addr[3]<<16) |
		(entry->vid_mac.mac.addr[4]<<8) |
		(entry->vid_mac.mac.addr[5]<<0));
	
#ifdef CONFIG_VTSS_ARCH_SPARX
	if (pgid_no == VTSS_PGID_NONE) {
		/* IPv4/IPv6 multicast entry */
		ulong mask;
		
		locked = 1;
		aged = 1;
		mask = vtss_portmask2chip(vtss_mac_state.pgid_table[pgid_no].member); 
		if (entry->vid_mac.mac.addr[0] == 0x01) {
			/* IPv4 multicast entry */
#ifdef CONFIG_VTSS_ARCH_SPARX_28
			/* Encode port mask directly */
			macl = ((macl & 0x00FFFFFF) | ((mask<<24) & 0xFF000000));
			mach = ((mach & 0xFFFF0000) | ((mask>>8) & 0x0000FFFF));
			pgid = ((mask>>24) & 0xF);
#endif /* SPARX_28 */
		} else {
			/* IPv6 multicast entry */
			fwd_kill = 1;
#ifdef CONFIG_VTSS_ARCH_SPARX_28
			/* Encode port mask directly */
			mach = ((mach & 0xFFFF0000) | (mask & 0x0000FFFF));
			pgid = ((mask>>16) & 0x3F);
			ipv6_mask = ((mask>>22) & 0x3F);
#endif
		}
	} else
#endif
	{
		/* Not IP multicast entry */
		pgid = vtss_pgid2chip(pgid_no);
		locked = entry->locked;
		aged = 0;
	}
	HT_WR(ANALYZER, 0, MACHDATA,mach);
	HT_WR(ANALYZER, 0, MACLDATA,macl);
	HT_WR(ANALYZER, 0, MACACCES,
	      (ipv6_mask<<16) |
	      (MAKEBOOL01(entry->copy_to_cpu)<<14) |
	      (fwd_kill<<13) |      /* forward kill */
	      (0<<12) |             /* Ignore VLAN */
	      (MAKEBOOL01(aged)<<11) |
	      (1<<10) |             /* Valid */
	      (MAKEBOOL01(locked)<<9) |
	      (pgid<<3) |
	      (MAC_CMD_LEARN<<0));
	return ht_mac_table_idle();
}

/* Unlearn (VID, MAC) */
vtss_rc vtss_ll_mac_table_unlearn(const vtss_vid_mac_t *vid_mac)
{
	uint locked = 0, aged = 0, fwd_kill = 0;
	
#ifdef CONFIG_VTSS_ARCH_SPARX
	if (
		VTSS_MAC_IPV4_MC(vid_mac->mac.addr)) {
		/* IPv4 multicast */
		locked = 1;
	} 
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	if (
		VTSS_MAC_IPV6_MC(vid_mac->mac.addr)) {
		/* IPv6 multicast */
		locked = 1;
		fwd_kill = 1;
	}
#endif
	if (locked) {
		aged = 1;
	}
#endif
	HT_WR(ANALYZER,0, MACHDATA,
	      (vid_mac->vid        <<16) |
	      (vid_mac->mac.addr[0]<< 8) |
	      (vid_mac->mac.addr[1]<< 0));
	HT_WR(ANALYZER, 0, MACLDATA,
	      (vid_mac->mac.addr[2]<<24) |
	      (vid_mac->mac.addr[3]<<16) |
	      (vid_mac->mac.addr[4]<<8) |
	      (vid_mac->mac.addr[5]<<0));
	HT_WR(ANALYZER, 0, MACACCES, 
	      (fwd_kill<<13) |
	      (MAKEBOOL01(aged)<<11) |
	      (MAKEBOOL01(locked)<<9) |
	      MAC_CMD_FORGET<<0);
	return ht_mac_table_idle();
}

/* Note: Updates pgid_no, doesn't update entry->destination list. */
/* Note: Leaves entry and pgid_no unchanged if result not valid. */
static vtss_rc ht_get_mac_table_result(vtss_mac_table_entry_t * const entry,
                                       vtss_pgid_no_t * const pgid_no)
{
	ulong value, mach, macl;
	uint  pgid;
	
	HT_RD(ANALYZER, 0, MACACCES, &value);
	
	/* Check if entry is valid */
	if (!(value & (1<<10))) 
		return VTSS_ENTRY_NOT_FOUND;
	
	entry->copy_to_cpu = MAKEBOOL01(value & (1<<14));
	entry->aged        = MAKEBOOL01(value & (1<<11));
	entry->locked      = MAKEBOOL01(value & (1<<9));
	pgid = ((value>>3) & 0x3F);
	
	HT_RD(ANALYZER, 0, MACHDATA, &mach);
	HT_RD(ANALYZER, 0, MACLDATA, &macl);
	
#ifdef CONFIG_VTSS_ARCH_SPARX
	if (entry->locked && 
		entry->aged) {
		/* IPv4/IPv6 multicast address */
		ulong          mask;
		vtss_port_no_t port_no;
		
		*pgid_no = VTSS_PGID_NONE;
		
		/* Read encoded port mask and update address registers */
		if (value & (1<<13)) {
			/* IPv6 entry (FWD_KILL set) */
			mask = ((((value>>16) & 0x3F)<<22) | (pgid<<16) | (mach & 0xFFFF));
			mach = ((mach & 0xFFFF0000) | 0x00003333);
		} else {
			/* IPv4 entry */
			mask = ((pgid<<24) | ((mach<<8) & 0xFFFF00) | ((macl>>24) & 0xFF));
			mach = ((mach & 0xFFFF0000) | 0x00000100);
			macl = ((macl & 0x00FFFFFF) | 0x5E000000);
		}    
		
		/* Convert port mask */
		for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
			vtss_mac_state.pgid_table[*pgid_no].member[port_no] = 
				MAKEBOOL01(mask & (1 << vtss_mac_state.port_map.chip_port[port_no]));
		}
	} else
#endif
	{
		*pgid_no = vtss_chip2pgid(pgid);
	}
	entry->vid_mac.vid = ((mach>>16) & 0xFFF);
	entry->vid_mac.mac.addr[0] = (uchar)((mach>>8) & 0xFF);
	entry->vid_mac.mac.addr[1] = (uchar)((mach>>0) & 0xFF);
	entry->vid_mac.mac.addr[2] = (uchar)((macl>>24) & 0xFF);
	entry->vid_mac.mac.addr[3] = (uchar)((macl>>16) & 0xFF);
	entry->vid_mac.mac.addr[4] = (uchar)((macl>>8) & 0xFF);
	entry->vid_mac.mac.addr[5] = (uchar)((macl>>0) & 0xFF);
	
	return VTSS_OK;
}

/* Note: Updates pgid_no, doesn't update entry->destination list. */
/* Note: index is different than in datasheet. The API uses the
 *       two LSBits for bucket and the MSBits for line in the MAC table. */
static vtss_rc ht_mac_table_read(uint index_on_chip,
                                 vtss_mac_table_entry_t * const entry,
                                 vtss_pgid_no_t * const pgid_no,
                                 BOOL shadow)
{
	HT_WR(ANALYZER, 0, MACTINDX,
	      (shadow                        <<13) |
	      ((index_on_chip&0x3)/*bucket*/ <<11) |
	      ((index_on_chip>>2)/*index*/   << 0) );
	HT_WR(ANALYZER, 0, MACACCES, MAC_CMD_READ<<0);
	VTSS_RC(ht_mac_table_idle());
	return ht_get_mac_table_result(entry, pgid_no);
}

/* Lookup (VID, MAC) */
/* Note: Updates pgid_no, doesn't update entry->destination list. */
vtss_rc vtss_ll_mac_table_lookup(vtss_mac_table_entry_t *entry, vtss_pgid_no_t *pgid_no)
{
	uint                   vid = entry->vid_mac.vid;
	uchar                  *mac = &entry->vid_mac.mac.addr[0];
	uint                   bucket;
	vtss_mac_table_entry_t try_entry;
	vtss_pgid_no_t         try_pgid_no;
	
	ulong mach = (((vid & 0x3F)<<16) | (mac[0]<<8) | (mac[1]<<0));
	ulong macl = ((mac[2]<<24) | (mac[3]<<16) | (mac[4]<<8) | mac[5]);
	uint hash4 = 4*(((mach>>12) & 0x7FF) ^
		     ((mach>>1) & 0x7FF) ^
		     (((mach<<10) & 0x400) | ((macl>>22) & 0x3FF)) ^
		     ((macl>>11) & 0x7FF) ^
		     (macl & 0x7FF));
	
	for (bucket = 0; bucket < 4; bucket++) {
        /* Note: idx is different than in datasheet. The API uses the
		*       two LSBits for bucket and the MSBits for line in the MAC table. */
		VTSS_RC(ht_mac_table_read(hash4 | bucket, &try_entry, &try_pgid_no, 1));
		if (try_entry.vid_mac.vid == vid &&
		    try_entry.vid_mac.mac.addr[0] == mac[0] &&
		    try_entry.vid_mac.mac.addr[1] == mac[1] &&
		    try_entry.vid_mac.mac.addr[2] == mac[2] &&
		    try_entry.vid_mac.mac.addr[3] == mac[3] &&
		    try_entry.vid_mac.mac.addr[4] == mac[4] &&
		    try_entry.vid_mac.mac.addr[5] == mac[5]) {
			*entry = try_entry;
			*pgid_no = try_pgid_no;
			return VTSS_OK;
		}
	}
	return VTSS_ENTRY_NOT_FOUND;
}

/* Direct MAC read */
/* Note: index is different than in datasheet. The API uses the
 *       two LSBits for bucket and the MSBits for line in the MAC table. */
vtss_rc vtss_ll_mac_table_read(uint idx, vtss_mac_table_entry_t *entry, 
                               vtss_pgid_no_t *pgid_no)
{
	return ht_mac_table_read(idx-VTSS_MAC_ADDR_START,
				 entry, pgid_no, 0/*shadow*/);
}

/* Get next */
vtss_rc vtss_ll_mac_table_get_next(const vtss_vid_mac_t *vid_mac,
				   vtss_mac_table_entry_t *entry,
                                   vtss_pgid_no_t *pgid_no)
{
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	HT_WR(ANALYZER, 0, ANAGEFIL, 0);
	HT_WR(ANALYZER, 0, MACHDATA,
	      (vid_mac->vid<<16) |
	      (vid_mac->mac.addr[0]<<8) |
	      (vid_mac->mac.addr[1]<<0));
	HT_WR(ANALYZER, 0, MACLDATA,
	      (vid_mac->mac.addr[2]<<24) |
	      (vid_mac->mac.addr[3]<<16) |
	      (vid_mac->mac.addr[4]<<8) |
	      (vid_mac->mac.addr[5]<<0));
	HT_WR(ANALYZER, 0, MACACCES, MAC_CMD_GET_NEXT<<0);
	VTSS_RC(ht_mac_table_idle());
	VTSS_RC(ht_get_mac_table_result(entry, pgid_no));
#endif /* SPARX_28 */
	return VTSS_OK;
}

/* Get MAC table status */
vtss_rc vtss_ll_mac_table_status_get(vtss_mac_table_status_t *status) 
{
	ulong value;
	
	/* Read and clear sticky register */
	HT_RD(ANALYZER, 0, ANEVENTS, &value);
	HT_WR(ANALYZER, 0, ANEVENTS, value & ((1<<15) | (1<<16) | (1<<17) | (1<<20)));
	
	/* Detect learn events */
	status->learned = ((value & (1<<16)) ? 1 : 0);
	
	/* Detect replace events */
	status->replaced = ((value & (1<<17)) ? 1 : 0);
	
	/* Detect port move events */
	status->moved = ((value & (1<<15)) ? 1 : 0);
	
	/* Detect age events */
	status->aged = ((value & (1<<20)) ? 1 : 0);
	return VTSS_OK;
}

/* Write PGID entry */
vtss_rc vtss_ll_pgid_table_write(vtss_pgid_no_t pgid_no, BOOL member[VTSS_PORT_ARRAY_SIZE])
{
	vtss_port_no_t port_no;
	BOOL           port_member[VTSS_PORT_ARRAY_SIZE], mirror = 0;
	ulong          mask;
	uint           pgid;
	
	/* Enable mirror port if egress mirroring enabled on a member port */
	for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
		port_member[port_no] = member[port_no];
		if (member[port_no] && vtss_mac_state.mirror_egress[port_no])
			mirror = 1;
	}
	mask = vtss_portmask2chip(port_member);
	pgid = vtss_pgid2chip(pgid_no);
	HT_WR(ANALYZER, 0, DSTMASKS + pgid, mask);
	
	return VTSS_OK;
}

/* Write source port entry */
vtss_rc vtss_ll_src_table_write(vtss_port_no_t port_no, BOOL member[VTSS_PORT_ARRAY_SIZE])
{
	uint  port_on_chip = HT_CHIP_PORT(port_no);
	ulong value, mask;
	
	mask = (VTSS_CHIP_PORTMASK<<0);
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	mask |= (1<<(VTSS_CHIP_PORTS + 2)); /* Remove CPU copy flag */
#else
					    /* If egress mirroring enabled and ingress mirroring disabled,
	forwarding to mirror port is disabled */
	if (vtss_mac_state.mirror_egress[port_no] && !vtss_mac_state.mirror_ingress[port_no])
		member[vtss_mac_state.mirror_port] = 0;
#endif	
	value = vtss_portmask2chip(member);
	HT_WRM(ANALYZER, 0, SRCMASKS + port_on_chip, value, mask);
	return VTSS_OK;
}

/* Set monitor port for mirroring */
vtss_rc vtss_ll_mirror_port_set(vtss_port_no_t port_no)
{
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	BOOL           member[VTSS_PORT_ARRAY_SIZE];
	vtss_port_no_t port;
	
	/* Calculate mirror ports */
	for (port = VTSS_PORT_NO_START; port < VTSS_PORT_NO_END; port++) {
		member[port] = (port == port_no ? 1 : 0);
	}
	HT_WR(ANALYZER, 0, MIRRORPORTS, vtss_portmask2chip(member));
#else
	uint port_on_chip = HT_CHIP_PORT(port_no);
	
	HT_WRF(ANALYZER, 0, AGENCNTL, 0, 0x1f, port_on_chip);
#endif
	return VTSS_OK;
}

/* Enable/disable egress mirroring of port */
vtss_rc vtss_ll_dst_mirror_set(vtss_port_no_t port_no, BOOL enable)
{
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	uint port_on_chip = HT_CHIP_PORT(port_no);
	
	HT_WRF(ANALYZER, 0, EMIRRORMASK, port_on_chip, 0x1, enable ? 1 : 0);
	return VTSS_OK;
#else
	return VTSS_NOT_IMPLEMENTED;
#endif
}

/* Enable/disable ingress mirroring of port */
vtss_rc vtss_ll_src_mirror_set(vtss_port_no_t port_no, BOOL enable)
{
	uint  port_on_chip = HT_CHIP_PORT(port_no);
	ulong offset;
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	offset = (VTSS_CHIP_PORTS + 1);
#endif	
	HT_WRF(ANALYZER, 0, SRCMASKS + port_on_chip, offset, 0x1, enable ? 1 : 0);
	return VTSS_OK;
}

/* Write aggregation entry */
vtss_rc vtss_ll_aggr_table_write(vtss_ac_no_t ac, BOOL member[VTSS_PORT_ARRAY_SIZE])
{
	ulong mask = vtss_portmask2chip(member);
	
	HT_WR(ANALYZER, 0, AGGRMSKS + ac - VTSS_AC_START, mask);
	return VTSS_OK;
}

static vtss_rc ht_ipmc_flood_mask_set(ulong mask)
{
	HT_WR(ANALYZER, 0, IFLODMSK, mask);
	return VTSS_OK;
}

/* Set IP Multicast flooding ports */
vtss_rc vtss_ll_ipmc_flood_mask_set(const BOOL member[VTSS_PORT_ARRAY_SIZE])
{
	return ht_ipmc_flood_mask_set(vtss_portmask2chip(member));
}

/* PI/SI register read */
vtss_rc vtss_ll_register_read(ulong reg, ulong *value)
{
#if VTSS_ACL_DUMP
	if (reg == 0) {
		VTSS_RC(ht_acl_dump());
	}
#endif /* VTSS_ACL_DUMP */    
	
	return ht_rd((reg>>12) & 0x7, (reg>>8) & 0xF, reg & 0xFF, value);
}

/* PI/SI register write */
vtss_rc vtss_ll_register_write(ulong reg, ulong value)
{
	return ht_wr((reg>>12) & 0x7, (reg>>8) & 0xF, reg & 0xFF, value);
}

/* Get chip ID and revision */
/* Note: Use chipid=NULL for checking CPU access to the switch chip. */
vtss_rc vtss_ll_chipid_get(vtss_chipid_t *chipid)
{
	ulong value;
	
	HT_RD(SYSTEM, 0, CHIPID, &value);
	
	if (value >= 1 && value < 8) {
		/* Heathrow-II */
		if (chipid) {
			chipid->part_number = 0x7301;
			chipid->revision = value;
		}
	} else if (((value>>1) & 0x7FF)==0x74) {
		if (chipid) {
			chipid->part_number = (ushort)((value>>12) & 0xFFFF);
			chipid->revision = (value>>28)+1; /* Start at revision 1. */
			/* Map SparX-G5/SparX-G5e rev 2 to SparX-G5m */
			if (chipid->part_number == 0x7395 && chipid->revision == 3) {
				chipid->part_number = 0x7396;
			}
		}
	} else if (value == 0x00000000 || value == 0xFFFFFFFF) {
		vtss_log(VTSS_LOG_ERR,
			 "SPARX: CPU interface error, chipid=0x%08lx", value);
		return VTSS_DATA_NOT_READY;
	} else {
		vtss_log(VTSS_LOG_ERR,
			 "SPARX: CPU interface error, chipid=0x%08lx", value);
		return VTSS_UNSPECIFIED_ERROR;
	}
	return VTSS_OK;
}

/* - Runtime Optimisations ----------------------------------------- */
/* Optimization function called every second */
vtss_rc vtss_ll_optimize_1sec(void)
{
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	vtss_port_no_t port_no;
	vtss_port_setup_t *ps;
	uint port_on_chip;
	ulong value;
	
	for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
		/* Skip ports not running 10fdx or down */
		ps = &vtss_mac_state.setup[port_no];
		if (ps->fdx || ps->interface_mode.speed != VTSS_SPEED_10M ||
		    !VTSS_STP_UP(vtss_mac_state.stp_state[port_no]))
			continue;
		
		/* Read Tx packet counter, skip port if changed */
		port_on_chip = HT_CHIP_PORT(port_no);
		HT_WR(PORT, port_on_chip, C_CNTADDR, (1<<31) | (0x3042<<0));
		HT_RD(PORT, port_on_chip, C_CNTDATA, &value);
		if (vtss_mac_state.tx_packets[port_no] != value) {
			vtss_mac_state.tx_packets[port_no] = value;
			continue;
		}
		
		/* Read egress FIFO used slices */
		HT_RD(PORT, port_on_chip, FREEPOOL, &value);
		HT_RDF(PORT, port_on_chip, FREEPOOL, 0, 0x1f, &value);
		if (value == 0)
			continue;
		
		/* No Tx activity and FIFO not empty, restart port */
		vtss_log(VTSS_LOG_DEBUG, "SPARX: optimizing port, port=%d", port_no);
		VTSS_RC(vtss_ll_port_speed_mode_gmii(port_no, ps));
	}
#endif
	return VTSS_OK;
}

/* Optimization function called every 100th millisecond */
vtss_rc vtss_ll_optimize_100msec(void)
{
	return VTSS_OK;
}

#ifdef VTSS_FEATURE_SERIAL_LED
vtss_rc vtss_ll_serial_led_set(const vtss_led_port_t port, 
                               const vtss_led_mode_t mode[3])
{
	ulong value, val0, i;
	
	if (port > 29) {
		vtss_log(VTSS_LOG_ERR,
			 "SPARX: illegal port, port=%d", port);
		return VTSS_INVALID_PARAMETER;
	}
	
	/* Set shared LED/GPIO pins to LED mode */
	HT_WRM(SYSTEM, 0, GPIOCTRL, (0<<11) | (0<<10), (1<<11) | (1<<10));
	
	/* Select port */
	HT_WR(SYSTEM, 0, LEDTIMER,
	      (0<<31) | (port<<26) | (178<<16) | (25000<<0));
	
	/* Setup LED mode */
	value = 0;
	for (i = 0; i < 3; i++) {
		switch (mode[i]) {
		case VTSS_LED_MODE_DISABLED:
			val0 = 7;
			break;
		case VTSS_LED_MODE_OFF:
			val0 = 0;
			break;
		case VTSS_LED_MODE_ON:
			val0 = 1;
			break;
		case VTSS_LED_MODE_2_5:
			val0 = 2;
			break;
		case VTSS_LED_MODE_5:
			val0 = 3;
			break;
		case VTSS_LED_MODE_10:
			val0 = 4;
			break;
		case VTSS_LED_MODE_20:
			val0 = 5;
			break;
		default:
			val0 = 0;
			break;
		}
		value |= (val0 << (3*i));
	}
	HT_WR(SYSTEM, 0, LEDMODES, value);
	
	return VTSS_OK;
}
#endif

/* ================================================================= *
 *  Chip Reset, polarity/endian Setup and Interrupts
 * ================================================================= */

static vtss_rc ht_reset_memories(void)
{
	uint i, memid;
	
	for (i = 0; i < 2; i++) {
		for (memid = 0; memid <= MEMINIT_MAXID; memid++) {
			/* Skip certain blocks */
			if (memid >= MEMID_SKIP_MIN && memid <= MEMID_SKIP_MAX) 
				continue; 
			
			if (i == 0) {
				/* First round: Memory initialization */
				HT_WR(MEMINIT, S_MEMINIT, MEMINIT,(MEMINIT_CMD_INIT<<8) | memid);
				VTSS_MSLEEP(1);
			} else {
				/* Second round: Memory check */
#ifdef CONFIG_VTSS_ARCH_SPARX
				/* Skip this for SparX */
				break;
#else
				ulong value;
				
				HT_WR(MEMINIT, S_MEMINIT, MEMINIT,(MEMINIT_CMD_READ<<8) | memid);
				VTSS_MSLEEP(1);
				HT_RD(MEMINIT, S_MEMINIT, MEMRES, &value);
				if ((value & 0x3) != MEMRES_OK) {
					vtss_log(VTSS_LOG_ERR,
						 "SPARX: memory init failed, block=%d", memid);
					return VTSS_UNSPECIFIED_ERROR;
				}
#endif
			}
		}
		if (i == 0)
			VTSS_MSLEEP(30);
	}
	
	HT_WR(ANALYZER, 0, MACACCES, MAC_CMD_TABLE_CLEAR<<0);  /* Clear MAC table */
	HT_WR(ANALYZER, 0, VLANACES, VLAN_CMD_TABLE_CLEAR<<0); /* Clear VLAN table */

	vtss_acl_enable();
	
	VTSS_MSLEEP(40);
	return VTSS_OK;
}

/* Setup SI/PI CPU Inteface */
static vtss_rc ht_setup_cpu_if(const vtss_init_setup_t *setup)
{
	ulong value;
	
	/* Setup endianness. SI is always big-endian. PI depends on CPU endianness */
	value = 0x01020304; /* Endianness test value used in next line */
	value = ((setup->use_cpu_si || *((uchar *)&value) == 0x01) ? 0x99999999 : 0x81818181);
	HT_WR(SYSTEM, 0, CPUMODE, value);
	
	if (setup->use_cpu_si) {
#ifdef CONFIG_VTSS_ARCH_SPARX
		HT_WR(SYSTEM, 0, SIPAD, 0);
#endif
	}
	return VTSS_OK;
}

/* Initialize low level layer */
vtss_rc vtss_ll_start(const vtss_init_setup_t *setup)
{
	ulong         value;
	vtss_chipid_t chipid;
	
	/* I/O Layer */
	vtss_io_start();
	
	if (setup->reset_chip) {
		VTSS_RC(ht_setup_cpu_if(setup));
		
		/* Write to RESET register */
		value = (1<<0);
#ifdef CONFIG_VTSS_GROCX
		HT_WR(SYSTEM, 0, GLORESET, (1<<4)|(1<<3)|(1<<2)); /* Lock iCPU and MEM locks */
		VTSS_NSLEEP(VTSS_T_RESET);
#endif
		HT_WR(SYSTEM, 0, GLORESET, value);
		VTSS_NSLEEP(VTSS_T_RESET);
		
		VTSS_RC(ht_setup_cpu_if(setup)); /* Setup again after reset */
		
#ifdef CONFIG_VTSS_ARCH_SPARX_28
		HT_WR(SYSTEM, 0, TIMECMP, HT_TIMECMP_DEFAULT);
#ifdef CONFIG_VTSS_REF_CLK_SINGLE_ENDED
		/* Improve clock for single ended ref clock mode */
		HT_WRM(SYSTEM, 0, MACRO_CTRL, (1 << 3), (1 << 3));
#endif /* VTSS_OPT_REF_CLK_SINGLE_ENDED */
#endif		
		VTSS_RC(vtss_ll_chipid_get(&chipid));
		vtss_log(VTSS_LOG_DEBUG,
			 "SPARX: found chip, chip=VSC%04x_%d",
			 chipid.part_number, chipid.revision);
		VTSS_RC(ht_reset_memories());     /* Reset the memories. */
		
		VTSS_RC(ht_cpu_frame_reset());
		
		VTSS_RC(ht_vlan_table_clear());
#ifdef CONFIG_VTSS_GROCX
		/* Release internal PHYs from reset */
		HT_WRM(SYSTEM, 0, GLORESET, (1<<1), (1<<1));
#endif
#ifdef CONFIG_VTSS_ARCH_SPARX
		{
			uint port_on_chip;
			
			/* Setup all used chip ports */
			for (port_on_chip = 0; port_on_chip <= VTSS_CHIP_PORTS; port_on_chip++) {
				if (
					(VTSS_CHIP_PORTMASK & (1<<port_on_chip)) == 0)
					continue;
				
				/* Allow zero SMAC/DMAC, discard multicast SMAC */
				HT_WRM(PORT, port_on_chip, CAT_DROP, (1<<6) | (0<<0), (1<<6) | (1<<0));
#ifdef CONFIG_VTSS_ARCH_SPARX_28
				/* Clear counters */
				HT_WR(PORT, port_on_chip, C_CLEAR, 0);
				
				/* Enable ACLs and physical port in IFH for LACP frames */
#ifdef CONFIG_VTSS_GROCX
				if (port_on_chip != 4) /* ACL disabled on internal port */
#endif
					HT_WR(PORT, port_on_chip, MISCCFG, (1<<3) | (1<<2));
#endif
				ht_acl_reset_port(port_on_chip);
			}
#ifdef CONFIG_VTSS_ARCH_SPARX_28
			/* E-StaX: 32/16 kB for Q0/Q1. G-RocX: 16/8 kB for Q0/Q1 */
			HT_WR(PORT, VTSS_CHIP_PORT_CPU, Q_EGRESS_WM, (0<<24) | (0<<16) | (8<<8) | (16<<0));
			HT_WR(PORT, VTSS_CHIP_PORT_CPU, MISCCFG, 1<<1); /* Enable dropping */
#else
			/* Reset capture block */
			HT_WR(CAPTURE, S_CAPTURE_RESET, CAPRST, 0);
#endif
		}
#endif
		{
			vtss_ac_no_t ac;
			ulong        mask = VTSS_CHIP_PORTMASK;
			
			/* Enable all ports in IP MC flood mask */
			VTSS_RC(ht_ipmc_flood_mask_set(mask));
			
#ifdef CONFIG_VTSS_ARCH_SPARX_28
			/* Ignore SMAC flags to avoid frames from own MAC addresses
			 * to be looped back when injected via CPU device
			 */
			HT_WRF(ANALYZER, 0, AGENCNTL, 13, 0x1, 1);
			mask |= (1<<VTSS_CHIP_PORT_CPU);
#endif
			/* Enable all ports in RECVMASK */
			HT_WR(ANALYZER, 0, RECVMASK, mask);
			
			/* Clear aggregation masks */
			for (ac = VTSS_AC_START; ac < VTSS_AC_END; ac++) {
				HT_WR(ANALYZER, 0, AGGRMSKS + ac - VTSS_AC_START, 0);
			}
		}
		
		/* Disable learning for frames discarded by VLAN ingress filtering */
		HT_WRF(ANALYZER, 0, ADVLEARN, 29, 0x1, 0x1);
		/* Disable learning */
		HT_WR(ANALYZER, 0, LERNMASK, 0);
		return VTSS_OK;
	} 
	return VTSS_NOT_IMPLEMENTED;
}
