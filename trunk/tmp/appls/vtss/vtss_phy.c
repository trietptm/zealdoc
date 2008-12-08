/*

 Vitesse PHY API software.

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
 
 $Id: vtss_phy.c,v 1.12 2008-12-08 06:10:10 zhenglv Exp $
 $Revision: 1.12 $

*/

#include "vtss_priv.h"
#include <setjmp.h>
#include <eloop.h>

/* Global variables for API state information */
/* PHY I/O Layer state information */
vtss_phy_io_state_t vtss_phy_io_state;
vtss_phy_state_t vtss_phy_state;

vtss_rc vtss_phy_start(void)
{
	vtss_port_no_t port_no;

	memset(&vtss_phy_io_state, 0, sizeof (vtss_phy_io_state_t));
	memset(&vtss_phy_state, 0, sizeof (vtss_phy_state));
	
	vtss_phy_state.port_array_size = VTSS_PORTS;
	for (port_no=0; port_no < vtss_phy_state.port_array_size; port_no++) {
		vtss_phy_state.port[port_no].map.bus = -1;
		vtss_phy_state.port[port_no].map.addr = -1;
		vtss_phy_state.port[port_no].status.link = 0;
	}
	return VTSS_OK;
}

vtss_rc vtss_phy_port_map_set(const vtss_phy_mapped_port_t * mapped_ports)
{
	vtss_port_no_t port_no;
	
	for (port_no=0; port_no<vtss_phy_state.port_array_size; port_no++) {
		vtss_phy_state.port[port_no].map = mapped_ports[port_no];
	}
	return VTSS_OK;
}

vtss_rc vtss_phy_map_port(const vtss_port_no_t port_no,
                          const vtss_phy_io_bus_t bus,
                          const int addr )
{
	vtss_log(VTSS_LOG_DEBUG,
		 "PHY: map phy port, port_no=%d, bus=0x%04x, addr=0x%02x",
		 port_no, bus, addr);
	vtss_phy_state.port[port_no].map.bus = bus;
	vtss_phy_state.port[port_no].map.addr = addr;
	return VTSS_OK;
}

#if 0
static const char *optional_page_str(const ushort page)
{
	switch (page) {
        case (VTSS_PHY_REG_STANDARD>>5): return "";
        case (VTSS_PHY_REG_EXTENDED>>5): return "(E)";
        case (VTSS_PHY_REG_GPIO>>5):     return "(GPIO)";
        case (VTSS_PHY_REG_TEST>>5):     return "(TEST)";
        case (VTSS_PHY_REG_TR>>5):       return "(TR)";
        default:                         return "(?)";
	}
}
#endif

vtss_rc vtss_phy_read(const vtss_port_no_t port_no,
                      const uint reg,
                      ushort *const value)
{
	ushort page;
	const vtss_phy_port_state_info_t *const port_state = &vtss_phy_state.port[port_no];
	
	/* Determine page */
	page = (reg>>5);
	
	if (page) {
		VTSS_RC(vtss_phy_io_write(port_state->map.bus,
					  port_state->map.addr,
					  0x1F/*Page Selection register*/,
					  page/*Select non-standard Page registers*/) );
	}
	VTSS_RC(vtss_phy_io_read(port_state->map.bus,
				 port_state->map.addr, reg & 0x1F, value));
	if (page) {
		VTSS_RC(vtss_phy_io_write(port_state->map.bus,
					  port_state->map.addr,
					  0x1F/*Page Selection register*/,
					  0x0000/*Select Standard Page registers*/));
	}
	return VTSS_OK;
}

vtss_rc vtss_phy_write(const vtss_port_no_t port_no,
                       const uint reg, const ushort value)
{
	ushort page;
	const vtss_phy_port_state_info_t *const port_state = &vtss_phy_state.port[port_no];
	
	/* determine page */
	page = (reg>>5);
	
	if (page) {
		VTSS_RC(vtss_phy_io_write(port_state->map.bus,
					  port_state->map.addr,
					  0x1F/*Page Selection register*/,
					  page/*Select non-standard Page registers*/) );
	}
	VTSS_RC(vtss_phy_io_write(port_state->map.bus, port_state->map.addr,
				  reg & 0x1F, value));
	if (page) {
		VTSS_RC(vtss_phy_io_write(port_state->map.bus,
					  port_state->map.addr,
					  0x1F/*Page Selection register*/,
					  0x0000/*Select Standard Page registers*/));
	}
	return VTSS_OK;
}
  
vtss_rc vtss_phy_writemasked(const vtss_port_no_t port_no,
                             const uint reg, const ushort value,
                             const ushort mask)
{
	ushort regval;
	ushort page;
	const vtss_phy_port_state_info_t *const port_state = &vtss_phy_state.port[port_no];
	
	/* Determine page */
	page = (reg>>5);
	
	if (page) {
		VTSS_RC(vtss_phy_io_write(port_state->map.bus,
					  port_state->map.addr,
					  0x1F/*Page Selection register*/,
					  page/*Select non-standard Page registers*/) );
	}
	VTSS_RC(vtss_phy_io_read(port_state->map.bus,
				 port_state->map.addr,
				 reg & 0x1F, &regval));
	regval &= ~mask;
	regval |= value & mask;
	VTSS_RC(vtss_phy_io_write(port_state->map.bus,
				  port_state->map.addr,
				  reg & 0x1F, regval));
	if (page) {
		VTSS_RC(vtss_phy_io_write(port_state->map.bus,
					  port_state->map.addr,
					  0x1F/*Page Selection register*/,
					  0x0000/*Select Standard Page registers*/));
	}
	return VTSS_OK;
}

static const char *mac_if_txt(vtss_port_interface_t mac_if)
{
	switch (mac_if) {
	case VTSS_PORT_INTERFACE_NO_CONNECTION: return "N/C";
	case VTSS_PORT_INTERFACE_LOOPBACK:      return "LOOPBACK";
	case VTSS_PORT_INTERFACE_INTERNAL:      return "INTERNAL";
	case VTSS_PORT_INTERFACE_MII:           return "MII";
	case VTSS_PORT_INTERFACE_GMII:          return "GMII";
	case VTSS_PORT_INTERFACE_RGMII:         return "RGMII";
	case VTSS_PORT_INTERFACE_TBI:           return "TBI";
	case VTSS_PORT_INTERFACE_RTBI:          return "RTBI";
	case VTSS_PORT_INTERFACE_SGMII:         return "SGMII";
	case VTSS_PORT_INTERFACE_SERDES:        return "SERDES";
	case VTSS_PORT_INTERFACE_VAUI:          return "VAUI";
	case VTSS_PORT_INTERFACE_XGMII:         return "XGMII";
	}
	return "????";
}

/* Calculate RGMII clock skew value */
static uint rgmii_clock_skew(uint ps)
{
	return (ps == 0 ? 0 : ps < 1600 ? 1 : ps < 2050 ? 2 : 3);
}

static vtss_rc vtss_phy_page_std(vtss_port_no_t port_no) 
{
	return vtss_phy_write(port_no, 31, VTSS_PHY_PAGE_STANDARD);
}

static vtss_rc vtss_phy_page_ext(vtss_port_no_t port_no) 
{
	return vtss_phy_write(port_no, 31, VTSS_PHY_PAGE_EXTENDED);
}

static vtss_rc vtss_phy_page_gpio(vtss_port_no_t port_no) 
{
	return vtss_phy_write(port_no, 31, VTSS_PHY_PAGE_GPIO);
}

static vtss_rc vtss_phy_page_test(vtss_port_no_t port_no) 
{
	return vtss_phy_write(port_no, 31, VTSS_PHY_PAGE_TEST);
}

static vtss_rc vtss_phy_page_tr(vtss_port_no_t port_no) 
{
	return vtss_phy_write(port_no, 31, VTSS_PHY_PAGE_TR);
}

static vtss_rc vtss_phy_optimize_receiver_init(vtss_port_no_t port_no)
{
	/* BZ 1776/1860/2095/2107, part 1/3 and 2/3 */
	VTSS_RC(vtss_phy_page_test(port_no));
	VTSS_RC(vtss_phy_writemasked(port_no, 12, 0x0200, 0x0300));
	return vtss_phy_page_std(port_no);
}

static vtss_rc vtss_phy_optimize_dsp (vtss_port_no_t port_no)
{
	/* BZ 2226/2227/2228/2229/2230/2231/2232/2294 */
	VTSS_RC(vtss_phy_page_tr(port_no));                     
	VTSS_RC(vtss_phy_write(port_no, 16, 0xaf8a));               /*- Read PMA internal Register 5 */
	VTSS_RC(vtss_phy_writemasked(port_no, 18, 0x0000, 0x0000)); /*- Configure PMA */
	VTSS_RC(vtss_phy_writemasked(port_no, 17, 0x0008, 0x000c)); /*- Register 5 */
	VTSS_RC(vtss_phy_write(port_no, 16, 0x8f8a));               /*- Write PMA internal Register 5 */
	VTSS_RC(vtss_phy_write(port_no, 16, 0xaf86));               /*- Read PMA internal Register 3 */       
	VTSS_RC(vtss_phy_writemasked(port_no, 18, 0x0008, 0x000c)); /*- Configure PMA */          
	VTSS_RC(vtss_phy_writemasked(port_no, 17, 0x0000, 0x0000)); /*- Register 3 */
	VTSS_RC(vtss_phy_write(port_no, 16, 0x8f86));               /*- Write PMA internal Register 3 */
	VTSS_RC(vtss_phy_write(port_no, 16, 0xaf82));               /*- Read PMA internal Register 1 */    
	VTSS_RC(vtss_phy_writemasked(port_no, 18, 0x0000, 0x0000)); /*- Configure PMA */          
	VTSS_RC(vtss_phy_writemasked(port_no, 17, 0x0100, 0x0180)); /*- Register 1 */          
	VTSS_RC(vtss_phy_write(port_no, 16, 0x8f82));               /*- Write PMA internal Register 1 */     
	
	return VTSS_OK;
}

static vtss_rc vtss_phy_optimize_receiver_reconfig(vtss_port_no_t port_no)
{
	ushort vga_state_a;
	
	/* BZ 1776/1860/2095/2107 part 3/3 */
	VTSS_RC(vtss_phy_page_tr(port_no));
	VTSS_RC(vtss_phy_write(port_no, 16, 0xaff0));
	vtss_phy_read(port_no, 17, &vga_state_a);
	vga_state_a = ((vga_state_a >> 4) & 0x01f);
	if ((vga_state_a < 16) || (vga_state_a > 20)) {
		VTSS_RC(vtss_phy_page_test(port_no));
		VTSS_RC(vtss_phy_writemasked(port_no, 12, 0x0000, 0x0300));
	}
	return vtss_phy_page_std(port_no);
}

