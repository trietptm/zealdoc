#ifndef __VTSS_HEATHROW_H_INCLUDE__
#define __VTSS_HEATHROW_H_INCLUDE__

typedef enum _ht_tx_clock_select_t {
	HT_TX_CLOCK_OFF      = 0,
	HT_TX_CLOCK_GIGA     = 1,
	HT_TX_CLOCK_100M     = 2,
	HT_TX_CLOCK_10M      = 3,
	HT_TX_CLOCK_MISC     = 4 
} ht_tx_clock_select_t;

/* SparX-G16/G24 revision B tail drop in MISCFIFO bit 2 */
#define MISCFIFO_TAILDROP (0<<2)

/* Frame ageing timer */
#ifdef CONFIG_VTSS_ARCH_SPARX_28
#define HT_TIMECMP_DEFAULT 0x3ffffff
#else
#define HT_TIMECMP_DEFAULT 0x3b9aca0
#endif

/* ================================================================= *
 * helper functions
 * ================================================================= */
#define HT_IS_NOT_MII_PORT(port_no)     (1)
#define HT_CHIP_PORT(port_no)           (vtss_mac_state.port_map.chip_port[port_no])

/* ================================================================= *
 * register access
 * ================================================================= */
#define HT_RD(blk, sub, reg, value) \
{ \
	vtss_rc rc; \
	if ((rc = ht_rd_wr(B_##blk, sub, R_##blk##_##reg, value, 0)) < 0) \
		return rc; \
}

#define HT_WR(blk, sub, reg, value) \
{ \
	vtss_rc rc; \
	ulong   val = value; \
	if ((rc = ht_rd_wr(B_##blk, sub, R_##blk##_##reg, &val, 1)) < 0) \
		return rc; \
}

#define HT_WRM(blk, sub, reg, value, mask) \
{ \
	vtss_rc rc; \
	if ((rc = ht_wrm(B_##blk, sub, R_##blk##_##reg, value, mask)) < 0) \
		return rc; \
}

#define HT_RDF(blk, sub, reg, offset, mask, value) \
{ \
	vtss_rc rc; \
	if ((rc = ht_rdf(B_##blk, sub, R_##blk##_##reg, offset, mask, value)) < 0) \
		return rc; \
}

#define HT_WRF(blk, sub, reg, offset, mask, value) \
{ \
	vtss_rc rc; \
	if ((rc = ht_wrf(B_##blk, sub, R_##blk##_##reg, offset, mask, value)) < 0) \
		return rc; \
}

vtss_rc ht_rd_wr(uint blk, uint sub, uint reg, ulong *value, BOOL do_write);
vtss_rc ht_rd(uint blk, uint sub, uint reg, ulong *value);
vtss_rc ht_wr(uint blk, uint sub, uint reg, ulong value);
vtss_rc ht_wrm(uint blk, uint sub, uint reg, ulong value, ulong mask);
vtss_rc ht_rdf(uint blk, uint sub, uint reg, uint offset, ulong mask, ulong *value);
vtss_rc ht_wrf(uint blk, uint sub, uint reg, uint offset, ulong mask, ulong value);

/* CPU interface reset */
#ifdef CONFIG_VTSS_CPU_FRAME
vtss_rc ht_cpu_frame_reset(void);
#else
static inline vtss_rc ht_cpu_frame_reset(void) { return VTSS_OK; }
#endif

#ifdef CONFIG_VTSS_ACL
void ht_acl_reset_port(uint port_on_chip);
#else
static inline void ht_acl_reset_port(uint port_on_chip) {};
#endif

/* Calculate packet rate register field */
ulong calc_packet_rate(vtss_packet_rate_t rate, ulong *unit);

/* Convert from vtss_pgid_no to destination index on chip */
uint vtss_pgid2chip(const vtss_pgid_no_t pgid_no);
/* Convert from destination index on chip to vtss_pgid_no */
vtss_pgid_no_t vtss_chip2pgid(const uint chip_pgid);
/* Convert from chip port bitfield to vtss_port_no bitfield. */
ulong vtss_chip2portmask(const ulong chip_port_bitfield);
/* Convert from vtss_port_no list to chip port bitfield, */
ulong vtss_portmask2chip(const BOOL member[VTSS_PORT_ARRAY_SIZE]);

#endif
