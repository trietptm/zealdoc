#ifndef __VTSS_SWITCH_H_INCLUDE__
#define __VTSS_SWITCH_H_INCLUDE__

/* - Port Numbers -------------------------------------------------- */
/* Port Number: 1..VTSS_PORTS */
typedef uint vtss_port_no_t;    /* VTSS_PORT_NO_START..(VTSS_PORT_NO_END-1) */

/* - MAC interface ------------------------------------------------- */
/* The different interfaces for connecting MAC and PHY. */
typedef enum _vtss_port_interface_t {
	VTSS_PORT_INTERFACE_NO_CONNECTION, /* No connection */
	VTSS_PORT_INTERFACE_LOOPBACK,      /* Internal loopback in MAC */
	VTSS_PORT_INTERFACE_INTERNAL,      /* Internal interface */
	VTSS_PORT_INTERFACE_MII,           /* MII (RMII does not exist) */
	VTSS_PORT_INTERFACE_GMII,          /* GMII */
	VTSS_PORT_INTERFACE_RGMII,         /* RGMII */
	VTSS_PORT_INTERFACE_TBI,           /* TBI */
	VTSS_PORT_INTERFACE_RTBI,          /* RTBI */
	VTSS_PORT_INTERFACE_SGMII,         /* SGMII */
	VTSS_PORT_INTERFACE_SERDES,        /* SERDES */
	VTSS_PORT_INTERFACE_VAUI,          /* VAUI */
	VTSS_PORT_INTERFACE_XGMII          /* XGMII */
} vtss_port_interface_t;

/* - Speed --------------------------------------------------------- */
/* Speed type */
typedef enum _vtss_speed_t {
	VTSS_SPEED_UNDEFINED,
	VTSS_SPEED_10M,
	VTSS_SPEED_100M,
	VTSS_SPEED_1G,
	VTSS_SPEED_2500M, /* 2.5G */
	VTSS_SPEED_5G,    /* 5G or 2x2.5G */
	VTSS_SPEED_10G
} vtss_speed_t;

/* - Port Status --------------------------------------------------- */
typedef struct _vtss_port_status_t {
	vtss_event_t link_down;  /* link down event occurred since last call */
	BOOL         link;       /* link is up. Remaining fields only valid if TRUE */
	vtss_speed_t speed;      /* speed */
	BOOL         fdx;        /* full duplex */
	
	/* auto negotiation result */
	struct {
		BOOL obey_pause;     /* this port should obey PAUSE frames */
		BOOL generate_pause; /* link partner obeys PAUSE frames */
	} aneg;
} vtss_port_status_t;

/* ================================================================= *
 *  General definitions, macros, types, variables and functions
 * ================================================================= */

/* - VLAN and 802.1Q Tag ------------------------------------------- */

/* VLAN Identifier */
typedef uint vtss_vid_t; /* 0-4095 */

#define VTSS_VID_NULL     ((const vtss_vid_t)0)     /* NULL VLAN ID */
#define VTSS_VID_DEFAULT  ((const vtss_vid_t)1)     /* Default VLAN ID */
#define VTSS_VID_RESERVED ((const vtss_vid_t)0xFFF) /* Reserved VLAN ID */
#define VTSS_VIDS         ((const vtss_vid_t)4096)  /* Number of VLAN IDs */
/* For use in vtss_vlan_port_mode_t.untagged_vid field only: */
#define VTSS_VID_ALL      ((const vtss_vid_t)0x1000)/* All VLAN IDs */

/* Tag Priority */
typedef uint vtss_tagprio_t;   /* 0-7 */

/* Tag Control Information (according to IEEE 802.1Q) */
typedef struct _vtss_tci_t {
	vtss_vid_t     vid;     /* VLAN ID */
	BOOL           cfi;     /* Canonical Format Indicator */
	vtss_tagprio_t tagprio; /* Tag priority */
} vtss_tci_t;

/* MAC Address */
typedef struct _vtss_mac_t {
	uchar addr[6]; /* Network byte order */
} vtss_mac_t;

/* For easy setup of structures */
#define VTSS_MAC_NULL {{0,0,0,0,0,0}}

/* MAC Address in specific VLAN */
typedef struct _vtss_vid_mac_t {
	vtss_vid_t  vid; /* VLAN ID */
	vtss_mac_t  mac; /* MAC address */
} vtss_vid_mac_t;

/* For easy setup of structures */
#define VTSS_VID_MAC_NULL {VTSS_VID_NULL,VTSS_MAC_NULL}

/* Ethernet Type */
typedef ushort vtss_etype_t; 

/* DSCP */
typedef uchar vtss_dscp_t;

/* IP address/mask */
typedef ulong vtss_ip_t;

/* UDP/TCP port number */
typedef ushort vtss_udp_tcp_t;

/******************************************************************************
 * Description: Return text string corresponding to return code.
 *
 * \param rc (input): Return code.
 *
 * \return : Return code text.
 ******************************************************************************/
const char *vtss_error_txt(vtss_rc rc);

/* I/O architecture */
#ifdef CONFIG_VTSS_ARCH_HEATHROW
/* Heathrow chip i/o architecture */
#define VTSS_IO_HEATHROW 1
#endif

/* I/O layer state information */
typedef struct _vtss_mac_io_state_t {
#ifdef CONFIG_VTSS_VITGENIO
	int   fd; /* File descriptor */
#endif
	int dummy;
} vtss_mac_io_state_t;

/* Initialize I/O Layer, must be called before any other function */
void vtss_io_start(void);

#ifdef VTSS_IO_HEATHROW
vtss_rc vtss_io_si_rd(uint block, uint subblock, const uint reg, ulong * const value);
vtss_rc vtss_io_si_wr(uint block, uint subblock, const uint reg, const ulong value);
vtss_rc vtss_io_pi_rd(uint block, uint subblock, const uint reg, ulong * const value);
vtss_rc vtss_io_pi_wr(uint block, uint subblock, const uint reg, const ulong value);
#endif

/* ================================================================= *
 *  Supplemental
 * ================================================================= */

/* - Initialization ------------------------------------------------ */
/* Initialization setup */
typedef struct _vtss_init_setup_t {
	/* TRUE if chip must be reset (recommended) */
	BOOL reset_chip;
	/* TRUE if CPU Serial Interface is used.
	 * FALSE if CPU Parallel Interface is used.
	 */
	BOOL use_cpu_si;
} vtss_init_setup_t;

/******************************************************************************
 * Description: Initialize chip and API.
 *
 * \param setup (input): Pointer to initialization setup.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_api_start(const vtss_init_setup_t *const setup);

/* - Chip ID and revision ------------------------------------------ */
/* Chip ID */
typedef struct _vtss_chipid_t {
	ushort  part_number; /* BCD encoded part number */
	uint    revision;    /* Chip revision */
} vtss_chipid_t;

/******************************************************************************
 * Description: Get chip ID and revision.
 *
 * \param chipid (output): Pointer to chip ID structure.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_chipid_get(vtss_chipid_t * const chipid);

/* - Optimization functions ---------------------------------------- */
/******************************************************************************
 * Description: Optimization function called every second.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_optimize_1sec(void);

/******************************************************************************
 * Description: Optimization function called every 100th millisecond.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_optimize_100msec(void);

/* - Serial LED ---------------------------------------------------- */
#ifdef VTSS_FEATURE_SERIAL_LED
/* LED mode */
typedef enum {
	VTSS_LED_MODE_DISABLED, /* Disabled */
	VTSS_LED_MODE_OFF,      /* Off */
	VTSS_LED_MODE_ON,       /* On */
	VTSS_LED_MODE_2_5,      /* 2.5 Hz */
	VTSS_LED_MODE_5,        /* 5 Hz */
	VTSS_LED_MODE_10,       /* 10 Hz */
	VTSS_LED_MODE_20        /* 20 Hz */
} vtss_led_mode_t;

/* LED port number */
typedef uint vtss_led_port_t;

/******************************************************************************
 * Description: Setup serial LED mode.
 *
 * \param port (input): Serial LED port, 0-29 or 0-15 (G_ROCX).
 * \param mode (input): Serial LED mode for three LEDs.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_serial_led_set(const vtss_led_port_t port, 
                            const vtss_led_mode_t mode[3]);
#endif

/* - Direct register access (for debugging only) ------------------- */

/******************************************************************************
 * Description: Read value from target register.
 *
 * \param chip_no (input): Chip number (for multi chip targets only).
 * \param reg (input)    : Target ID (bit 8-15) and address (bit 0-7) to read.
 * \param value (output) : Register value.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_register_read(const ulong reg, ulong * const value);

/******************************************************************************
 * Description: Write value to target register.
 *
 * \param chip_no (input): Chip number (for multi chip targets only).
 * \param reg (input)    : Target ID (bit 8-15) and address (bit 0-7) to read.
 * \param value (input)  : Register value.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_register_write(const ulong reg, 
                            const ulong value);

/******************************************************************************
 * Description: Read, modify and write value to target register.
 *
 * \param chip_no (input): Chip number (for multi chip targets only).
 * \param reg (input)    : Target ID (bit 8-15) and address (bit 0-7) to read.
 * \param value (input)  : Register value.
 * \param mask (input)   : Register mask, only bits enabled are changed.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_register_writemasked(const ulong reg, 
                                  const ulong value, 
                                  const ulong mask);

/* ================================================================= *
 *  Port Control
 * ================================================================= */