static vtss_rc vtss_phy_optimize_jumbo(vtss_port_no_t port_no)
{
	/* BZ 1799/1801 */
	VTSS_RC(vtss_phy_page_test(port_no));
	VTSS_RC(vtss_phy_writemasked(port_no, 8, 0x0200, 0x0200));
	VTSS_RC(vtss_phy_page_tr(port_no));
	VTSS_RC(vtss_phy_write(port_no, 18, 0x0000));
	VTSS_RC(vtss_phy_write(port_no, 17, 0x0E35));
	VTSS_RC(vtss_phy_write(port_no, 16, 0x9786));
	VTSS_RC(vtss_phy_page_test(port_no));
	VTSS_RC(vtss_phy_writemasked(port_no, 8, 0x0000, 0x0200));
	return vtss_phy_page_std(port_no);
}

static vtss_rc vtss_phy_optimize_rgmii_strength(vtss_port_no_t port_no)
{
	/* BZ 2094 */
	VTSS_RC(vtss_phy_page_test(port_no));
	VTSS_RC(vtss_phy_write(port_no, 3, 0xf082));
	VTSS_RC(vtss_phy_write(port_no, 3, 0xf082));
	return vtss_phy_page_std(port_no);
}

vtss_rc vtss_phy_detect(const ushort reg2, const ushort reg3,
			const vtss_port_interface_t mac_if,
			vtss_phy_port_state_info_t* ps)
{
	ushort  model;
	ulong   oui;
	vtss_rc rc = VTSS_OK;
	
	oui = (reg2 << 6); 
	oui |= ((reg3 >> 10) & 0x3F);
	model = ((reg3 >> 4) & 0x3F);
	ps->revision = (reg3 & 0xF);
	
	/* Lookup PHY Family and Type */
	if (oui == 0x0001C1) {
		/* Vitesse OUI */
		switch (model) {
		case 0x02:
			ps->type = VTSS_PHY_TYPE_8601;
			ps->family = VTSS_PHY_FAMILY_COOPER;
			break;
		case 0x03:
			ps->type = VTSS_PHY_TYPE_8641;
			ps->family = VTSS_PHY_FAMILY_COOPER;
			break;
		case 0x05:
			ps->type = VTSS_PHY_TYPE_7385;
			ps->family = VTSS_PHY_FAMILY_LUTON;
			break;
		case 0x08:
			if (mac_if == VTSS_PORT_INTERFACE_INTERNAL) {
				ps->type = VTSS_PHY_TYPE_7388;
				ps->family = VTSS_PHY_FAMILY_LUTON;
			} else {
				/* VSC8538 revision A1 */
				ps->type = VTSS_PHY_TYPE_8538;
				ps->family = VTSS_PHY_FAMILY_SPYDER;
			}
			break;
		case 0x09: /* VSC7389 SparX-G16 */
		case 0x11: /* VSC7391 SparX-G16R */
			ps->type = VTSS_PHY_TYPE_7389;
			ps->family = VTSS_PHY_FAMILY_LUTON24;
			break;
		case 0x10:
			ps->type = VTSS_PHY_TYPE_7390;
			ps->family = VTSS_PHY_FAMILY_LUTON24;
			break;
		case 0x15:
			ps->type = VTSS_PHY_TYPE_7395;
			ps->family = VTSS_PHY_FAMILY_LUTON_E;
			break;
		case 0x18:
			if (mac_if == VTSS_PORT_INTERFACE_INTERNAL) {
				ps->type = VTSS_PHY_TYPE_7398;
				ps->family = VTSS_PHY_FAMILY_LUTON_E;
			} else {
				/* VSC8558 revision A1 */
				ps->type = VTSS_PHY_TYPE_8558;
				ps->family = VTSS_PHY_FAMILY_SPYDER;
			}  
			break;
		case 0x28:
			ps->type = VTSS_PHY_TYPE_8538;
			ps->family = VTSS_PHY_FAMILY_SPYDER;
			break;
		case 0x38:
			ps->type = VTSS_PHY_TYPE_8558;
			ps->family = VTSS_PHY_FAMILY_SPYDER;
			break;
		case 0x35:
			ps->type = VTSS_PHY_TYPE_8658;
			ps->family = VTSS_PHY_FAMILY_SPYDER;    
			break;
		case 0x30:    
			ps->type = VTSS_PHY_TYPE_7500;
			ps->family = VTSS_PHY_FAMILY_INTRIGUE;
			break;            
		case 0x39:
			ps->type = VTSS_PHY_TYPE_7501;
			ps->family = VTSS_PHY_FAMILY_INTRIGUE;
			break;                    
		case 0x3a:
			ps->type = VTSS_PHY_TYPE_7502;
			ps->family = VTSS_PHY_FAMILY_INTRIGUE;
			break;            
		case 0x3b:
			ps->type = VTSS_PHY_TYPE_7503;
			ps->family = VTSS_PHY_FAMILY_INTRIGUE;
			break;            
		case 0x3c:
			ps->type = VTSS_PHY_TYPE_7504;
			ps->family = VTSS_PHY_FAMILY_INTRIGUE;
			break;            
		case 0x3d:
			ps->type = VTSS_PHY_TYPE_7505;
			ps->family = VTSS_PHY_FAMILY_INTRIGUE;
			break;                    
		case 0x3e:
			ps->type = VTSS_PHY_TYPE_7506;
			ps->family = VTSS_PHY_FAMILY_INTRIGUE;
			break;                    
		case 0x3f:
			ps->type = VTSS_PHY_TYPE_7507;
			ps->family = VTSS_PHY_FAMILY_INTRIGUE;
			break;            
		default:
			ps->type = VTSS_PHY_TYPE_NONE;
			ps->family = VTSS_PHY_FAMILY_NONE;        
			rc = VTSS_ENTRY_NOT_FOUND;
			break;
		}
	} 
	else if (oui == 0x0003F1) {
		/* Cicada OUI */
		switch (model) {
		case 0x01:
			ps->type = VTSS_PHY_TYPE_8201;
			ps->family = VTSS_PHY_FAMILY_MUSTANG;
			break;
		case 0x04:
			ps->type = VTSS_PHY_TYPE_8204;
			ps->family = VTSS_PHY_FAMILY_BLAZER;
			break;
		case 0x0B:
			ps->type = VTSS_PHY_TYPE_8211;
			ps->family = VTSS_PHY_FAMILY_COBRA;
			break;
		case 0x15:
			ps->type = VTSS_PHY_TYPE_8221;
			ps->family = VTSS_PHY_FAMILY_COBRA;
			break;
		case 0x18:
			ps->type = VTSS_PHY_TYPE_8224;
			ps->family = VTSS_PHY_FAMILY_QUATTRO;
			break;
		case 0x22:
			ps->type = VTSS_PHY_TYPE_8234;
			ps->family = VTSS_PHY_FAMILY_QUATTRO;
			break;
		case 0x2C:
			ps->type = VTSS_PHY_TYPE_8244;
			ps->family = VTSS_PHY_FAMILY_QUATTRO;
			break;
		default:
			ps->type = VTSS_PHY_TYPE_NONE;
			ps->family = VTSS_PHY_FAMILY_NONE;
			rc = VTSS_ENTRY_NOT_FOUND;
			break;
		}
	} else {
		ps->type = VTSS_PHY_TYPE_NONE;
		ps->family = VTSS_PHY_FAMILY_NONE;
		rc = VTSS_NOT_IMPLEMENTED;
	}
	
	return rc;
}

vtss_rc vtss_phy_reset(const vtss_port_no_t port_no,
                       const vtss_phy_reset_setup_t * const setup)

