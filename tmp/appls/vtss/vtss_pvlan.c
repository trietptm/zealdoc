#include "vtss_priv.h"

/* - PGID functions -------------------------------------------------------- */
/* Determine whether a port number is PGID member considering aggregations */
BOOL vtss_pgid_member(vtss_pgid_no_t pgid_no, vtss_port_no_t port)
{
	vtss_pgid_entry_t *pgid_entry;
	vtss_poag_no_t    poag_no;
	vtss_port_no_t    port_no;
	BOOL              member;
	
	pgid_entry = &vtss_mac_state.pgid_table[pgid_no];
	member = pgid_entry->member[port];
	
	if ((poag_no = vtss_mac_state.port_poag_no[port]) != port) {
		/* Port is aggregated, check aggregated ports */
		for (port_no = VTSS_PORT_NO_START;
		     port_no < VTSS_PORT_NO_END; port_no++) {
			if (vtss_mac_state.port_poag_no[port_no] == poag_no &&
			    pgid_entry->member[port_no])
				member = 1;
		}
	}
	return member;
}

/* Write PGID member to chip */
vtss_rc vtss_pgid_table_write(const vtss_pgid_no_t pgid_no)
{
	vtss_port_no_t port_no;
	BOOL member[VTSS_PORT_ARRAY_SIZE];
	
	/* Avoid updating unicast entries for unmapped ports */
	if (pgid_no<VTSS_PGID_UNICAST_END &&
	    vtss_mac_state.port_map.vtss_port_unused[pgid_no]) {
		vtss_log(VTSS_LOG_DEBUG,
			"SWITCH: skipping unmapped pgid, pgid=%d", pgid_no);
		return VTSS_OK;
	}
	
	for (port_no = VTSS_PORT_NO_START;
	     port_no < VTSS_PORT_NO_END; port_no++) {
		member[port_no] = vtss_pgid_member(pgid_no, port_no);
		
		/* Ensure that unmapped ports are not included */
		if (vtss_mac_state.port_map.vtss_port_unused[port_no])
			member[port_no] = 0;
	}
	
	/* Update PGID table */
	return vtss_ll_pgid_table_write(pgid_no, member);
}

/* Allocate PGID */
vtss_rc vtss_pgid_alloc(vtss_pgid_no_t *pgid_no, BOOL resv,
                        const BOOL member[VTSS_PORT_ARRAY_SIZE])
{
	vtss_pgid_no_t pgid, pgid_start, pgid_free;
	vtss_pgid_entry_t *pgid_entry;
	vtss_port_no_t port_no;
	
	/* Search for matching or unused entry in PGID table */
	pgid_free = vtss_mac_state.pgid_end;
	pgid_start = VTSS_PGID_START;
	for (pgid = pgid_start; pgid < vtss_mac_state.pgid_end; pgid++)  {
		pgid_entry = &vtss_mac_state.pgid_table[pgid];
		
		if (pgid_entry->references == 0) {
			/* Check if the first unused entry is found */
			if (pgid_free == vtss_mac_state.pgid_end)
				pgid_free = pgid;
		} else if (!resv && !pgid_entry->resv) {
			/* Check if an existing entry matches */
			for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++)
				if (member[port_no] != pgid_entry->member[port_no])
					break;
				if (port_no == VTSS_PORT_NO_END) {
					pgid_entry->references++;
					vtss_log(VTSS_LOG_DEBUG,
						 "SWITCH: reusing pgid, pgid=%d", pgid);
					*pgid_no = pgid;
					return VTSS_OK;
				}
		}
	}
	
	/* No pgid found */
	if (pgid_free == vtss_mac_state.pgid_end)
		return VTSS_ENTRY_NOT_FOUND;
	
	/* Unused PGID found */
	*pgid_no = pgid_free;
	vtss_log(VTSS_LOG_DEBUG,
		 "SWITCH: using pgid, pgid=%d", *pgid_no);

	pgid_entry = &vtss_mac_state.pgid_table[*pgid_no];
	pgid_entry->resv = resv;
	pgid_entry->references = 1;
	for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++)
		pgid_entry->member[port_no] = member[port_no];
	return vtss_pgid_table_write(*pgid_no);
}

/* Free PGID */
vtss_rc vtss_pgid_free(const vtss_pgid_no_t pgid_no)
{
	vtss_pgid_entry_t *pgid_entry;
	
	pgid_entry = &vtss_mac_state.pgid_table[pgid_no];
	if (pgid_entry->references == 0) {
		vtss_log(VTSS_LOG_ERR,
			 "SWITCH: pgid already freed, pgid=%d", pgid_no);
		return VTSS_INVALID_PARAMETER;
	}
	pgid_entry->resv = 0;
	pgid_entry->references--;
	return VTSS_OK;
}