/* Number of ports may optionally be less than number of chip ports */
#define VTSS_PORTS CONFIG_VTSS_PORT_COUNT

/* The first logical port number is 1. */
#define VTSS_PORT_NO_START   ((vtss_port_no_t)1) 
#define VTSS_PORT_NO_END     (VTSS_PORT_NO_START+VTSS_PORTS)
#define VTSS_PORT_ARRAY_SIZE VTSS_PORT_NO_END

#define VTSS_PORT_IS_PORT(x) (x>=VTSS_PORT_NO_START && x<VTSS_PORT_NO_END)

/* Port or Aggregation Number identifying a port or aggregation.
   The same Aggregation Number is shared by all the ports in an aggregation.
   Non-aggregated ports use their port_no. */
typedef vtss_port_no_t vtss_poag_no_t; /* not aggregated: port_no, aggregated: VTSS_AGGR_NO_START..(VTSS_AGGR_NO_END-1) */

/* Number of aggregations and number of ports/aggregations */
#define VTSS_AGGRS           (VTSS_PORTS/2)
#define VTSS_POAGS           (VTSS_PORTS+VTSS_AGGRS)

#define VTSS_AGGR_NO_START   ((vtss_poag_no_t)VTSS_PORT_NO_END)
#define VTSS_AGGR_NO_END     (VTSS_AGGR_NO_START+VTSS_AGGRS)
#define VTSS_POAG_NO_START   VTSS_PORT_NO_START
#define VTSS_POAG_NO_END     VTSS_AGGR_NO_END
#define VTSS_POAG_ARRAY_SIZE VTSS_AGGR_NO_END
#define VTSS_POAG_IS_POAG(x) ((x>=VTSS_POAG_NO_START) && (x<VTSS_POAG_NO_END))

#define VTSS_POAG_IS_PORT(x) (x<VTSS_AGGR_NO_START)
#define VTSS_POAG_IS_AGGR(x) (x>=VTSS_AGGR_NO_START)

/* MII Management controller */
typedef enum _vtss_miim_controller_t {
#ifdef CONFIG_VTSS_ARCH_SPARX
	VTSS_MIIM_CONTROLLER_0    = 0, /* MIIM controller 0 (internal), clause 22 only */
	VTSS_MIIM_CONTROLLER_1    = 1, /* MIIM controller 1 (external), clause 22 only */
#endif
	VTSS_MIIM_CONTROLLERS,         /* Number of MIIM controllers */
	VTSS_MIIM_CONTROLLER_NONE = -1 /* Unassigned MIIM controller */
} vtss_miim_controller_t;


/* - Port mapping -------------------------------------------------- */

typedef struct _vtss_mapped_port_t {
	int                    chip_port;       /* Set to -1 if not used */
	vtss_miim_controller_t miim_controller; /* MII Management controller */
	/* PHY address, ignored for VTSS_MIIM_CONTROLLER_NONE */
	int                    phy_addr;
} vtss_mapped_port_t;

/******************************************************************************
 * Description: Setup port mapping.
 *
 * \param mapped_ports (input): Port map array indexed by vtss_port_no_t.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_port_map_set(
    const vtss_mapped_port_t mapped_ports[VTSS_PORT_ARRAY_SIZE]);


/* - IEEE 802.3 clause 22 PHY access functions --------------------- */

/******************************************************************************
 * Description: Read value from PHY register.
 *
 * \param chip_no (input)        : Chip number (for multi chip targets only).
 * \param miim_controller (input): MII Management controller.
 * \param addr (input)           : PHY address on MIIM bus.
 * \param reg (input)            : PHY register address.
 * \param value (output)         : PHY register value.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_miim_phy_read(const vtss_miim_controller_t miim_controller,
                           const uint                   addr,
                           const uint                   reg,
                           ushort *const                value);

/******************************************************************************
 * Description: Write value to PHY register.
 *
 * \param chip_no (input)        : Chip number (for multi chip targets only).
 * \param miim_controller (input): MII Management controller.
 * \param addr (input)           : PHY address on MIIM bus.
 * \param reg (input)            : PHY register address.
 * \param value (input)          : PHY register value.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_miim_phy_write(const vtss_miim_controller_t miim_controller,
                            const uint                   addr,
                            const uint                   reg,
                            const ushort                 value);

/******************************************************************************
 * Description: Read, modify and write value to PHY register.
 *
 * \param chip_no (input)        : Chip number (for multi chip targets only).
 * \param miim_controller (input): MII Management controller.
 * \param phy_addr (input)       : PHY address on MIIM bus.
 * \param phy_reg (input)        : PHY register address.
 * \param value  (input)         : Register value (16 bit).
 * \param mask (input)           : Register mask, only bits enabled are changed.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_miim_phy_writemasked(const vtss_miim_controller_t miim_controller,
                                  const uint                   addr,
                                  const uint                   reg,
                                  const ushort                 value,
                                  const ushort                 mask);

/******************************************************************************
 * Description: Read value from PHY register.
 *
 * \param chip_no (input)        : Chip number (for multi chip targets only).
 * \param miim_controller (input): MII Management controller.
 * \param phy_addr (input)       : PHY address on MIIM bus.
 * \param phy_reg (input)        : PHY register (page and) address.
 *
 * \return : If negative, a vtss_rc_t error code. Else 16-bit register value.
 ******************************************************************************/
vtss_rc vtss_register_phy_read(const vtss_miim_controller_t miim_controller,
                               const uint                   phy_addr,
                               const uint                   phy_reg,
                               ushort *const                value);


/******************************************************************************
 * Description: Write value to PHY register.
 *
 * \param chip_no (input)        : Chip number (for multi chip targets only).
 * \param miim_controller (input): MII Management controller.
 * \param phy_addr (input)       : PHY address on MIIM bus.
 * \param phy_reg (input)        : PHY register (page and) address.
 * \param value  (input)         : Register value (16 bit).
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_register_phy_write(const vtss_miim_controller_t miim_controller,
                                const uint                   phy_addr,
                                const uint                   phy_reg,
                                const ushort                 value);


/******************************************************************************
 * Description: Read, modify and write value to PHY register.
 *
 * \param chip_no (input)        : Chip number (for multi chip targets only).
 * \param miim_controller (input): MII Management controller.
 * \param phy_addr (input)       : PHY address on MIIM bus.
 * \param phy_reg (input)        : PHY register (page and) address.
 * \param value  (input)         : Register value (16 bit).
 * \param mask (input)           : Register mask, only bits enabled are changed.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_register_phy_writemasked(const vtss_miim_controller_t miim_controller,
                                      const uint                   phy_addr,
                                      const uint                   phy_reg,
                                      const ushort                 value,
                                      const ushort                 mask);

/* - Port configuration and reset ---------------------------------- */

/* Interface type and speed */
typedef struct _vtss_port_interface_mode_t {
    vtss_port_interface_t interface_type; 
    vtss_speed_t          speed;
} vtss_port_interface_mode_t;

/* Flow control */
typedef struct _vtss_flowcontrol_setup_t {
    BOOL       obey;     /* TRUE if PAUSE frames should be obeyed */
    BOOL       generate; /* TRUE if PAUSE frames should generated */
    vtss_mac_t smac;     /* Port MAC address used as SMAC in PAUSE frames */
} vtss_flowcontrol_setup_t;

/* Use this value for all frame gaps to get default values */
#define VTSS_FRAME_GAP_DEFAULT 0

/* Inter frame gap structure */
typedef struct _vtss_frame_gaps_t {
	uint hdx_gap_1;        /* Half duplex: First part of Rx to Tx gap */
	uint hdx_gap_2;        /* Half duplex: Second part of Rx to Tx gap */
	uint fdx_gap;          /* Full duplex: Tx to Tx gap */
} vtss_frame_gaps_t;

/* A selection of max frame lengths */
#define VTSS_MAXFRAMELENGTH_STANDARD 1518    /* IEEE 802.3 standard */
#define VTSS_MAXFRAMELENGTH_TAGGED   (VTSS_MAXFRAMELENGTH_STANDARD+4)

#ifdef CONFIG_VTSS_ARCH_SPARX
#define VTSS_MAXFRAMELENGTH_MAX      9600
#endif

/* VLAN awareness for frame length check */
typedef enum {
	VTSS_LENGTH_TAG_NONE,   /* No extra tags allowed */
	VTSS_LENGTH_TAG_SINGLE, /* Single tag allowed */
	VTSS_LENGTH_TAG_DOUBLE  /* Single and double tag allowed */
} vtss_length_check_t;

