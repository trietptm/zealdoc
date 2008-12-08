#include "vtss_priv.h"

#if (CONFIG_VTSS_MAC_NEXT_MAX != 0)
static vtss_mac_entry_t *vtss_mac_table_get(ulong mach, ulong macl, BOOL next)
{
	uint first, last, mid;
	vtss_mac_entry_t *cur, *old;
	
	/* Check if table is empty */
	if (vtss_mac_state.mac_ptr_count == 0)
		return NULL;
	
	/* Locate page using binary search */
	for (first = 0, last = vtss_mac_state.mac_ptr_count-1; first != last; ) {
		mid = (first + last)/2;
		cur = vtss_mac_state.mac_list_ptr[mid];
		if (cur->mach > mach || (cur->mach == mach && cur->macl >= macl)) {
			/* Entry greater or equal, search lower half */
			last = mid; 
		} else {
			/* Entry is smaller, search upper half */
			first = mid + 1;
		}
	}
	
	cur = vtss_mac_state.mac_list_ptr[first];
	/* Go back one page if entry greater */
	if (first != 0 && (cur->mach > mach ||
	    (cur->mach == mach && cur->macl > macl)))
		cur = vtss_mac_state.mac_list_ptr[first-1]; 
	
	/* Look for greater entry using linear search */
	for (old = NULL; cur != NULL; old = cur, cur = cur->next) {
		if (cur->mach > mach || (cur->mach == mach && cur->macl > macl))
			break;
	}
	return (next ? cur : old);
}

/* Update MAC table page pointers */
static void vtss_mac_pages_update(void)
{
	uint i, count;
	vtss_mac_entry_t *cur;
	
	for (i = 0, cur = vtss_mac_state.mac_list_used;
	     i < VTSS_MAC_PTR_SIZE && cur != NULL ; i++) {
		vtss_mac_state.mac_list_ptr[i] = cur;
		
		/* Move one page forward */
		for (count = 0; count != VTSS_MAC_PAGE_SIZE && cur != NULL;
		     cur = cur->next, count++)
			;
	}
	vtss_mac_state.mac_ptr_count = i;
}

static void vtss_mach_macl_get(const vtss_vid_mac_t *vid_mac,
			       ulong *mach, ulong *macl)
{
	*mach = ((vid_mac->vid<<16) | (vid_mac->mac.addr[0]<<8) |
		vid_mac->mac.addr[1]);
	*macl = ((vid_mac->mac.addr[2]<<24) | (vid_mac->mac.addr[3]<<16) | 
		(vid_mac->mac.addr[4]<<8) | vid_mac->mac.addr[5]);
}

/* Add MAC table entry */
static vtss_mac_entry_t *vtss_mac_table_add(const vtss_vid_mac_t *vid_mac,
					    BOOL update)
{
	ulong mach, macl;
	vtss_mac_entry_t *cur, *tmp;
	
	/* Calculate MACH and MACL */
	vtss_mach_macl_get(vid_mac, &mach, &macl);
	
	/* Look for previous or existing entry in used list */
	if ((tmp = vtss_mac_table_get(mach, macl, 0)) != NULL && 
		tmp->mach == mach && tmp->macl == macl)
		return tmp;
	
	/* Allocate entry from free list */
	if ((cur = vtss_mac_state.mac_list_free) == NULL) {
		vtss_log(VTSS_LOG_ERR, "SWITCH: no free MAC entries");
		return NULL;
	}
	vtss_mac_state.mac_list_free = cur->next;
	cur->mach = mach;
	cur->macl = macl;
	
	if (tmp == NULL) {
		/* Insert first */
		cur->next = vtss_mac_state.mac_list_used;
		vtss_mac_state.mac_list_used = cur;
		/* Update first page pointer */
		vtss_mac_state.mac_list_ptr[0] = cur;
		if (vtss_mac_state.mac_ptr_count == 0)
			vtss_mac_state.mac_ptr_count = 1;
	} else {
		/* Insert after previous entry */
		cur->next = tmp->next;
		tmp->next = cur;
	}
	
	/* Update page pointers */
	if (update)
		vtss_mac_pages_update();
	return cur;
}

