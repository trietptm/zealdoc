#include "vtss_priv.h"

/* - Access Control Lists ------------------------------------------ */

#ifdef CONFIG_VTSS_ACL

#ifdef VTSS_FEATURE_ACL
vtss_acl_state_t vtss_acl_state;      /* Short hand for above */

vtss_rc vtss_acl_policer_rate_set(const vtss_acl_policer_no_t policer_no,
                                  const vtss_packet_rate_t    rate)
{
	return vtss_ll_acl_policer_rate_set(policer_no, rate);
}

vtss_rc vtss_acl_port_action_set(const vtss_port_no_t            port_no,
                                 const vtss_acl_action_t * const action)
{
	return vtss_ll_acl_port_action_set(port_no, action);
}

vtss_rc vtss_acl_port_counter_get(const vtss_port_no_t            port_no,
                                  vtss_acl_port_counter_t * const counter)
{
	return vtss_ll_acl_port_counter_get(port_no, counter);
}

vtss_rc vtss_acl_port_counter_clear(const vtss_port_no_t port_no)
{
	return vtss_ll_acl_port_counter_clear(port_no);
}

vtss_rc vtss_acl_policy_no_set(const vtss_port_no_t       port_no,
                               const vtss_acl_policy_no_t policy_no)
{
	return vtss_ll_acl_policy_no_set(port_no, policy_no);
}

vtss_rc vtss_ace_init(const vtss_ace_type_t type,
                      vtss_ace_t * const    ace)
{
	memset(ace, 0, sizeof(*ace));
	ace->type = type;
	ace->action.forward = 1;
	ace->action.learn = 1;
	ace->dmac_bc = VTSS_ACE_BIT_ANY;
	ace->dmac_mc = VTSS_ACE_BIT_ANY;
	ace->vlan.cfi = VTSS_ACE_BIT_ANY;
	
	switch (type) {
	case VTSS_ACE_TYPE_ANY:
	case VTSS_ACE_TYPE_ETYPE:
	case VTSS_ACE_TYPE_LLC:
	case VTSS_ACE_TYPE_SNAP:
	case VTSS_ACE_TYPE_IPV6:
		break;
	case VTSS_ACE_TYPE_ARP:
		ace->frame.arp.arp = VTSS_ACE_BIT_ANY;
		ace->frame.arp.req = VTSS_ACE_BIT_ANY;
		ace->frame.arp.unknown = VTSS_ACE_BIT_ANY;
		ace->frame.arp.smac_match = VTSS_ACE_BIT_ANY;
		ace->frame.arp.dmac_match = VTSS_ACE_BIT_ANY;
		ace->frame.arp.length = VTSS_ACE_BIT_ANY;
		ace->frame.arp.ip = VTSS_ACE_BIT_ANY;
		ace->frame.arp.ethernet = VTSS_ACE_BIT_ANY;
		break;
	case VTSS_ACE_TYPE_IPV4:
		ace->frame.ipv4.ttl = VTSS_ACE_BIT_ANY;
		ace->frame.ipv4.fragment = VTSS_ACE_BIT_ANY;
		ace->frame.ipv4.options = VTSS_ACE_BIT_ANY;
		ace->frame.ipv4.sport.high = 0xffff;
		ace->frame.ipv4.sport.in_range = 1;
		ace->frame.ipv4.dport.high = 0xffff;
		ace->frame.ipv4.dport.in_range = 1;
		ace->frame.ipv4.tcp_fin = VTSS_ACE_BIT_ANY;
		ace->frame.ipv4.tcp_syn = VTSS_ACE_BIT_ANY;
		ace->frame.ipv4.tcp_rst = VTSS_ACE_BIT_ANY;
		ace->frame.ipv4.tcp_psh = VTSS_ACE_BIT_ANY;
		ace->frame.ipv4.tcp_ack = VTSS_ACE_BIT_ANY;
		ace->frame.ipv4.tcp_urg = VTSS_ACE_BIT_ANY;
		break;
	default:
		vtss_log(VTSS_LOG_ERR,
			 "SWITCH: unknown type, type=%d", type);
		return VTSS_INVALID_PARAMETER;
	}
	
	return VTSS_OK;
}

vtss_rc vtss_ace_add(const vtss_ace_id_t      ace_id,
                     const vtss_ace_t * const ace)
{
	return vtss_ll_ace_add(ace_id, ace);
}

vtss_rc vtss_ace_del(const vtss_ace_id_t ace_id)
{
	return vtss_ll_ace_del(ace_id);
}

vtss_rc vtss_ace_counter_get(const vtss_ace_id_t        ace_id,
                             vtss_ace_counter_t * const counter)
{
	return vtss_ll_ace_counter_get(ace_id, counter);
}

vtss_rc vtss_ace_counter_clear(const vtss_ace_id_t ace_id)
{
	return vtss_ll_ace_counter_clear(ace_id);
}


/* Apply ACL command */
static vtss_rc ht_acl_cmd(uint cmd, uint idx)
{
	ulong busy;
	
	HT_WR(ACL, S_ACL, UPDATE_CTRL, 
	      (0<<20) | 
	      (1<<17) |
	      ((idx < VTSS_ACES ? 1 : 0)<<16) | 
	      (idx<<4) | 
	      (cmd<<1) | 
	      (1<<0));
	
	/* Wait until idle */
	while (1) {
		HT_RDF(ACL, S_ACL, UPDATE_CTRL, 0, 0x1, &busy);
		if (!busy)
			break;
	}
	return VTSS_OK;
}

#define VTSS_ACL_DUMP 1

#if VTSS_ACL_DUMP
static char *ht_acl_bits(char *p, const char *txt, ulong *data, int offset, int len)
{
	int   i, count = 0;
	ulong bit_mask;
	
	if (txt == NULL)
		p += sprintf(p, ".");
	else
		p += sprintf(p, " %s: ", txt);
	for (i = (len - 1); i >= 0; i--) {
		if (count && (count % 8) == 0) {
			*p = '.';
			p++;
		}
		count++;
		
		bit_mask = (1<<(offset+i));
		*p = ((data[1] & bit_mask) ? 'X' : (data[0] & bit_mask) ? '1' : '0'); 
		p++;
	}
	
	*p = 0;
	return p;
}

