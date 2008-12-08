#ifndef __VTSS_PHY_H_INCLUDE__
#define __VTSS_PHY_H_INCLUDE__

#include <vtss.h>

typedef int vtss_phy_io_bus_t;

#ifdef CONFIG_VTSS_API
/* Conversion from phy_bus <-> chip_no,miim_controller */
#define VTSS_PHY_CHIPMIIM(phy_bus)	(phy_bus&0xF)
#define VTSS_PHY_BUS(miim_controller)	(miim_controller&0xF)
#endif

/* PHY I/O Layer state information */
typedef struct _vtss_phy_io_state_t {
#ifdef CONFIG_VTSS_API
	int unused; /* Dummy to avoid empty struct */
#endif
#if (defined(CONFIG_VTSS_FORCE_CPLD_MIIM) || !defined(CONFIG_VTSS_API)) && \
    defined(CONFIG_VTSS_VITGENIO)
	int fd; /* File descriptor */
#endif /* VTSS_VITGENIO */
} vtss_phy_io_state_t;

/* Initialize PHY I/O Layer, must be called before any other function */
void vtss_phy_io_start(void);

/* Read PHY register */
vtss_rc vtss_phy_io_read(const vtss_phy_io_bus_t bus,
                         const uint addr, const uint reg,
			 ushort *const value);

/* Write PHY register */
vtss_rc vtss_phy_io_write(const vtss_phy_io_bus_t bus,
                          const uint addr, const uint reg,
                          const ushort value);


/* PHY type */
typedef enum _vtss_phy_type_t {
	VTSS_PHY_TYPE_NONE = 0,    /* Unknown */
	VTSS_PHY_TYPE_8201 = 8201, /* VSC8201 (Mustang) */
	VTSS_PHY_TYPE_8204 = 8204, /* VSC8204 (Blazer) */
	VTSS_PHY_TYPE_8211 = 8211, /* VSC8211 (Cobra) */
	VTSS_PHY_TYPE_8221 = 8221, /* VSC8221 (Cobra) */
	VTSS_PHY_TYPE_8224 = 8224, /* VSC8224 (Quattro) */
	VTSS_PHY_TYPE_8234 = 8234, /* VSC8234 (Quattro) */
	VTSS_PHY_TYPE_8244 = 8244, /* VSC8244 (Quattro) */
	VTSS_PHY_TYPE_8538 = 8538, /* VSC8538 (Spyder) */
	VTSS_PHY_TYPE_8558 = 8558, /* VSC8558 (Spyder) */
	VTSS_PHY_TYPE_8658 = 8658, /* VSC8658 (GTO) */
	VTSS_PHY_TYPE_8601 = 8601, /* VSC8601 (Cooper) */
	VTSS_PHY_TYPE_8641 = 8641, /* VSC8641 (Cooper) */
	VTSS_PHY_TYPE_7385 = 7385, /* VSC7385 (Northolt) */
	VTSS_PHY_TYPE_7388 = 7388, /* VSC7388 (Luton) */
	VTSS_PHY_TYPE_7389 = 7389, /* VSC7389, VSC7391 (Luton16/16r) */
	VTSS_PHY_TYPE_7390 = 7390, /* VSC7390 (Luton24) */
	VTSS_PHY_TYPE_7395 = 7395, /* VSC7395, VSC7396 (Luton5e/5m) */
	VTSS_PHY_TYPE_7398 = 7398,  /* VSC7398, (Luton8) */
	VTSS_PHY_TYPE_7500 = 7500,  /* VSC7500 (Intrigue) */
	VTSS_PHY_TYPE_7501 = 7501,  /* VSC7501 (Intrigue) */
	VTSS_PHY_TYPE_7502 = 7502,  /* VSC7502 (Intrigue) */
	VTSS_PHY_TYPE_7503 = 7503,  /* VSC7503 (Intrigue) */
	VTSS_PHY_TYPE_7504 = 7504,  /* VSC7504 (Intrigue) */
	VTSS_PHY_TYPE_7505 = 7505,  /* VSC7505 (Intrigue) */
	VTSS_PHY_TYPE_7506 = 7506,  /* VSC7506 (Intrigue) */
	VTSS_PHY_TYPE_7507 = 7507   /* VSC7507 (Intrigue) */    
} vtss_phy_type_t; 