#ifdef CONFIG_VTSS_ARCH_SPARX_28
/* Delete MAC table entry */
static vtss_rc vtss_mac_table_del(const vtss_vid_mac_t *vid_mac)
{
	ulong mach, macl;
	vtss_mac_entry_t *cur, *old;
	
	/* Calculate MACH and MACL */
	vtss_mach_macl_get(vid_mac, &mach, &macl);
	
	/* Look for entry */
	for (old = NULL, cur = vtss_mac_state.mac_list_used; cur != NULL;
	     old = cur, cur = cur->next) {
		if (cur->mach == mach && cur->macl == macl) {
			/* Remove from used list */
			if (old == NULL)
				vtss_mac_state.mac_list_used = cur->next;
			else
				old->next = cur->next;
			
			/* Insert in free list */
			cur->next = vtss_mac_state.mac_list_free;
			vtss_mac_state.mac_list_free = cur;
			break;
		}
	}
	
	vtss_mac_pages_update();
	return VTSS_OK;
}

/* Update IPv4 and IPv6 multicast entries on aggregation changes */
vtss_rc vtss_mac_table_update(void) 
{
	vtss_pgid_no_t pgid_no;
	vtss_mac_entry_t *cur;
	vtss_port_no_t port_no;
	vtss_pgid_entry_t *pgid_entry;
	vtss_mac_table_entry_t mac_entry;
	vtss_vid_mac_t *vid_mac;
	
	pgid_no = VTSS_PGID_NONE;
	pgid_entry = &vtss_mac_state.pgid_table[pgid_no];
	mac_entry.copy_to_cpu = 0;
	mac_entry.locked = 1;
	mac_entry.aged = 0;
	vid_mac = &mac_entry.vid_mac;
	for (cur = vtss_mac_state.mac_list_used; cur != NULL; cur = cur->next) {
		for (port_no = VTSS_PORT_NO_START;
		     port_no < VTSS_PORT_NO_END; port_no++)
			pgid_entry->member[port_no] = VTSS_PORT_BF_GET(cur->member, port_no);
		for (port_no = VTSS_PORT_NO_START;
		     port_no < VTSS_PORT_NO_END; port_no++)
			pgid_entry->member[port_no] = vtss_pgid_member(pgid_no, port_no);
		vid_mac->vid = ((cur->mach>>16) & 0xfff);
		vid_mac->mac.addr[0] = (uchar)((cur->mach>>8) & 0xff);
		vid_mac->mac.addr[1] = (uchar)((cur->mach>>0) & 0xff);
		vid_mac->mac.addr[2] = (uchar)((cur->macl>>24) & 0xff);
		vid_mac->mac.addr[3] = (uchar)((cur->macl>>16) & 0xff);
		vid_mac->mac.addr[4] = (uchar)((cur->macl>>8) & 0xff);
		vid_mac->mac.addr[5] = (uchar)((cur->macl>>0) & 0xff);
		vtss_ll_mac_table_learn(&mac_entry, pgid_no);
	}
	
	return VTSS_OK;
}

#else

/* - Optimization functions ---------------------------------------- */
static vtss_rc vtss_mac_table_optimize(void)
{
	uint i, block;
	vtss_mac_table_entry_t entry;
	vtss_pgid_no_t pgid_no;
	vtss_mac_entry_t *cur, *old, *tmp;
	
	/* If update is not in progress, check if MAC address table has changed */
	if (vtss_mac_state.mac_index_next == 0) {
		VTSS_RC(vtss_mac_table_status_read());
		if (vtss_mac_state.mac_status_next.learned || 
			vtss_mac_state.mac_status_next.replaced || 
			vtss_mac_state.mac_status_next.aged) {
			vtss_mac_state.mac_status_next.learned = 0;
			vtss_mac_state.mac_status_next.replaced = 0;
			vtss_mac_state.mac_status_next.aged = 0;
		} else {
			return VTSS_OK;
		}
	}
	
	vtss_log(VTSS_LOG_DEBUG, "SWITCH: update start, index=%d",
		 vtss_mac_state.mac_index_next));
	
	/* Delete entries from current block */
	block = vtss_mac_state.mac_index_next/CONFIG_VTSS_MAC_NEXT_MAX;
	for (cur = vtss_mac_state.mac_list_used, old = NULL; cur != NULL; ) {
		if (cur->block == block) {
			/* Remove from used list */
			if (old == NULL)
				vtss_mac_state.mac_list_used = cur->next;
			else
				old->next = cur->next;
			
			/* Insert in free list */
			tmp = cur;
			cur = cur->next;
			tmp->next = vtss_mac_state.mac_list_free;
			vtss_mac_state.mac_list_free = tmp;
		} else {
			old = cur;
			cur = cur->next;
		}
	}
	
	/* Update page pointers */
	vtss_mac_pages_update();
	
	/* Insert entries from current block */
	for (i = vtss_mac_state.mac_index_next; (i/CONFIG_VTSS_MAC_NEXT_MAX) == block ; i++) {
		/* Read and add MAC address entry */
		if (vtss_ll_mac_table_read(i + VTSS_MAC_ADDR_START, &entry, &pgid_no) == VTSS_OK &&
			(cur = vtss_mac_table_add(&entry.vid_mac, 0)) != NULL)
			cur->block = block;
	}
	vtss_mac_state.mac_index_next = (i==vtss_mac_state.mac_addrs ? 0 : i);
	
	/* Update page pointers */
	vtss_mac_pages_update();
	vtss_log(VTSS_LOG_DEBUG, "SWITCH: update done, index=%d",
		 vtss_mac_state.mac_index_next);
	return VTSS_OK;
}
#endif /* SPARX_28 */
#endif