/* Port setup, which may change dynamically, e.g. after auto-negotiation */
typedef struct _vtss_port_setup_t {
	vtss_port_interface_mode_t interface_mode; /* Interface type and speed */
	BOOL                       powerdown;      /* Disable and power down the port */
	BOOL                       fdx;            /* TRUE if full duplex */
	vtss_flowcontrol_setup_t   flowcontrol;    /* Flow control setup */
	uint                       maxframelength; /* Maximum frame length */
	vtss_length_check_t        length_check;   /* VLAN awareness for length check */
	vtss_frame_gaps_t          frame_gaps;     /* Not XGMII: Interframe gaps */
	
#ifdef VTSS_FEATURE_EXC_COL_CONT
	BOOL                       exc_col_cont;   /* Excessive collision continuation */
#endif
} vtss_port_setup_t;

/******************************************************************************
 * Description: Setup port.
 *
 * \param port_no (input): Port number.
 * \param setup (input)  : Port setup structure.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_port_setup(const vtss_port_no_t            port_no,
                        const vtss_port_setup_t * const setup);


/* - Port Link Status ---------------------------------------------- */

/******************************************************************************
 * Description: Get port status.
 *
 * \param port_no (input): Port number.
 * \param setup (output) : Status structure.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_port_status_get(const vtss_port_no_t       port_no,
                             vtss_port_status_t * const status);


/* - Number of priorities (for QoS statistics) --------------------- */

#ifdef CONFIG_VTSS_ARCH_SPARX
#define VTSS_PRIOS  4
#define VTSS_QUEUES 4
#endif

/* - Counters ----------------------------------------------------- */

typedef struct _vtss_poag_counters_t {
	/* RMON counters (RFC 2819) */
	struct {
		/* Rx counters */
		vtss_counter_t rx_etherStatsDropEvents;
		vtss_counter_t rx_etherStatsOctets;
		vtss_counter_t rx_etherStatsPkts;
#ifdef VTSS_FEATURE_PORT_CNT_PKT_CAST
		vtss_counter_t rx_etherStatsBroadcastPkts;
		vtss_counter_t rx_etherStatsMulticastPkts;
#endif
#ifdef VTSS_FEATURE_PORT_CNT_RMON_ADV
		vtss_counter_t rx_etherStatsCRCAlignErrors;
		vtss_counter_t rx_etherStatsUndersizePkts;
		vtss_counter_t rx_etherStatsOversizePkts;
		vtss_counter_t rx_etherStatsFragments;
		vtss_counter_t rx_etherStatsJabbers;
		vtss_counter_t rx_etherStatsPkts64Octets;
		vtss_counter_t rx_etherStatsPkts65to127Octets;
		vtss_counter_t rx_etherStatsPkts128to255Octets;
		vtss_counter_t rx_etherStatsPkts256to511Octets;
		vtss_counter_t rx_etherStatsPkts512to1023Octets;
		vtss_counter_t rx_etherStatsPkts1024to1518Octets;
#endif
#ifdef VTSS_FEATURE_PORT_CNT_JUMBO
		vtss_counter_t rx_etherStatsPkts1519toMaxOctets;  /* Proprietary */
#endif
		/* Tx counters */
		vtss_counter_t tx_etherStatsDropEvents;
		vtss_counter_t tx_etherStatsOctets;
		vtss_counter_t tx_etherStatsPkts;
#ifdef VTSS_FEATURE_PORT_CNT_PKT_CAST
		vtss_counter_t tx_etherStatsBroadcastPkts;
		vtss_counter_t tx_etherStatsMulticastPkts;
#endif
		vtss_counter_t tx_etherStatsCollisions;
#ifdef VTSS_FEATURE_PORT_CNT_RMON_ADV
		vtss_counter_t tx_etherStatsPkts64Octets;
		vtss_counter_t tx_etherStatsPkts65to127Octets;
		vtss_counter_t tx_etherStatsPkts128to255Octets;
		vtss_counter_t tx_etherStatsPkts256to511Octets;
		vtss_counter_t tx_etherStatsPkts512to1023Octets;
		vtss_counter_t tx_etherStatsPkts1024to1518Octets;
#endif
#ifdef VTSS_FEATURE_PORT_CNT_JUMBO
		vtss_counter_t tx_etherStatsPkts1519toMaxOctets;  /* Proprietary */
#endif
	} rmon;
	
	/* Interfaces Group counters (RFC 2863) */ 
	struct {
		/* Rx counters */
		vtss_counter_t ifInOctets;
#ifdef VTSS_FEATURE_PORT_CNT_PKT_CAST
		vtss_counter_t ifInUcastPkts;
		vtss_counter_t ifInMulticastPkts;
		vtss_counter_t ifInBroadcastPkts;
		vtss_counter_t ifInNUcastPkts;
#endif
		vtss_counter_t ifInDiscards;
		vtss_counter_t ifInErrors;
		
		/* Tx counters */
		vtss_counter_t ifOutOctets;
#ifdef VTSS_FEATURE_PORT_CNT_PKT_CAST
		vtss_counter_t ifOutUcastPkts;
		vtss_counter_t ifOutMulticastPkts;
		vtss_counter_t ifOutBroadcastPkts;
		vtss_counter_t ifOutNUcastPkts;
#endif
		vtss_counter_t ifOutDiscards;
		vtss_counter_t ifOutErrors;
	} if_group;
	
#ifdef VTSS_FEATURE_PORT_CNT_BRIDGE
	/* Bridge counters (RFC 4188) */
	struct {
		vtss_counter_t dot1dTpPortInDiscards;
	} bridge;
#endif
	
#ifdef VTSS_FEATURE_PORT_CNT_QOS
	/* Proprietary counters */
	struct {
		vtss_counter_t rx_prio[VTSS_PRIOS];
		vtss_counter_t tx_prio[VTSS_PRIOS];
	} prop;
#endif
} vtss_poag_counters_t;

/******************************************************************************
 * Description: Update counters for port.
 *
 * \param port_no (input): Port number.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_port_counters_update(const vtss_port_no_t port_no);


/******************************************************************************
 * Description: Clear counters for port.
 *
 * \param poag_no (input): Port/aggregation number.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_poag_counters_clear(const vtss_poag_no_t poag_no);


/******************************************************************************
 * Description: Get counters for port.
 *
 * \param poag_no (input)  : Port/aggregation number.
 * \param counters (output): Counter structure.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_poag_counters_get(const vtss_poag_no_t poag_no,
                               vtss_poag_counters_t * const counters);

/* ================================================================= *
 *  Quality of Service                                                   
 * ================================================================= */

/* Priority number: 1..VTSS_PRIOS */
typedef uint vtss_prio_t; /* VTSS_PRIO_START..(VTSS_PRIO_END-1) */

/* Queue number: 1..VTSS_QUEUES */
typedef uint vtss_queue_t; /* VTSS_QUEUE_START..(VTSS_QUEUE_END-1) */

#define VTSS_PRIO_START ((vtss_prio_t)1)             /* Lowest priority */
#define VTSS_PRIO_END   (VTSS_PRIO_START+VTSS_PRIOS) 
#define VTSS_PRIO_ARRAY_SIZE VTSS_PRIO_END

#define VTSS_QUEUE_START ((vtss_queue_t)1)
#define VTSS_QUEUE_END   (VTSS_QUEUE_START+VTSS_QUEUES)
#define VTSS_QUEUE_ARRAY_SIZE VTSS_QUEUE_END

/* Policer/Shaper bit rate. 
   Multiply vtss_bitrate_t value by 1000 to get the rate in BPS. */
typedef ulong vtss_bitrate_t; 

/* Special value that disables policer/shaper feature */
#define VTSS_BITRATE_FEATURE_DISABLED   ((ulong)-1)

/* Policer packet rate in PPS */
typedef ulong vtss_packet_rate_t;

/* Special value for disabling packet policer */
#define VTSS_PACKET_RATE_DISABLED ((ulong)-1)

/* Weight for port WFQ: 1, 2, 4 or 8 */
typedef enum {
	VTSS_WEIGHT_1,
	VTSS_WEIGHT_2,
	VTSS_WEIGHT_4,
	VTSS_WEIGHT_8
} vtss_weight_t;

/* - Per Chip ------------------------------------------------------ */

