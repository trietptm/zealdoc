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
 
 $Id: vtss_io.c,v 1.9 2008-12-08 06:10:10 zhenglv Exp $
 $Revision: 1.9 $

*/

#include "vtss_priv.h"

vtss_mac_io_state_t vtss_mac_io_state;

/* Reset I/O Layer, must be called before any other function */
void vtss_io_start(void)
{
#ifdef CONFIG_VTSS_VITGENIO
	int fd;
#ifndef CONFIG_VTSS_GROCX
	struct vitgenio_cs_setup_timing timing;
#endif
	
	memset(&vtss_mac_io_state, 0, sizeof (vtss_mac_io_state_t));

	/* Open driver */
	if ((fd = open(_PATH_VITGENIO, 0)) < 0) {
		vtss_log(VTSS_LOG_ERR, "BOARD: open(/dev/vitgenio) failure");
		return;
	}
	
#ifdef CONFIG_VTSS_USE_CPU_SI
	{
		/* Setup SI interface */
		struct vitgenio_cpld_spi_setup timing = {
			/* char ss_select; Which of the GPIOs is used for Slave Select */
			VITGENIO_SPI_SS_CPLD_GPIO0,
			VITGENIO_SPI_SS_ACTIVE_LOW, /* char ss_activelow; Slave Select (Chip Select) active low: true, active high: false */
			VITGENIO_SPI_CPOL_0, /* char sck_activelow; CPOL=0: false, CPOL=1: true */
			VITGENIO_SPI_CPHA_0, /* char sck_phase_early; CPHA=0: false, CPHA=1: true */
			VITGENIO_SPI_MSBIT_FIRST, /* char bitorder_msbfirst; */
			0, /* char reserved1; currently unused, only here for alignment purposes */
			0, /* char reserved2; currently unused, only here for alignment purposes */
			0, /* char reserved3; currently unused, only here for alignment purposes */
			500 /* unsigned int ndelay; minimum delay in nanoseconds, two of these delays are used per clock cycle */
		};
		
		vtss_log(VTSS_LOG_DEBUG, "BOARD: setting up SPI timing");
		/* Setup the SPI timing of the I/O Layer driver */
		ioctl(fd, VITGENIO_ENABLE_CPLD_SPI);
		ioctl(fd, VITGENIO_CPLD_SPI_SETUP, &timing);
	}
#endif
	
	/* Setup Parallel Interface timing */
#ifndef CONFIG_VTSS_GROCX
	vtss_log(VTSS_LOG_DEBUG, "BOARD: using CS%d", timing.cs);
	ioctl(fd, VITGENIO_CS_SELECT, timing.cs );
#endif
	vtss_mac_io_state.fd = fd;
#endif
}

#ifdef VTSS_IO_HEATHROW
vtss_rc vtss_io_si_rd(uint block, uint subblock,
		      const uint reg, ulong * const value)
{
	vtss_rc rc = VTSS_OK;
#ifdef CONFIG_VTSS_VITGENIO
	struct vitgenio_cpld_spi_readwrite spibuf;
	
	spibuf.length = 6;
	
	spibuf.buffer[0] = (block<<5) | (0/*READ*/<<4) | (subblock<<0);
	spibuf.buffer[1] = reg;
	spibuf.buffer[2] = 0;
	spibuf.buffer[3] = 0;
	spibuf.buffer[4] = 0;
	spibuf.buffer[5] = 0;
	if (ioctl(vtss_mac_io_state.fd, VITGENIO_CPLD_SPI_READWRITE, (unsigned long)&spibuf) < 0) {
		vtss_log(VTSS_LOG_ERR,
			 "IO: ioctl(VITGENIO_CPLD_SPI_READWRITE) failed");
		*value = 0;
		rc = VTSS_IO_READ_ERROR;
	} else {
		*value = ((spibuf.buffer[2]<<24) | (spibuf.buffer[3]<<16) | \
			  (spibuf.buffer[4]<<8) | (spibuf.buffer[5]<<0));
	}
#endif
    return rc;
}