{
	vtss_phy_port_state_info_t *ps;
	ushort reg, reg2, reg3, model;
	ulong oui;
	vtss_mtimer_t timer;
	
	/* save reset setup */
	ps = &vtss_phy_state.port[port_no];
	ps->reset = *setup;
	
	/* -- step 1: detect PHY type and family -- */
	VTSS_RC(vtss_phy_read(port_no, 2, &reg2));
	oui = (reg2 << 6); 
	VTSS_RC(vtss_phy_read(port_no, 3, &reg3));
	oui |= ((reg3 >> 10) & 0x3F);
	model = ((reg3 >> 4) & 0x3F);
	
	vtss_phy_detect(reg2, reg3, setup->mac_if, ps);
	
	vtss_log(VTSS_LOG_DEBUG,
		 "PHY: detected PHY, port=%d, type=%s-%d_%d(model 0x%02x)", 
		 port_no,
		 ps->type != VTSS_PHY_TYPE_NONE ? "Vitesse" : "Unknown",
		 ps->type, ps->revision, model);         
	
	/* -- step 2: pre-reset setup of MAC and Media interface -- */
	switch (ps->family) {
	case VTSS_PHY_FAMILY_MUSTANG:
		/* TBD */
		break;
	case VTSS_PHY_FAMILY_BLAZER:
		/* interface is setup after reset */
		if (ps->revision < 4 || ps->revision> 6) {
			vtss_log(VTSS_LOG_ERR,
				 "PHY: unsupported Blazer revision, port=%d, rev=%d",
				 port_no, ps->revision);
			return VTSS_NOT_IMPLEMENTED;
		}
		break;
	case VTSS_PHY_FAMILY_COBRA:
		/* TBD */
		break;
	case VTSS_PHY_FAMILY_QUATTRO:
		switch (setup->mac_if) {
		case VTSS_PORT_INTERFACE_SGMII:
			/* VSC8234 */
			reg = ((0xB<<12) | (0<<1)); /* 4-pin SGMII, auto negotiation disabled */
			break;
		case VTSS_PORT_INTERFACE_RGMII:
			/* VSC8224/44 */
			reg = (((setup->media_if == VTSS_PHY_MEDIA_INTERFACE_AMS_COPPER ||
			      setup->media_if == VTSS_PHY_MEDIA_INTERFACE_AMS_FIBER ? 0 : 1)<<12) |
			      (rgmii_clock_skew(setup->rgmii.tx_clk_skew_ps)<<10) |
			      (rgmii_clock_skew(setup->rgmii.rx_clk_skew_ps)<<8) |
			      ((setup->media_if == VTSS_PHY_MEDIA_INTERFACE_COPPER ||
			      setup->media_if == VTSS_PHY_MEDIA_INTERFACE_AMS_COPPER ? 2 : 1)<<1));
			break;
		case VTSS_PORT_INTERFACE_RTBI:
			/* VSC8224/44 */
			reg = (((setup->tbi.aneg_enable ? 4 : 5)<<12) |
				((setup->tbi.aneg_enable ? 1 : 0)<<1));
			break;
		default:
			vtss_log(VTSS_LOG_ERR,
				 "PHY: MAC interfaces not supported for Quattro, port=%d, intfc=%s", 
				 port_no, mac_if_txt(setup->mac_if));
			return VTSS_INVALID_PARAMETER;
		}
		reg |= (1<<5); /* Rx Idle Clock Enable, must be 1 */
		VTSS_RC(vtss_phy_write(port_no, 23, reg));
		break;
	case VTSS_PHY_FAMILY_SPYDER:
		if (ps->revision == 0) {
			/* BZ 2027 */
			VTSS_RC(vtss_phy_page_gpio(port_no));
			VTSS_RC(vtss_phy_write(port_no, 0, 0x4c19));
			VTSS_RC(vtss_phy_writemasked(port_no, 0, 0x8000, 0x8000));
			VTSS_RC(vtss_phy_page_std(port_no));
		}
		switch (setup->mac_if) {
		case VTSS_PORT_INTERFACE_SGMII:
			reg = ((0<<13) | (0<<12)); /* SGMII, auto negotiation disabled */
			break;
		case VTSS_PORT_INTERFACE_SERDES:
			reg = ((0<<13) | (1<<12)); /* SERDES, auto negotiation disabled */
			break;
		default:
			vtss_log(VTSS_LOG_ERR,
				 "PHY: MAC interface not supported for Spyder, port=%d, intfc=%s", 
				 port_no, mac_if_txt(setup->mac_if));
			return VTSS_INVALID_PARAMETER;
		}
		switch (setup->media_if) {
		case VTSS_PHY_MEDIA_INTERFACE_COPPER:
			reg |= (0<<8);
			break;
		case VTSS_PHY_MEDIA_INTERFACE_FIBER:
			reg |= (2<<8);
			break;
		case VTSS_PHY_MEDIA_INTERFACE_AMS_COPPER:
			reg |= ((1<<11) | (6<<8));
			break;
		case VTSS_PHY_MEDIA_INTERFACE_AMS_FIBER:
			reg |= ((0<<11) | (6<<8));
			break;
		default:
			vtss_log(VTSS_LOG_ERR,
				 "PHY: media interface not supported for Spyder, port=%d",
				 port_no);
			return VTSS_INVALID_PARAMETER;
		}
		VTSS_RC(vtss_phy_write(port_no, 23, reg));
		break;
	case VTSS_PHY_FAMILY_COOPER:
		/* NOP - Values set by CMODE pins */
		break;    
	case VTSS_PHY_FAMILY_LUTON:
	case VTSS_PHY_FAMILY_LUTON24:
	case VTSS_PHY_FAMILY_LUTON_E:
		switch (setup->mac_if) {
		case VTSS_PORT_INTERFACE_INTERNAL:
			/* Register 23 is setup correctly by default */
			break;
		default:
			vtss_log(VTSS_LOG_ERR,
				 "PHY: MAC interface not supported for SparX, port=%d, intfc=%s", 
				 port_no, mac_if_txt(setup->mac_if));
			return VTSS_INVALID_PARAMETER;
		}
		break;
	case VTSS_PHY_FAMILY_INTRIGUE:
		break;
	case VTSS_PHY_FAMILY_NONE:
	default:
		if (oui == 0x005043 && model == 2 && ps->revision == 4) {
			switch (setup->mac_if) {
			case VTSS_PORT_INTERFACE_RGMII:
				VTSS_RC(vtss_phy_write(port_no, 27, 0x848B));
				VTSS_RC(vtss_phy_write(port_no, 20, 0x0CE2));
				break;
			case VTSS_PORT_INTERFACE_RTBI:
				VTSS_RC(vtss_phy_write(port_no, 27, 0x8489));
				VTSS_RC(vtss_phy_write(port_no, 20, 0x0CE2));
				break;
			default:
				break;
			}
			break;
		}    
		break;
	}
	
	/* -- step 3: reset PHY -- */
	VTSS_RC(vtss_phy_write(port_no, 0, 1<<15));

	VTSS_NSLEEP(4000); /* 4000 ns pause after reset */

	/* TODO: use eloop_register_timeout */
	VTSS_MTIMER_START(&timer, 5000); /* wait up to 5 seconds */
	while (1) {
		if (vtss_phy_read(port_no, 0, &reg) == VTSS_OK &&
		    (reg & (1<<15)) == 0)
			break;
		VTSS_MSLEEP(20);
		if (VTSS_MTIMER_TIMEOUT(&timer)) {
			vtss_log(VTSS_LOG_ERR,
				 "PHY: reset timeout, port=%d", port_no);
			return VTSS_PHY_TIMEOUT;
		}
	}
	VTSS_MTIMER_CANCEL(&timer);
	
	/* -- step 4: run startup scripts -- */    
	switch (ps->family) {
	case VTSS_PHY_FAMILY_MUSTANG:
		/* TBD */
		break;
		
	case VTSS_PHY_FAMILY_BLAZER:
		vtss_phy_init_seq_blazer(ps, port_no);
		
		/* setup MAC interface */
		switch (setup->mac_if) {
		case VTSS_PORT_INTERFACE_MII:
		case VTSS_PORT_INTERFACE_GMII:
			reg = (0<<12);
			break;
		case VTSS_PORT_INTERFACE_RGMII:
			reg = (1<<12);
			break;
		case VTSS_PORT_INTERFACE_TBI:
			reg = (2<<12);
			break;
		case VTSS_PORT_INTERFACE_RTBI:
			reg = (3<<12);
			break;
		default:
			vtss_log(VTSS_LOG_ERR,
				 "PHY: MAC interface not supported for Blazer, port=%d, intfc=%s", 
				 port_no, mac_if_txt(setup->mac_if));
			return VTSS_INVALID_PARAMETER;
			break;
		}
		reg |= (((ps->revision == 6 ? 1 : 0)<<9) | /* 2.5V */
			((setup->rgmii.rx_clk_skew_ps || setup->rgmii.tx_clk_skew_ps ? 1 : 0)<<8) | 
			(0x14<<0)); /* Reserved */
		VTSS_RC(vtss_phy_write(port_no, 23, reg));
		VTSS_RC(vtss_phy_write(port_no, 22, 0x3000));
		break;
	
	case VTSS_PHY_FAMILY_COBRA:
		/* TBD */
		break;
		
	case VTSS_PHY_FAMILY_QUATTRO:
		vtss_phy_init_seq_quattro(ps, setup, port_no);
		break;
		
	case VTSS_PHY_FAMILY_SPYDER:
		vtss_phy_init_seq_spyder(ps, port_no);
		break;
		
	case VTSS_PHY_FAMILY_COOPER:
		vtss_phy_init_seq_cooper(ps, port_no);
		break;
		
	case VTSS_PHY_FAMILY_LUTON:
	case VTSS_PHY_FAMILY_LUTON_E:
	case VTSS_PHY_FAMILY_LUTON24:
		vtss_phy_init_seq_luton(ps, setup, port_no);
		break;
		
	case VTSS_PHY_FAMILY_INTRIGUE:
		vtss_phy_init_seq_intrigue(ps, setup, port_no);
		break;      
		
	case VTSS_PHY_FAMILY_NONE:    
	default:
		break;
	}
	return VTSS_OK;
}

vtss_rc vtss_phy_setup(const vtss_port_no_t port_no, 
                       const vtss_phy_setup_t * const setup)
{
	ushort reg;
	vtss_phy_family_t family;
	uint revision;
	
	/* Save setup */
	vtss_phy_state.port[port_no].setup = *setup;
	family = vtss_phy_state.port[port_no].family;
	revision = vtss_phy_state.port[port_no].revision;
	
	switch (setup->mode) {
	case VTSS_PHY_MODE_ANEG:
		/* Setup register 4 */
		reg = (ushort)(((setup->aneg.asymmetric_pause ? 1 : 0) << 11) |
			       ((setup->aneg.symmetric_pause ? 1 : 0) << 10) |
			       ((setup->aneg.speed_100m_fdx ? 1 : 0) << 8) |
			       ((setup->aneg.speed_100m_hdx ? 1 : 0) << 7) |
			       ((setup->aneg.speed_10m_fdx ? 1 : 0) << 6) |
			       ((setup->aneg.speed_10m_hdx ? 1 : 0) << 5) |
			       (1 << 0));
		VTSS_RC(vtss_phy_write(port_no, 4, reg));
		
		/* Setup register 9 */
		VTSS_RC(vtss_phy_read(port_no, 9, &reg));
		reg &= (1<<10); /* Preserve 'Port Type' field */
		reg |= ((setup->aneg.speed_1g_fdx ? 1 : 0)<<9);
		VTSS_RC(vtss_phy_write(port_no, 9, reg));
		
		/* Use register 0 to restart auto negotiation */
		VTSS_RC(vtss_phy_write(port_no, 0, (1<<12) | (1<<9)));
		break;
	case VTSS_PHY_MODE_FORCED:
		/* Setup register 0 */
		reg = (((setup->forced.speed == VTSS_SPEED_100M ? 1 : 0)<<13) |
			(0<<12) |
		       ((setup->forced.fdx ? 1 : 0)<<8) |
		       ((setup->forced.speed == VTSS_SPEED_1G ? 1 : 0)<<6));
		VTSS_RC(vtss_phy_write(port_no, 0, reg));
		
		if (setup->forced.speed != VTSS_SPEED_1G) { 
			/* Enable Auto MDI/MDI-X in forced 10/100 mode */
			switch (family) {
			case VTSS_PHY_FAMILY_QUATTRO:
				VTSS_RC(vtss_phy_page_test(port_no));
				VTSS_RC(vtss_phy_writemasked(port_no,
							     8,
							     0x0200,
							     0x0200));
				VTSS_RC(vtss_phy_page_tr(port_no));
				VTSS_RC(vtss_phy_write(port_no, 18, 0x0012));
				VTSS_RC(vtss_phy_write(port_no, 17, 0x2803));
				VTSS_RC(vtss_phy_write(port_no, 16, 0x87fa));
				VTSS_RC(vtss_phy_page_test(port_no));
				VTSS_RC(vtss_phy_writemasked(port_no,
							     8,
							     0x0000,
							     0x0200));
				VTSS_RC(vtss_phy_page_std(port_no));
				break;
			case VTSS_PHY_FAMILY_LUTON:
				if (revision == 0) {
					VTSS_RC(vtss_phy_page_test(port_no));
					VTSS_RC(vtss_phy_writemasked(port_no,
								     8,
								     0x0200,
								     0x0200));
					VTSS_RC(vtss_phy_page_tr(port_no));
					VTSS_RC(vtss_phy_write(port_no, 18,
							       0x0012));
					VTSS_RC(vtss_phy_write(port_no, 17,
							       0x2803));
					VTSS_RC(vtss_phy_write(port_no, 16,
							       0x87fa));
					VTSS_RC(vtss_phy_page_test(port_no));
					VTSS_RC(vtss_phy_writemasked(port_no,
								     8,
								     0x0000,
								     0x0200));
					VTSS_RC(vtss_phy_page_std(port_no));
				} else {
					VTSS_RC(vtss_phy_page_std(port_no));
					VTSS_RC(vtss_phy_writemasked(port_no,
								     18,
								     0x0000,
								     0x0080));
				}
				break;
			case VTSS_PHY_FAMILY_SPYDER:
			case VTSS_PHY_FAMILY_LUTON_E:
				VTSS_RC(vtss_phy_page_std(port_no));
				VTSS_RC(vtss_phy_writemasked(port_no, 18,
							     0x0000, 0x0080));
				break;
			case VTSS_PHY_FAMILY_LUTON24:
				VTSS_RC(vtss_phy_page_test(port_no));
				VTSS_RC(vtss_phy_writemasked(port_no, 8,
							     0x0200, 0x0200));
				VTSS_RC(vtss_phy_page_tr(port_no));
				VTSS_RC(vtss_phy_write(port_no, 18, 0x0092));
				VTSS_RC(vtss_phy_write(port_no, 17, 0x2803));
				VTSS_RC(vtss_phy_write(port_no, 16, 0x87FA));
				VTSS_RC(vtss_phy_page_test(port_no));
				VTSS_RC(vtss_phy_writemasked(port_no, 8,
							     0x0000, 0x0200));
				VTSS_RC(vtss_phy_page_std(port_no));
				break;
			default:
				break;
			}
		}
		break;
	case VTSS_PHY_MODE_POWER_DOWN:
		/* Setup register 0 */
		VTSS_RC(vtss_phy_write(port_no, 0, 1<<11));
		if (family == VTSS_PHY_FAMILY_QUATTRO) {
			/* Briefly power up-down to make sure the link status bit clears */
			VTSS_RC(vtss_phy_write(port_no, 0, 0<<11));
			VTSS_RC(vtss_phy_write(port_no, 0, 1<<11));
		}
		break;
	default:
		vtss_log(VTSS_LOG_ERR,
			 "PHY: unknown mode, port=%d, mode=%d",
			 port_no, setup->mode);
		return VTSS_INVALID_PARAMETER;
	}
	
	return VTSS_OK;
}