/******************************************************************************
 * Description: Set the number of active priority queues/traffic classes.
 *
 * \param prios (input): Number of priorities.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_qos_prios_set(const vtss_prio_t prios);

/* All parameters below are defined per chip */
typedef struct _vtss_qos_setup_t {
#ifdef VTSS_FEATURE_QOS_DSCP_REMARK
	BOOL           dscp_remark[64];   /* DSCP remarking */
#endif
	
#ifdef VTSS_FEATURE_QOS_POLICER_CPU_SWITCH
	vtss_packet_rate_t policer_mac;   /* MAC table CPU policer */
	vtss_packet_rate_t policer_cat;   /* BPDU, GARP, IGMP, IP MC Control and MLD CPU policer */
	vtss_packet_rate_t policer_learn; /* Learn frame policer */
#endif
	
#ifdef VTSS_FEATURE_QOS_POLICER_UC_SWITCH
	vtss_packet_rate_t policer_uc;    /* Unicast packet policer */
#endif
	
#ifdef VTSS_FEATURE_QOS_POLICER_MC_SWITCH
	vtss_packet_rate_t policer_mc;    /* Multicast packet policer */
#endif
	
#ifdef VTSS_FEATURE_QOS_POLICER_BC_SWITCH
	vtss_packet_rate_t policer_bc;    /* Broadcast packet policer */
#endif
	
#ifdef CONFIG_VTSS_ARCH_HEATHROW
	ulong       dummy;              /* Unused. Ensures that struct is not empty */
#endif
} vtss_qos_setup_t;


/******************************************************************************
 * Description: Get QoS setup for switch.
 *
 * \param qos (output): QoS setup structure.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_qos_setup_get(vtss_qos_setup_t * const qos);


/******************************************************************************
 * Description: Set QoS setup for switch.
 *
 * \param qos (input): QoS setup structure.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_qos_setup_set(const vtss_qos_setup_t * const qos);


/* - Per Port ------------------------------------------------------ */

#ifdef VTSS_FEATURE_QCL_PORT
/* QCL ID type */
typedef uint vtss_qcl_id_t; 
#define VTSS_QCL_ID_NONE  0 /* Means QCLs disabled for port */
#define VTSS_QCL_ID_START 1
#define VTSS_QCL_IDS      (VTSS_PORTS)
#define VTSS_QCL_ID_END   (VTSS_QCL_ID_START+VTSS_QCL_IDS)
#define VTSS_QCL_ARRAY_SIZE VTSS_QCL_ID_END
#endif

#ifdef VTSS_FEATURE_QOS_DSCP_REMARK
/* DSCP mode for ingress port */
typedef enum {
	VTSS_DSCP_MODE_NONE, /* DSCP not remarked */
	VTSS_DSCP_MODE_ZERO, /* DSCP value zero remarked */
	VTSS_DSCP_MODE_SEL,  /* DSCP values selected above (dscp_remark) are remarked */
	VTSS_DSCP_MODE_ALL   /* DSCP remarked for all values */
} vtss_dscp_mode_t;
#endif

/* All parameters below are defined per port */
typedef struct _vtss_port_qos_setup_t {
	/* Basic classification mode */
#ifdef VTSS_FEATURE_QCL_PORT
	vtss_qcl_id_t  qcl_id;               /* QCL ID or VTSS_QCL_ID_NONE */
#endif
#if defined(VTSS_FEATURE_QOS_L4_PORT)
	BOOL           udp_tcp_enable;       /* Classification on UDP/TCP ports */
#endif
	BOOL           dscp_enable;          /* Classification on DSCP */
#ifdef VTSS_FEATURE_QOS_IP_TOS_PORT
	BOOL           tos_enable;           /* Classification on DSCP 3 MSBit - overrules dscp_enable */
#endif
	BOOL           tag_enable;           /* Classification on VLAN tag prio */
#ifdef VTSS_FEATURE_QOS_ETYPE_PORT
	BOOL           etype_enable;         /* Classification on Ethernet type */
#endif
#ifdef VTSS_FEATURE_QOS_L4_PORT
	vtss_udp_tcp_t udp_tcp_val[10];      /* UDP/TCP port numbers */
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	vtss_prio_t    udp_tcp_prio[10];     /* Priority for UDP/TCP port numbers */
#endif /* SPARX_28 */
#endif
	vtss_prio_t    dscp_prio[64];        /* Map from DSCP to priority */
	vtss_prio_t    tag_prio[8];          /* Map from VLAN tag prio. to prio. */
#ifdef VTSS_FEATURE_QOS_ETYPE_PORT
#ifdef CONFIG_VTSS_ARCH_SPARX
	ushort         etype_val;            /* Ethernet type value */
	vtss_prio_t    etype_prio;           /* Priority for Ethernet type value */
#endif
#endif
	vtss_prio_t    default_prio;         /* Default port priority */
#ifdef CONFIG_VTSS_ARCH_HEATHROW
	vtss_tagprio_t usr_prio;             /* Default ingress VLAN tag priority */
#endif
#ifdef VTSS_FEATURE_QOS_DSCP_REMARK
	vtss_dscp_mode_t dscp_mode;          /* Ingress DSCP mode */
	BOOL             dscp_remark;        /* Egress DSCP mode */
	vtss_dscp_t      dscp_map[VTSS_PRIO_ARRAY_SIZE]; /* Egress mapping from priority to DSCP */
#endif
#ifdef VTSS_FEATURE_QOS_TAG_REMARK
	BOOL             tag_remark;         /* Egress tag priority mode */
	vtss_tagprio_t   tag_map[VTSS_PRIO_ARRAY_SIZE]; /* Egress mapping from priority to tag priority */
#endif
#ifdef VTSS_FEATURE_QOS_POLICER_PORT
	vtss_bitrate_t policer_port;          /* Ingress port policer, all frames */
#endif
#ifdef VTSS_FEATURE_QOS_POLICER_CIR_PIR_QUEUE
	vtss_bitrate_t policer_cir_queue[VTSS_QUEUE_ARRAY_SIZE];/* Ingress policer, CIR */
	vtss_bitrate_t policer_pir_queue[VTSS_QUEUE_ARRAY_SIZE];/* Ingress policer, PIR */
#endif
#ifdef VTSS_FEATURE_QOS_SHAPER_PORT
	vtss_bitrate_t shaper_port;           /* Egress shaping */
#endif
#ifdef VTSS_FEATURE_QOS_WFQ_PORT
	/* Weighted fairness queueing */
	BOOL           wfq_enable; 
	vtss_weight_t  weight[VTSS_QUEUE_ARRAY_SIZE];
#endif
} vtss_port_qos_setup_t;

/******************************************************************************
 * Description: Get QoS setup for port.
 *
 * \param port_no (input): Port number.
 * \param qos (output)   : QoS setup structure.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_port_qos_get(const vtss_port_no_t port_no,
                          vtss_port_qos_setup_t * const qos);


/******************************************************************************
 * Description: Set QoS setup for port.
 *
 * \param port_no (input): Port number.
 * \param qos (input)    : QoS setup structure.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_port_qos_set(const vtss_port_no_t port_no,
                          const vtss_port_qos_setup_t * const qos);

/* - QoS Control Lists --------------------------------------------- */

#ifdef VTSS_FEATURE_QCL_PORT

/* QCE ID */
typedef ulong vtss_qce_id_t;
#define VTSS_QCE_ID_LAST 0 /* Special value used to add last in list */

/* QCE type */
typedef enum _vtss_qce_type_t {
	VTSS_QCE_TYPE_ETYPE,   /* Ethernet Type */
	VTSS_QCE_TYPE_VLAN,    /* VLAN ID */
	VTSS_QCE_TYPE_UDP_TCP, /* UDP/TCP port */
	VTSS_QCE_TYPE_DSCP,    /* IP DSCP */
	VTSS_QCE_TYPE_TOS,     /* IP ToS */
	VTSS_QCE_TYPE_TAG      /* VLAN Tag */
} vtss_qce_type_t;

/* QoS Control Entry */
typedef struct _vtss_qce_t {
	vtss_qce_id_t   id;
	vtss_qce_type_t type;
	
	union {
		/* Type VTSS_QCE_TYPE_ETYPE */
		struct {
			vtss_etype_t val;  /* Ethernet Type */ 
			vtss_prio_t  prio; /* Priority mapping */
		} etype;
		
		/* Type VTSS_QCE_TYPE_VLAN_ID */
		struct {
			vtss_vid_t  vid;  /* VLAN ID value */
			vtss_prio_t prio; /* Priority mapping */
		} vlan;
		
		/* Type VTSS_QCE_TYPE_UDP_TCP */
		struct {
			vtss_udp_tcp_t low;  /* UDP/TCP port range low value */
			vtss_udp_tcp_t high; /* UDP/TCP port range high value */
			vtss_prio_t prio;    /* Priority mapping */
		} udp_tcp;
		
		/* Type VTSS_QCE_TYPE_DSCP */
		struct {
			vtss_dscp_t dscp_val; /* DSCP value */
			vtss_prio_t prio; /* Priority mapping */
		} dscp;
		
		/* Type VTSS_QCE_TYPE_TOS */
		vtss_prio_t tos_prio[8]; /* ToS priority mapping */
		
		/* Type VTSS_QCE_TYPE_TAG */
		vtss_prio_t tag_prio[8]; /* Tag priority mapping */
	} frame;
} vtss_qce_t;