/* - MAC Table ----------------------------------------------------- */

vtss_rc vtss_mac_table_flush(void)
{
	return vtss_ll_mac_table_flush(0, 0, 0, 0);
}

vtss_rc vtss_mac_table_learn(const vtss_mac_table_entry_t * const entry)
{
	vtss_rc        rc;
	vtss_pgid_no_t pgid_no;
	vtss_port_no_t port_no;
	BOOL           member[VTSS_PORT_ARRAY_SIZE];
	vtss_vid_mac_t vid_mac;
	
	vid_mac = entry->vid_mac;
	
	/* Unlearn entry if it already exists */
	rc = vtss_mac_table_forget_vid_mac(&vid_mac);
	if (rc<0 && rc!=VTSS_ENTRY_NOT_FOUND)
		return rc;
	
	/* Allocate PGID */
#ifdef CONFIG_VTSS_ARCH_SPARX
	if (
		VTSS_MAC_IPV4_MC(entry->vid_mac.mac.addr)) {
		/* IPv4 multicast address, use pseudo PGID */
		pgid_no = VTSS_PGID_NONE;
	} else {
#ifdef CONFIG_VTSS_ARCH_SPARX_28
		if (
			VTSS_MAC_IPV6_MC(entry->vid_mac.mac.addr)) {
			/* IPv6 multicast address, use pseudo PGID */
			pgid_no = VTSS_PGID_NONE;
		} else
#endif
#endif
		{
			for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++)
				member[port_no] = MAKEBOOL01(entry->destination[port_no]);
			if (vtss_pgid_alloc(&pgid_no, 0, member)<0) {
				vtss_log(VTSS_LOG_ERR,
					 "SWITCH: pgid allocation failed");
				return VTSS_UNSPECIFIED_ERROR;
			}
		}
	}
	
	vtss_mac_state.mac_status_appl.learned = 1;
	vtss_mac_state.mac_status_next.learned = 1;
	vtss_mac_state.mac_status_sync.learned = 1;
	
	if (pgid_no == VTSS_PGID_NONE) {
		/* IPv4/IPv6 multicast address */
		for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++)
			vtss_mac_state.pgid_table[pgid_no].member[port_no] = MAKEBOOL01(entry->destination[port_no]);
		for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++)
			vtss_mac_state.pgid_table[pgid_no].member[port_no] = vtss_pgid_member(pgid_no, port_no);
	}
	
	rc = vtss_ll_mac_table_learn(entry, pgid_no);
	
#if (CONFIG_VTSS_MAC_NEXT_MAX != 0)
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	if (entry->locked && rc == VTSS_OK && pgid_no == VTSS_PGID_NONE) {
		vtss_mac_entry_t *mac_entry;
		
		/* Learn IPv4/IPv6 multicast entry for GET_NEXT operations */
		if ((mac_entry = vtss_mac_table_add(&entry->vid_mac, 1)) != NULL)
			for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++)
				VTSS_PORT_BF_SET(mac_entry->member, port_no, entry->destination[port_no]);
	}