vtss_rc vtss_phy_status_get(const vtss_port_no_t port_no,
                            vtss_port_status_t * const status)
{
	vtss_phy_port_state_info_t *ps;
	ushort reg, reg10;
	BOOL sym_pause,asym_pause,lp_sym_pause,lp_asym_pause;
	
	/* Read link status from register 1 */
	VTSS_RC(vtss_phy_read(port_no, 1, &reg));
	status->link_down = (reg & (1<<2) ? 0 : 1);
	if (status->link_down) {
		/* Read status again if link down (latch low field) */
		VTSS_RC(vtss_phy_read(port_no, 1, &reg));
		status->link = (reg & (1<<2) ? 1 : 0);
	} else {
		status->link = 1;
	} 
	
	ps = &vtss_phy_state.port[port_no];
	if (status->link) {
		switch (ps->setup.mode) {
		case VTSS_PHY_MODE_ANEG:
			if ((reg & (1<<5))==0) {
				/* Auto negotiation not complete, link considered down */
				status->link = 0;
				break;
			}
			
			/* Use register 5 to determine flow control result */
			VTSS_RC(vtss_phy_read(port_no, 5, &reg));
			sym_pause = ps->setup.aneg.symmetric_pause;
			asym_pause = ps->setup.aneg.asymmetric_pause;
			lp_sym_pause = (reg & (1<<10) ? 1 : 0);
			lp_asym_pause = (reg & (1<<11) ? 1 : 0);
			status->aneg.obey_pause = 
				(sym_pause && (lp_sym_pause || (asym_pause && lp_asym_pause)) ? 1 : 0);
			status->aneg.generate_pause = 
				(lp_sym_pause && (sym_pause || (asym_pause && lp_asym_pause)) ? 1 : 0);
			
			if (ps->family == VTSS_PHY_FAMILY_NONE) {
				/* Standard PHY, use register 10 and 5 to determine speed/duplex */
				VTSS_RC(vtss_phy_read(port_no, 10, &reg10));
				if (ps->setup.aneg.speed_1g_fdx &&  /* 1G fdx advertised */
				    (reg10 & (1<<15))==0 &&         /* No master/slave fault */
				    (reg10 & (1<<11))) {            /* 1G fdx advertised by LP */ 
					status->speed = VTSS_SPEED_1G;
					status->fdx = 1;
				} else if (ps->setup.aneg.speed_100m_fdx && /* 100M fdx advertised */
					   (reg & (1<<8))) {                /* 100M fdx advertised by LP */
					status->speed = VTSS_SPEED_100M;
					status->fdx = 1;
				} else if (ps->setup.aneg.speed_100m_hdx && /* 100M hdx advertised */
					   (reg & (1<<7))) {                /* 100M hdx advertised by LP */
					status->speed = VTSS_SPEED_100M;
					status->fdx = 0;
				} else if (ps->setup.aneg.speed_10m_fdx &&  /* 10M fdx advertised */
					   (reg & (1<<6))) {                /* 10M fdx advertised by LP */
					status->speed = VTSS_SPEED_10M;
					status->fdx = 1;
				} else if (ps->setup.aneg.speed_10m_hdx &&  /* 10M hdx advertised */
					   (reg & (1<<5))) {                /* 10M hdx advertised by LP */
					status->speed = VTSS_SPEED_10M;
					status->fdx = 0;
				} else {
					status->speed = VTSS_SPEED_UNDEFINED;
					status->fdx = 0;
				}
			} else {
				/* Vitesse PHY, use register 28 to determine speed/duplex */
				VTSS_RC(vtss_phy_read(port_no, 28, &reg));
				switch ((reg>>3) & 0x3) {
				case 0:
					status->speed = VTSS_SPEED_10M;
					break;
				case 1:
					status->speed = VTSS_SPEED_100M;
					break;
				case 2:
					status->speed = VTSS_SPEED_1G;
					break;
				case 3:
					status->speed = VTSS_SPEED_UNDEFINED;
					break;
				}
				status->fdx = (reg & (1<<5) ? 1 : 0);
			}
			break;
		case VTSS_PHY_MODE_FORCED:
			status->speed = ps->setup.forced.speed;
			status->fdx = ps->setup.forced.fdx;
			break;
		case VTSS_PHY_MODE_POWER_DOWN:
			break;
		default:
			vtss_log(VTSS_LOG_ERR,
				 "PHY: unknown mode , port=%d, mode=%d",
				 port_no, ps->setup.mode);
			return VTSS_INVALID_PARAMETER;
		}
	}
	
	/* Handle link down event */
	if ((!status->link || status->link_down) && ps->status.link) {
		vtss_log(VTSS_LOG_DEBUG,
			 "PHY: link down event, port=%d", port_no);
		
		switch (ps->family) {
		case VTSS_PHY_FAMILY_QUATTRO:
		case VTSS_PHY_FAMILY_COBRA:
			/* BZ ? */
			VTSS_RC(vtss_phy_read(port_no, 0, &reg));
			if ((reg & (1<<11))==0) {
			/* Briefly power down PHY to avoid problem sometimes seen when 
				changing speed on SmartBit */
				VTSS_RC(vtss_phy_write(port_no, 0,
						       (ushort)(reg | (ushort)(1 << 11))));
				status->link = 0;
				VTSS_MSLEEP(100);
				VTSS_RC(vtss_phy_write(port_no, 0, reg));
			}
		case VTSS_PHY_FAMILY_SPYDER:
		case VTSS_PHY_FAMILY_LUTON:
		case VTSS_PHY_FAMILY_LUTON_E:
		case VTSS_PHY_FAMILY_LUTON24:
			VTSS_RC(vtss_phy_optimize_receiver_init(port_no));
			
			if (ps->family != VTSS_PHY_FAMILY_QUATTRO) {
				VTSS_RC(vtss_phy_page_test(port_no));
				VTSS_RC(vtss_phy_writemasked(port_no, 20, 0x0000, 0x1000));  /*- Clear MDI Impedance Force Bit - Safety  */
				VTSS_RC(vtss_phy_page_std(port_no));
			}
			break;
		case VTSS_PHY_FAMILY_MUSTANG:
		case VTSS_PHY_FAMILY_BLAZER:
		case VTSS_PHY_FAMILY_COOPER:
		case VTSS_PHY_FAMILY_INTRIGUE:
		case VTSS_PHY_FAMILY_NONE:
		default:
			break;
		}
	}
	
	/* Handle link up event */
	if (status->link && (!ps->status.link || status->link_down)) {
		vtss_log(VTSS_LOG_DEBUG,
			 "PHY: link up event, port=%d", port_no);
		
		switch (ps->family) {
                        
		case VTSS_PHY_FAMILY_QUATTRO:        
			if (status->speed == VTSS_SPEED_1G) {
				VTSS_RC(vtss_phy_optimize_receiver_reconfig(port_no));
			}
			break;
			
		case VTSS_PHY_FAMILY_SPYDER:
			if (status->speed == VTSS_SPEED_1G) {
				VTSS_RC(vtss_phy_optimize_receiver_reconfig(port_no));                       
			} else if (status->speed == VTSS_SPEED_100M) {
				VTSS_RC(vtss_phy_page_test(port_no));
				VTSS_RC(vtss_phy_writemasked (port_no, 20, 0x0000, 0x1000));  /*- Clear Force Bit */
				VTSS_RC(vtss_phy_writemasked (port_no, 20, 0x2000, 0x6000));  /*- MDI Impedance offset -1 */
				VTSS_RC(vtss_phy_page_std(port_no));            
				VTSS_RC(vtss_phy_writemasked (port_no, 24, 0xa004, 0xe00e));   /*- 10/100 Edge Rate/Amplitude Control */           
			} else if (status->speed == VTSS_SPEED_10M) {
				VTSS_RC(vtss_phy_page_test(port_no));
				VTSS_RC(vtss_phy_writemasked (port_no, 20, 0x0000, 0x1000));  /*- Clear Force Bit */                
				VTSS_RC(vtss_phy_writemasked (port_no, 20, 0x6000, 0x6000));  /*- MDI Impedance Offset = +1 */
				VTSS_RC(vtss_phy_page_std(port_no));                        
			}
			break;
			
		case VTSS_PHY_FAMILY_LUTON:
			if (status->speed == VTSS_SPEED_1G) {
				VTSS_RC(vtss_phy_optimize_receiver_reconfig(port_no));                       
			} else if (status->speed == VTSS_SPEED_100M) {
				VTSS_RC(vtss_phy_page_test(port_no));
				VTSS_RC(vtss_phy_writemasked (port_no, 20, 0x0000, 0x1000));  /*- Clear Force Bit */                                
				VTSS_RC(vtss_phy_writemasked (port_no, 20, 0x0000, 0x6000));  /*- MDI Impedance offset default */
				VTSS_RC(vtss_phy_page_std(port_no));            
				/*- MII 24 can be 0xa000(0xe000), if customer has 100-BT rise time issues */            
				VTSS_RC(vtss_phy_writemasked (port_no, 24, 0x8002, 0xe00e));   /*- 10/100 Edge Rate/Amplitude Control */           
			} else if (status->speed == VTSS_SPEED_10M) {
				VTSS_RC(vtss_phy_page_test(port_no));
				VTSS_RC(vtss_phy_writemasked (port_no, 20, 0x0000, 0x1000));  /*- Clear Force Bit */                                
				VTSS_RC(vtss_phy_writemasked (port_no, 20, 0x2000, 0x6000));  /*- MDI Impedance Offset = -1 */
				VTSS_RC(vtss_phy_page_std(port_no));                        
			}
			break;
			
		case VTSS_PHY_FAMILY_LUTON_E:        
			if (status->speed == VTSS_SPEED_1G) {
				VTSS_RC(vtss_phy_optimize_receiver_reconfig(port_no));
				VTSS_RC(vtss_phy_page_ext (port_no));
				VTSS_RC(vtss_phy_writemasked (port_no, 20, 0x2000, 0xe000));  /*- 1000-BT DAC Amplitude control = +2% */
				VTSS_RC(vtss_phy_page_test (port_no));
				VTSS_RC(vtss_phy_writemasked (port_no, 24, 0x0030, 0x0038));  /*- 1000-BT Edge Rate Control */
				VTSS_RC(vtss_phy_writemasked (port_no, 20, 0x0000, 0x1000));  /*- Clear Force Bit */                                
				VTSS_RC(vtss_phy_writemasked (port_no, 20, 0x4000, 0x6000));  /*- MDI Impedance Offset = +2 */
				VTSS_RC(vtss_phy_page_std(port_no));                                                            
			} else if (status->speed == VTSS_SPEED_100M) {
				VTSS_RC(vtss_phy_page_test(port_no));
				VTSS_RC(vtss_phy_writemasked (port_no, 20, 0x0000, 0x1000));  /*- Clear Force Bit */                                
				VTSS_RC(vtss_phy_writemasked (port_no, 20, 0x2000, 0x6000));  /*- MDI Impedance Offset = -1 */
				VTSS_RC(vtss_phy_page_std(port_no));            
				/*- MII 24 can be 0xa000(0xe000), if customer has 100-BT rise time issues */
				VTSS_RC(vtss_phy_writemasked (port_no, 24, 0x8002, 0xe00e));   /*- 10/100 Edge Rate/Amplitude Control */             
			} else if (status->speed == VTSS_SPEED_10M) {
				VTSS_RC(vtss_phy_page_test(port_no));
				VTSS_RC(vtss_phy_writemasked (port_no, 20, 0x0000, 0x1000));  /*- Clear Force Bit */                                
				VTSS_RC(vtss_phy_writemasked (port_no, 20, 0x2000, 0x6000));   /*- MDI Impedance Offset = -1 */
				VTSS_RC(vtss_phy_page_std(port_no));                        
			}
			break;
			
		case VTSS_PHY_FAMILY_LUTON24:      
			if (status->speed == VTSS_SPEED_1G) {
				VTSS_RC(vtss_phy_optimize_receiver_reconfig(port_no));                       
			} else if (status->speed == VTSS_SPEED_100M) {
				VTSS_RC(vtss_phy_page_test(port_no));
				VTSS_RC(vtss_phy_writemasked (port_no, 20, 0x0000, 0x1000));  /*- Clear Force Bit */                                
				VTSS_RC(vtss_phy_writemasked (port_no, 20, 0x0000, 0x6000));   /*- MDI Impedance offset default */
				VTSS_RC(vtss_phy_page_std(port_no));            
				VTSS_RC(vtss_phy_writemasked (port_no, 24, 0xa002, 0xe00e));    /*- 10/100 Edge Rate/Amplitude Control */         
			} else if (status->speed == VTSS_SPEED_10M) {
				VTSS_RC(vtss_phy_page_test(port_no));
				VTSS_RC(vtss_phy_writemasked (port_no, 20, 0x0000, 0x1000));  /*- Clear Force Bit */                                
				VTSS_RC(vtss_phy_writemasked (port_no, 20, 0x2000, 0x6000));   /*- MDI Impedance Offset = -1 */
				VTSS_RC(vtss_phy_page_std(port_no));                        
			}
			break;
			
		case VTSS_PHY_FAMILY_MUSTANG:
		case VTSS_PHY_FAMILY_BLAZER:
		case VTSS_PHY_FAMILY_COBRA:
		case VTSS_PHY_FAMILY_COOPER:
		case VTSS_PHY_FAMILY_INTRIGUE:
		case VTSS_PHY_FAMILY_NONE:
		default:
			break;
		}
	}
	
	/* Save status */
	ps->status = *status;
	return VTSS_OK;
}