/******************************************************************************
 * Description: Add QCE to QCL.
 *
 * \param qcl_id (input): QCL ID. 
 * \param qce_id (input): QCE ID. The QCE will be added before the entry with 
 *                        this ID. VTSS_QCE_ID_LAST is reserved for inserting
 *                        last.
 * \param qce (input)   : QCE setup structure.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_qce_add(const vtss_qcl_id_t qcl_id,
                     const vtss_qce_id_t qce_id,
                     const vtss_qce_t * const qce);

/******************************************************************************
 * Description: Delete QCE from QCL.
 *
 * \param qcl_id (input): QCL ID.
 * \param qce_id (input): QCE ID.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_qce_del(const vtss_qcl_id_t qcl_id,
                     const vtss_qce_id_t qce_id);

#endif /* VTSS_FEATURE_QCL_PORT */

/* ================================================================= *
 *  Security
 * ================================================================= */

/* - Port Based Network Access Control, 802.1X --------------------- */

/* Authentication state */
typedef enum _vtss_auth_state_t {
    VTSS_AUTH_STATE_NONE,   /* Not authenticated */
    VTSS_AUTH_STATE_EGRESS, /* Authenticated in egress direction */
    VTSS_AUTH_STATE_BOTH    /* Authenticated in both directions */
} vtss_auth_state_t;

/******************************************************************************
 * Description: Set 802.1X Authentication state for a port.
 *
 * \param port_no (input)   : Port number.
 * \param auth_state (input): Authentication state.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_port_auth_state_set(const vtss_port_no_t port_no,
                                 const vtss_auth_state_t auth_state);


/* ================================================================= *
 *  Layer 2
 * ================================================================= */

/* - MAC address table --------------------------------------------- */

/* MAC address entry */
typedef struct _vtss_mac_table_entry_t {
    vtss_vid_mac_t vid_mac;                           /* VLAN ID and MAC addr */
    BOOL           destination[VTSS_POAG_ARRAY_SIZE]; /* Dest. ports */
    BOOL           copy_to_cpu;                       /* CPU copy flag */
    BOOL           locked;                            /* Locked/static flag */
    BOOL           aged;                              /* Age flag */
} vtss_mac_table_entry_t;

/******************************************************************************
 * Description: Learn MAC address entry. 
 *
 * \param entry (input): MAC address entry structure.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_mac_table_learn(const vtss_mac_table_entry_t * const entry);


/******************************************************************************
 * Description: Forget/unlearn MAC address entry. 
 *
 * \param vid_mac (input): VLAN ID and MAC address structure.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_mac_table_forget_vid_mac(const vtss_vid_mac_t * const vid_mac);

/* The following flush/age/forget function can only be used if unlocked 
   entries have been not been learned from the CPU. Such entries can only
   be removed using vtss_mac_table_forget_vid_mac. */


/******************************************************************************
 * Description: Flush MAC address table, i.e. remove all unlocked entries.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_mac_table_flush(void);

#ifdef VTSS_FEATURE_MAC_AGE_AUTO
typedef ulong vtss_mac_age_time_t;

/******************************************************************************
 * Description: Set MAC address table age time.
 *
 * \param age_time (input): MAC age time in seconds. Value zero disables aging. 
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_mac_table_age_time_set(const vtss_mac_age_time_t age_time);
#endif

/******************************************************************************
 * Description: Do age scan of the MAC address table. This should be done 
 *              periodically with interval T/2, where T is the age timer.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_mac_table_age(void);


/******************************************************************************
 * Description: Do VLAN specific age scan of the MAC address table.
 *              This can be used if the age time is VLAN specific.
 *
 * \param vid (input): VLAN ID.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_mac_table_age_vlan(const vtss_vid_t vid);


/******************************************************************************
 * Description: Forget MAC address entries learned on port.
 *
 * \param port_no (input): Port number.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_mac_table_forget_port(const vtss_port_no_t port_no);

/******************************************************************************
 * Description: Forget MAC address entries learned on VLAN ID.
 *
 * \param vid (input)    : VLAN ID.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_mac_table_forget_vlan(const vtss_vid_t vid);

/******************************************************************************
 * Description: Forget MAC address entries learned on port and VLAN ID.
 *
 * \param port_no (input): Port number.
 * \param vid (input)    : VLAN ID.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_mac_table_forget_port_in_vlan(const vtss_port_no_t port_no,
                                           const vtss_vid_t     vid);

/* Maximum number entries in the MAC address table */

#ifdef CONFIG_VTSS_ARCH_SPARX
#define VTSS_MAC_ADDRS 8192
#endif

#define VTSS_MAC_ADDR_START 1
#define VTSS_MAC_ADDR_END   (VTSS_MAC_ADDR_START+VTSS_MAC_ADDRS)

/******************************************************************************
 * Description: Read MAC address entry on a specific index.
 *
 * \param idx (input)   : MAC address table index.
 * \param entry (output): MAC address entry.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_mac_table_read(const uint                     idx,
                            vtss_mac_table_entry_t * const entry);


/******************************************************************************
 * Description: Lookup MAC address entry.
 *
 * \param vid_mac (input): VLAN ID and MAC address.
 * \param entry (output) : MAC address entry.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_mac_table_lookup(const vtss_vid_mac_t * const   vid_mac,
                              vtss_mac_table_entry_t * const entry);


/* MAC address table status */
typedef struct _vtss_mac_table_status_t {
	vtss_event_t learned;  /* One or more entries were learned */
	vtss_event_t replaced; /* One or more entries were replaced */
	vtss_event_t moved;    /* One or more entries moved to another port */
	vtss_event_t aged;     /* One or more entries were aged */
} vtss_mac_table_status_t;

/******************************************************************************
 * Description: Get MAC address table status.
 *
 * \param status (output): MAC address table status. 
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_mac_table_status_get(vtss_mac_table_status_t * const status);

/******************************************************************************
 * Description: Lookup next MAC address entry.
 *
 * \param vid_mac (input): VLAN ID and MAC address.
 * \param entry (output) : MAC address entry.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_mac_table_get_next(const vtss_vid_mac_t * const   vid_mac,
                                vtss_mac_table_entry_t * const entry);


/* - Learning options ---------------------------------------------- */

/* Learning mode */
typedef struct _vtss_learn_mode_t {
	BOOL automatic; /* Automatic learning done by switch chip (default enabled) */
	BOOL cpu;       /* Learn frames copied to CPU (default disabled) */
	BOOL discard;   /* Learn frames discarded (default disabled) */
} vtss_learn_mode_t;

#ifdef VTSS_FEATURE_LEARN_PORT
/******************************************************************************
 * Description: Set the learn mode for a port.
 *
 * \param port_no (input)   : Port number.
 * \param learn_mode (input): Learn mode.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_learn_port_mode_set(const vtss_port_no_t      port_no,
                                 vtss_learn_mode_t * const learn_mode);
#endif

#ifdef VTSS_FEATURE_LEARN_SWITCH
/******************************************************************************
 * Description: Set the learn mode for the switch.
 *
 * \param learn_mode (input): Learn mode.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_learn_mode_set(vtss_learn_mode_t * const learn_mode);
#endif


/* - Port STP State ------------------------------------------------ */

/* Spanning Tree state */
typedef enum _vtss_stp_state_t {
	VTSS_STP_STATE_DISABLED,   /* STP state disabled (admin/operational down) */
	VTSS_STP_STATE_BLOCKING,   /* STP state blocking */
	VTSS_STP_STATE_LISTENING,  /* STP state listening */
	VTSS_STP_STATE_LEARNING,   /* STP state learning */
	VTSS_STP_STATE_FORWARDING, /* STP state forwarding */
	VTSS_STP_STATE_ENABLED     /* STP unaware */
} vtss_stp_state_t;

/******************************************************************************
 * Description: Get Spanning Tree state for a port.
 *
 * \param port_no (input)   : Port number.
 * \param stp_state (output): STP state.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_port_stp_state_get(const vtss_port_no_t port_no,
                                vtss_stp_state_t * const stp_state);

/******************************************************************************
 * Description: Set Spanning Tree state for a port.
 *
 * \param port_no (input)  : Port number.
 * \param stp_state (input): STP state.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_port_stp_state_set(const vtss_port_no_t port_no,
                                const vtss_stp_state_t stp_state);


/* MSTP instance number: VTSS_MSTI_START..(VTSS_MSTI_END-1) */
typedef uint vtss_msti_t;

/* Number of MSTP instances */
#define VTSS_MSTIS           (65) /* Fairoaks-Ie/Gatwick-Ie */
#define VTSS_MSTIS_GW        (16) /* Fairoaks/Gatwick */
#define VTSS_MSTI_START      ((vtss_msti_t)1)
#define VTSS_MSTI_END        (VTSS_MSTI_START+VTSS_MSTIS)
#define VTSS_MSTI_ARRAY_SIZE VTSS_MSTI_END

/* MSTP state */
typedef enum _vtss_mstp_state_t {
	VTSS_MSTP_STATE_DISCARDING,
	VTSS_MSTP_STATE_LEARNING,
	VTSS_MSTP_STATE_FORWARDING
} vtss_mstp_state_t;

