#ifndef __VTSS_CPU_H_INCLUDE__
#define __VTSS_CPU_H_INCLUDE__

#ifdef CONFIG_VTSS_ARCH_SPARX_28
#define VTSS_CPU_RX_QUEUES      4
#endif

#define VTSS_CPU_RX_QUEUE_START ((vtss_cpu_rx_queue_t)1)
#define VTSS_CPU_RX_QUEUE_END   (VTSS_CPU_RX_QUEUE_START+VTSS_CPU_RX_QUEUES)

/* - Packet state definitions -------------------------------------- */
#define VTSS_FCS_SIZE 4 /* MAC frame CRC size */

/* CPU queue Number */
typedef uint vtss_cpu_rx_queue_t;   /* 1..VTSS_CPU_RX_QUEUES */

/* System frame header describing received frame */
typedef struct _vtss_system_frame_header_t {
	uint                   length;         /* Frame length excluding CRC */
	vtss_port_no_t         source_port_no; /* Source port number */
	BOOL                   arrived_tagged; /* TRUE is frame was tagged */
	vtss_tci_t             tag;            /* VLAN tag from frame or VLAN port setup */
#ifdef VTSS_FEATURE_CPU_RX_LEARN
	BOOL                   learn;          /* TRUE if learn frame */
#endif
} vtss_system_frame_header_t;

/* Information about frame */
typedef struct _vtss_cpu_frame_info_t {
	vtss_port_no_t port_no; /* Ingress port (or zero) */
	vtss_vid_t     vid;     /* Egress VID (or VTSS_VID_NULL) */
	vtss_port_no_t port_tx; /* Egress port (or zero) */
} vtss_cpu_frame_info_t;

/* CPU filter */
typedef enum {
	VTSS_CPU_FILTER_DISCARD, /* Discard */
	VTSS_CPU_FILTER_TAGGED,  /* Tagged transmission */
	VTSS_CPU_FILTER_UNTAGGED /* Untagged transmission */
} vtss_cpu_filter_t;

#ifdef CONFIG_VTSS_ARCH_SPARX_28
typedef struct {
	BOOL                       used;         /* Packet used flag */
	uint                       words_read;   /* Number of words read */
	vtss_system_frame_header_t sys_header;   /* Rx frame header */ 
} vtss_packet_rx_t;
#endif

/* CPU frame registration */
typedef struct _vtss_cpu_rx_registration_t {
	BOOL bpdu_cpu_only;         /* Redirect BPDUs (DMAC 01-80-C2-00-00-0X) */
	BOOL ipmc_ctrl_cpu_copy;    /* Copy IP MC control (DIP 224.0.0.x) to CPU */
	BOOL mcast_igmp_cpu_only;   /* Redirect IGMP frames to the CPU */
} vtss_cpu_rx_registration_t;

#if (VTSS_CPU_RX_QUEUES > 1)
/* CPU Rx queue map */
typedef struct _vtss_cpu_rx_queue_map_t {
#if defined(CONFIG_VTSS_ARCH_HEATHROW) || defined(CONFIG_VTSS_ARCH_SPARX)
	vtss_cpu_rx_queue_t bpdu_queue;      /* BPDUs */
	vtss_cpu_rx_queue_t garp_queue;      /* GARP frames */
#endif
	vtss_cpu_rx_queue_t learn_queue;     /* Learn frames */
	vtss_cpu_rx_queue_t igmp_queue;      /* IGMP/MLD frames */
	vtss_cpu_rx_queue_t ipmc_ctrl_queue; /* IP multicast control frames */
	vtss_cpu_rx_queue_t mac_vid_queue;   /* MAC address table */
} vtss_cpu_rx_queue_map_t;

#ifdef CONFIG_VTSS_ARCH_SPARX_28
/* CPU Rx queue buffer size in bytes */
typedef uint vtss_cpu_rx_queue_size_t;

/* For SparX-28, the total queue size is 48*1024 (48 kB).
   The default configuration is:
   Queue 1: 32 kB
   Queue 2: 16 kB
   Queue 3:  0 kB
   Queue 4:  0 kB */
#define VTSS_CPU_RX_QUEUE_SIZE_LOW  (16*1024) /* 16 kB */
#define VTSS_CPU_RX_QUEUE_SIZE_HIGH (32*1024) /* 32 kB */
#endif
#endif

typedef struct {
	/* --- Packet state --- */
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	vtss_packet_rx_t         rx_packet[VTSS_CPU_RX_QUEUES];
#endif
	vtss_cpu_rx_registration_t rx_registration;
} vtss_cpu_state_t;

/* - Rx frame registration ----------------------------------------- */

#if (VTSS_CPU_RX_QUEUES > 1)
/******************************************************************************
 * Description: Set CPU Rx queue map.
 *
 * \param map (input): CPU Rx queue map structure.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_cpu_rx_queue_map_set(vtss_cpu_rx_queue_map_t * const map);

#ifdef CONFIG_VTSS_ARCH_SPARX_28
/******************************************************************************
 * Description: Set CPU Rx queue size.
 *
 * \param queue_no (input): CPU queue number.
 * \param size (input)    : CPU queue size in bytes.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_cpu_rx_queue_size_set(const vtss_cpu_rx_queue_t      queue_no, 
                                   const vtss_cpu_rx_queue_size_t size);
#endif /* HAWX/SPARX_28 */    
#endif /* VTSS_CPU_RX_QUEUES>1 */