vtss_rc vtss_phy_optimize_1sec(const vtss_port_no_t port_no) 
{
	vtss_phy_port_state_info_t *ps;
	
	ps = &vtss_phy_state.port[port_no];
	
	if (ps->family == VTSS_PHY_FAMILY_BLAZER && ps->revision == 6 &&
		ps->setup.mode == VTSS_PHY_MODE_FORCED &&
		ps->setup.forced.speed == VTSS_SPEED_100M &&
		ps->status.link == 0) {
		/* VSC8204 B5 errata #2 */
		VTSS_RC(vtss_phy_reset(port_no, &ps->reset));
		VTSS_RC(vtss_phy_setup(port_no, &ps->setup));
	}
	
	return VTSS_OK;
}

/* PHY Init scripts begin here */

/* Init Scripts for VSC/CIS 8204 - aka Blazer */
vtss_rc vtss_phy_init_seq_blazer(vtss_phy_port_state_info_t* ps, const vtss_port_no_t port_no)
{
	if (ps->revision == 4) {
		/* VSC8204 B3 */
		VTSS_RC(vtss_phy_write(port_no, 31, 0xf01d));
		VTSS_RC(vtss_phy_page_std(port_no));
		VTSS_RC(vtss_phy_write(port_no,  0, 0x9040));
		VTSS_RC(vtss_phy_page_std(port_no));
		VTSS_RC(vtss_phy_write(port_no, 27, 0x0115));
		VTSS_RC(vtss_phy_page_test(port_no));
		VTSS_RC(vtss_phy_write(port_no,  1, 0x8000));
		VTSS_RC(vtss_phy_page_std(port_no));
		VTSS_RC(vtss_phy_write(port_no,  9, 0x0300));
		VTSS_RC(vtss_phy_page_test(port_no));
		VTSS_RC(vtss_phy_write(port_no,  8, 0x0208));
		VTSS_RC(vtss_phy_page_test(port_no));
		VTSS_RC(vtss_phy_write(port_no,  5, 0x8000));
		VTSS_RC(vtss_phy_page_tr(port_no));
		VTSS_RC(vtss_phy_write(port_no,  2, 0x0085));
		VTSS_RC(vtss_phy_write(port_no,  1, 0x21f0));
		VTSS_RC(vtss_phy_write(port_no,  0, 0x8770));
		VTSS_RC(vtss_phy_page_tr(port_no));
		VTSS_RC(vtss_phy_write(port_no,  2, 0x0085));
		VTSS_RC(vtss_phy_write(port_no,  1, 0x23f0));
		VTSS_RC(vtss_phy_write(port_no,  0, 0x8770));
		VTSS_RC(vtss_phy_page_tr(port_no));
		VTSS_RC(vtss_phy_write(port_no,  2, 0x0000));
		VTSS_RC(vtss_phy_write(port_no,  1, 0x1b00));
		VTSS_RC(vtss_phy_write(port_no,  0, 0x8fa0));
		VTSS_RC(vtss_phy_page_tr(port_no));
		VTSS_RC(vtss_phy_write(port_no,  2, 0x0000));
		VTSS_RC(vtss_phy_write(port_no,  1, 0x7d00));
		VTSS_RC(vtss_phy_write(port_no,  0, 0x8fac));
		VTSS_RC(vtss_phy_page_tr(port_no));
		VTSS_RC(vtss_phy_write(port_no,  2, 0x00e0));
		VTSS_RC(vtss_phy_write(port_no,  1, 0xe30d));
		VTSS_RC(vtss_phy_write(port_no,  0, 0x870c));
		VTSS_RC(vtss_phy_page_tr(port_no));
		VTSS_RC(vtss_phy_write(port_no,  2, 0x0000));
		VTSS_RC(vtss_phy_write(port_no,  1, 0x0010));
		VTSS_RC(vtss_phy_write(port_no,  0, 0x8f84));
		VTSS_RC(vtss_phy_page_tr(port_no));
		VTSS_RC(vtss_phy_write(port_no,  2, 0x0000));
		VTSS_RC(vtss_phy_write(port_no,  1, 0x0012));
		VTSS_RC(vtss_phy_write(port_no,  0, 0x8f84));
		VTSS_RC(vtss_phy_page_tr(port_no));
		VTSS_RC(vtss_phy_write(port_no,  2, 0x0000));
		VTSS_RC(vtss_phy_write(port_no,  1, 0x0040));
		VTSS_RC(vtss_phy_write(port_no,  0, 0x8f86));
		VTSS_RC(vtss_phy_page_test(port_no));
		VTSS_RC(vtss_phy_write(port_no,  8, 0x0008));
		VTSS_RC(vtss_phy_page_std(port_no));
		VTSS_RC(vtss_phy_write(port_no, 31, 0x2daf));
		VTSS_RC(vtss_phy_page_test(port_no));
		VTSS_RC(vtss_phy_write(port_no,  1, 0x0000));
		VTSS_RC(vtss_phy_page_std(port_no));
		VTSS_RC(vtss_phy_write(port_no,  0, 0x1840));
		VTSS_RC(vtss_phy_page_std(port_no));
		VTSS_RC(vtss_phy_write(port_no,  0, 0x1040));
		VTSS_RC(vtss_phy_page_std(port_no));
		VTSS_RC(vtss_phy_write(port_no,  9, 0x0300));
		VTSS_RC(vtss_phy_page_test(port_no));
		VTSS_RC(vtss_phy_write(port_no,  5, 0x8000));
		VTSS_RC(vtss_phy_page_test(port_no));
		VTSS_RC(vtss_phy_write(port_no,  8, 0x0208));
		VTSS_RC(vtss_phy_page_tr(port_no));
		VTSS_RC(vtss_phy_write(port_no,  2, 0x0000));
		VTSS_RC(vtss_phy_write(port_no,  1, 0x0000));
		VTSS_RC(vtss_phy_write(port_no,  0, 0x9782));
		VTSS_RC(vtss_phy_page_tr(port_no));
		VTSS_RC(vtss_phy_write(port_no,  2, 0x0000));
		VTSS_RC(vtss_phy_write(port_no,  1, 0x0000));
		VTSS_RC(vtss_phy_write(port_no,  0, 0x9782));
		VTSS_RC(vtss_phy_page_tr(port_no));
		VTSS_RC(vtss_phy_write(port_no,  2, 0x003c));
		VTSS_RC(vtss_phy_write(port_no,  1, 0x3800));
		VTSS_RC(vtss_phy_write(port_no,  0, 0x8f8a));
		VTSS_RC(vtss_phy_page_tr(port_no));
		VTSS_RC(vtss_phy_write(port_no,  2, 0x003c));
		VTSS_RC(vtss_phy_write(port_no,  1, 0x3800));
		VTSS_RC(vtss_phy_write(port_no,  0, 0x8f8a));
		VTSS_RC(vtss_phy_page_test(port_no));
		VTSS_RC(vtss_phy_write(port_no,  8, 0x0008));
		VTSS_RC(vtss_phy_page_std(port_no));
		VTSS_RC(vtss_phy_write(port_no,  0, 0x1240));
	} else if (ps->revision == 5) {
		/* VSC8204 B4 */
		VTSS_RC(vtss_phy_write(port_no, 23, 0x1100));
		VTSS_RC(vtss_phy_page_std(port_no));
		VTSS_RC(vtss_phy_write(port_no, 31, 0xF01D));
		VTSS_RC(vtss_phy_page_test(port_no));
		VTSS_RC(vtss_phy_write(port_no,  1, 0x8000));
		VTSS_RC(vtss_phy_page_test(port_no));
		VTSS_RC(vtss_phy_write(port_no,  8, 0x0208));
		VTSS_RC(vtss_phy_page_tr(port_no));
		VTSS_RC(vtss_phy_write(port_no,  2, 0x0085));
		VTSS_RC(vtss_phy_write(port_no,  1, 0x23F0));
		VTSS_RC(vtss_phy_write(port_no,  0, 0x8770));
		VTSS_RC(vtss_phy_page_tr(port_no));
		VTSS_RC(vtss_phy_write(port_no,  2, 0x0000));
		VTSS_RC(vtss_phy_write(port_no,  1, 0x0400));
		VTSS_RC(vtss_phy_write(port_no,  0, 0x8F88));
		VTSS_RC(vtss_phy_page_tr(port_no));
		VTSS_RC(vtss_phy_write(port_no,  2, 0x0000));
		VTSS_RC(vtss_phy_write(port_no,  1, 0x0400));
		VTSS_RC(vtss_phy_write(port_no,  0, 0x8F88));
		VTSS_RC(vtss_phy_page_test(port_no));
		VTSS_RC(vtss_phy_write(port_no,  8, 0x0008));
		VTSS_RC(vtss_phy_page_test(port_no));
		VTSS_RC(vtss_phy_write(port_no,  1, 0x0000));
		VTSS_RC(vtss_phy_page_std(port_no));
		VTSS_RC(vtss_phy_write(port_no,  0, 0x1840));
		VTSS_RC(vtss_phy_page_std(port_no));
		VTSS_RC(vtss_phy_write(port_no,  0, 0x1040));
		VTSS_RC(vtss_phy_page_std(port_no));
		VTSS_RC(vtss_phy_write(port_no, 31, 0x2DAF));
		VTSS_RC(vtss_phy_page_std(port_no));
	} else if (ps->revision == 6) {
		/* VSC8204 B5 */
		VTSS_RC(vtss_phy_write(port_no, 27, 0x0114)); /* LED configuration */
	}
	return VTSS_OK;
}