/* PHY family */
typedef enum _vtss_phy_family_t {
	VTSS_PHY_FAMILY_NONE,     /* Unknown */
	VTSS_PHY_FAMILY_MUSTANG,  /* VSC8201 */
	VTSS_PHY_FAMILY_BLAZER,   /* VSC8204 */
	VTSS_PHY_FAMILY_COBRA,    /* VSC8211/21 */
	VTSS_PHY_FAMILY_QUATTRO,  /* VSC8224/34/44 */
	VTSS_PHY_FAMILY_SPYDER,   /* VSC8538/58/8658 */
	VTSS_PHY_FAMILY_COOPER,   /* VSC8601/41 */
	VTSS_PHY_FAMILY_LUTON,    /* VSC7385/88 */
	VTSS_PHY_FAMILY_LUTON24,  /* VSC7389/90/91 */
	VTSS_PHY_FAMILY_LUTON_E,   /* VSC7395/96/98 */
	VTSS_PHY_FAMILY_INTRIGUE  /* VSC7500-7507 */
} vtss_phy_family_t;

typedef struct _vtss_phy_mapped_port_t {
	vtss_phy_io_bus_t bus;
	int addr;
} vtss_phy_mapped_port_t;

/* PHY setup */
typedef struct _vtss_phy_setup_t {
	/* PHY mode */
	enum {
		VTSS_PHY_MODE_ANEG,      /* Auto negoatiation */
		VTSS_PHY_MODE_FORCED,    /* Forced mode */
		VTSS_PHY_MODE_POWER_DOWN /* Power down (disabled) */
	} mode;
	
	/* Forced mode */
	struct {
		vtss_speed_t speed; /* Speed */
		BOOL         fdx;   /* Full duplex */
	} forced;
	
	/* Auto negotiation advertisement */
	struct {
		BOOL speed_10m_hdx;    /* 10Mbps, half duplex */
		BOOL speed_10m_fdx;    /* 10Mbps, full duplex */
		BOOL speed_100m_hdx;   /* 100Mbps, half duplex */
		BOOL speed_100m_fdx;   /* 100Mbps, full duplex */
		BOOL speed_1g_fdx;     /* 1000Mpbs, full duplex */
		BOOL symmetric_pause;  /* Symmetric pause */
		BOOL asymmetric_pause; /* Asymmetric pause */
	} aneg;
} vtss_phy_setup_t;

/* Media interface type */
typedef enum _vtss_phy_media_interface_t {
	VTSS_PHY_MEDIA_INTERFACE_COPPER,      /* Copper interface */
	VTSS_PHY_MEDIA_INTERFACE_FIBER,       /* SerDes/Fiber interface */
	VTSS_PHY_MEDIA_INTERFACE_AMS_COPPER,  /* Automatic selection, copper preferred */
	VTSS_PHY_MEDIA_INTERFACE_AMS_FIBER    /* Automatic selection, fiber preferred */
} vtss_phy_media_interface_t;

typedef struct _vtss_phy_reset_t {
	vtss_port_interface_t mac_if;   /* MAC interface */
	vtss_phy_media_interface_t media_if; /* Media inteface */
	
	/* RGMII setup */
	struct {
		uint rx_clk_skew_ps; /* Rx clock skew in pico seconds */
		uint tx_clk_skew_ps; /* Tx clock skew in pico seconds */
	} rgmii;
	
	/* TBI setup */
	struct {
		BOOL aneg_enable; /* Enable auto negotiation */
	} tbi;
} vtss_phy_reset_setup_t;

/* State per PHY */
typedef struct _vtss_phy_port_state_info_t {
	vtss_phy_mapped_port_t map;	/* PHY map */
	vtss_phy_reset_setup_t reset;	/* Reset setup */
	vtss_phy_family_t family;	/* Family */
	vtss_phy_type_t type;		/* Type */
	uint revision;			/* Revision number */
	vtss_phy_setup_t setup;		/* Setup */
	vtss_port_status_t status;	/* Status */
} vtss_phy_port_state_info_t;

/* This struct is followed by a
 * vtss_phy_port_state_info_t[port_array_size] array.
 */
typedef struct _vtss_phy_state_t {
	uint port_array_size;
	vtss_phy_port_state_info_t port[VTSS_PORT_ARRAY_SIZE];
} vtss_phy_state_t;

/* ================================================================= *
 *  Supplemental
 * ================================================================= */

/* - Initialization ------------------------------------------------ */
/**********************************************************************
 * Initialization setup
 *  
 * typedef struct _vtss_phy_init_setup_t {
 *      uint                ports;
 * } vtss_phy_init_setup_t;
 * 
***********************************************************************/

/******************************************************************************
 * Description: Initialize PHY API.
 *
 * \param setup (input): Pointer to initialization setup.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_phy_start(void);

/* - Port mapping -------------------------------------------------- */

/****************************************************************************
 * 
 * typedef struct _vtss_phy_mapped_port_t {
 *      vtss_phy_io_bus_t   bus;
 *      int                 addr;
 * } vtss_phy_mapped_port_t;
 * 
*****************************************************************************/