static vtss_rc ht_acl_dump(void)
{
	int i;
	vtss_acl_entry_t *cur = NULL;
	ulong valid, redir, cnt, mask, status;
	char buf[128], *p;
	
	for (i = (VTSS_ACES + VTSS_CHIP_PORTS); i >= 0; i--) {
		/* Read entry */
		VTSS_RC(ht_acl_cmd(ACL_CMD_READ, i));
		
		HT_RD(ACL, S_ACL, STATUS, &status);
		
		if (i == (VTSS_ACES + VTSS_CHIP_PORTS))
			sprintf(buf, "%3d (egress) ", i);
		else if (i < VTSS_ACES) {
			cur = (i == (VTSS_ACES - 1) ? vtss_acl_state.acl_list : 
		cur != NULL ? cur->next : NULL);
		sprintf(buf, "%3d (ace %d, id 0x%08lx)", 
			i, VTSS_ACES - i - 1, cur == NULL ? 0 : cur->ace.id);
		} else
			sprintf(buf, "%3d (port %2d)", i, i - VTSS_ACES);
		if (status & (1<<4)) {
			/* Egress action */
			HT_RD(ACL, S_ACL, EG_VLD, &valid);
			HT_RD(ACL, S_ACL, EG_PORT_MASK, &mask);
			HT_RD(ACL, S_ACL, EG_CNT, &cnt);
			vtss_log(VTSS_LOG_DEBUG,
				 "SPARX: %s, %s, mask=0x%08lx, cnt=%ld",
				 buf, valid ? "V" : "v", mask, cnt);
		} else {
			ulong misc;

			/* Ingress action */
			HT_RD(ACL, S_ACL, IN_VLD, &valid);
			HT_RD(ACL, S_ACL, IN_MISC_CFG, &misc);
			HT_RD(ACL, S_ACL, IN_REDIR_CFG, &redir);
			HT_RD(ACL, S_ACL, IN_CNT, &cnt);
			vtss_log(VTSS_LOG_DEBUG,
				 "SPARX: %s %s %s %s (%ld) %s (%ld) %s %s, %s (%ld) %ld", 
				 buf, 
				 valid ? "V" : "v", 
				 misc & (1<<20) ? "H" : "h",
				 misc & (1<<12) ? "C" : "c",
				 (misc>>16) & 0x3,
				 misc & (1<<3) ? "P" : "p",
				 (misc>>4) & 0xf,
				 misc & (1<<1) ? "L" : "l",
				 misc & (1<<0) ? "F" : "f",
				 redir & (1<<0) ? "R" : "r",
				 (redir>>4) & 0x3f,
				 cnt);
		}
		
		/* Frame info */
		if (i < VTSS_ACES) {
			int   j;
			ulong type[2], offset;
			ulong misc[2], smac_hi[2], smac_lo[2], dmac_hi[2], dmac_lo[2];
			ulong etype[2], llc[2], snap_hi[2], snap_lo[2], l3_arp[2], l3_sip[8], l3_dip[2];
			ulong l3_misc[2], l4_port[2], l4_misc[2], l3_ip_hi[2], l3_ip_lo[2];
			
			/* Type value and mask (mask can not be read back - always zero) */
			type[0] = (status & 0x7);
			HT_RDF(ACL, S_ACL, UPDATE_CTRL, 20, 0x7, &type[1]);
			
			/* Read data and mask */
			for (j = 0; j < 2; j++) {
				offset = (j == 0 ? ACE_DATA_OFFS : ACE_MASK_OFFS);
				switch (type[0]) {
				case ACL_TYPE_ETYPE:
					strcpy(buf, "ETYPE/ANY");
					HT_RD(ACL, S_ACL, ETYPE_TYPE + offset, &misc[j]);
					HT_RD(ACL, S_ACL, ETYPE_L2_SMAC_HIGH + offset, &smac_hi[j]);
					HT_RD(ACL, S_ACL, ETYPE_L2_SMAC_LOW + offset, &smac_lo[j]);
					HT_RD(ACL, S_ACL, ETYPE_L2_DMAC_HIGH + offset, &dmac_hi[j]);
					HT_RD(ACL, S_ACL, ETYPE_L2_DMAC_LOW + offset, &dmac_lo[j]);
					HT_RD(ACL, S_ACL, ETYPE_L2_ETYPE + offset, &etype[j]);
					break;
				case ACL_TYPE_LLC:
					strcpy(buf, "LLC");
					HT_RD(ACL, S_ACL, LLC_TYPE + offset, &misc[j]);
					HT_RD(ACL, S_ACL, LLC_L2_SMAC_HIGH + offset, &smac_hi[j]);
					HT_RD(ACL, S_ACL, LLC_L2_SMAC_LOW + offset, &smac_lo[j]);
					HT_RD(ACL, S_ACL, LLC_L2_DMAC_HIGH + offset, &dmac_hi[j]);
					HT_RD(ACL, S_ACL, LLC_L2_DMAC_LOW + offset, &dmac_lo[j]);
					HT_RD(ACL, S_ACL, LLC_L2_LLC + offset, &llc[j]);
					break;
				case ACL_TYPE_SNAP:
					strcpy(buf, "SNAP");
					HT_RD(ACL, S_ACL, SNAP_TYPE + offset, &misc[j]);
					HT_RD(ACL, S_ACL, SNAP_L2_SMAC_HIGH + offset, &smac_hi[j]);
					HT_RD(ACL, S_ACL, SNAP_L2_SMAC_LOW + offset, &smac_lo[j]);
					HT_RD(ACL, S_ACL, SNAP_L2_DMAC_HIGH + offset, &dmac_hi[j]);
					HT_RD(ACL, S_ACL, SNAP_L2_DMAC_LOW + offset, &dmac_lo[j]);
					HT_RD(ACL, S_ACL, SNAP_L2_SNAP_HIGH + offset, &snap_hi[j]);
					HT_RD(ACL, S_ACL, SNAP_L2_SNAP_LOW + offset, &snap_lo[j]);
					break;
				case ACL_TYPE_ARP:
					strcpy(buf, "ARP");
					HT_RD(ACL, S_ACL, ARP_TYPE + offset, &misc[j]);
					HT_RD(ACL, S_ACL, ARP_L2_SMAC_HIGH + offset, &smac_hi[j]);
					HT_RD(ACL, S_ACL, ARP_L2_SMAC_LOW + offset, &smac_lo[j]);
					HT_RD(ACL, S_ACL, ARP_L3_ARP + offset, &l3_arp[j]);
					HT_RD(ACL, S_ACL, ARP_L3_IPV4_DIP + offset, &l3_dip[j]);
					HT_RD(ACL, S_ACL, ARP_L3_IPV4_SIP + offset, &l3_sip[j]);
					break;
				case ACL_TYPE_UDP_TCP:
					strcpy(buf, "UDP/TCP");
					HT_RD(ACL, S_ACL, UDP_TCP_TYPE + offset, &misc[j]);
					HT_RD(ACL, S_ACL, UDP_TCP_L3_MISC_CFG + offset, &l3_misc[j]);
					HT_RD(ACL, S_ACL, UDP_TCP_L4_PORT + offset, &l4_port[j]);
					HT_RD(ACL, S_ACL, UDP_TCP_L4_MISC + offset, &l4_misc[j]);
					HT_RD(ACL, S_ACL, UDP_TCP_L3_IPV4_DIP + offset, &l3_dip[j]);
					HT_RD(ACL, S_ACL, UDP_TCP_L3_IPV4_SIP + offset, &l3_sip[j]);
					break;
				case ACL_TYPE_IPV4:
					strcpy(buf, "IPv4");
					HT_RD(ACL, S_ACL, IPV4_TYPE + offset, &misc[j]);
					HT_RD(ACL, S_ACL, IPV4_L3_MISC_CFG + offset, &l3_misc[j]);
					HT_RD(ACL, S_ACL, IPV4_DATA_0 + offset, &l3_ip_lo[j]);
					HT_RD(ACL, S_ACL, IPV4_DATA_1 + offset, &l3_ip_hi[j]);
					HT_RD(ACL, S_ACL, IPV4_L3_IPV4_DIP + offset, &l3_dip[j]);
					HT_RD(ACL, S_ACL, IPV4_L3_IPV4_SIP + offset, &l3_sip[j]);
					break;
				case ACL_TYPE_IPV6:
					strcpy(buf, "IPv6");
					HT_RD(ACL, S_ACL, IPV6_TYPE + offset, &misc[j]);
					HT_RD(ACL, S_ACL, IPV6_L3_MISC_CFG + offset, &l3_misc[j]);
					HT_RD(ACL, S_ACL, IPV6_L3_IPV6_SIP_0 + offset, &l3_sip[0+j]);
					HT_RD(ACL, S_ACL, IPV6_L3_IPV6_SIP_1 + offset, &l3_sip[2+j]);
					HT_RD(ACL, S_ACL, IPV6_L3_IPV6_SIP_2 + offset, &l3_sip[4+j]);
					HT_RD(ACL, S_ACL, IPV6_L3_IPV6_SIP_3 + offset, &l3_sip[6+j]);
					break;
				default:
					vtss_log(VTSS_LOG_ERR,
						 "SPARX: illegal type, type=%ld",
						 type[0]);
					misc[0] = 0; /* Treat as invalid entry */
					misc[1] = 0;
					break;
				}
			}
			
			/* Skip invalid entries */
			if ((misc[0] & (1<<0)) == 0)
				continue;
			
			/* Type */
			p = &buf[strlen(buf)];
			p = ht_acl_bits(p, "type", type, 0, 3);
			vtss_log(VTSS_LOG_DEBUG, buf);
			
			/* Common fields */
			p = &buf[0];
			p = ht_acl_bits(p, "bc", misc, 29, 1);
			p = ht_acl_bits(p, "mc", misc, 28, 1);
			p = ht_acl_bits(p, "vid", misc, 12, 12);
			p = ht_acl_bits(p, "uprio", misc, 24, 3);
			p = ht_acl_bits(p, "cfi", misc, 2, 1);
			p = ht_acl_bits(p, "pag", misc, 4, ACL_PAG_WIDTH);
			p = ht_acl_bits(p, "igr", misc, 1, 1);
			p = ht_acl_bits(p, "vld", misc, 0, 1);
			vtss_log(VTSS_LOG_DEBUG, buf);
			
			if (type[0] <= ACL_TYPE_ARP) {
				p = &buf[0];
				p = ht_acl_bits(p, "smac", smac_hi, 0, 16);
				p = ht_acl_bits(p, NULL, smac_lo, 0, 32);
				vtss_log(VTSS_LOG_DEBUG, buf);
			}
			if (type[0] <= ACL_TYPE_SNAP) {
				p = &buf[0];
				p = ht_acl_bits(p, "dmac", dmac_hi, 0, 16);
				p = ht_acl_bits(p, NULL, dmac_lo, 0, 32);
				vtss_log(VTSS_LOG_DEBUG, buf);
			}
			if (type[0] == ACL_TYPE_ETYPE) {
				p = &buf[0];
				p = ht_acl_bits(p, "etype", etype, 0, 16);
				p = ht_acl_bits(p, "data", etype, 16, 16);
				vtss_log(VTSS_LOG_DEBUG, buf);
			}
			if (type[0] == ACL_TYPE_LLC) {
				p = &buf[0];
				p = ht_acl_bits(p, "llc", etype, 0, 32);
				vtss_log(VTSS_LOG_DEBUG, buf);
			}
			if (type[0] == ACL_TYPE_SNAP) {
				p = &buf[0];
				p = ht_acl_bits(p, "snap", snap_hi, 0, 8);
				p = ht_acl_bits(p, NULL, snap_hi, 0, 32);
				vtss_log(VTSS_LOG_DEBUG, buf);
			}
			if (type[0] == ACL_TYPE_ARP) {
				p = &buf[0];
				p = ht_acl_bits(p, "arp", l3_arp, 1, 1);
				p = ht_acl_bits(p, "req", l3_arp, 0, 1);
				p = ht_acl_bits(p, "unkn", l3_arp, 2, 1);
				p = ht_acl_bits(p, "s_match", l3_arp, 3, 1);
				p = ht_acl_bits(p, "d_match", l3_arp, 4, 1);
				p = ht_acl_bits(p, "len", l3_arp, 5, 1);
				p = ht_acl_bits(p, "ip", l3_arp, 6, 1);
				p = ht_acl_bits(p, "eth", l3_arp, 7, 1);
				vtss_log(VTSS_LOG_DEBUG, buf);
			}
			if (type[0] == ACL_TYPE_UDP_TCP || type[0] == ACL_TYPE_IPV4) {
				p = &buf[0];
				p = ht_acl_bits(p, "proto", l3_misc, 12, 8);
				p = ht_acl_bits(p, "ds", l3_misc, 4, 8);
				p = ht_acl_bits(p, "options", l3_misc, 2, 1);
				p = ht_acl_bits(p, "fragment", l3_misc, 1, 1);
				p = ht_acl_bits(p, "ttl", l3_misc, 0, 1);
				vtss_log(VTSS_LOG_DEBUG, buf);
			}
			if (type[0] >= ACL_TYPE_ARP && type[0] <= ACL_TYPE_IPV4) {
				p = &buf[0];
				p = ht_acl_bits(p, "sip", l3_sip, 0, 32);
				vtss_log(VTSS_LOG_DEBUG, buf);
				p = &buf[0];
				p = ht_acl_bits(p, "dip", l3_dip, 0, 32);
				vtss_log(VTSS_LOG_DEBUG, buf);
			}
			if (type[0] == ACL_TYPE_UDP_TCP) {
				p = &buf[0];
				p = ht_acl_bits(p, "sport", l4_port, 16, 16);
				p = ht_acl_bits(p, "dport", l4_port, 0, 16);
				vtss_log(VTSS_LOG_DEBUG, buf);
				p = &buf[0];
				p = ht_acl_bits(p, "fin", l4_misc, 13, 1);
				p = ht_acl_bits(p, "syn", l4_misc, 12, 1);
				p = ht_acl_bits(p, "rst", l4_misc, 11, 1);
				p = ht_acl_bits(p, "psh", l4_misc, 10, 1);
				p = ht_acl_bits(p, "ack", l4_misc, 9, 1);
				p = ht_acl_bits(p, "urg", l4_misc, 8, 1);
				p = ht_acl_bits(p, "range", l4_misc, 0, 8);
				vtss_log(VTSS_LOG_DEBUG, buf);
			}
			if (type[0] == ACL_TYPE_IPV4) {
				p = &buf[0];
				p = ht_acl_bits(p, "data", l3_ip_hi, 0, 16);
				p = ht_acl_bits(p, NULL, l3_ip_lo, 0, 32);
				vtss_log(VTSS_LOG_DEBUG, buf);
			}
			if (type[0] == ACL_TYPE_IPV6) {
				p = &buf[0];
				ht_acl_bits(p, "sip", &l3_sip[6], 0, 32);
				vtss_log(VTSS_LOG_DEBUG, buf);
				ht_acl_bits(p, "sip", &l3_sip[4], 0, 32);
				vtss_log(VTSS_LOG_DEBUG, buf);
				ht_acl_bits(p, "sip", &l3_sip[2], 0, 32);
				vtss_log(VTSS_LOG_DEBUG, buf);
				ht_acl_bits(p, "sip", &l3_sip[0], 0, 32);
				vtss_log(VTSS_LOG_DEBUG, buf);
			}
		}
	}
	return VTSS_OK;
}
#endif /* VTSS_ACL_DUMP */

