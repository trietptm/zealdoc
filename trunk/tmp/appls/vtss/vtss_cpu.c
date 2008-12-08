#include "vtss_priv.h"
#include <vtss_cpu.h>

#ifdef CONFIG_VTSS_CPU_FRAME

/* ================================================================= *
 *  Packet Control
 * ================================================================= */
vtss_cpu_state_t vtss_cpu_state;

/* ================================================================= *
 * CPU frame extraction
 * ================================================================= */

/* Enable/disable frame ready interrupt */
vtss_rc vtss_ll_frame_rx_ready_int(vtss_cpu_rx_queue_t queue_no, BOOL enable)
{
#ifdef CONFIG_VTSS_ARCH_SPARX
	ulong offset = (queue_no-VTSS_CPU_RX_QUEUE_START);
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	offset += 9;
#endif
	HT_WRF(SYSTEM, 0, CAPCTRL, offset, 0x1, enable ? 1 : 0);
#endif
	return VTSS_OK;
}

#if (VTSS_CPU_RX_QUEUES>1)
/* Setup Rx queue map */
vtss_rc vtss_ll_cpu_rx_queue_map_set(vtss_cpu_rx_queue_map_t * const map)
{
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	ulong i, value = 0;
	
	HT_WR(ANALYZER, 0, CAPQUEUE,
	      ((map->learn_queue-VTSS_CPU_RX_QUEUE_START)<<12) |
	      ((map->mac_vid_queue-VTSS_CPU_RX_QUEUE_START)<<10) |
	      ((map->igmp_queue-VTSS_CPU_RX_QUEUE_START)<<8) |
	      ((map->ipmc_ctrl_queue-VTSS_CPU_RX_QUEUE_START)<<6) |
	      ((map->igmp_queue-VTSS_CPU_RX_QUEUE_START)<<4) |
	      (0<<2) | /* 01-80-C2-00-00-10 */
	      ((map->bpdu_queue-VTSS_CPU_RX_QUEUE_START)<<0));
	
	for (i = 0; i < 16; i++) {
		value |= ((map->garp_queue-VTSS_CPU_RX_QUEUE_START)<<(i*2));
	}
	HT_WR(ANALYZER, 0, CAPQUEUEGARP, value);
#endif
	return VTSS_OK;
}

#ifdef CONFIG_VTSS_ARCH_SPARX_28
/* Set Rx queue size */
vtss_rc vtss_ll_cpu_rx_queue_size_set(const vtss_cpu_rx_queue_t queue_no, 
                                      const vtss_cpu_rx_queue_size_t size)
{
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	HT_WRF(PORT, VTSS_CHIP_PORT_CPU, Q_EGRESS_WM, 
	       (queue_no-VTSS_CPU_RX_QUEUE_START)*8, 0x1F, size/2048);
#endif
	return VTSS_OK;
}
#endif /* SPARX_28 */
#endif /* VTSS_CPU_RX_QUEUES>1 */

/* Set frame registration */
vtss_rc vtss_ll_frame_reg_set(const vtss_cpu_rx_registration_t *reg)
{
	vtss_port_no_t port_no;
	uint port_on_chip;
	
	for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
		port_on_chip = HT_CHIP_PORT(port_no);
		if (port_on_chip >= 0) {
		}
	}
#ifdef CONFIG_VTSS_ARCH_SPARX
	{
		ulong value = 0;
		
		value |= ((MAKEBOOL01(reg->bpdu_cpu_only)<<16) |
			  (MAKEBOOL01(reg->mcast_igmp_cpu_only)<<18));
#ifdef CONFIG_VTSS_ARCH_SPARX_28
		value |= (MAKEBOOL01(reg->ipmc_ctrl_cpu_copy)<<19);
#endif
		/* Update all CAPENAB fields */
		HT_WR(ANALYZER, 0, CAPENAB, value);
	}
#endif
	return VTSS_OK;
}