/******************************************************************************
 * Description: Setup port mapping.
 *
 * \param mapped_ports (input): PHY port map array indexed by vtss_port_no_t.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_phy_port_map_set( const vtss_phy_mapped_port_t * mapped_ports );

/******************************************************************************
 * Description: Setup port mapping for a port. Port 0 can be used for accessing
 *              PHYs by their (bus,address) location.
 *
 * \param port_no (input): Port number connected to PHY.
 * \param bus (input)    : PHY MIIM bus.
 * \param addr (input)   : PHY address on MIIM bus.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_phy_map_port( const vtss_port_no_t    port_no,
                           const vtss_phy_io_bus_t bus,
                           const int               addr );


/* - PHY reset ----------------------------------------------------- */

/******************************************************************************
 * Media interface type
 *  
 * typedef enum _vtss_phy_media_interface_t {
 *  VTSS_PHY_MEDIA_INTERFACE_COPPER,      // Copper interface
 *  VTSS_PHY_MEDIA_INTERFACE_FIBER,       // SerDes/Fiber interface 
 *  VTSS_PHY_MEDIA_INTERFACE_AMS_COPPER,  // Automatic selection, copper preferred 
 *  VTSS_PHY_MEDIA_INTERFACE_AMS_FIBER    // Automatic selection, fiber preferred 
 * } vtss_phy_media_interface_t;
 * 
 * typedef struct _vtss_phy_reset_t {
 *  vtss_port_interface_t      mac_if;   // MAC interface
 *  vtss_phy_media_interface_t media_if; // Media inteface
 *  
 *      // RGMII setup 
 *      struct {
 *          uint rx_clk_skew_ps; // Rx clock skew in pico seconds 
 *          uint tx_clk_skew_ps; // Tx clock skew in pico seconds 
 *      } rgmii;
 * 
 *      // TBI setup 
 *      struct {
 *          BOOL aneg_enable; // Enable auto negotiation 
 *      } tbi;
 * } vtss_phy_reset_setup_t;
 * 
 *******************************************************************************/
 
/******************************************************************************
 * Description: Reset PHY.
 *
 * \param port_no (input): Port number of PHY.
 * \param setup (input)  : Reset setup structure.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_phy_reset( const vtss_port_no_t port_no, 
                        const vtss_phy_reset_setup_t * const setup );

/******************************************************************************
 * Description: Detect PHY Type, Family, Revision.
 *
 * \param reg2    (input)   : MII Register 2 value.
 * \param reg3    (input)   : MII Register 3 value.
 * \param mac_if  (input)   : MAC I/F being used.
 * \param ps      (output)  : PHY port state populated with the PHY Type, Family 
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_phy_detect(const ushort reg2, const ushort reg3, 
                        const vtss_port_interface_t mac_if,
                        vtss_phy_port_state_info_t* ps );                      
                          
/* - PHY setup ----------------------------------------------------- */
/******************************************************************************
 * typedef struct _vtss_phy_setup_t {
 *  // PHY mode 
 *  enum {
 *      VTSS_PHY_MODE_ANEG,      // Auto negoatiation 
 *      VTSS_PHY_MODE_FORCED,    // Forced mode 
 *      VTSS_PHY_MODE_POWER_DOWN // Power down (disabled) 
 *  } mode;
 *
 *  // Forced mode 
 *  struct {
 *      vtss_speed_t speed; // Speed 
 *      BOOL         fdx;   // Full duplex 
 *  } forced;
 *  
 *  // Auto negotiation advertisement 
 *  struct {
 *      BOOL speed_10m_hdx;    // 10Mbps, half duplex 
 *      BOOL speed_10m_fdx;    // 10Mbps, full duplex 
 *      BOOL speed_100m_hdx;   // 100Mbps, half duplex 
 *      BOOL speed_100m_fdx;   // 100Mbps, full duplex 
 *      BOOL speed_1g_fdx;     // 1000Mpbs, full duplex 
 *      BOOL symmetric_pause;  // Symmetric pause 
 *      BOOL asymmetric_pause; // Asymmetric pause 
 *  } aneg;
 * } vtss_phy_setup_t;
 * 
 *******************************************************************************/

/******************************************************************************
 * Description: Setup PHY
 *
 * \param port_no (input): Port number of PHY.
 * \param setup (input)  : Setup structure.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_phy_setup( const vtss_port_no_t           port_no, 
                        const vtss_phy_setup_t * const setup ); 

/* - PHY status ---------------------------------------------------- */

/******************************************************************************
 * Description: Get PHY status.
 *
 * \param port_no (input): Port number of PHY.
 * \param status (output): Status structure.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_phy_status_get(const vtss_port_no_t port_no,
                            vtss_port_status_t * const status);
                               
/* - PHY optimization ---------------------------------------------- */

/******************************************************************************
 * Description: PHY optimization called approximately every second.
 *
 * \param port_no (input): Port number of PHY.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_phy_optimize_1sec(const vtss_port_no_t port_no);

/* - PHY read/write ------------------------------------------------ */