#endif
#endif
	return rc;
}
#ifdef VTSS_FEATURE_MAC_AGE_AUTO
vtss_rc vtss_mac_table_age_time_set(const vtss_mac_age_time_t age_time)
{
	return vtss_ll_mac_table_age_time_set(age_time);
}
#endif

vtss_rc vtss_mac_table_age(void)
{
	return vtss_ll_mac_table_age(0, 0, 0, 0);
}

vtss_rc vtss_mac_table_age_vlan(const vtss_vid_t vid)
{
	return vtss_ll_mac_table_age(0, 0, 1, vid);
}

vtss_rc vtss_mac_table_forget_vid_mac(const vtss_vid_mac_t * const vid_mac)
{
	vtss_mac_table_entry_t entry;
	vtss_pgid_no_t         pgid_no;
	
	entry.vid_mac = *vid_mac;
	VTSS_RC(vtss_ll_mac_table_lookup(&entry, &pgid_no));
	
	if (entry.locked && 
		pgid_no!=VTSS_PGID_NONE)
		vtss_pgid_free(pgid_no);
	
	vtss_mac_state.mac_status_appl.aged = 1;
	vtss_mac_state.mac_status_next.aged = 1;
	vtss_mac_state.mac_status_sync.aged = 1;
	
#if (CONFIG_VTSS_MAC_NEXT_MAX != 0)
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	if (entry.locked && pgid_no == VTSS_PGID_NONE) {
		/* Unlearn IPv4/IPv6 multicast entry for GET_NEXT operations */
		vtss_mac_table_del(vid_mac);
	}
#endif /* SPARX_28 */
#endif
	return vtss_ll_mac_table_unlearn(vid_mac);
}

vtss_rc vtss_mac_table_forget_port(const vtss_port_no_t port_no)
{
	return vtss_ll_mac_table_flush(1, port_no, 0, 0);
}

vtss_rc vtss_mac_table_forget_vlan(const vtss_vid_t vid)
{
	return vtss_ll_mac_table_flush(0, 0, 1, vid);
}

vtss_rc vtss_mac_table_forget_port_in_vlan(const vtss_port_no_t port_no,
                                           const vtss_vid_t vid)
{
	return vtss_ll_mac_table_flush(1, port_no, 1, vid);
}

/* Set the destination list for a MAC address entry given a PGID */
static void vtss_mac_entry_destination_set(vtss_mac_table_entry_t * const entry, 
                                           vtss_pgid_no_t pgid_no) 
{
	vtss_poag_no_t poag_no;
	
	for (poag_no = VTSS_POAG_NO_START;
	     poag_no < VTSS_POAG_NO_END; poag_no++) {
		if (VTSS_POAG_IS_PORT(poag_no)) {
			entry->destination[poag_no] =
				vtss_pgid_member(pgid_no, poag_no);
		} else {
			entry->destination[poag_no] = 0;
		}
	}
}

vtss_rc vtss_mac_table_read(const uint idx,
                            vtss_mac_table_entry_t * const entry)
{
	vtss_pgid_no_t pgid_no;
	
	if (idx < VTSS_MAC_ADDR_START || idx > vtss_mac_state.mac_addrs) {
		vtss_log(VTSS_LOG_ERR,
			 "SWITCH: illegal idx, idx=%d", idx);
		return VTSS_INVALID_PARAMETER;
	}
	
	VTSS_RC(vtss_ll_mac_table_read(idx, entry, &pgid_no));
	
	/* Update entry->destination list based on pgid_no. */
	vtss_mac_entry_destination_set(entry, pgid_no);
	return VTSS_OK;
}

vtss_rc vtss_mac_table_lookup(const vtss_vid_mac_t * const vid_mac,
                              vtss_mac_table_entry_t * const entry)
{
	vtss_pgid_no_t pgid_no;
	
	entry->vid_mac = *vid_mac;
	VTSS_RC(vtss_ll_mac_table_lookup(entry, &pgid_no));
	
	vtss_mac_entry_destination_set(entry, pgid_no);
	return VTSS_OK;
}