/******************************************************************************
 * Description: Set MSTP instance mapping for a VLAN.
 *
 * \param vid (input) : VLAN ID.
 * \param msti (input): MSTP instance.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_mstp_vlan_set(const vtss_vid_t vid, 
                           const vtss_msti_t msti);

/******************************************************************************
 * Description: Set MSTP state for a port and MSTP instance.
 *
 * \param port_no (input): Port number.
 * \param msti (input)   : MSTP instance.
 * \param state (input)  : MSTP state.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_port_mstp_state_set(const vtss_port_no_t port_no, 
                                 const vtss_msti_t msti, 
                                 const vtss_mstp_state_t state);

/* - VLAN Port Mode ------------------------------------------------ */

/* VLAN port modes according to 802.1Q section 12.10.1 */

/* VLAN acceptable frame type */
typedef enum _vtss_vlan_frame_t {
	VTSS_VLAN_FRAME_ALL,    /* Accept all frames */
	VTSS_VLAN_FRAME_TAGGED,  /* Accept tagged frames only */
#ifdef CONFIG_VTSS_ARCH_SPARX
	VTSS_VLAN_FRAME_UNTAGGED,  /* Accept untagged frames only */
#endif /* SPARX */
} vtss_vlan_frame_t;

/* VLAN port parameters */
typedef struct _vtss_vlan_port_mode_t {
	BOOL              aware;          /* VLAN awareness */
	vtss_vid_t        pvid;           /* Port VLAN ID (ingress) */
	vtss_vid_t        untagged_vid;   /* Port untagged VLAN ID  (egress) */
	vtss_vlan_frame_t frame_type;     /* Acceptable frame type (ingress) */
#ifdef VTSS_FEATURE_VLAN_INGR_FILTER_PORT
	BOOL              ingress_filter; /* Ingress filtering */
#endif
#ifdef VTSS_FEATURE_VLAN_TYPE_STAG
	BOOL              stag;           /* S-Tag type */
#endif
} vtss_vlan_port_mode_t;

/******************************************************************************
 * Description: Get VLAN mode for port.
 *
 * \param port_no (input)   : Port number.
 * \param vlan_mode (output): VLAN port mode structure.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_vlan_port_mode_get(const vtss_port_no_t          port_no,
                                vtss_vlan_port_mode_t * const vlan_mode);


/******************************************************************************
 * Description: Set VLAN mode for port.
 *
 * \param port_no (input)  : Port number.
 * \param vlan_mode (input): VLAN port mode structure.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_vlan_port_mode_set(const vtss_port_no_t                port_no,
                                const vtss_vlan_port_mode_t * const vlan_mode);


/* - VLAN table ---------------------------------------------------- */

/******************************************************************************
 * Description: Get VLAN membership.
 *
 * \param vid (input)    : VLAN ID.
 * \param member (output): VLAN port member array.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_vlan_port_members_get(
    const vtss_vid_t vid,
    BOOL             member[VTSS_PORT_ARRAY_SIZE]);


/******************************************************************************
 * Description: Set VLAN membership.
 *
 * \param vid (input)   : VLAN ID.
 * \param member (input): VLAN port member array.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_vlan_port_members_set(
    const vtss_vid_t vid,
    BOOL             member[VTSS_PORT_ARRAY_SIZE]);


/* - Port Isolation------------------------------------------------- */

#ifdef VTSS_FEATURE_ISOLATED_PORT

/******************************************************************************
 * Description: Enable/disable port isolation for VLAN.   
 *
 * \param vid (input)     : VLAN ID.
 * \param isolated (input): VLAN isolation enable/disable option.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_isolated_vlan_set(const vtss_vid_t vid,
                               const BOOL       isolated);

/******************************************************************************
 * Description: Set the isolated port set.   
 *
 * \param member (input): Port members.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_isolated_ports_set(const BOOL member[VTSS_PORT_ARRAY_SIZE]);

#endif


/* - Private VLAN (PVLAN) ------------------------------------------ */

/* Private VLAN Number: 1..VTSS_PVLANS */
typedef uint vtss_pvlan_no_t;  /* VTSS_PVLAN_NO_START..(VTSS_PVLAN_NO_END-1) */

#define VTSS_PVLANS            (VTSS_PORTS)

#define VTSS_PVLAN_NO_START    ((vtss_pvlan_no_t)1)
#define VTSS_PVLAN_NO_END      (VTSS_PVLAN_NO_START+VTSS_PVLANS)
#define VTSS_PVLAN_ARRAY_SIZE  VTSS_PVLAN_NO_END

#define VTSS_PVLAN_NO_DEFAULT  ((vtss_pvlan_no_t)1)


/******************************************************************************
 * Description: Get Private VLAN membership.
 *
 * \param pvlan_no (input): Private VLAN group number.
 * \param member (output) : Private VLAN port member array.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_pvlan_port_members_get(const vtss_pvlan_no_t pvlan_no,
				    BOOL member[VTSS_PORT_ARRAY_SIZE]);


/******************************************************************************
 * Description: Set Private VLAN membership.
 *
 * \param pvlan_no (input): Private VLAN group number.
 * \param member (input)  : Private VLAN port member array.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_pvlan_port_members_set(const vtss_pvlan_no_t pvlan_no,
				    const BOOL member[VTSS_PORT_ARRAY_SIZE]);


/* - Aggregation --------------------------------------------------- */

/******************************************************************************
 * Description: Get aggregation port members.
 *
 * \param poag_no (input): Port/aggregation number.
 * \param member (output): Aggregation port member array.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_aggr_port_members_get(const vtss_poag_no_t poag_no,
				   BOOL member[VTSS_PORT_ARRAY_SIZE]);


/******************************************************************************
 * Description: Set aggregation port members.
 *
 * \param poag_no (input): Port/aggregation number.
 * \param member (input) : Aggregation port member array.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_aggr_port_members_set(const vtss_poag_no_t poag_no,
				   const BOOL member[VTSS_PORT_ARRAY_SIZE]);


/******************************************************************************
 * Description: Get aggregation number for port.
 *
 * \param port_no (input): Port number.
 *
 * \return : Port/aggregation number. If the port is not a member of an 
 *           aggregation, the port number itself is returned.
 ******************************************************************************/
vtss_poag_no_t vtss_aggr_port_member_get(const vtss_port_no_t port_no);


/******************************************************************************
 * Description: Set aggregation number for port.
 *
 * \param port_no (input): Port number.
 * \param poag_no (input): Port/aggregation number. If the port number itself
 *                         is used, the port is not a member of an aggregation.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_aggr_port_member_set(const vtss_port_no_t port_no,
                                  const vtss_poag_no_t poag_no);


/* Aggregation traffic distribution mode */
typedef struct _vtss_aggr_mode_t {
	BOOL smac_enable;        /* Source MAC address */
	BOOL dmac_enable;        /* Destination MAC address */   
#ifdef VTSS_FEATURE_AGGR_MODE_ADV
#ifdef CONFIG_VTSS_ARCH_HEATHROW
	BOOL sip_dip_enable;     /* Source and destination IP address */
	BOOL sport_dport_enable; /* Source and destination UDP/TCP port */
#endif
#endif
#ifdef VTSS_FEATURE_AGGR_MODE_RANDOM
	BOOL random;             /* Pseudo random distribution */
#endif
} vtss_aggr_mode_t;

/******************************************************************************
 * Description: Set aggregation traffic distribution mode.
 *
 * \param mode (input): Distribution mode structure.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_aggr_mode_set(const vtss_aggr_mode_t * const mode);    

/* - Mirroring ----------------------------------------------------- */

/******************************************************************************
 * Description: Set the mirror destination port.
 *
 * \param port_no (input): Port number.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_mirror_monitor_port_set(const vtss_port_no_t port_no);


/******************************************************************************
 * Description: Set the mirror ingress ports.
 *
 * \param member (input): Port number array. If a port is enabled in this array,
 *                        frames received on the port are mirrored.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_mirror_ingress_ports_set(const BOOL member[VTSS_PORT_ARRAY_SIZE]);


/******************************************************************************
 * Description: Set the mirror egress ports.
 *
 * \param member (input): Port number array. If a port is enabled in this array,
 *                        frames transmitted on the port are mirrored.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_mirror_egress_ports_set(const BOOL member[VTSS_PORT_ARRAY_SIZE]);


/* - IGMP  ----------------------------------------------------- */