/* Init Scripts for VSC8224/VSC8234/VSC8244 aka Quattro */
vtss_rc vtss_phy_init_seq_quattro(vtss_phy_port_state_info_t* ps, 
                                  const vtss_phy_reset_setup_t * const setup, 
                                  const vtss_port_no_t port_no) 
{
	ushort reg;
	
	/* BZ 1380 - PLL Error Detect Bit Enable */
	VTSS_RC(vtss_phy_page_test(port_no));
	VTSS_RC(vtss_phy_writemasked(port_no, 2, 0x8000, 0x8000));
	VTSS_RC(vtss_phy_page_std(port_no));
	
	if (ps->revision < 3) {
		/* BZ 1671 */
		VTSS_RC(vtss_phy_page_test(port_no));
		VTSS_RC(vtss_phy_writemasked(port_no, 8, 0x0200, 0x0200));
		VTSS_RC(vtss_phy_page_tr(port_no));
		VTSS_RC(vtss_phy_write(port_no, 18, 0x0004));
		VTSS_RC(vtss_phy_write(port_no, 17, 0x0671));
		VTSS_RC(vtss_phy_write(port_no, 16, 0x8fae));
		VTSS_RC(vtss_phy_page_test(port_no));
		VTSS_RC(vtss_phy_writemasked(port_no, 8, 0x0000, 0x0200));
		VTSS_RC(vtss_phy_page_std(port_no));
		VTSS_RC(vtss_phy_writemasked(port_no, 18, 0x0040, 0x0040));
		
		/* BZ 1746 */
		VTSS_RC(vtss_phy_page_test(port_no));
		VTSS_RC(vtss_phy_writemasked(port_no, 8, 0x0200, 0x0200));
		VTSS_RC(vtss_phy_page_tr(port_no));
		VTSS_RC(vtss_phy_write(port_no, 18, 0x000f));
		VTSS_RC(vtss_phy_write(port_no, 17, 0x492a));
		VTSS_RC(vtss_phy_write(port_no, 16, 0x8fa4));
		VTSS_RC(vtss_phy_page_test(port_no));
		VTSS_RC(vtss_phy_writemasked(port_no, 8, 0x0000, 0x0200));
		VTSS_RC(vtss_phy_page_std(port_no));
		
		/* BZ 1799 */
		VTSS_RC(vtss_phy_optimize_jumbo(port_no));
		
		/* BZ 2094 */   
		if (setup->mac_if == VTSS_PORT_INTERFACE_RGMII) {
			VTSS_RC(vtss_phy_optimize_rgmii_strength(port_no));
		}
	}        
	
	/* BZ 1776 */
	VTSS_RC(vtss_phy_optimize_receiver_init(port_no));
	
	/* BZ 2229 */
	VTSS_RC(vtss_phy_page_test(port_no));
	VTSS_RC(vtss_phy_writemasked(port_no, 8, 0x0200, 0x0200));
	VTSS_RC(vtss_phy_optimize_dsp (port_no));
	VTSS_RC(vtss_phy_page_test(port_no));        
	VTSS_RC(vtss_phy_writemasked(port_no, 8, 0x0000, 0x0200));
	VTSS_RC(vtss_phy_page_std(port_no));        
	
	/* BZ 2080 */
	VTSS_RC(vtss_phy_page_test(port_no));
	VTSS_RC(vtss_phy_writemasked(port_no, 8, 0x0200, 0x0200));
	VTSS_RC(vtss_phy_page_tr(port_no));
	VTSS_RC(vtss_phy_write(port_no, 16, 0xaf82));
	VTSS_RC(vtss_phy_read(port_no, 17, &reg));
	reg = (reg & 0xffef) | 0;           /*- Enable DFE in 100BT */
	VTSS_RC(vtss_phy_write(port_no, 17, reg));
	VTSS_RC(vtss_phy_write(port_no, 16, 0x8f82));
	VTSS_RC(vtss_phy_page_test(port_no));
	VTSS_RC(vtss_phy_writemasked(port_no, 8, 0x0000, 0x0200));
	VTSS_RC(vtss_phy_page_std(port_no));        
	
	/* Enable signal detect input (active low) if not copper media */
	VTSS_RC(vtss_phy_write(port_no, 19 | VTSS_PHY_REG_EXTENDED, 
		(ushort)(setup->media_if == VTSS_PHY_MEDIA_INTERFACE_COPPER ? 
		0x0002 : 0x0001))); 
	
	/* Disable down shifting */
	VTSS_RC(vtss_phy_writemasked(port_no, 20 | VTSS_PHY_REG_EXTENDED,
		0x0000, 0x0010));
	return VTSS_OK;    
}