vtss_rc vtss_io_si_wr(uint block, uint subblock, const uint reg, const ulong value)
{
	vtss_rc rc = VTSS_OK;
#ifdef CONFIG_VTSS_VITGENIO
	struct vitgenio_cpld_spi_readwrite spibuf;
	
	spibuf.length = 6;
	
	spibuf.buffer[0] = (block<<5) | (1/*WRITE*/<<4) | (subblock<<0);
	spibuf.buffer[1] = reg;
	spibuf.buffer[2] = (char)((value>>24) & 0xff);
	spibuf.buffer[3] = (char)((value>>16) & 0xff);
	spibuf.buffer[4] = (char)((value>>8) & 0xff);
	spibuf.buffer[5] = (char)(value & 0xff);
	if (ioctl(vtss_mac_io_state.fd, VITGENIO_CPLD_SPI_READWRITE, (unsigned long)&spibuf) < 0) {
		vtss_log(VTSS_LOG_ERR,
			 "IO: ioctl(VITGENIO_CPLD_SPI_READWRITE) failed");
		rc = VTSS_IO_WRITE_ERROR;
	}
#endif
	return rc;
}

static vtss_rc vtss_io_pi_rd_fast(uint block, uint subblock,
				  const uint reg, ulong *value)
{
	vtss_rc rc = VTSS_OK;
#ifdef CONFIG_VTSS_VITGENIO
	struct vitgenio_32bit_readwrite pibuf;
	
	pibuf.offset = (block<<12)|(subblock<<8)|(reg<<0);
	if (ioctl(vtss_mac_io_state.fd, VITGENIO_32BIT_READ, (unsigned long)&pibuf) < 0) {
		vtss_log(VTSS_LOG_ERR, "IO: ioctl(VITGENIO_32BIT_READ) failed");
		*value = 0;
		rc = VTSS_IO_READ_ERROR;
	} else {
		*value=pibuf.value;
	}
#endif
	return rc;
}

vtss_rc vtss_io_pi_rd(uint block, uint subblock, const uint reg, ulong * const value)
{
	vtss_rc rc;
	
	if (
		block == B_CAPTURE || block == B_SYSTEM) {
		rc = vtss_io_pi_rd_fast(block, subblock, reg, value);
	} else {
		int   i;
		ulong val;
		
		if ((rc = vtss_io_pi_rd_fast(block,subblock,reg,&val)) < 0)
			return rc;
		for (i=0;;i++) {
			if ((rc = vtss_io_pi_rd_fast(B_SYSTEM, 0, R_SYSTEM_CPUCTRL, &val)) < 0)
				break;
			if (val & (1 << 4)) {
				rc = vtss_io_pi_rd_fast(B_SYSTEM, 0, R_SYSTEM_SLOWDATA, value);
				break;
			}
			if (i == 25) {
				vtss_log(VTSS_LOG_ERR,
					 "IO: DONE failed after %d retries, block=0x%02x, subblock=0x%02x, reg=0x%02x",
					 i, block, subblock, reg);
				rc=VTSS_IO_READ_ERROR;
				break;
			}
		};
	}
	return rc;
}

vtss_rc vtss_io_pi_wr(uint block, uint subblock, const uint reg, const ulong value)
{
	vtss_rc rc = VTSS_OK;
#ifdef CONFIG_VTSS_VITGENIO
	struct vitgenio_32bit_readwrite pibuf;
	
	pibuf.offset = (block<<12)|(subblock<<8)|(reg<<0);
	pibuf.value = value;
	if (ioctl(vtss_mac_io_state.fd, VITGENIO_32BIT_WRITE, (unsigned long)&pibuf) < 0) {
		vtss_log(VTSS_LOG_ERR, "IO: ioctl(VITGENIO_32BIT_WRITE) failed");
		rc=VTSS_IO_WRITE_ERROR;
	}
#endif
	return rc;
}
#endif