/******************************************************************************
 * Description: Set IPv4 multicast flood mask.
 *
 * \param member (input): Port number array. Ports connected to IPv4 multicast
 *                        routers should be enabled.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_ipv4_mc_flood_mask_set(const BOOL member[VTSS_PORT_ARRAY_SIZE]);

#ifdef CONFIG_VTSS_ARCH_HEATHROW

#ifdef CONFIG_VTSS_ARCH_SPARX_28
#ifdef CONFIG_VTSS_GROCX
#define VTSS_CHIP_PORTS		VTSS_PORTS
#else
#define VTSS_CHIP_PORTS		28
#endif
#define VTSS_CHIP_PORT_CPU	VTSS_CHIP_PORTS
#define VTSS_CHIP_PORTMASK	((1<<VTSS_CHIP_PORTS)-1)
#endif

#if !defined(VTSS_CHIP_PORTS)
#define VTSS_CHIP_PORTS		VTSS_PORTS
#endif

#if !defined(VTSS_CHIP_PORTMASK)
#define VTSS_CHIP_PORTMASK	((1 << VTSS_PORTS)-1)
#endif
#endif
#define VTSS_CHIP_PORT_AGGR_0 24 /* Chip port 24 and 25 are paired */
#define VTSS_CHIP_PORT_AGGR_1 26 /* Chip port 26 and 27 are paired */
#define VTSS_PORT_NO_AGGR_0	(VTSS_PORTS+1) /* Pseudo port mapping to chip port 25 */
#define VTSS_PORT_NO_AGGR_1	(VTSS_PORTS+2) /* Pseudo port mapping to chip port 27 */

#ifdef CONFIG_VTSS_ARCH_SPARX_28
#define VTSS_CHIP_PORT_VAUI_START 24
#endif

/* Port array including internally aggregated ports */
#define VTSS_PORT_EXT_ARRAY_SIZE (VTSS_PORT_ARRAY_SIZE)

/* Bit field macros */
#define VTSS_BF_SIZE(n)      ((n+7)/8)
#define VTSS_BF_GET(a, n)    ((a[(n)/8] & (1<<((n)%8))) ? 1 : 0)
#define VTSS_BF_SET(a, n, v) { if (v) { a[(n)/8] |= (1<<((n)%8)); } else { a[(n)/8] &= ~(1<<((n)%8)); }}
#define VTSS_BF_CLR(a, n)    (memset(a, 0, VTSS_BF_SIZE(n)))

/* Port member bit field macros */
#define VTSS_PORT_BF_SIZE                VTSS_BF_SIZE(VTSS_PORTS)
#define VTSS_PORT_BF_GET(a, port_no)     VTSS_BF_GET(a, port_no - VTSS_PORT_NO_START)
#define VTSS_PORT_BF_SET(a, port_no, v)  VTSS_BF_SET(a, port_no - VTSS_PORT_NO_START, v)
#define VTSS_PORT_BF_CLR(a)              VTSS_BF_CLR(a, VTSS_PORTS)

/* - Port state definitions ---------------------------------------- */

/* Port map entry */
typedef struct {
	vtss_port_no_t vtss_port[VTSS_CHIP_PORTS]; /* Map from chip_port to vtss_port */
	int            chip_port[VTSS_PORT_EXT_ARRAY_SIZE];  /* Map from vtss_port to chip_port */
	uint           phy_addr[VTSS_PORT_ARRAY_SIZE];       /* Map from vtss_port_no_t to phy add */
	uint           miim_controller[VTSS_PORT_ARRAY_SIZE];/* Map vtss_port to miim controller */
	int            chip_ports_all[VTSS_PORT_ARRAY_SIZE]; /* Array of all chip ports */
	BOOL           vtss_port_unused[VTSS_PORT_ARRAY_SIZE]; /* Port is unused */
} vtss_port_map_t;

typedef ulong vtss_chip_counter_t;

typedef struct _vtss_chip_counters_t {
#ifdef CONFIG_VTSS_ARCH_HEATHROW
	vtss_chip_counter_t rx_octets;
	vtss_chip_counter_t tx_octets;
	
	vtss_chip_counter_t rx_drops;
	vtss_chip_counter_t rx_packets;
	vtss_chip_counter_t rx_broadcasts;
	vtss_chip_counter_t rx_multicasts;
	vtss_chip_counter_t rx_crc_align_errors;
	vtss_chip_counter_t rx_shorts;
	vtss_chip_counter_t rx_longs;
	vtss_chip_counter_t rx_fragments;
	vtss_chip_counter_t rx_jabbers;
	vtss_chip_counter_t rx_64;
	vtss_chip_counter_t rx_65_127;
	vtss_chip_counter_t rx_128_255;
	vtss_chip_counter_t rx_256_511;
	vtss_chip_counter_t rx_512_1023;
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	vtss_chip_counter_t rx_1024_1526;
	vtss_chip_counter_t rx_1527_max;
#endif
	
	vtss_chip_counter_t tx_drops;
	vtss_chip_counter_t tx_packets;
	vtss_chip_counter_t tx_broadcasts;
	vtss_chip_counter_t tx_multicasts;
	vtss_chip_counter_t tx_collisions;
	vtss_chip_counter_t tx_64;
	vtss_chip_counter_t tx_65_127;
	vtss_chip_counter_t tx_128_255;
	vtss_chip_counter_t tx_256_511;
	vtss_chip_counter_t tx_512_1023;
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	vtss_chip_counter_t tx_1024_1526;
	vtss_chip_counter_t tx_1527_max;
#endif
	
	vtss_chip_counter_t tx_fifo_drops;
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	vtss_chip_counter_t rx_pauses;
	vtss_chip_counter_t tx_pauses;
	vtss_chip_counter_t rx_classified_drops;
	vtss_chip_counter_t rx_class[VTSS_PRIOS]; /* Always use direct chip values, regardless of current prios. */
	vtss_chip_counter_t tx_class[VTSS_PRIOS]; /* Always use direct chip values, regardless of current prios. */
	vtss_chip_counter_t rx_local_drops;
	vtss_chip_counter_t rx_unicast;
	vtss_chip_counter_t tx_unicast;
	vtss_chip_counter_t tx_aging;
#endif
#endif
} vtss_chip_counters_t;

/* Structure for accumulated chip specific counters */
typedef struct {
#ifdef CONFIG_VTSS_ARCH_HEATHROW
	vtss_counter_t  rx_octets;
	vtss_counter_t  tx_octets;
	
	vtss_counter_t  rx_drops;
	vtss_counter_t  rx_packets;
	vtss_counter_t  rx_broadcasts;
	vtss_counter_t  rx_multicasts;
	vtss_counter_t  rx_crc_align_errors;
	vtss_counter_t  rx_shorts;
	vtss_counter_t  rx_longs;
	vtss_counter_t  rx_fragments;
	vtss_counter_t  rx_jabbers;
	vtss_counter_t  rx_64;
	vtss_counter_t  rx_65_127;
	vtss_counter_t  rx_128_255;
	vtss_counter_t  rx_256_511;
	vtss_counter_t  rx_512_1023;
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	vtss_counter_t  rx_1024_1526;
	vtss_counter_t  rx_1527_max;
#endif /* SPARX_28 */
	
	vtss_counter_t  tx_drops;
	vtss_counter_t  tx_packets;
	vtss_counter_t  tx_broadcasts;
	vtss_counter_t  tx_multicasts;
	vtss_counter_t  tx_collisions;
	vtss_counter_t  tx_64;
	vtss_counter_t  tx_65_127;
	vtss_counter_t  tx_128_255;
	vtss_counter_t  tx_256_511;
	vtss_counter_t  tx_512_1023;
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	vtss_counter_t  tx_1024_1526;
	vtss_counter_t  tx_1527_max;
#endif /* SPARX_28 */
	
	vtss_counter_t  tx_fifo_drops;
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	vtss_counter_t  rx_pauses;
	vtss_counter_t  tx_pauses;
	vtss_counter_t  rx_classified_drops;
	vtss_counter_t  rx_class[VTSS_PRIOS];
	vtss_counter_t  tx_class[VTSS_PRIOS];
	vtss_counter_t  rx_local_drops;
	vtss_counter_t  rx_unicast;
	vtss_counter_t  tx_unicast;
	vtss_counter_t  tx_aging;
#endif
#endif
} vtss_port_big_counters_t;

/* - L2 state definitions ------------------------------------------ */

/* Port Group ID */
typedef uint vtss_pgid_no_t;

#ifdef CONFIG_VTSS_ARCH_HEATHROW
#define VTSS_CHIP_PGIDS      64
#endif

#define VTSS_PGID_NONE            (0)
#define VTSS_PGID_START           (1)
#define VTSS_PGID_SIZE            (VTSS_CHIP_PGIDS-(VTSS_CHIP_PORTS-VTSS_PORTS))
#define VTSS_PGID_END             (VTSS_PGID_START+VTSS_PGID_SIZE)
#define VTSS_PGID_UNICAST_START   (VTSS_PGID_START)
#define VTSS_PGID_UNICAST_END     (VTSS_PGID_START+VTSS_PORTS)
#define VTSS_PGID_KILL            (VTSS_PGID_UNICAST_END)
#define VTSS_PGID_BC              (VTSS_PGID_KILL+1)
#define VTSS_PGID_MULTICAST_START (VTSS_PGID_BC+1)
#define VTSS_PGID_MULTICAST_END   (VTSS_PGID_END)
#define VTSS_PGID_ARRAY_SIZE      VTSS_PGID_END