vtss_rc vtss_mac_table_status_read(void) 
{
	vtss_mac_table_status_t st;
	
	/* Read and clear sticky bits */
	VTSS_RC(vtss_ll_mac_table_status_get(&st));
	
	/* Save API state */
	if (st.learned) {
		vtss_mac_state.mac_status_appl.learned = 1;
		vtss_mac_state.mac_status_next.learned = 1;
		vtss_mac_state.mac_status_sync.learned = 1;
	}
	if (st.replaced) {
		vtss_mac_state.mac_status_appl.replaced = 1;
		vtss_mac_state.mac_status_next.replaced = 1;
		vtss_mac_state.mac_status_sync.replaced = 1;
	}
	if (st.moved) {
		vtss_mac_state.mac_status_appl.moved = 1;
		vtss_mac_state.mac_status_next.moved = 1;
		vtss_mac_state.mac_status_sync.moved = 1;
	}
	if (st.aged) {
		vtss_mac_state.mac_status_appl.aged = 1;
		vtss_mac_state.mac_status_next.aged = 1;
		vtss_mac_state.mac_status_sync.aged = 1;
	}
	return VTSS_OK;
}

vtss_rc vtss_mac_table_status_get(vtss_mac_table_status_t * const status) 
{
	VTSS_RC(vtss_mac_table_status_read());
	
	/* Use API event state */
	if (vtss_mac_state.mac_status_appl.learned) {
		vtss_mac_state.mac_status_appl.learned = 0;
		status->learned = 1;
	}
	if (vtss_mac_state.mac_status_appl.replaced) {
		vtss_mac_state.mac_status_appl.replaced = 0;
		status->replaced = 1;
	}
	if (vtss_mac_state.mac_status_appl.moved) {
		vtss_mac_state.mac_status_appl.moved = 0;
		status->moved = 1;
	}
	if (vtss_mac_state.mac_status_appl.aged) {
		vtss_mac_state.mac_status_appl.aged = 0;
		status->aged = 1;
	}
	return VTSS_OK;
}

vtss_rc vtss_mac_table_get_next(const vtss_vid_mac_t * const   vid_mac,
                                vtss_mac_table_entry_t * const entry)
{
	vtss_rc          rc = VTSS_ENTRY_NOT_FOUND;
#if (CONFIG_VTSS_MAC_NEXT_MAX != 0)
	ulong            mach, macl;
	vtss_mac_entry_t *cur;
	vtss_vid_mac_t   vid_mac_next;
#endif
	
#if (CONFIG_VTSS_MAC_NEXT_MAX != 0)
	vtss_mach_macl_get(vid_mac, &mach, &macl);
	
	for (cur = vtss_mac_table_get(mach, macl, 1); cur != NULL; cur = cur->next) {
		/* Lookup in chip */
		vid_mac_next.vid = ((cur->mach >> 16) & 0xFFFF);
		vid_mac_next.mac.addr[0] = (uchar)((cur->mach >> 8) & 0xFF);
		vid_mac_next.mac.addr[1] = (uchar)(cur->mach & 0xFF);
		vid_mac_next.mac.addr[2] = (uchar)((cur->macl >> 24) & 0xFF);
		vid_mac_next.mac.addr[3] = (uchar)((cur->macl >> 16) & 0xFF);
		vid_mac_next.mac.addr[4] = (uchar)((cur->macl >> 8) & 0xFF);
		vid_mac_next.mac.addr[5] = (uchar)(cur->macl & 0xFF);
		if ((rc = vtss_mac_table_lookup(&vid_mac_next, entry)) == VTSS_OK)
			break;
	}
#endif
	
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	{
		vtss_pgid_no_t         pgid_no;
		vtss_mac_table_entry_t mac_entry;
		
		/* Do get next operation in chip */
		if (vtss_ll_mac_table_get_next(vid_mac, &mac_entry, &pgid_no) == VTSS_OK) {
			vtss_mac_entry_destination_set(&mac_entry, pgid_no);
#if (CONFIG_VTSS_MAC_NEXT_MAX != 0)
			vtss_mach_macl_get(&mac_entry.vid_mac, &mach, &macl);
			if (rc != VTSS_OK || (mach < cur->mach || (mach == cur->mach && macl < cur->macl)))
#endif
				*entry = mac_entry;
			rc = VTSS_OK;
		}
	}
#endif /* SPARX_28 */    
	
	return rc;
}