/* Determine if a frame is ready in the CPU Rx queue */
/* Returns: 0: no frame ready, <0: error, >0: frame ready */
vtss_rc vtss_ll_frame_rx_ready(vtss_cpu_rx_queue_t queue_no)
{
	ulong ready;
#ifdef CONFIG_VTSS_ARCH_SPARX
	ulong offset;
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	offset = 28;
#endif
	HT_RDF(SYSTEM, 0, CAPCTRL,
	       offset + queue_no-VTSS_CPU_RX_QUEUE_START, 0x1, &ready);
#endif
	return ready;
}

static vtss_rc ht_cpurx_readbuffer(vtss_cpu_rx_queue_t queue_no,
				   const uint addr, ulong *value)
{
	uint sub, offset;
	
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	sub = (S_CAPTURE_DATA+(queue_no-VTSS_CPU_RX_QUEUE_START)*2);
	offset = 0; /* SparX-28 uses FIFO mode */
#endif
	HT_RD(CAPTURE, sub, FRAME_DATA + offset, value);
	return VTSS_OK;
}

static ulong ht_cpu_ulong_from_buffer(const uchar *const buffer)
{
	return (buffer[0] << 24) |
	       (buffer[1] << 16) |
	       (buffer[2] <<  8) |
	       (buffer[3]      ) ;
}

static vtss_rc ht_cpurx_systemheader(vtss_cpu_rx_queue_t queue_no, 
                                     vtss_system_frame_header_t * const sys_header)
{
	ulong ifh0, ifh1;
	
	VTSS_RC(ht_cpurx_readbuffer(queue_no, 0, &ifh0));
	VTSS_RC(ht_cpurx_readbuffer(queue_no, 1, &ifh1));
	
	sys_header->length = (IFH_GET(ifh0, ifh1, LENGTH) - VTSS_FCS_SIZE);
	sys_header->source_port_no = vtss_mac_state.port_map.vtss_port[IFH_GET(ifh0, ifh1, PORT)];
	sys_header->arrived_tagged = IFH_GET(ifh0, ifh1, TAGGED);
	sys_header->tag.tagprio = IFH_GET(ifh0, ifh1, UPRIO);
	sys_header->tag.cfi = IFH_GET(ifh0, ifh1, CFI);
	sys_header->tag.vid = IFH_GET(ifh0, ifh1, VID);
#ifdef VTSS_FEATURE_CPU_RX_LEARN
	sys_header->learn = IFH_GET(ifh0, ifh1, LEARN);
#endif
	
	/* Check port */
	if (!VTSS_PORT_IS_PORT(sys_header->source_port_no)) {
		vtss_log(VTSS_LOG_ERR,
			 "SPARX: illegal portno, port=%u, ifh0=0x%08lx, ifh1=0x%08lx",
			 sys_header->source_port_no, ifh0, ifh1);
		return VTSS_DATA_NOT_READY;
	}
	return VTSS_OK;
}