/* Setup action */
static vtss_rc ht_acl_action_set(const vtss_acl_action_t * action, ulong counter)
{
	HT_WR(ACL, S_ACL, IN_VLD, 1<<0);
	HT_WR(ACL, S_ACL, IN_MISC_CFG,
	      ((action->cpu_once ? 1 : 0)<<20) |
	      (((action->cpu || action->cpu_once) ? 
	      (action->cpu_queue - VTSS_CPU_RX_QUEUE_START) : 0)<<16) | 
	      ((action->cpu ? 1 : 0)<<12) |
	      ((action->police ? (action->policer_no - VTSS_ACL_POLICER_NO_START) : 0)<<4) |
	      ((action->police ? 1 : 0)<<3) |
	      ((action->learn ? 1 : 0)<<1) |
	      ((action->forward ? 1 : 0)<<0));
	HT_WR(ACL, S_ACL, IN_REDIR_CFG,
	      ((action->port_forward ? vtss_pgid2chip(action->port_no) : 0)<<4) |
	      ((action->port_forward ? 1 : 0)<<0));
	HT_WR(ACL, S_ACL, IN_CNT, counter);
	return VTSS_OK;
}

/* Allocate UDP/TCP range checker */
static uchar ht_acl_range_alloc(const vtss_ace_udp_tcp_t *port, BOOL sport, BOOL alloc)
{
	int i, new = 0xff;
	vtss_acl_range_t *range;
	
	if (port->in_range && 
		/* Exact match or wildcard - no range checker required */
		(port->low == port->high || (port->low == 0 && port->high == 0xffff))) {
		return 0x00;
	}
	
	for (i = 0; i < VTSS_ACL_RANGES; i++) {
		range = &vtss_acl_state.acl_range[i];
		
		/* Free range found */
		if (new == 0xff && range->count == 0)
			new = i;
		
		/* Matching entry found */
		if (range->count != 0 && range->sport == sport && 
			range->port.low == port->low && range->port.high == port->high) {
			new = i;
			break;
		}
	}
	
	/* No matching or free entry found */
	if (new == 0xff) {
		return new;
	}
	
	range = &vtss_acl_state.acl_range[new];
	range->port = *port;
	range->sport = sport;
	range->count++;
	if (range->count == 1 && alloc) {
		/* Update range checker in chip */
		HT_WR(ACL, S_ACL, TCP_RNG_ENA_CFG_0 + 2*new, 
		      ((sport ? 1 : 0)<<1) | ((sport ? 0 : 1)<<0));
		HT_WR(ACL, S_ACL, TCP_RNG_VALUE_CFG_0 + 2*new, (port->high<<16) | port->low);
	}
	vtss_log(VTSS_LOG_DEBUG,
		 "SPARX: %s range %d for %s, hi/lo=%d/%d, in_range=%d", 
		range->count == 1 ? "new" : "reusing", 
		new, sport ? "sport" : "dport",
		port->high, port->low, port->in_range);
	return (1<<new);
}

/* Free all range checkers */
static void ht_acl_range_free(void)
{
	int i;
	
	for (i = 0; i < VTSS_ACL_RANGES; i++) {
		vtss_acl_state.acl_range[i].count = 0;
	}
}

vtss_rc vtss_ll_acl_policer_rate_set(vtss_acl_policer_no_t policer_no,
                                     vtss_packet_rate_t    rate)
{
	ulong unit, policer;
	
	policer = (policer_no - VTSS_ACL_POLICER_NO_START);
	HT_WR(ANALYZER, 0, ACLPOLIDX, policer);
	HT_WRF(ANALYZER, 0, STORMLIMIT, 24, 0xf, calc_packet_rate(rate, &unit));
	HT_WRF(ANALYZER, 0, STORMLIMIT_ENA, 6, 0x1, 1);
#ifdef CONFIG_VTSS_GROCX
	policer = 0; /* For G-RocX, the same field is used for all policers */
#endif
	HT_WRF(ANALYZER, 0, STORMPOLUNIT, 6 + policer, 0x1, unit);
	return VTSS_OK;
}

static vtss_rc ht_port_acl_action_set(uint port_on_chip, const vtss_acl_action_t * action)
{
	ulong counter;
	
	VTSS_RC(ht_acl_cmd(ACL_CMD_READ, VTSS_ACES + port_on_chip));
	HT_RD(ACL, S_ACL, IN_CNT, &counter);
	VTSS_RC(ht_acl_action_set(action, counter));
	VTSS_RC(ht_acl_cmd(ACL_CMD_WRITE, VTSS_ACES + port_on_chip));
	return VTSS_OK;
}