/* Init scripts for VSC8538/VSC8558/VSC8658 aka Spyder/GTO */
vtss_rc vtss_phy_init_seq_spyder(vtss_phy_port_state_info_t* ps,
				 const vtss_port_no_t port_no)
{
	ushort reg;
	
	if (ps->type == VTSS_PHY_TYPE_8538 || ps->type == VTSS_PHY_TYPE_8558) {
		if (ps->revision == 0) { /* VSC8538/58 Rev A */
			/* BZ 2020 */
			VTSS_RC(vtss_phy_page_test(port_no));
			VTSS_RC(vtss_phy_writemasked(port_no, 27, 0x8000, 0x8000));
			VTSS_RC(vtss_phy_writemasked(port_no, 19, 0x0300, 0x0f00));
			VTSS_RC(vtss_phy_page_std(port_no));
			
			/* BZ 2063 */
			VTSS_RC(vtss_phy_page_tr(port_no));
			VTSS_RC(vtss_phy_write(port_no, 16, 0xa60c));
			VTSS_RC(vtss_phy_write(port_no, 16, 0xa60c));
			VTSS_RC(vtss_phy_read(port_no, 17, &reg));
			if ((reg & (1<<3)) == 0) {
				/* !RxTrLock */
				VTSS_RC(vtss_phy_write(port_no, 17, 0x0010));
				VTSS_RC(vtss_phy_write(port_no, 16, 0x8604));
				VTSS_RC(vtss_phy_write(port_no, 17, 0x00df));
				VTSS_RC(vtss_phy_write(port_no, 16, 0x8600));
				VTSS_RC(vtss_phy_write(port_no, 17, 0x00ff));
				VTSS_RC(vtss_phy_write(port_no, 16, 0x8600));
				VTSS_RC(vtss_phy_write(port_no, 17, 0x0000));
				VTSS_RC(vtss_phy_write(port_no, 16, 0x8604));
				VTSS_RC(vtss_phy_write(port_no, 16, 0xa60c));
				VTSS_RC(vtss_phy_write(port_no, 16, 0xa60c));
			}
			VTSS_RC(vtss_phy_page_std(port_no));
			
			/* BZ 2069/2086 */
			VTSS_RC(vtss_phy_page_ext(port_no));
			VTSS_RC(vtss_phy_writemasked(port_no, 17, 0x0000, 0x0001));
			VTSS_RC(vtss_phy_page_std(port_no));
			
			/* BZ 2084 */
			if ((ps->map.addr % 8) == 0) {
				VTSS_RC(vtss_phy_page_gpio(port_no));
				VTSS_RC(vtss_phy_write(port_no,  0, 0x7009)); /*- Hold 8051 in SW Reset,Enable auto incr address and patch clock,Disable the 8051 clock */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x5000)); /*- Dummy write to start off */
				VTSS_RC(vtss_phy_write(port_no, 11, 0xffff)); /*- Dummy write to start off */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x5002)); /*- Dummy write to addr 16384= 02 */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x5040)); /*- Dummy write to addr 16385= 40 */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x500C)); /*- Dummy write to addr 16386= 0C */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x5002)); /*- Dummy write to addr 16387= 02 */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x5040)); /*- Dummy write to addr 16388= 40 */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x5021)); /*- Dummy write to addr 16389= 21 */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x5002)); /*- Dummy write to addr 16390= 02 */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x5040)); /*- Dummy write to addr 16391= 40 */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x5022)); /*- Dummy write to addr 16392= 22 */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x5002)); /*- Dummy write to addr 16393= 02 */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x5040)); /*- Dummy write to addr 16391= 40 */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x5023)); /*- Dummy write to addr 16392= 23 */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x50C2)); /*- Dummy write to addr 16396= C2 */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x50AD)); /*- Dummy write to addr 16397= AD */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x50C2)); /*- Dummy write to addr 16396= C2 */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x50CA)); /*- Dummy write to addr 16399= CA */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x5075)); /*- Dummy write to addr 16400= 75 */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x50CB)); /*- Dummy write to addr 16401= CB */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x509A)); /*- Dummy write to addr 16402= 9A */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x5075)); /*- Dummy write to addr 16400= 75 */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x50CA)); /*- Dummy write to addr 16399= CA */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x5046)); /*- Dummy write to addr 16405= 46 */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x5085)); /*- Dummy write to addr 16406= 85 */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x50CB)); /*- Dummy write to addr 16401= CB */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x50CD)); /*- Dummy write to addr 16408= CD */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x5085)); /*- Dummy write to addr 16406= 85 */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x50CA)); /*- Dummy write to addr 16399= CA */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x50CC)); /*- Dummy write to addr 16411= CC */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x50D2)); /*- Dummy write to addr 16412= D2 */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x50CA)); /*- Dummy write to addr 16399= CA */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x50D2)); /*- Dummy write to addr 16414= D2 */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x50AD)); /*- Dummy write to addr 16415= AD */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x5022)); /*- Dummy write to addr 16416= 22 */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x5022)); /*- Dummy write to addr 16416= 22 */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x5022)); /*- Dummy write to addr 16416= 22 */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x5022)); /*- Dummy write to addr 16416= 22 */
				VTSS_RC(vtss_phy_write(port_no, 12, 0x0000)); /*- Clear internal memory access */
				VTSS_RC(vtss_phy_write(port_no,  0, 0x4099)); /*- Allow 8051 to run again, with patch enabled */
				VTSS_RC(vtss_phy_write(port_no,  0, 0xc099)); /*- Allow 8051 to run again, with patch enabled */
				VTSS_RC(vtss_phy_page_std(port_no));
			}
			
			/* BZ 2087 */
			VTSS_RC(vtss_phy_page_tr(port_no));
			if (ps->type == VTSS_PHY_TYPE_8558) {
				VTSS_RC(vtss_phy_write(port_no, 16, 0xa606));    /*- Request read, Media SerDes Control */
				VTSS_RC(vtss_phy_read(port_no, 17, &reg)); 
				reg = (reg & 0xfff8) | 5;                        /*- Optimize sample delay setting */
				VTSS_RC(vtss_phy_write(port_no, 18, 0x0000));    /*- Is this OK? */
				VTSS_RC(vtss_phy_write(port_no, 17, reg));
				VTSS_RC(vtss_phy_write(port_no,16, 0x8606));    /* Write Media SerDes Control Word */                
			}
			VTSS_RC(vtss_phy_write(port_no, 16, 0xae06));       /* Request read, MAC SerDes control */
			VTSS_RC(vtss_phy_read(port_no, 17, &reg));         
			reg = (reg & 0xfff8) | 5;                           /* Optimize sample delay setting */ 
			VTSS_RC(vtss_phy_write(port_no, 18, 0x0000));       /*- Is this OK? */
			VTSS_RC(vtss_phy_write(port_no, 17, reg));
			VTSS_RC(vtss_phy_write(port_no,16, 0x8e06));        /* Write MAC SerDes Control Word */                
			VTSS_RC(vtss_phy_page_std(port_no));            
		} else if (ps->revision == 1) {             
			/* BZ 2112 */
			/*- Turn off Carrier Extensions */
			VTSS_RC(vtss_phy_page_ext(port_no));
			VTSS_RC(vtss_phy_writemasked(port_no, 20, 0x8000, 0x8000));
			VTSS_RC(vtss_phy_page_std(port_no));
		}
		
		/* BZ 2411 - PLL Error Detector Bit Enable */
		VTSS_RC(vtss_phy_page_test(port_no));
		VTSS_RC(vtss_phy_writemasked(port_no, 2, 0x8000, 0x8000));
		VTSS_RC(vtss_phy_page_std(port_no));
		
		/* BZ 2230 - DSP Optimization, BZ 2230 */
		VTSS_RC(vtss_phy_optimize_dsp (port_no));
		
		/* BZ 1971 */
		VTSS_RC(vtss_phy_writemasked(port_no, 18, 0x0040, 0x0040));
		
		/* BZ 1860 */
		VTSS_RC(vtss_phy_optimize_receiver_init(port_no));
		
		/* BZ 2114 */
		VTSS_RC(vtss_phy_page_tr(port_no));
		VTSS_RC(vtss_phy_write(port_no, 16, 0xaf82));
		VTSS_RC(vtss_phy_read(port_no, 17, &reg));
		reg = (reg & 0xffef) | 0;           /*- Enable DFE in 100BT */
		VTSS_RC(vtss_phy_write(port_no, 17, reg));
		VTSS_RC(vtss_phy_write(port_no, 16, 0x8f82));
		VTSS_RC(vtss_phy_page_std(port_no));        
		
		/* BZ 2221*/
		VTSS_RC(vtss_phy_page_ext(port_no));
		VTSS_RC(vtss_phy_writemasked(port_no, 20, 0x1800, 0x1800));
		VTSS_RC(vtss_phy_page_std(port_no));
		
		
		/* BZ 2012 */
		VTSS_RC(vtss_phy_page_test(port_no));
		VTSS_RC(vtss_phy_writemasked(port_no, 20, 0x0000, 0x1000));  /*- Clear Force Bit */
		VTSS_RC(vtss_phy_writemasked(port_no, 20, 0x6000, 0x6000));  /* MDI Impedance setting = +2 */
		VTSS_RC(vtss_phy_page_std(port_no)); 
		VTSS_RC(vtss_phy_writemasked(port_no, 24, 0xa002, 0xe00e));  /* 100-BT Amplitude/Slew Rate Control */
	/* (ps->type == VTSS_PHY_TYPE_8538 || ps->type == VTSS_PHY_TYPE_8558) */ 
	} else if (ps->type == VTSS_PHY_TYPE_8658) {
		if (ps->revision == 0) {
			/* BZ 2112 */
			/*- Turn off Carrier Extensions */
			VTSS_RC(vtss_phy_page_ext(port_no));
			VTSS_RC(vtss_phy_writemasked(port_no, 20, 0x8000, 0x8000));
			VTSS_RC(vtss_phy_page_std(port_no));
		} 
	}
	
	/* Init scripts common to all Octal PHY devices */
	/* BZ 2486/2487 */
	VTSS_RC(vtss_phy_page_std(port_no));
	
	return VTSS_OK;
}

/* Init scripts for VSC8601/VSC8641 aka Cooper */
vtss_rc vtss_phy_init_seq_cooper(vtss_phy_port_state_info_t* ps,
				 const vtss_port_no_t port_no) 
{
	if (ps->revision == 0) {
		/*- Rev A */
		/* BZ 2231 */
		VTSS_RC(vtss_phy_optimize_dsp(port_no));
		
		/* BZ 2253 */
		VTSS_RC(vtss_phy_page_ext(port_no));
		/*- Enab Enh mode for Reg17E chg */
		VTSS_RC(vtss_phy_writemasked(port_no, 17, 0x0010, 0x0010));
		/*- sets the LEDs combine/separate */
		VTSS_RC(vtss_phy_writemasked(port_no, 17, 0x0007, 0x0007));
		/*- sets back to simple mode */
		VTSS_RC(vtss_phy_writemasked(port_no, 17, 0x0000, 0x0010));
		VTSS_RC(vtss_phy_page_std(port_no));                       
		
		/* BZ 2234 */
		/*- Port Type #2234 */
		VTSS_RC(vtss_phy_writemasked(port_no, 9, 0x0000, 0x0400));
		/*- nVidia Req */
		VTSS_RC(vtss_phy_writemasked(port_no, 27, 0x0004, 0x0004));
	} 
	
	/* BZ 2474 */
	/*- Writing a "0" to MII Reg 31 ensures one SMI transaction before
	 * Reg 4/9 may be accessed
	 */
	VTSS_RC(vtss_phy_page_std(port_no));       
	
	return VTSS_OK;
}