/* Get frame from CPU Rx queue */
vtss_rc vtss_ll_frame_rx(vtss_cpu_rx_queue_t queue_no,
			 vtss_system_frame_header_t *sys_header,
                         uchar *frame, uint maxlength)
{
	uint w, len;
	ulong data;
	uint addr = 2;
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	vtss_packet_rx_t *packet;
	
	packet = &vtss_cpu_state.rx_packet[queue_no-VTSS_CPU_RX_QUEUE_START];
	if (packet->used) {
		/* Header has already been read, copy from API state */
		*sys_header = packet->sys_header;
	} else {
		/* Read header and copy to API state */
		VTSS_RC(ht_cpurx_systemheader(queue_no, sys_header));
		packet->used = 1;
		packet->sys_header = *sys_header;
	}
#else
	VTSS_RC(ht_cpurx_systemheader(queue_no, sys_header));
#endif
	
	/* Check if the application is just requesting the header */
	if (frame == NULL)
		return VTSS_PACKET_BUF_SMALL;
	
	len = (sys_header->length<maxlength) ? sys_header->length : maxlength;
	for (w = 0; w < (len+3)/4; w++) {
		VTSS_RC(ht_cpurx_readbuffer(queue_no, addr++, &data));
		frame[w*4+0] = (uchar)((data>>24) & 0xff);
		frame[w*4+1] = (uchar)((data>>16) & 0xff);
		frame[w*4+2] = (uchar)((data>>8) & 0xff);
		frame[w*4+3] = (uchar)(data & 0xff);
	}
	
	if (maxlength<sys_header->length) {
#ifdef CONFIG_VTSS_ARCH_SPARX_28
		vtss_log(VTSS_LOG_ERR, "SPARX: buffer too small");
#endif
		return VTSS_PACKET_BUF_SMALL;
	}
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	packet->words_read = w;
#endif
	return vtss_ll_frame_rx_discard(queue_no);
}

/* Discard frame from CPU Rx queue */
vtss_rc vtss_ll_frame_rx_discard(vtss_cpu_rx_queue_t queue_no)
{
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	vtss_packet_rx_t *packet;
	ulong w, w_max, value;
	
	packet = &vtss_cpu_state.rx_packet[queue_no-VTSS_CPU_RX_QUEUE_START];
	if (packet->used) {
		/* Calculate number of words to read */
		w_max = (packet->sys_header.length+7)/4; /* Include FCS */
		if (w_max & 1)
			w_max++; /* Read even number of words */
		
		/* Read out rest of frame */
		for (w = packet->words_read; w < w_max; w++) {
			VTSS_RC(ht_cpurx_readbuffer(queue_no, 0, &value));
		}
		packet->used = 0;
		packet->words_read = 0;
	}
#endif
	return VTSS_OK;
}

/* ================================================================= *
 * CPU frame injection
 * ================================================================= */

static vtss_rc ht_cputx_autoupdate_crc(const BOOL enable)
{
	vtss_port_no_t port_no;
	uint port_on_chip;
	ulong mask = (1<<1);
	
	for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++)
	{
		port_on_chip = HT_CHIP_PORT(port_no);
		HT_WRM(PORT, port_on_chip, TXUPDCFG, enable ? mask : 0, mask);
	}
	return VTSS_OK;
}

#define VTSS_CPUTX_WAIT_MAXRETRIES 1000000

static vtss_rc ht_wr_cputxdat_chipport(uint port_on_chip, const ulong value)
{
	uint  read_attempt=0;
	ulong pending;
	
	HT_WR(PORT, port_on_chip, CPUTXDAT, value);
	do {
		HT_RDF(PORT, port_on_chip, MISCSTAT, 8, 0x1, &pending);
		if (!pending)
			return VTSS_OK; /* Done */
	} while (read_attempt++<VTSS_CPUTX_WAIT_MAXRETRIES);
	
	/* Timeout, cancel transmission */
	HT_WR(PORT, port_on_chip, MISCFIFO, (1<<1) | MISCFIFO_TAILDROP);
	return VTSS_TIMEOUT_RETRYLATER;
}