/******************************************************************************
 * Description: Get CPU frame registration.
 *
 * \param registration (output): Frame registration structure.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_cpu_rx_registration_get(vtss_cpu_rx_registration_t * const registration);


/******************************************************************************
 * Description: Set CPU frame registration.
 *
 * \param registration (input): Frame registration structure.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_cpu_rx_registration_set(const vtss_cpu_rx_registration_t * const registration);


/* - Rx frames ----------------------------------------------------- */

/******************************************************************************
 * Description: Enable interrupt if a frame is ready in a CPU queue.
 *
 * \param queue_no (input): CPU queue number.
 * \param enable (input)  : Boolean, TRUE to enable interrupt.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_cpu_rx_frameready_int_enable(const vtss_cpu_rx_queue_t queue_no,
                                          const BOOL enable);

/******************************************************************************
 * Description: Determine if a frame is ready in a CPU queue.
 *
 * \param queue_no (input): CPU queue number.
 *
 * \return : Negative value (error), 0 (no frame ready) or 1 (frame ready).
 ******************************************************************************/
vtss_rc vtss_cpu_rx_frameready(const vtss_cpu_rx_queue_t queue_no);


/******************************************************************************
 * Description: Copy a received frame from a CPU queue. If the frame buffer
 *              is NULL or too small, the system header is filled out and 
 *              VTSS_PACKET_BUF_SMALL is returned. The application can then 
 *              allocate a buffer and call this function again.
 *
 * \param queue_no (input)   : CPU queue number.
 * \param sys_header (output): Frame header.
 * \param frame (output)     : Frame buffer. 
 * \param maxlength (input)  : Length of frame buffer.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_cpu_rx_frame(const vtss_cpu_rx_queue_t          queue_no,
                          vtss_system_frame_header_t * const sys_header,
                          uchar * const                      frame,
                          const uint                         maxlength);


/******************************************************************************
 * Description: Discard a received frame from a CPU queue.
 *
 * \param queue_no (input)   : CPU queue number.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_cpu_rx_discard_frame(const vtss_cpu_rx_queue_t queue_no);


/* - Tx frames ----------------------------------------------------- */

/******************************************************************************
 * Description: Send frame unmodified on port/aggregation.
 *
 * \param poag_no (input): Port/aggregation number.
 * \param frame (input)  : Frame buffer excluding room for CRC.
 * \param length (input) : Frame length excluding CRC.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_cpu_tx_raw_frame(const vtss_poag_no_t poag_no,
                              const uchar * const  frame,
                              const uint           length);


/******************************************************************************
 * Description: Send frame on port/aggregation applying VLAN, STP and 
 *              aggregation rules.
 *
 * \param poag_no (input): Port/aggregation number.
 * \param vid (input)    : VLAN ID inserted if the frame is tagged on egress.
 * \param frame (input)  : Frame buffer excluding room for CRC.
 * \param length (input) : Frame length excluding CRC.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_cpu_tx_poag_frame(const vtss_poag_no_t poag_no,
                               const vtss_vid_t    vid,
                               const uchar * const frame,
                               const uint          length);


/******************************************************************************
 * Description: Send frame on port/aggregation applying VLAN, STP, 
 *              aggregation rules and (DMAC, VID) lookup.
 *
 * \param vid (input)   : VLAN ID inserted if the frame is tagged on egress.
 * \param frame (input) : Frame buffer excluding room for CRC.
 * \param length (input): Frame length excluding CRC.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_cpu_tx_vlan_frame(const vtss_vid_t    vid,
                               const uchar * const frame,
                               const uint          length);

/******************************************************************************
 * Description: Determine ingress/egress filter for frame. The function may be 
 *              used for ingress filtering, egress filtering or both.
 *
 * \param info (input)   : Frame information.
 * \param filter (output): Frame filter.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_cpu_filter(const vtss_cpu_frame_info_t * const info,
                        vtss_cpu_filter_t * const           filter);

/* Enable/disable frame ready interrupt */
vtss_rc vtss_ll_frame_rx_ready_int(vtss_cpu_rx_queue_t queue_no, BOOL enable);

#if (VTSS_CPU_RX_QUEUES > 1)
/* Setup Rx queue map */
vtss_rc vtss_ll_cpu_rx_queue_map_set(vtss_cpu_rx_queue_map_t * const map);
#endif

#ifdef CONFIG_VTSS_ARCH_SPARX_28
/* Set Rx queue size */
vtss_rc vtss_ll_cpu_rx_queue_size_set(const vtss_cpu_rx_queue_t queue_no, 
                                      const vtss_cpu_rx_queue_size_t size);
#endif
/* Set frame registration */
vtss_rc vtss_ll_frame_reg_set(const vtss_cpu_rx_registration_t *reg);
/* determine if a frame is ready in the CPU Rx queue */
/* returns: 0: no frame ready, <0: error, >0: frame ready */
vtss_rc vtss_ll_frame_rx_ready(vtss_cpu_rx_queue_t queue_no);
/* get frame from CPU Rx queue */
vtss_rc vtss_ll_frame_rx(vtss_cpu_rx_queue_t queue_no, vtss_system_frame_header_t *sys_header,
                         uchar *frame, uint maxlength);
/* discard frame from CPU Rx queue */
vtss_rc vtss_ll_frame_rx_discard(vtss_cpu_rx_queue_t queue_no);
/* transmit frame on port, optionally with tag */
vtss_rc vtss_ll_frame_tx(vtss_port_no_t port_no, const uchar *frame, uint length, 
                         vtss_vid_t vid);

#ifdef CONFOG_VTSS_CPU_FRAME
vtss_rc vtss_cpu_start(void);
#else
static inline vtss_rc vtss_cpu_start(void) { return VTSS_OK; }
#endif

#endif /* __VTSS_CPU_H_INCLUDE__ */