vtss_rc vtss_ll_acl_port_action_set(vtss_port_no_t            port_no,
                                    const vtss_acl_action_t * action)
{
	uint  port_on_chip = HT_CHIP_PORT(port_no);
	
	VTSS_RC(ht_port_acl_action_set(port_on_chip, action));
	return VTSS_OK;
}

static vtss_rc ht_acl_port_counter_get(uint port_on_chip, ulong *counter)
{
	VTSS_RC(ht_acl_cmd(ACL_CMD_READ, VTSS_ACES + port_on_chip));
	HT_RD(ACL, S_ACL, IN_CNT, counter);
	return VTSS_OK;
}

vtss_rc vtss_ll_acl_port_counter_get(const vtss_port_no_t            port_no,
                                     vtss_acl_port_counter_t * const counter)
{
	uint port_on_chip = HT_CHIP_PORT(port_no);
	
	VTSS_RC(ht_acl_port_counter_get(port_on_chip, counter));
	return VTSS_OK;
}

static vtss_rc ht_acl_port_counter_clear(uint port_on_chip)
{
	VTSS_RC(ht_acl_cmd(ACL_CMD_READ, VTSS_ACES + port_on_chip));
	HT_WR(ACL, S_ACL, IN_CNT, 0);
	VTSS_RC(ht_acl_cmd(ACL_CMD_WRITE, VTSS_ACES + port_on_chip));
	return VTSS_OK;
}

vtss_rc vtss_ll_acl_port_counter_clear(const vtss_port_no_t port_no)
{
	uint port_on_chip = HT_CHIP_PORT(port_no);
	
	VTSS_RC(ht_acl_port_counter_clear(port_on_chip));
	return VTSS_OK;
}

vtss_rc vtss_ll_acl_policy_no_set(vtss_port_no_t       port_no,
                                  vtss_acl_policy_no_t policy_no)
{
	uint port_on_chip = HT_CHIP_PORT(port_no);
	ulong enabled = (policy_no == VTSS_ACL_POLICY_NO_NONE ? 0 : 1);
	
	/* Enable/disable ACL on port */
	HT_WRF(PORT, port_on_chip, MISCCFG, 2, 0x1, enabled);
	
	if (enabled) {
		HT_WR(ACL, S_ACL, PAG_CFG + port_on_chip, 
		      ((policy_no - VTSS_ACL_POLICY_NO_START)<<(ACL_PAG_WIDTH - 3)) | 
		      (port_on_chip<<0));
	}
	return VTSS_OK;
}