/* Transmit frame on port, optionally with tag. Padding may be done */
vtss_rc vtss_ll_frame_tx(vtss_port_no_t port_no, const uchar *frame, 
                         uint length, vtss_vid_t vid)
{
	uint  port_on_chip = HT_CHIP_PORT(port_no);
	uint  w, addw, framelen;
	ulong ifh0 = 0, ifh1 = 0;
	
	/* Calculate and send IFH0 and IFH1 */
	addw = 1; /* CRC */
	if (vid != VTSS_VID_NULL)
		addw++; /* VLAN tag */
	
	framelen = (length + addw*4);
	IFH_PUT(ifh0, ifh1, LENGTH, framelen < 64 ? 64 : framelen);
	
	VTSS_RC(ht_wr_cputxdat_chipport(port_on_chip, ifh0));
	VTSS_RC(ht_wr_cputxdat_chipport(port_on_chip, ifh1));
	
	/* Send frame data */
	for (w=0; w <= (length-1)/4; w++) {
		if (w==3) {
			if (vid != VTSS_VID_NULL) {
				/* Insert tag */
				VTSS_RC(ht_wr_cputxdat_chipport(port_on_chip, 
					(0x8100<<16) | (0<<13) | (0<<12) | vid));
			}
		}
		VTSS_RC(ht_wr_cputxdat_chipport(port_on_chip, 
			ht_cpu_ulong_from_buffer(&(frame[w*4]))));
	}
	
	/* Add padding when the frame is too short */
	w += addw;
	while (w < (64/4)) {
		VTSS_RC(ht_wr_cputxdat_chipport(port_on_chip, 0));
		w++;
	}
	
	/* Add dummy CRC */
	VTSS_RC(ht_wr_cputxdat_chipport(port_on_chip, 0));
	
	/* We must write an even number of longs, so write an extra */
	if (w & 1)
		VTSS_RC(ht_wr_cputxdat_chipport(port_on_chip, 0));
	
	/* Transmit data */
	HT_WR(PORT, port_on_chip, MISCFIFO, (1<<0) | MISCFIFO_TAILDROP);
	
	return VTSS_OK;
}

vtss_rc ht_cpu_frame_reset(void)
{
	return ht_cputx_autoupdate_crc(1/*TRUE*/);
}

/* ================================================================= *
 *  Packet RX/TX
 * ================================================================= */

/* - RX frame registration ----------------------------------------- */

#if (VTSS_CPU_RX_QUEUES>1)
vtss_rc vtss_cpu_rx_queue_map_set(vtss_cpu_rx_queue_map_t * const map)
{
	vtss_cpu_rx_queue_t queue_no;
	int i;
	
	/* Check queues */
	for (i=0; i<(sizeof(*map)/sizeof(queue_no)); i++) {
		queue_no = *((vtss_cpu_rx_queue_t *)map+i);
		if (queue_no<VTSS_CPU_RX_QUEUE_START || queue_no>=VTSS_CPU_RX_QUEUE_END) {
			vtss_log(VTSS_LOG_ERR,
				 "SWITCH: illegal queue, queue=%d, index=%d",
				 queue_no, i);
			return VTSS_INVALID_PARAMETER;
		}
		vtss_log(VTSS_LOG_DEBUG,
			 "SWITCH: legal queue, queue=%d, index=%d",
			 queue_no, i);
	}
	
	return vtss_ll_cpu_rx_queue_map_set(map);
}

#ifdef CONFIG_VTSS_ARCH_SPARX_28
vtss_rc vtss_cpu_rx_queue_size_set(const vtss_cpu_rx_queue_t queue_no, 
                                   const vtss_cpu_rx_queue_size_t size)
{
	if (queue_no<VTSS_CPU_RX_QUEUE_START || queue_no>=VTSS_CPU_RX_QUEUE_END) {
		vtss_log(VTSS_LOG_ERR,
			 "SWITCH: illegal queue, queue=%d",
			 queue_no);
		return VTSS_INVALID_PARAMETER;
	}
	return vtss_ll_cpu_rx_queue_size_set(queue_no, size);
}
#endif /* SPARX_28 */    
#endif /* VTSS_CPU_RX_QUEUES>1 */

vtss_rc vtss_cpu_rx_registration_get(vtss_cpu_rx_registration_t *const registration)
{
	*registration = vtss_cpu_state.rx_registration;
	return VTSS_OK;
}

vtss_rc vtss_cpu_rx_registration_set(const vtss_cpu_rx_registration_t *const registration)
{
	vtss_cpu_state.rx_registration = *registration;
	return vtss_ll_frame_reg_set(registration);
}