/* Init scripts for VSC7385/VSC7388/VSC7395/VSC7398/VSC7389/VSC7390/VSC7391
 * aka "Luton" family
 */
vtss_rc vtss_phy_init_seq_luton(vtss_phy_port_state_info_t* ps,
                                const vtss_phy_reset_setup_t * const setup, 
                                const vtss_port_no_t port_no)
{        
	/* BZ 1801, 1973 */
	/* 100 Base-TX jumbo frame support */
	VTSS_RC(vtss_phy_optimize_jumbo(port_no));            
	
	/* BZ 2094 */
	/* Insufficient RGMII drive-strength seen especially on long traces */
	if (ps->family != VTSS_PHY_FAMILY_LUTON24) {
		if (setup->mac_if == VTSS_PORT_INTERFACE_RGMII) {
			VTSS_RC(vtss_phy_optimize_rgmii_strength(port_no));
		}
	}
	
	/* BZ 1682, BZ 2345 */        
	/* Enable PLL Error Detector Bit */
	/* Applicable to VSC 7385/7385 Rev A, VSC 7389/90 */        
	if ((ps->family == VTSS_PHY_FAMILY_LUTON24) || 
		(ps->family == VTSS_PHY_FAMILY_LUTON && ps->revision == 0)) { 
		VTSS_RC(vtss_phy_page_test(port_no));
		VTSS_RC(vtss_phy_writemasked(port_no, 2, 0x8000, 0x8000));
		VTSS_RC(vtss_phy_page_std(port_no));
	}
	
	/* Applicable to VSC 7385/7388 Rev A*/
	if (ps->family == VTSS_PHY_FAMILY_LUTON && ps->revision == 0) {
		/* BZ 1954 */
		/* Tweak 100 Base-TX DSP setting for VGA */
		VTSS_RC(vtss_phy_page_test(port_no));
		VTSS_RC(vtss_phy_writemasked(port_no,  8, 0x0200, 0x0200));
		VTSS_RC(vtss_phy_page_tr(port_no));
		VTSS_RC(vtss_phy_write(port_no, 18, 0x0000));
		VTSS_RC(vtss_phy_write(port_no, 17, 0x0689));
		VTSS_RC(vtss_phy_write(port_no, 16, 0x8f92));            
		VTSS_RC(vtss_phy_page_test(port_no));            
		VTSS_RC(vtss_phy_writemasked(port_no,  8, 0x0000, 0x0200));
		
		/* BZ 1933 */
		/* Kick start Tx line-driver common-mode voltage */
		VTSS_RC(vtss_phy_write(port_no, 23, 0xFF80));
		VTSS_RC(vtss_phy_write(port_no, 23, 0x0000));
		VTSS_RC(vtss_phy_page_std(port_no));
	}
	
	/* BZ 2226/2227/2228 - DSP Optimization */
	VTSS_RC(vtss_phy_page_test(port_no));
	VTSS_RC(vtss_phy_writemasked(port_no, 8, 0x0200, 0x0200));
	VTSS_RC(vtss_phy_optimize_dsp(port_no));
	VTSS_RC(vtss_phy_page_test(port_no));
	VTSS_RC(vtss_phy_writemasked(port_no, 8, 0x0000, 0x0200));
	
	/* BZ 2095/2107 - DSP short cable optimization */
	VTSS_RC(vtss_phy_optimize_receiver_init(port_no));
	/*- Additional Init for parts that */
	VTSS_RC(vtss_phy_page_test(port_no));
	/*- lack Viterbi decoder */
	VTSS_RC(vtss_phy_writemasked(port_no, 8, 0x0200, 0x0200));
	/*- to prevent DSP drift from */
	VTSS_RC(vtss_phy_page_tr(port_no));
	/*- leading to bit errors */
	VTSS_RC(vtss_phy_write(port_no, 16, 0xb68a));
	VTSS_RC(vtss_phy_writemasked(port_no, 18, 0x0003, 0xff07));
	VTSS_RC(vtss_phy_writemasked(port_no, 17, 0x00a2, 0x00ff)); 
	VTSS_RC(vtss_phy_write(port_no, 16, 0x968a));
	VTSS_RC(vtss_phy_page_test(port_no));
	VTSS_RC(vtss_phy_writemasked(port_no, 8, 0x0000, 0x0200));
	VTSS_RC(vtss_phy_page_std(port_no));
	
	/* BZ 1742/1971/2000/2034 */
	VTSS_RC(vtss_phy_writemasked(port_no, 18, 0x0040, 0x0040));
	
	/* Amplitude/ Z-Cal Adjustments at startup */
	/* These values are start-up values and will get readjusted based
	 * on link-up speed
	 */
	if (ps->family == VTSS_PHY_FAMILY_LUTON24) {
		VTSS_RC(vtss_phy_writemasked(port_no, 24, 0xa002, 0xe00e));
	} else if (ps->family == VTSS_PHY_FAMILY_LUTON && ps->revision == 0) {
		/* BZ 2012 - Applies to VSC 7385/7388 Rev A */
		VTSS_RC(vtss_phy_page_test(port_no));
		/* Trim MDI Termination Impedance */
		VTSS_RC(vtss_phy_writemasked(port_no, 20, 0x4000, 0x6000));
		VTSS_RC(vtss_phy_page_std(port_no));
		/* Trim 100/1000 Tx amplitude & edge rate */
		VTSS_RC(vtss_phy_writemasked(port_no, 24, 0xa00e, 0xe00e));
	} else {
		/* BZ 2043 - Applies to VSC 7385/7388 Rev B and later,
		 * VSC 7395/7398 all revs
		 */
		VTSS_RC(vtss_phy_page_test(port_no));
		/* Trim reference currents */
		VTSS_RC(vtss_phy_writemasked(port_no, 22, 0x0240, 0x0fc0));
		/* Trim termination impedance */
		VTSS_RC(vtss_phy_writemasked(port_no, 20, 0x4000, 0x6000));
		/* 1000-BT Edge Rate Ctrl */
		VTSS_RC(vtss_phy_writemasked(port_no, 24, 0x0030, 0x0038));
		VTSS_RC(vtss_phy_page_ext(port_no));
		/* Trim 1000 Base-T amplitude */
		VTSS_RC(vtss_phy_writemasked(port_no, 20, 0x2000, 0xe000));
		VTSS_RC(vtss_phy_page_std(port_no));
		/* 100-BT Edge Rate Ctrl */
		VTSS_RC(vtss_phy_writemasked(port_no, 24, 0x8002, 0xe00e));
	}
	return VTSS_OK;
}

/* Init scripts for VSC7500-VSC7507 aka "Intrigue" family */
vtss_rc vtss_phy_init_seq_intrigue (vtss_phy_port_state_info_t* ps,
                                    const vtss_phy_reset_setup_t * const setup, 
                                    const vtss_port_no_t port_no)
{
	VTSS_RC(vtss_phy_page_std(port_no));
	
	/*- Set 100-Base-T Tx Edge Rate */
	VTSS_RC(vtss_phy_writemasked(port_no, 24, 0xa000, 0xe000));
	/*- Boost 1000-BaseT Amplitude */
	VTSS_RC(vtss_phy_page_ext(port_no));
	VTSS_RC(vtss_phy_writemasked(port_no, 20, 0xe000, 0xe000));
	/*- Boost 100-BaseT Amplitude */
	if (port_no == 1) {
		VTSS_RC(vtss_phy_page_test(port_no));
		VTSS_RC(vtss_phy_writemasked(port_no, 28, 0x0080, 0x01c0));
	}
	
	VTSS_RC(vtss_phy_page_std(port_no));
	return VTSS_OK;
}                                    

/* PHY Init scripts end here */
#include <linux/vitgenio.h>
#include <linux/ioctl.h> /* ioctl() */

/* Reset PHY I/O Layer, must be called before any other function */
void vtss_phy_io_start(void)
{
#if (defined(CONFIG_VTSS_FORCE_CPLD_MIIM) || !defined(CONFIG_VTSS_API)) && \
    defined(CONFIG_VTSS_VITGENIO)
	vtss_log(VTSS_LOG_DEBUG, "SWITCH: fd set for phy_io");
	vtss_phy_io_state.fd = vtss_mac_io_state.fd;
#endif
}

/* Read PHY register */
vtss_rc vtss_phy_io_read(const vtss_phy_io_bus_t bus,
                         const uint addr, const uint reg,
                         ushort *const value)
{
	vtss_rc rc;

#if (defined(CONFIG_VTSS_FORCE_CPLD_MIIM) || !defined(CONFIG_VTSS_API)) && \
    defined(CONFIG_VTSS_VITGENIO)
	int val;
	/* arg=(phy_address<<5)|phy_register, returns result */
	val = ioctl(vtss_phy_io_state->fd, VITGENIO_GET_CPLD_MIIM,
		    addr << 5|reg);
	if (val<0) {
		rc = VTSS_UNSPECIFIED_ERROR;
	} else {
		*value = val;
		rc = VTSS_OK;
	}
#else
#ifdef CONFIG_VTSS_API
	/* Use VTSS Switch API for accessing PHY management bus. */
	rc = vtss_miim_phy_read(VTSS_PHY_CHIPMIIM(bus), addr, reg, value);
#endif
#endif
	if (rc < VTSS_WARNING) {
		vtss_log(VTSS_LOG_WARN,
			 "PHY: value ERROR, bus=0x%04x, addr=0x%02x, reg=0x%02x",
			 bus, addr, reg);
	}
	return rc;
}

/* Write PHY register */
vtss_rc vtss_phy_io_write(const vtss_phy_io_bus_t bus,
                          const uint addr, const uint reg,
                          const ushort value)
{
	vtss_rc rc;
#if (defined(CONFIG_VTSS_FORCE_CPLD_MIIM) || !defined(CONFIG_VTSS_API)) && \
    defined(CONFIG_VTSS_VITGENIO)
	/* arg=(phy_address<<(16+5))|(phy_register<<16)|value */
	if (ioctl(vtss_phy_io_state->fd, VITGENIO_SET_CPLD_MIIM,
		  addr<<(16+5) | reg<<16 | value) < 0)
		rc = VTSS_UNSPECIFIED_ERROR;
	else
		rc = VTSS_OK;
#else
#ifdef CONFIG_VTSS_API
	/* Use VTSS Switch API for accessing PHY management bus. */
	rc = vtss_miim_phy_write(VTSS_PHY_CHIPMIIM(bus), addr, reg, value);
#endif
#endif
	return rc;
}