/* Write ACE data and mask */
#define HT_ACE_WR(reg, data, mask) \
{ \
	vtss_rc rc; \
	ulong val = data; \
	ulong msk = ~(mask); \
	if ((rc = ht_rd_wr(B_ACL, S_ACL, R_ACL_##reg + ACE_DATA_OFFS, &val, 1)) < 0) \
		return rc; \
	if ((rc = ht_rd_wr(B_ACL, S_ACL, R_ACL_##reg + ACE_MASK_OFFS, &msk, 1)) < 0) \
		return rc; \
}

/* Convert to 32-bit MAC data/mask register values */
#define HT_ACE_MAC(mac_hi, mac_lo, mac) \
	mac_hi[0] = ((mac.value[0]<<8) | mac.value[1]); \
	mac_hi[1] = ((mac.mask[0]<<8) | mac.mask[1]); \
	mac_lo[0] = ((mac.value[2]<<24) | (mac.value[3]<<16) | (mac.value[4]<<8) | mac.value[5]); \
	mac_lo[1] = ((mac.mask[2]<<24) | (mac.mask[3]<<16) | (mac.mask[4]<<8) | mac.mask[5]);

/* Convert to 32-bit IPv6 data/mask registers */
#define HT_ACE_IPV6(ipv6, addr, i) \
	ipv6[0] = ((addr.value[i+0]<<24) | (addr.value[i+1]<<16) | \
		   (addr.value[i+2]<<8) | addr.value[i+3]); \
	ipv6[1] = ((addr.mask[i+0]<<24) | (addr.mask[i+1]<<16) | \
		   (addr.mask[i+2]<<8) | addr.mask[i+3]);

/* Determine if ACE is UDP/TCP entry */
#define HT_ACE_UDP_TCP(ace) \
	((ace)->type == VTSS_ACE_TYPE_IPV4 && (ace)->frame.ipv4.proto.mask == 0xff && \
	((ace)->frame.ipv4.proto.value == 6 || (ace)->frame.ipv4.proto.value == 17) ? 1 : 0)

static vtss_rc ht_acl_commit(void) 
{
	int i;
	vtss_acl_entry_t *cur;
	vtss_ace_t *ace;
	ulong misc[2], smac_hi[2], smac_lo[2], dmac_hi[2], dmac_lo[2];
	ulong l3_misc[2] = {0, 0}, l3_hi[2], l3_lo[2], ipv6[2];
	BOOL udp_tcp = 0, tcp = 0;
	uint port, policy, type, type_mask;
	vtss_ace_udp_tcp_t *sport, *dport;
	uchar srange, drange;
	
	ht_acl_range_free();
	
	/* Add entries */
	for (cur = vtss_acl_state.acl_list, i = (VTSS_ACES - 1); 
	     cur != NULL && i >= 0; cur = cur->next, i--) {
		ace = &cur->ace;
		
		/* Check rule type and determine PAG fields */
		switch (ace->rule) {
		case VTSS_ACE_RULE_PORT:
			policy = 0;
			port = ace->port_no;
			break;
		case VTSS_ACE_RULE_POLICY:
			policy = ace->policy_no;
			port = 0;
			break;
		case VTSS_ACE_RULE_SWITCH:
			policy = 0;
			port = 0;
			break;
		default:
			vtss_log(VTSS_LOG_ERR,
				 "SPARX: illegal rule, rule=%d", ace->rule);
			return VTSS_INVALID_PARAMETER;
		}
		
		/* Determine frame type and mask */
		type_mask = 0x0;
		switch (ace->type) {
		case VTSS_ACE_TYPE_ANY:
			type = ACL_TYPE_IPV6;
			type_mask = 0x7;
			memset(&ace->frame, 0, sizeof(ace->frame));
			break;
		case VTSS_ACE_TYPE_ETYPE:
			type = ACL_TYPE_ETYPE;
			break;
		case VTSS_ACE_TYPE_LLC:
			type = ACL_TYPE_LLC;
			break;
		case VTSS_ACE_TYPE_SNAP:
			type = ACL_TYPE_SNAP;
			break;
		case VTSS_ACE_TYPE_ARP:
			type = ACL_TYPE_ARP;
			break;
		case VTSS_ACE_TYPE_IPV4:
			udp_tcp = HT_ACE_UDP_TCP(ace);
			tcp = (udp_tcp && ace->frame.ipv4.proto.value == 6);
			if (udp_tcp) { 
				/* UDP/TCP */
				type = ACL_TYPE_UDP_TCP;
			} else {
				/* IPv4 */
				type = ACL_TYPE_IPV4;
				type_mask = 0x1;
			}
			l3_misc[0] = ((ace->frame.ipv4.proto.value<<12) | 
				      (ace->frame.ipv4.ds.value<<4) |
				      ((ace->frame.ipv4.options == VTSS_ACE_BIT_1 ? 1 : 0)<<2) |
				      ((ace->frame.ipv4.fragment == VTSS_ACE_BIT_1 ? 1 : 0)<<1) |
				      ((ace->frame.ipv4.ttl == VTSS_ACE_BIT_1 ? 1 : 0)<<0));
			l3_misc[1] = ((ace->frame.ipv4.proto.mask<<12) | 
				      (ace->frame.ipv4.ds.mask<<4) |
				      ((ace->frame.ipv4.options == VTSS_ACE_BIT_ANY ? 0 : 1)<<2) |
				      ((ace->frame.ipv4.fragment == VTSS_ACE_BIT_ANY ? 0 : 1)<<1) |
				      ((ace->frame.ipv4.ttl == VTSS_ACE_BIT_ANY ? 0 : 1)<<0));
			break;
		case VTSS_ACE_TYPE_IPV6:
			type = ACL_TYPE_IPV6;
			break;
		default:
			vtss_log(VTSS_LOG_ERR,
				 "SPARX: illegal type, type=%d", ace->type);
			return VTSS_INVALID_PARAMETER;
		}
		
		misc[0] = (((ace->dmac_bc == VTSS_ACE_BIT_1 ? 1 : 0)<<29) |
			  ((ace->dmac_mc == VTSS_ACE_BIT_1 ? 1 : 0)<<28) |
			  ((ace->vlan.usr_prio.value & 0x7)<<24) |
			  ((ace->vlan.vid.value & 0xfff)<<12) |
			  ((policy == 0 ? 0 : (policy - VTSS_ACL_POLICY_NO_START))<<(ACL_PAG_WIDTH + 1)) | 
			  ((port == 0 ? 0 : HT_CHIP_PORT(port))<<4) |
			  ((ace->vlan.cfi == VTSS_ACE_BIT_1 ? 1 : 0)<<2) |
			  (1<<1) |
			  (1<<0));
		misc[1] = (((ace->dmac_bc == VTSS_ACE_BIT_ANY ? 0 : 1)<<29) |
			  ((ace->dmac_mc == VTSS_ACE_BIT_ANY ? 0 : 1)<<28) |
			  ((ace->vlan.usr_prio.mask & 0x7)<<24) |
			  ((ace->vlan.vid.mask & 0xfff)<<12) |
			  (((policy == 0 ? 0 : 0x7))<<(ACL_PAG_WIDTH + 1)) | 
			  (((port == 0 ? 0 : ((1<<(ACL_PAG_WIDTH - 3)) - 1)))<<4) |
			  ((ace->vlan.cfi == VTSS_ACE_BIT_ANY ? 0 : 1)<<2) |
			  (1<<1) |
			  (1<<0));
		
		if (type_mask != 0) {
			/* Update type mask before writing data mask */
			HT_WR(ACL, S_ACL, UPDATE_CTRL, 
			      (type_mask<<20) |  (0<<17) |
			      (0<<16) | (0<<4) |  (0<<1) | (0<<0));
		}
		
		switch (type) {
		case ACL_TYPE_ETYPE:
			HT_ACE_WR(ETYPE_TYPE, misc[0], misc[1]);
			HT_ACE_MAC(smac_hi, smac_lo, ace->frame.etype.smac);
			HT_ACE_MAC(dmac_hi, dmac_lo, ace->frame.etype.dmac);
			HT_ACE_WR(ETYPE_L2_SMAC_HIGH, smac_hi[0], smac_hi[1]);
			HT_ACE_WR(ETYPE_L2_SMAC_LOW, smac_lo[0], smac_lo[1]);
			HT_ACE_WR(ETYPE_L2_DMAC_HIGH, dmac_hi[0], dmac_hi[1]);
			HT_ACE_WR(ETYPE_L2_DMAC_LOW, dmac_lo[0], dmac_lo[1]);
			HT_ACE_WR(ETYPE_L2_ETYPE, 
				  (ace->frame.etype.data.value[0]<<24) |
				  (ace->frame.etype.data.value[1]<<16) |
				  (ace->frame.etype.etype.value[0]<<8) |
				  (ace->frame.etype.etype.value[1]),
				  ((ace->frame.etype.data.mask[0]<<24) |
				  (ace->frame.etype.data.mask[1]<<16) |
				  (ace->frame.etype.etype.mask[0]<<8) |
				  (ace->frame.etype.etype.mask[1])));
			break;
		case ACL_TYPE_LLC:
			HT_ACE_WR(LLC_TYPE, misc[0], misc[1]);
			HT_ACE_MAC(smac_hi, smac_lo, ace->frame.llc.smac);
			HT_ACE_MAC(dmac_hi, dmac_lo, ace->frame.llc.dmac);
			HT_ACE_WR(LLC_L2_SMAC_HIGH, smac_hi[0], smac_hi[1]);
			HT_ACE_WR(LLC_L2_SMAC_LOW, smac_lo[0], smac_lo[1]);
			HT_ACE_WR(LLC_L2_DMAC_HIGH, dmac_hi[0], dmac_hi[1]);
			HT_ACE_WR(LLC_L2_DMAC_LOW, dmac_lo[0], dmac_lo[1]);
			HT_ACE_WR(LLC_L2_LLC, 
				  (ace->frame.llc.llc.value[0]<<24) |
				  (ace->frame.llc.llc.value[1]<<16) |
				  (ace->frame.llc.llc.value[2]<<8) |
				  ace->frame.llc.llc.value[3],
				  (ace->frame.llc.llc.mask[0]<<24) |
				  (ace->frame.llc.llc.mask[1]<<16) |
				  (ace->frame.llc.llc.mask[2]<<8) |
				  ace->frame.llc.llc.mask[3]);
			break;
		case ACL_TYPE_SNAP:
			HT_ACE_WR(SNAP_TYPE, misc[0], misc[1]);
			HT_ACE_MAC(smac_hi, smac_lo, ace->frame.snap.smac);
			HT_ACE_MAC(dmac_hi, dmac_lo, ace->frame.snap.dmac);
			HT_ACE_WR(SNAP_L2_SMAC_HIGH, smac_hi[0], smac_hi[1]);
			HT_ACE_WR(SNAP_L2_SMAC_LOW, smac_lo[0], smac_lo[1]);
			HT_ACE_WR(SNAP_L2_DMAC_HIGH, dmac_hi[0], dmac_hi[1]);
			HT_ACE_WR(SNAP_L2_DMAC_LOW, dmac_lo[0], dmac_lo[1]);
			HT_ACE_WR(SNAP_L2_SNAP_HIGH, 
				  ace->frame.snap.snap.value[0], ace->frame.snap.snap.mask[0]);
			HT_ACE_WR(SNAP_L2_SNAP_LOW, 
				  (ace->frame.snap.snap.value[1]<<24) |
				  (ace->frame.snap.snap.value[2]<<16) |
				  (ace->frame.snap.snap.value[3]<<8) |
				  ace->frame.snap.snap.value[4],
				  (ace->frame.snap.snap.mask[1]<<24) |
				  (ace->frame.snap.snap.mask[2]<<16) |
				  (ace->frame.snap.snap.mask[3]<<8) |
				  ace->frame.snap.snap.mask[4]);
			break;
		case ACL_TYPE_ARP:
			HT_ACE_WR(ARP_TYPE, misc[0], misc[1]);
			HT_ACE_MAC(smac_hi, smac_lo, ace->frame.arp.smac);
			HT_ACE_WR(ARP_L2_SMAC_HIGH, smac_hi[0], smac_hi[1]);
			HT_ACE_WR(ARP_L2_SMAC_LOW, smac_lo[0], smac_lo[1]);
			HT_ACE_WR(ARP_L3_ARP, 
				  ((ace->frame.arp.ethernet == VTSS_ACE_BIT_1 ? 1 : 0)<<7) |
				  ((ace->frame.arp.ip == VTSS_ACE_BIT_1 ? 1 : 0)<<6) |
				  ((ace->frame.arp.length == VTSS_ACE_BIT_1 ? 1 : 0)<<5) |
				  ((ace->frame.arp.dmac_match == VTSS_ACE_BIT_1 ? 1 : 0)<<4) |
				  ((ace->frame.arp.smac_match == VTSS_ACE_BIT_1 ? 1 : 0)<<3) |
				  ((ace->frame.arp.unknown == VTSS_ACE_BIT_1 ? 1 : 0)<<2) |
				  ((ace->frame.arp.arp == VTSS_ACE_BIT_1 ? 0 : 1)<<1) |
				  ((ace->frame.arp.req == VTSS_ACE_BIT_1 ? 0 : 1)<<0),
				  ((ace->frame.arp.ethernet == VTSS_ACE_BIT_ANY ? 0 : 1)<<7) |
				  ((ace->frame.arp.ip == VTSS_ACE_BIT_ANY ? 0 : 1)<<6) |
				  ((ace->frame.arp.length == VTSS_ACE_BIT_ANY ? 0 : 1)<<5) |
				  ((ace->frame.arp.dmac_match == VTSS_ACE_BIT_ANY ? 0 : 1)<<4) |
				  ((ace->frame.arp.smac_match == VTSS_ACE_BIT_ANY ? 0 : 1)<<3) |
				  ((ace->frame.arp.unknown == VTSS_ACE_BIT_ANY ? 0 : 1)<<2) |
				  ((ace->frame.arp.arp == VTSS_ACE_BIT_ANY ? 0 : 1)<<1) |
				  ((ace->frame.arp.req == VTSS_ACE_BIT_ANY ? 0 : 1)<<0));
			HT_ACE_WR(ARP_L3_IPV4_DIP, ace->frame.arp.dip.value, ace->frame.arp.dip.mask);
			HT_ACE_WR(ARP_L3_IPV4_SIP, ace->frame.arp.sip.value, ace->frame.arp.sip.mask);
			break;
		case ACL_TYPE_UDP_TCP:
			HT_ACE_WR(UDP_TCP_TYPE, misc[0], misc[1]);
			HT_ACE_WR(UDP_TCP_L3_MISC_CFG, l3_misc[0], l3_misc[1]); 
			HT_ACE_WR(UDP_TCP_L3_IPV4_DIP, ace->frame.ipv4.dip.value, ace->frame.ipv4.dip.mask);
			HT_ACE_WR(UDP_TCP_L3_IPV4_SIP, ace->frame.ipv4.sip.value, ace->frame.ipv4.sip.mask);
			sport = &ace->frame.ipv4.sport;
			dport = &ace->frame.ipv4.dport;
			srange = (udp_tcp ? ht_acl_range_alloc(sport, 1, 1) : 0);
			drange = (udp_tcp ? ht_acl_range_alloc(dport, 0, 1) : 0);
			HT_ACE_WR(UDP_TCP_L4_PORT, 
				  (sport->low<<16) | dport->low,
				  ((srange == 0 && sport->low == sport->high ? 0xffff : 0)<<16) |
				  (drange == 0 && dport->low == dport->high ? 0xffff : 0));
			HT_ACE_WR(UDP_TCP_L4_MISC,
				  ((ace->frame.ipv4.tcp_fin == VTSS_ACE_BIT_1 ? 1 : 0)<<13) |
				  ((ace->frame.ipv4.tcp_syn == VTSS_ACE_BIT_1 ? 1 : 0)<<12) |
				  ((ace->frame.ipv4.tcp_rst == VTSS_ACE_BIT_1 ? 1 : 0)<<11) |
				  ((ace->frame.ipv4.tcp_psh == VTSS_ACE_BIT_1 ? 1 : 0)<<10) |
				  ((ace->frame.ipv4.tcp_ack == VTSS_ACE_BIT_1 ? 1 : 0)<<9) |
				  ((ace->frame.ipv4.tcp_urg == VTSS_ACE_BIT_1 ? 1 : 0)<<8) |
				  (((sport->in_range ? srange : 0) | (dport->in_range ? drange : 0))<<0),
				  ((tcp && ace->frame.ipv4.tcp_fin != VTSS_ACE_BIT_ANY ? 1 : 0)<<13) |
				  ((tcp && ace->frame.ipv4.tcp_syn != VTSS_ACE_BIT_ANY ? 1 : 0)<<12) |
				  ((tcp && ace->frame.ipv4.tcp_rst != VTSS_ACE_BIT_ANY ? 1 : 0)<<11) |
				  ((tcp && ace->frame.ipv4.tcp_psh != VTSS_ACE_BIT_ANY ? 1 : 0)<<10) |
				  ((tcp && ace->frame.ipv4.tcp_ack != VTSS_ACE_BIT_ANY ? 1 : 0)<<9) |
				  ((tcp && ace->frame.ipv4.tcp_urg != VTSS_ACE_BIT_ANY ? 1 : 0)<<8) |
				  ((srange | drange)<<0));
			break;
		case ACL_TYPE_IPV4:
			HT_ACE_WR(IPV4_TYPE, misc[0], misc[1]);
			HT_ACE_WR(IPV4_L3_MISC_CFG, l3_misc[0], l3_misc[1]); 
			HT_ACE_WR(IPV4_L3_IPV4_DIP, ace->frame.ipv4.dip.value, ace->frame.ipv4.dip.mask);
			HT_ACE_WR(IPV4_L3_IPV4_SIP, ace->frame.ipv4.sip.value, ace->frame.ipv4.sip.mask);
			HT_ACE_MAC(l3_hi, l3_lo, ace->frame.ipv4.data);
			HT_ACE_WR(IPV4_DATA_1, l3_hi[0], l3_hi[1]);
			HT_ACE_WR(IPV4_DATA_0, l3_lo[0], l3_lo[1]);
			break;
		case ACL_TYPE_IPV6:
			HT_ACE_WR(IPV6_TYPE, misc[0], misc[1]);
			HT_ACE_WR(IPV6_L3_MISC_CFG, 
				  ace->frame.ipv6.proto.value<<8, ace->frame.ipv6.proto.mask<<8);
			HT_ACE_IPV6(ipv6, ace->frame.ipv6.sip, 0);
			HT_ACE_WR(IPV6_L3_IPV6_SIP_3, ipv6[0], ipv6[1]); 
			HT_ACE_IPV6(ipv6, ace->frame.ipv6.sip, 4);
			HT_ACE_WR(IPV6_L3_IPV6_SIP_2, ipv6[0], ipv6[1]); 
			HT_ACE_IPV6(ipv6, ace->frame.ipv6.sip, 8);
			HT_ACE_WR(IPV6_L3_IPV6_SIP_1, ipv6[0], ipv6[1]); 
			HT_ACE_IPV6(ipv6, ace->frame.ipv6.sip, 12);
			HT_ACE_WR(IPV6_L3_IPV6_SIP_0, ipv6[0], ipv6[1]); 
			break;
		default:
			vtss_log(VTSS_LOG_ERR,
				 "SPARX: illegal type, type=%d", type);
			return VTSS_INVALID_PARAMETER;
		}
		
		/* Action */
		VTSS_RC(ht_acl_action_set(&ace->action, cur->counter));
		
		/* Add entry */
		VTSS_RC(ht_acl_cmd(ACL_CMD_WRITE, i));
	}
	
	/* Delete next entry */
	if (i >= 0) {
		HT_WR(ACL, S_ACL, ETYPE_TYPE + ACE_DATA_OFFS, 0<<0);
		HT_WR(ACL, S_ACL, ETYPE_TYPE + ACE_MASK_OFFS, 0<<0);
		HT_WR(ACL, S_ACL, IN_VLD, 0<<0);
		VTSS_RC(ht_acl_cmd(ACL_CMD_WRITE, i));
	}
	
	return VTSS_OK;
}

static const char *ht_ace_bit_txt(vtss_ace_bit_t flag)
{
	return (flag == VTSS_ACE_BIT_ANY ? "*" :
		flag == VTSS_ACE_BIT_1 ? "1" :
		flag == VTSS_ACE_BIT_0 ? "0" : "?");
}

static char *ht_ace_uchar6_txt(const vtss_ace_uchar6_t *data, char *buf)
{
	sprintf(buf, "%02x-%02x-%02x-%02x-%02x-%02x/%02x-%02x-%02x-%02x-%02x-%02x",
		data->value[0], data->value[1], data->value[2], 
		data->value[3], data->value[4], data->value[5], 
		data->mask[0], data->mask[1], data->mask[2], 
		data->mask[3], data->mask[4], data->mask[5]);
	return buf;
}

static char *ht_ace_uchar2_txt(const vtss_ace_uchar2_t *data, char *buf)
{
	sprintf(buf, "%02x%02x/%02x%02x",
		data->value[0], data->value[1],
		data->mask[0], data->mask[1]);
	return buf;
}

/* Read all ACE counters and store in state memory */
static vtss_rc ht_ace_counters_read(void) 
{
	int i;
	vtss_acl_entry_t *cur;
	
	for (cur = vtss_acl_state.acl_list, i = (VTSS_ACES - 1); 
	cur != NULL; 
	cur = cur->next, i--) {
		/* Read entry */
		VTSS_RC(ht_acl_cmd(ACL_CMD_READ, i));
		HT_RD(ACL, S_ACL, IN_CNT, &cur->counter);
	}
	return VTSS_OK;
}

vtss_rc vtss_ll_ace_add(vtss_ace_id_t ace_id,
                        const vtss_ace_t * ace)
{
	vtss_acl_entry_t *cur, *prev = NULL;
	vtss_acl_entry_t *new = NULL, *new_prev = NULL, *ins = NULL, *ins_prev = NULL;
	char buf[64];
	
	vtss_log(VTSS_LOG_DEBUG,
		"SPARX: ace_id: %ld", ace->id);
	vtss_log(VTSS_LOG_DEBUG,
		"SPARX: rule type: %s (%d)",
		ace->rule == VTSS_ACE_RULE_PORT ? "Port" :
	ace->rule == VTSS_ACE_RULE_POLICY ? "Policy" :
	ace->rule == VTSS_ACE_RULE_SWITCH ? "Switch" : "?",
		ace->rule == VTSS_ACE_RULE_PORT ? ace->port_no :
	ace->rule == VTSS_ACE_RULE_POLICY ? ace->policy_no : 0);
	vtss_log(VTSS_LOG_DEBUG,
		"SPARX: frame type: %s",
		ace->type == VTSS_ACE_TYPE_ANY ? "ANY" :
	ace->type == VTSS_ACE_TYPE_ETYPE ? "ETYPE" :
	ace->type == VTSS_ACE_TYPE_LLC ? "LLC" :
	ace->type == VTSS_ACE_TYPE_SNAP ? "SNAP" :
	ace->type == VTSS_ACE_TYPE_ARP ? "ARP" :
	ace->type == VTSS_ACE_TYPE_IPV4 ? "IPV4" :
	ace->type == VTSS_ACE_TYPE_IPV6 ? "IPV6" : "?");
	vtss_log(VTSS_LOG_DEBUG,
		 "SPARX: DMAC flags: bc: %s, mc: %s", 
		 ht_ace_bit_txt(ace->dmac_bc), ht_ace_bit_txt(ace->dmac_mc));  
	vtss_log(VTSS_LOG_DEBUG,
		 "SPARX: vid: %03x/%03x, usr_prio: %02x/%02x, cfi: %s",
		 ace->vlan.vid.value, ace->vlan.vid.mask,
		 ace->vlan.usr_prio.value, ace->vlan.usr_prio.mask,
		 ht_ace_bit_txt(ace->vlan.cfi));
	
	switch (ace->type) {
	case VTSS_ACE_TYPE_ETYPE:
		vtss_log(VTSS_LOG_DEBUG,
			 "SPARX: DMAC: %s", ht_ace_uchar6_txt(&ace->frame.etype.dmac, buf));
		vtss_log(VTSS_LOG_DEBUG,
			 "SPARX: SMAC: %s", ht_ace_uchar6_txt(&ace->frame.etype.smac, buf));
		vtss_log(VTSS_LOG_DEBUG,
			 "SPARX: EType: %s", ht_ace_uchar2_txt(&ace->frame.etype.etype, buf));
		vtss_log(VTSS_LOG_DEBUG,
			 "SPARX: Data: %s", ht_ace_uchar2_txt(&ace->frame.etype.data, buf));
		break;
	case VTSS_ACE_TYPE_LLC:
		vtss_log(VTSS_LOG_DEBUG,
			 "SPARX: DMAC: %s", ht_ace_uchar6_txt(&ace->frame.llc.dmac, buf));
		vtss_log(VTSS_LOG_DEBUG,
			 "SPARX: SMAC: %s", ht_ace_uchar6_txt(&ace->frame.llc.smac, buf));
		vtss_log(VTSS_LOG_DEBUG, "SPARX: LLC: %02x-%02x-%02x-%02x/%02x-%02x-%02x-%02x",
			 ace->frame.llc.llc.value[0], ace->frame.llc.llc.value[1], 
			 ace->frame.llc.llc.value[2], ace->frame.llc.llc.value[3], 
			 ace->frame.llc.llc.mask[0], ace->frame.llc.llc.mask[1], 
			 ace->frame.llc.llc.mask[2], ace->frame.llc.llc.mask[3]);
		break;
	case VTSS_ACE_TYPE_SNAP:
		vtss_log(VTSS_LOG_DEBUG,
			 "SPARX: DMAC: %s",
			 ht_ace_uchar6_txt(&ace->frame.snap.dmac, buf));
		vtss_log(VTSS_LOG_DEBUG,
			 "SPARX: SMAC: %s",
			 ht_ace_uchar6_txt(&ace->frame.snap.smac, buf));
		vtss_log(VTSS_LOG_DEBUG,
			 "SPARX: SNAP: %02x-%02x-%02x-%02x-%02x/%02x-%02x-%02x-%02x-%02x",
			 ace->frame.snap.snap.value[0],
			 ace->frame.snap.snap.value[1], 
			 ace->frame.snap.snap.value[2],
			 ace->frame.snap.snap.value[3], 
			 ace->frame.snap.snap.value[4],
			 ace->frame.snap.snap.mask[0],
			 ace->frame.snap.snap.mask[1], 
			 ace->frame.snap.snap.mask[2],
			 ace->frame.snap.snap.mask[3], 
			 ace->frame.snap.snap.mask[4]);
		break;
	case VTSS_ACE_TYPE_ARP:
		vtss_log(VTSS_LOG_DEBUG,
			 "SPARX: SMAC: %s",
			 ht_ace_uchar6_txt(&ace->frame.arp.smac, buf));
		vtss_log(VTSS_LOG_DEBUG,
			 "SPARX: ARP flags: arp: %s, req: %s, unknown: %s, smac: %s", 
			 ht_ace_bit_txt(ace->frame.arp.arp), 
			 ht_ace_bit_txt(ace->frame.arp.req), 
			 ht_ace_bit_txt(ace->frame.arp.unknown), 
			 ht_ace_bit_txt(ace->frame.arp.smac_match));
		vtss_log(VTSS_LOG_DEBUG,
			 "SPARX: ARP flags: dmac: %s, length: %s, ip: %s, ethernet: %s", 
			 ht_ace_bit_txt(ace->frame.arp.dmac_match), 
			 ht_ace_bit_txt(ace->frame.arp.length), 
			 ht_ace_bit_txt(ace->frame.arp.ip), 
			 ht_ace_bit_txt(ace->frame.arp.ethernet));
		vtss_log(VTSS_LOG_DEBUG,
			 "SPARX: sip: %08lx/%08lx, dip: %08lx/%08lx",
			 ace->frame.arp.sip.value, ace->frame.arp.sip.mask,
			 ace->frame.arp.dip.value, ace->frame.arp.dip.mask);
		break;
	case VTSS_ACE_TYPE_IPV4:
		vtss_log(VTSS_LOG_DEBUG,
			 "SPARX: IP flags: ttl: %s, fragment: %s, options: %s", 
			 ht_ace_bit_txt(ace->frame.ipv4.ttl), 
			 ht_ace_bit_txt(ace->frame.ipv4.fragment),  
			 ht_ace_bit_txt(ace->frame.ipv4.options));
		vtss_log(VTSS_LOG_DEBUG,
			 "SPARX: proto: %02x/%02x, ds: %02x/%02x",
			 ace->frame.ipv4.proto.value, ace->frame.ipv4.proto.mask,
			 ace->frame.ipv4.ds.value, ace->frame.ipv4.ds.mask);
		vtss_log(VTSS_LOG_DEBUG,
			 "SPARX: sip: %08lx/%08lx, dip: %08lx/%08lx",
			 ace->frame.ipv4.sip.value, ace->frame.ipv4.sip.mask,
			 ace->frame.ipv4.dip.value, ace->frame.ipv4.dip.mask);
		vtss_log(VTSS_LOG_DEBUG,
			 "SPARX: ip_data: %s",
			 ht_ace_uchar6_txt(&ace->frame.ipv4.data, buf));
		vtss_log(VTSS_LOG_DEBUG,
			 "SPARX: sport: %d-%d, in_range: %d", 
			 ace->frame.ipv4.sport.low, 
			 ace->frame.ipv4.sport.high, 
			 ace->frame.ipv4.sport.in_range);
		vtss_log(VTSS_LOG_DEBUG,
			 "SPARX: dport: %d-%d, in_range: %d", 
			 ace->frame.ipv4.dport.low, 
			 ace->frame.ipv4.dport.high, 
			 ace->frame.ipv4.dport.in_range);
		vtss_log(VTSS_LOG_DEBUG,
			 "SPARX: TCP flags: fin: %s, syn: %s, rst: %s, psh: %s, ack: %s, urg: %s", 
			 ht_ace_bit_txt(ace->frame.ipv4.tcp_fin), 
			 ht_ace_bit_txt(ace->frame.ipv4.tcp_syn), 
			 ht_ace_bit_txt(ace->frame.ipv4.tcp_rst), 
			 ht_ace_bit_txt(ace->frame.ipv4.tcp_psh), 
			 ht_ace_bit_txt(ace->frame.ipv4.tcp_ack), 
			 ht_ace_bit_txt(ace->frame.ipv4.tcp_urg));
		break;
	case VTSS_ACE_TYPE_IPV6:
		vtss_log(VTSS_LOG_DEBUG,
			 "SPARX: proto: %02x/%02x",
			 ace->frame.ipv6.proto.value,
			 ace->frame.ipv6.proto.mask);
		vtss_log(VTSS_LOG_DEBUG,
			 "SPARX: sip : %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x",
			 ace->frame.ipv6.sip.value[0], ace->frame.ipv6.sip.value[1],
			 ace->frame.ipv6.sip.value[2], ace->frame.ipv6.sip.value[3],
			 ace->frame.ipv6.sip.value[4], ace->frame.ipv6.sip.value[5],
			 ace->frame.ipv6.sip.value[6], ace->frame.ipv6.sip.value[7],
			 ace->frame.ipv6.sip.value[8], ace->frame.ipv6.sip.value[9],
			 ace->frame.ipv6.sip.value[10], ace->frame.ipv6.sip.value[11],
			 ace->frame.ipv6.sip.value[12], ace->frame.ipv6.sip.value[13],
			 ace->frame.ipv6.sip.value[14], ace->frame.ipv6.sip.value[15]);
		vtss_log(VTSS_LOG_DEBUG,
			 "SPARX: mask : %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x",
			 ace->frame.ipv6.sip.mask[0], ace->frame.ipv6.sip.mask[1],
			 ace->frame.ipv6.sip.mask[2], ace->frame.ipv6.sip.mask[3],
			 ace->frame.ipv6.sip.mask[4], ace->frame.ipv6.sip.mask[5],
			 ace->frame.ipv6.sip.mask[6], ace->frame.ipv6.sip.mask[7],
			 ace->frame.ipv6.sip.mask[8], ace->frame.ipv6.sip.mask[9],
			 ace->frame.ipv6.sip.mask[10], ace->frame.ipv6.sip.mask[11],
			 ace->frame.ipv6.sip.mask[12], ace->frame.ipv6.sip.mask[13],
			 ace->frame.ipv6.sip.mask[14], ace->frame.ipv6.sip.mask[15]);
		break;
	case VTSS_ACE_TYPE_ANY:
	default:
		break;
	}
    
	/* Check ACE ID */
	if (ace->id == VTSS_ACE_ID_LAST || ace->id == ace_id) {
		vtss_log(VTSS_LOG_ERR, "SPARX: illegal ace, ace=%ld", ace->id);
		return VTSS_INVALID_PARAMETER;
	}
	
	/* Check rule type */
	if (ace->rule > VTSS_ACE_RULE_SWITCH) {
		vtss_log(VTSS_LOG_ERR, "SPARX: illegal rule, rule=%d", ace->rule);
		return VTSS_INVALID_PARAMETER;
	}
	
	/* Check frame type */
	if (ace->type > VTSS_ACE_TYPE_IPV6) {
		vtss_log(VTSS_LOG_ERR, "SPARX: illegal type, type=%d", ace->type);
		return VTSS_INVALID_PARAMETER;
	}
	
	/* Free range checkers to be able to check if out of range checkers */
	ht_acl_range_free();
	
	/* Search for existing entry and place to add */
	for (cur = vtss_acl_state.acl_list; cur != NULL;
	     prev = cur, cur = cur->next) {
		if (cur->ace.id == ace->id) {
			/* Entry already exists */
			new_prev = prev;
			new = cur;
		} else if (HT_ACE_UDP_TCP(&cur->ace)) {
			/* Allocate range checkers for other entries */
			ht_acl_range_alloc(&cur->ace.frame.ipv4.sport, 1, 0);
			ht_acl_range_alloc(&cur->ace.frame.ipv4.dport, 0, 0);
		}
		
		if (cur->ace.id == ace_id) {
			/* Found insertion point */
			ins_prev = prev;
			ins = cur;
		}
	}

	if (ace_id == VTSS_ACE_ID_LAST)
	     ins_prev = prev;

	/* Check if the place to insert was found */
	if (ins == NULL && ace_id != VTSS_ACE_ID_LAST) {
	     vtss_log(VTSS_LOG_ERR,
		      "SPARX: could not find ace ID, ace=%ld",
		      ace_id);
	     return VTSS_ENTRY_NOT_FOUND;
	}

	/* Check if range checker can be allocated for new entry */
	if (HT_ACE_UDP_TCP(ace) && 
	    (ht_acl_range_alloc(&ace->frame.ipv4.sport, 1, 0) == 0xff ||
	    ht_acl_range_alloc(&ace->frame.ipv4.dport, 0, 0) == 0xff)) {
	     vtss_log(VTSS_LOG_ERR, "SPARX: no more range checkers available");
	     return VTSS_UNSPECIFIED_ERROR;
	}

	/* Read current ACE counters */
	VTSS_RC(ht_ace_counters_read());

	if (new == NULL) {
	     /* Take new entry from free list */
	     if ((new = vtss_acl_state.acl_free) == NULL) {
		     vtss_log(VTSS_LOG_ERR, "SPARX: ACL is full");
		     return VTSS_UNSPECIFIED_ERROR;
	     }
	     new->counter = 0;
	     vtss_acl_state.acl_free = new->next;
	} else {
	     /* Take existing entry out of list */
	     if (ins_prev == new)
		     ins_prev = new_prev;
	     if (new_prev == NULL)
		     vtss_acl_state.acl_list = new->next;
	     else
		     new_prev->next = new->next;
	}

	/* Insert new entry in list */
	if (ins_prev == NULL) {
	     new->next = vtss_acl_state.acl_list;
	     vtss_acl_state.acl_list = new;
	} else {
	     new->next = ins_prev->next;
	     ins_prev->next = new;
	}
	new->ace = *ace;

	return ht_acl_commit();
}

vtss_rc vtss_ll_ace_del(vtss_ace_id_t ace_id)
{
	vtss_acl_entry_t *cur, *prev;
	
	/* Read current ACE counters */
	VTSS_RC(ht_ace_counters_read());
	
	/* Search for entry */
	for (cur = vtss_acl_state.acl_list, prev = NULL; 
	cur != NULL; 
	prev = cur, cur = cur->next) {
		if (cur->ace.id == ace_id) {
			if (prev == NULL)
				vtss_acl_state.acl_list = cur->next;
			else
				prev->next = cur->next;
			cur->next = vtss_acl_state.acl_free;
			vtss_acl_state.acl_free = cur;
			break;
		}
	}
	return (cur == NULL ? VTSS_ENTRY_NOT_FOUND : ht_acl_commit());
}

vtss_rc vtss_ll_ace_counter_get(vtss_ace_id_t ace_id,
                                vtss_ace_counter_t * counter)
{
	int              i;
	vtss_acl_entry_t *cur;
	
	/* Lookup entry with ACE ID */
	for (cur = vtss_acl_state.acl_list, i = (VTSS_ACES - 1); 
	cur != NULL; 
	cur = cur->next, i--) {
		if (cur->ace.id == ace_id) {
			/* Read entry */
			VTSS_RC(ht_acl_cmd(ACL_CMD_READ, i));
			HT_RD(ACL, S_ACL, IN_CNT, counter);
			return VTSS_OK;
		}
	}
	return VTSS_ENTRY_NOT_FOUND;
}

vtss_rc vtss_ll_ace_counter_clear(vtss_ace_id_t ace_id)
{
	int              i;
	vtss_acl_entry_t *cur;
	
	/* Lookup entry with ACE ID */
	for (cur = vtss_acl_state.acl_list, i = (VTSS_ACES - 1); 
	cur != NULL; 
	cur = cur->next, i--) {
		if (cur->ace.id == ace_id) {
			/* Clear counter for ACE */
			VTSS_RC(ht_acl_cmd(ACL_CMD_READ, i));
			HT_WR(ACL, S_ACL, IN_CNT, 0);
			VTSS_RC(ht_acl_cmd(ACL_CMD_WRITE, i));
			return VTSS_OK;
		}
	}
	return VTSS_ENTRY_NOT_FOUND;
}

void vtss_acl_init_state(void)
{
	int i;
	memset(&vtss_acl_state, 0, sizeof (vtss_acl_state_t));

	/* Add ACL entries to free list */
	vtss_acl_state.acl_list = NULL;
	vtss_acl_state.acl_free = NULL;
	for (i = 0; i < VTSS_ACES; i++) {
		vtss_acl_entry_t *acl_entry;
		
		acl_entry = &vtss_acl_state.acl[i];
		acl_entry->next = vtss_acl_state.acl_free;
		vtss_acl_state.acl_free = acl_entry;
	}
}

vtss_rc vtss_acl_enable(void)
{
	/* Clear ACL table and enable ACLs */
	HT_WR(ACL, S_ACL, ACL_CFG, 1<<0);
	HT_WR(ACL, S_ACL, ETYPE_TYPE + ACE_DATA_OFFS, 0<<0);
	HT_WR(ACL, S_ACL, ETYPE_TYPE + ACE_MASK_OFFS, 0<<0);
	VTSS_RC(ht_acl_cmd(ACL_CMD_INIT, 0));
	HT_WR(ACL, S_ACL, EG_VLD, 1<<0);
#ifdef CONFIG_VTSS_GROCX
	HT_WR(ACL, S_ACL, EG_MISC, 0);
#endif
	HT_WR(ACL, S_ACL, EG_PORT_MASK, VTSS_CHIP_PORTMASK + (1<<VTSS_CHIP_PORT_CPU));
	VTSS_RC(ht_acl_cmd(ACL_CMD_WRITE, VTSS_ACES + VTSS_CHIP_PORTS));

	return VTSS_OK;
}

void ht_acl_reset_port(uint port_on_chip)
{
	uint value;

	/* Setup one-to-one PAG mapping taking internal aggregations into account */
	value = port_on_chip;
	HT_WR(ACL, S_ACL, PAG_CFG + port_on_chip, value);
}

void vtss_acl_reset_port(vtss_port_no_t port_no)
{
	vtss_acl_policy_no_set(port_no, VTSS_ACL_POLICY_NO_NONE);
}
#endif

#endif