/* - RX frames ----------------------------------------------------- */
vtss_rc vtss_cpu_rx_frameready_int_enable(const vtss_cpu_rx_queue_t queue_no,
                                          const BOOL enable)
{
	return vtss_ll_frame_rx_ready_int(queue_no, enable);
}

vtss_rc vtss_cpu_rx_frameready(const vtss_cpu_rx_queue_t queue_no)
{
	vtss_rc rc;
	
	rc = vtss_ll_frame_rx_ready(queue_no);
	return rc;
}

vtss_rc vtss_cpu_rx_frame(const vtss_cpu_rx_queue_t queue_no,
                          vtss_system_frame_header_t * const sys_header,
                          uchar * const frame, 
                          const uint maxlength)
{
	return vtss_ll_frame_rx(queue_no, sys_header, frame, maxlength);
}

vtss_rc vtss_cpu_rx_discard_frame(const vtss_cpu_rx_queue_t queue_no)
{
	return vtss_ll_frame_rx_discard(queue_no);
}

/* - TX frames ----------------------------------------------------- */

vtss_rc vtss_cpu_tx_raw_frame(const vtss_poag_no_t poag_no,
                              const uchar * const  frame,
                              const uint           length)
{
	vtss_port_no_t         port_no;
	
	if (VTSS_PORT_IS_PORT(poag_no)) {
		port_no = poag_no;
		if (vtss_mac_state.stp_state[port_no]==VTSS_STP_STATE_DISABLED) {
			vtss_log(VTSS_LOG_ERR,
				 "SWITCH: port_no STP disabled, port=%d", port_no);
			return VTSS_UNSPECIFIED_ERROR;
		}
	} else {
		for (port_no = VTSS_PORT_NO_START;
		     port_no < VTSS_PORT_NO_END; port_no++) {
			if (vtss_mac_state.port_poag_no[port_no] == poag_no && 
				VTSS_STP_FORWARDING(vtss_mac_state.stp_state[port_no])) {
				break;
			}
		}
		if (port_no == VTSS_PORT_NO_END) {
			vtss_log(VTSS_LOG_ERR,
				 "SWITCH: no ports up for poag, poag=%d", poag_no);
			return VTSS_UNSPECIFIED_ERROR;
		}
	}
	
	
	/* If egress mirroring is enabled, send on mirror port */
	if (vtss_mac_state.mirror_egress[port_no] && port_no != vtss_mac_state.mirror_port && 
		vtss_mac_state.mirror_port != 0 && VTSS_STP_FORWARDING(vtss_mac_state.stp_state[vtss_mac_state.mirror_port])) {
		vtss_rc rc;
		
		if ((rc = vtss_ll_frame_tx(vtss_mac_state.mirror_port, frame, length, VTSS_VID_NULL)) < 0)
			return rc;
	}
	
	return vtss_ll_frame_tx(port_no, frame, length, VTSS_VID_NULL);
}