/* Mapping between high level PGID and chip PGID is done in the low level layer */

/* PGID entry */
typedef struct {
	BOOL member[VTSS_PORT_ARRAY_SIZE]; /* Egress ports/aggregations */
	BOOL resv;                         /* Fixed reservation */
	uint references;                   /* Number references to entry */
} vtss_pgid_entry_t;

/* Aggregation table */
typedef uint vtss_ac_no_t;

#ifdef CONFIG_VTSS_ARCH_HEATHROW
#define VTSS_ACS      ((vtss_ac_no_t)16)
#endif
#define VTSS_AC_START ((vtss_ac_no_t)1)
#define VTSS_AC_END   (VTSS_AC_START+VTSS_ACS)

/* VLAN entry */
typedef struct {
	BOOL        enabled;                      /* One or more ports enabled */
	BOOL        member[VTSS_PORT_ARRAY_SIZE]; /* Egress ports */
	vtss_msti_t msti;                         /* MSTP intstance */
#ifdef VTSS_FEATURE_ISOLATED_PORT
	BOOL        isolated;
#endif
} vtss_vlan_entry_t;

/* MSTP entry */
typedef struct {
	vtss_mstp_state_t state[VTSS_PORT_ARRAY_SIZE];       /* MSTP state */
} vtss_mstp_entry_t;

#define VTSS_STP_FORWARDING(stp_state) (stp_state==VTSS_STP_STATE_FORWARDING || stp_state==VTSS_STP_STATE_ENABLED)
#define VTSS_STP_UP(stp_state)         (stp_state!=VTSS_STP_STATE_DISABLED)

/* PVLAN entry */
typedef struct {
	BOOL member[VTSS_PORT_ARRAY_SIZE]; /* Member ports */
} vtss_pvlan_entry_t;

/* Size of lookup page and pointer array */
#define VTSS_MAC_PAGE_SIZE 128 
#define VTSS_MAC_PTR_SIZE  (VTSS_MAC_ADDRS/VTSS_MAC_PAGE_SIZE)

/* MAC address table for get next operations */
typedef struct vtss_mac_entry_t {
	struct vtss_mac_entry_t *next;  /* Next in list */
	ulong                   mach;  /* VID and 16 MSB of MAC */
	ulong                   macl;  /* 32 LSB of MAC */
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	uchar                   member[VTSS_PORT_BF_SIZE];
#else
	ulong                   block; /* Table block index */
#endif /* SPARX_28 */
} vtss_mac_entry_t;

/* IPv4 and IPv6 multicast address */
#define VTSS_MAC_IPV4_MC(mac) (mac[0] == 0x01 && mac[1] == 0x00 && mac[2] == 0x5e && (mac[3] & 0x80) == 0x00)
#define VTSS_MAC_IPV6_MC(mac) (mac[0] == 0x33 && mac[1] == 0x33)

#ifdef VTSS_FEATURE_QCL_PORT
#define VTSS_QCL_LIST_SIZE 24

typedef struct vtss_qcl_entry_t {
    struct vtss_qcl_entry_t *next;  /* Next in list */
    vtss_qce_t               qce;   /* This entry */
} vtss_qcl_entry_t;

typedef struct {
    vtss_qcl_entry_t         *qcl_list_used;               /* Free entries for QCL usage */
    vtss_qcl_entry_t         *qcl_list_free;               /* Entries in QCL List */
    vtss_qcl_entry_t         qcl_list[VTSS_QCL_LIST_SIZE]; /* Actual storage for list members */
} vtss_qcl_t;
#endif

/* - Supplementary state definitions ------------------------------- */

/* Structure holding all the state information */
typedef struct {
	/* --- Supplementary state --- */
	vtss_init_setup_t        init_setup;       /* Init setup */
	BOOL                     debug_read_write; /* Enable VTSS_D trace for read/write */
	
	/* --- Port state --- */
	vtss_port_map_t          port_map;
	vtss_prio_t              prios;                            /* Number of priorities used */
	vtss_port_setup_t        setup[VTSS_PORT_EXT_ARRAY_SIZE];   /* Written by vtss_port_setup() only. */
	BOOL                     port_fc_rx[VTSS_PORT_ARRAY_SIZE]; /* Rx flow control (obey pause) */
	BOOL                     port_fc_tx[VTSS_PORT_ARRAY_SIZE]; /* Tx flow control (gen. pause) */
	BOOL                     jumbo;
#ifdef VTSS_FEATURE_QOS_WFQ_PORT
	BOOL                     wfq;
#endif
	vtss_chip_counters_t     port_last_counters[VTSS_PORT_EXT_ARRAY_SIZE];/* Last chip counters */
	vtss_port_big_counters_t poag_big_counters[VTSS_POAG_ARRAY_SIZE]; /* Accumulated counters */
	vtss_port_qos_setup_t    qos[VTSS_PORT_EXT_ARRAY_SIZE];
	vtss_qos_setup_t         qos_setup;
#ifdef VTSS_FEATURE_QCL_PORT
	vtss_qcl_t               qcl[VTSS_QCL_ARRAY_SIZE];     /* QCL setup */
#endif
	
	/* --- L2 state --- */
	vtss_poag_no_t           port_poag_no[VTSS_PORT_ARRAY_SIZE]; /* Aggregation setup */
	BOOL                     aggr_member[VTSS_PORT_ARRAY_SIZE];  /* Aggregation mask 0 */
	vtss_vlan_entry_t        vlan_table[VTSS_VIDS];
	vtss_vlan_port_mode_t    vlan_port_table[VTSS_PORT_ARRAY_SIZE];
	vtss_mac_table_status_t  mac_status_appl; /* Application status */
	vtss_mac_table_status_t  mac_status_next; /* Get next optimization status */
	vtss_mac_table_status_t  mac_status_sync; /* Synchronization status */
	uint                     mac_index_sync;  /* Index for MAC table optimization */
#if (CONFIG_VTSS_MAC_NEXT_MAX != 0)
	uint                     mac_index_next;  /* Index for MAC table get next */
	uint                     mac_table_count; /* Number of entries in mac_table */
	vtss_mac_entry_t         *mac_list_used;  /* Sorted list of entries */
	vtss_mac_entry_t         *mac_list_free;  /* Free list */
	vtss_mac_entry_t         mac_table[VTSS_MAC_ADDRS]; /* Sorted MAC address table */
	uint                     mac_ptr_count;   /* Number of valid pointers */
	vtss_mac_entry_t         *mac_list_ptr[VTSS_MAC_PTR_SIZE]; /* Pointer array */
#endif
	vtss_pgid_entry_t        pgid_table[VTSS_PGID_ARRAY_SIZE];
	vtss_stp_state_t         stp_state[VTSS_PORT_ARRAY_SIZE];
	vtss_mstp_entry_t        mstp_table[VTSS_MSTI_ARRAY_SIZE];
	vtss_auth_state_t        auth_state[VTSS_PORT_ARRAY_SIZE];
	vtss_pvlan_entry_t       pvlan_table[VTSS_PVLAN_ARRAY_SIZE];
	vtss_port_no_t           mirror_port;
	BOOL                     mirror_ingress[VTSS_PORT_ARRAY_SIZE];
	BOOL                     mirror_egress[VTSS_PORT_ARRAY_SIZE];
	
	/* --- Other state --- */
#ifdef CONFIG_VTSS_ARCH_HEATHROW
#endif
#ifdef CONFIG_VTSS_ARCH_SPARX_28
	ulong                    tx_packets[VTSS_PORT_ARRAY_SIZE];
#endif
	
	vtss_pgid_no_t           pgid_end;  /* End of PGID table */
	uint                     mac_addrs; /* Number of MAC addresses */
	vtss_msti_t              msti_end;  /* End of MSTP table */
} vtss_mac_state_t;

/* Read MAC address table status bits */
vtss_rc vtss_mac_table_status_read(void);

/* Write PGID member to chip */
vtss_rc vtss_pgid_table_write(const vtss_pgid_no_t pgid_no);

/* Allocate PGID */
vtss_rc vtss_pgid_alloc(vtss_pgid_no_t *pgid_no, BOOL resv, 
                        const BOOL member[VTSS_POAG_ARRAY_SIZE]);

/* Free PGID */
vtss_rc vtss_pgid_free(const vtss_pgid_no_t pgid_no);

/* Get port status */
vtss_rc vtss_port_status_get(const vtss_port_no_t port_no,
			     vtss_port_status_t * const status);

/* Port configuration */
typedef struct {
	BOOL         enable;       /* Admin enable/disable */
	BOOL         autoneg;      /* Auto negotiation */
	vtss_speed_t speed;        /* Forced port speed */
	BOOL         fdx;          /* Forced duplex mode */
	BOOL         flow_control; /* Flow control */
	uint         max_length;   /* Max frame length */
} vtss_port_conf_t;

#endif /* __VTSS_SWITCH_H_INCLUDE__ */