/* PHY register pages */
#define VTSS_PHY_PAGE_STANDARD 0x0000 /* Standard registers */
#define VTSS_PHY_PAGE_EXTENDED 0x0001 /* Extended registers */
#define VTSS_PHY_PAGE_GPIO     0x0010 /* GPIO registers */
#define VTSS_PHY_PAGE_TEST     0x2A30 /* Test registers */
#define VTSS_PHY_PAGE_TR       0x52B5 /* Token ring registers */

/* PHY register page access for a single register can be done 
   using an OR operation of the register address and the page below */
#define VTSS_PHY_REG_STANDARD (VTSS_PHY_PAGE_STANDARD<<5)
#define VTSS_PHY_REG_EXTENDED (VTSS_PHY_PAGE_EXTENDED<<5)
#define VTSS_PHY_REG_GPIO     (VTSS_PHY_PAGE_GPIO<<5)
#define VTSS_PHY_REG_TEST     (VTSS_PHY_PAGE_TEST<<5)
#define VTSS_PHY_REG_TR       (VTSS_PHY_PAGE_TR<<5)

/******************************************************************************
 * Description: Read value from PHY register.
 *
 * \param port_no (input): Port number connected to PHY.
 * \param reg (input)    : PHY register address.
 * \param value (output) : Register value (16 bit).
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_phy_read(const vtss_port_no_t port_no,
                      const uint           reg,
                      ushort *const        value );

/******************************************************************************
 * Description: Write value to PHY register.
 *
 * \param port_no (input): Port number connected to PHY.
 * \param reg (input)    : PHY register address.
 * \param value (input)  : Register value (16 bit).
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_phy_write(const vtss_port_no_t port_no,
                       const uint reg, const ushort value);

/******************************************************************************
 * Description: Read, modify and write value to PHY register.
 *
 * \param port_no (input): Port number connected to PHY.
 * \param reg (input)    : PHY register address.
 * \param value  (input) : Register value (16 bit).
 * \param mask (input)   : Register mask, only bits enabled are changed.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_phy_writemasked(const vtss_port_no_t port_no,
                             const uint           reg,
                             const ushort         value,
                             const ushort         mask );


/******************************************************************************
 * Description: Run Init scripts for VSC8204 
 *
 * \param port_no (input): Phy State .
 * \return : Return code, Default VTSS_OK.
 ******************************************************************************/
vtss_rc vtss_phy_init_seq_blazer(vtss_phy_port_state_info_t* ps, 
                                 const vtss_port_no_t port_no);

/******************************************************************************
 * Description: Run Init scripts for VSC8224/VSC8234/VSC8244 
 *
 * \param port_no (input): Phy State .
 * \return : Return code, Default VTSS_OK.
 ******************************************************************************/
vtss_rc vtss_phy_init_seq_quattro(vtss_phy_port_state_info_t* ps, 
                                  const vtss_phy_reset_setup_t * const setup,
                                  const vtss_port_no_t port_no);

/******************************************************************************
 * Description: Run Init scripts for VSC8538/VSC8558/VSC8658 
 *
 * \param port_no (input): Phy State .
 * \return : Return code, Default VTSS_OK.
 ******************************************************************************/
vtss_rc vtss_phy_init_seq_spyder(vtss_phy_port_state_info_t* ps, 
                                 const vtss_port_no_t port_no);

/******************************************************************************
 * Description: Run Init scripts for VSC8601/VSC8641 
 *
 * \param port_no (input): Phy State .
 * \return : Return code, Default VTSS_OK.
 ******************************************************************************/
vtss_rc vtss_phy_init_seq_cooper(vtss_phy_port_state_info_t* ps,
                                 const vtss_port_no_t port_no);

/******************************************************************************
 * Description: Run Init scripts for VSC7385/VSC7388/VSC7395/VSC7398/VSC7389/VSC7390/VSC7391
 *
 * \param port_no (input): Phy State .
 * \return : Return code, Default VTSS_OK.
 ******************************************************************************/
vtss_rc vtss_phy_init_seq_luton(vtss_phy_port_state_info_t* ps,
                                const vtss_phy_reset_setup_t * const setup,
                                const vtss_port_no_t port_no);

/******************************************************************************
 * Description: Run Init scripts for VSC7500-VSC7507
 *
 * \param port_no (input): Phy State .
 * \return : Return code, Default VTSS_OK.
 ******************************************************************************/

vtss_rc vtss_phy_init_seq_intrigue (vtss_phy_port_state_info_t* ps,
                                    const vtss_phy_reset_setup_t * const setup, 
                                    const vtss_port_no_t port_no);

#endif /* __VTSS_PHY_H_INCLUDE__ */