/* Transmit frame filtered on VLAN or aggregation */
static vtss_rc vtss_cpu_tx_frame(const vtss_poag_no_t poag_no,
                                 const vtss_vid_t     vid,
                                 const uchar * const  frame,
                                 const uint           length,
                                 BOOL                 vlan)
{
	vtss_rc                rc;
	vtss_port_no_t         port_no;
	vtss_pgid_entry_t      *pgid_entry;
	vtss_mac_table_entry_t entry;
	vtss_pgid_no_t         pgid_no;
	int                    i;
	BOOL                   mirror, mirror_done;
	vtss_cpu_frame_info_t  info;
	vtss_cpu_filter_t      filter;
	
	pgid_entry = NULL;
	if (vlan) {
		/* Lookup (VID, DMAC) */
		entry.vid_mac.vid = vid;
		for (i = 0; i < sizeof(entry.vid_mac.mac); i++)
			entry.vid_mac.mac.addr[i] = frame[i];
		rc = vtss_ll_mac_table_lookup(&entry, &pgid_no);
		if (rc >= 0) {
			pgid_entry = &vtss_mac_state.pgid_table[pgid_no];
		} else if (rc != VTSS_ENTRY_NOT_FOUND) {
			return rc;
		}
	}
	
	mirror = 0;
	mirror_done = 0;
	info.port_no = 0;
	info.vid = vid;
	/* Transmit on ports */
	for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
		/* Aggregation member check */
		if (!vlan && vtss_mac_state.port_poag_no[port_no] != poag_no) {
			vtss_log(VTSS_LOG_DEBUG,
				 "SWITCH: port not member of poag, port=%d, poag=%d",
				 port_no, poag_no);
			continue;
		}
		
		/* Destination port check */
		if (vlan && pgid_entry != NULL && !pgid_entry->member[port_no]) {
			BOOL           found_agg_member = 0;
			vtss_port_no_t i_port_no;
			vtss_poag_no_t poag;
			
			/* If the port is aggregated and any of the member ports are included in 
			the destination member set, the port is considered a destination member */
			if ((poag = vtss_mac_state.port_poag_no[port_no]) != port_no) {
				for (i_port_no = VTSS_PORT_NO_START; i_port_no < VTSS_PORT_NO_END; 
				i_port_no++) {    
					if (vtss_mac_state.port_poag_no[i_port_no] == poag && 
						pgid_entry->member[i_port_no]) {
						found_agg_member = 1;
					}
				}
			}
			if (!found_agg_member) {
				vtss_log(VTSS_LOG_DEBUG,
					 "SWITCH: port not a destination member, port=%d",
					 port_no);
				continue;
			}
		}
		
		/* Egress filtering */
		info.port_tx = port_no;
		if (vtss_cpu_filter(&info, &filter) != VTSS_OK || filter == VTSS_CPU_FILTER_DISCARD)
			continue;
		
		/* All checks successful, transmit on port */
		vtss_log(VTSS_LOG_DEBUG,
			 "SWITCH: Tx on port, port=%d", port_no);
		if ((rc = vtss_ll_frame_tx(port_no, frame, length,
			filter == VTSS_CPU_FILTER_TAGGED ? vid : VTSS_VID_NULL)) < 0)
			return rc;
		
		/* Check if egress mirroring must be done */
		if (vtss_mac_state.mirror_egress[port_no])
			mirror = 1;
		
		/* Check if egress mirroring has been done */
		if (port_no == vtss_mac_state.mirror_port)
			mirror_done = 1;
		
	}
	
	port_no = vtss_mac_state.mirror_port;
	if (mirror && !mirror_done && port_no != 0 &&
		VTSS_STP_FORWARDING(vtss_mac_state.stp_state[port_no])) {
		if ((rc = vtss_ll_frame_tx(port_no, frame, length,
			vtss_mac_state.vlan_port_table[port_no].untagged_vid != VTSS_VID_ALL &&
			vtss_mac_state.vlan_port_table[port_no].untagged_vid != vid ? 
			vid : VTSS_VID_NULL)) < 0)
		return rc;
	}
	return VTSS_OK;
}

vtss_rc vtss_cpu_tx_poag_frame(const vtss_poag_no_t poag_no,
                               const vtss_vid_t     vid,
                               const uchar * const  frame,
                               const uint           length)
{
	return vtss_cpu_tx_frame(poag_no, vid, frame, length, 0);
}

vtss_rc vtss_cpu_tx_vlan_frame(const vtss_vid_t    vid,
                               const uchar * const frame,
                               const uint          length)
{
	return vtss_cpu_tx_frame(0, vid, frame, length, 1);
}

vtss_rc vtss_cpu_filter(const vtss_cpu_frame_info_t * const info,
                        vtss_cpu_filter_t * const           filter)
{
	vtss_port_no_t    port_rx, port_tx;
	vtss_vlan_entry_t *vlan_entry;
	vtss_vid_t        uvid;
	
	/* Discard by default */
	*filter = VTSS_CPU_FILTER_DISCARD; 
	port_rx = info->port_no;
	port_tx = info->port_tx;
	vlan_entry = (info->vid == VTSS_VID_NULL ? NULL : &vtss_mac_state.vlan_table[info->vid]);
	
	if (port_rx) {
		if (port_tx && vtss_mac_state.port_poag_no[port_rx] == vtss_mac_state.port_poag_no[port_tx]) {
			/* Ingress LLAG filter */
			vtss_log(VTSS_LOG_DEBUG,
				 "SWITCH: rx/tx port are members of same LLAG %d, rx=%d, tx=%d",
				 vtss_mac_state.port_poag_no[port_tx], port_rx, port_tx);
			return VTSS_OK;
		}
		
		if (!VTSS_STP_FORWARDING(vtss_mac_state.stp_state[port_rx])) {
			/* Ingress STP filtering */
			vtss_log(VTSS_LOG_DEBUG,
				 "SWITCH: port is not STP forwarding, port_rx=%d", port_rx);
			return VTSS_OK;
		}
		
		if (vtss_mac_state.auth_state[port_rx] == VTSS_AUTH_STATE_NONE) {
			/* Ingress authentication filter */
			vtss_log(VTSS_LOG_DEBUG,
				 "SWITCH: port not authenticated, port_rx=%d",
				 port_rx);
			return VTSS_OK;
		}
		
		if (vlan_entry != NULL && !vlan_entry->member[port_rx] && 
			vtss_mac_state.vlan_port_table[port_rx].ingress_filter) {
			/* VLAN ingress filtering */
			vtss_log(VTSS_LOG_DEBUG,
				 "SWITCH: port not member of VLAN, port_rx=%d, vlan=%d",
				 port_rx, info->vid);
			return VTSS_OK;
		}
	}
	
	if (port_tx) {
		if (vlan_entry != NULL) {
			if (!vlan_entry->member[port_tx]) {
				/* Egress VLAN filter */
				vtss_log(VTSS_LOG_DEBUG,
					 "SWITCH: port not member of VLAN, port_tx=%d, vlan=%d",
					 port_tx, info->vid);
				return VTSS_OK;
			}
			
			if (vtss_mac_state.mstp_table[vlan_entry->msti].state[port_tx] != VTSS_MSTP_STATE_FORWARDING) {
				/* Egress MSTP filter */
				vtss_log(VTSS_LOG_DEBUG,
					 "SWITCH: port not MSTP forwarding, port_tx=%d, vlan=%d",
					 port_tx, info->vid);
				return VTSS_OK;
			}
		}
		
		if (!vtss_mac_state.aggr_member[port_tx]) {
			/* Egress LAG/STP check */
			vtss_log(VTSS_LOG_DEBUG,
				 "SWITCH: port not LAG/STP forwarding, port_tx=%d",
				 port_tx);
			return VTSS_OK;
		}
		
		/* Determine whether to send tagged or untagged */
		uvid = vtss_mac_state.vlan_port_table[port_tx].untagged_vid;
		*filter = (uvid != VTSS_VID_ALL && uvid != info->vid ? 
VTSS_CPU_FILTER_TAGGED : VTSS_CPU_FILTER_UNTAGGED);
	} else {
		/* No egress filtering */
		*filter = VTSS_CPU_FILTER_UNTAGGED;
	}
	return VTSS_OK;
}

vtss_rc vtss_cpu_start(void)
{
	vtss_cpu_rx_registration_t reg;
	/* Initialize CPU frame registration, enable BPDUs */
	memset(&reg, 0, sizeof(reg));
	reg.bpdu_cpu_only = 1;
	VTSS_RC(vtss_cpu_rx_registration_set(&reg));
	return VTSS_OK;
}

#endif
