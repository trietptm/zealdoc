#ifndef __VTSS_ACL_H_INCLUDE__
#define __VTSS_ACL_H_INCLUDE__

#include <vtss_cpu.h>

#define VTSS_ACL_POLICER_NO_START 1
#define VTSS_ACL_POLICERS         16
#define VTSS_ACL_POLICER_NO_END   (VTSS_ACL_POLICER_NO_START+VTSS_ACL_POLICERS)

#define VTSS_ACL_POLICY_NO_NONE  0 /* Means ACLs disabled on port */
#define VTSS_ACL_POLICY_NO_START 1
#define VTSS_ACL_POLICIES        8
#define VTSS_ACL_POLICY_NO_END   (VTSS_ACL_POLICY_NO_START+VTSS_ACL_POLICIES)

#define VTSS_ACE_ID_LAST 0 /* Special value used to add last in list */

/* UDP/TCP range checkers */
#define VTSS_ACL_RANGES 8

/* ACE hit counter */
typedef ulong vtss_ace_counter_t; 
/* ACE ID type */
typedef ulong vtss_ace_id_t;
/* ACL policy number, 1-8 */
typedef uint vtss_acl_policy_no_t;
/* ACL policer number, 1-16 */
typedef uint vtss_acl_policer_no_t;
/* ACL port counter */
typedef ulong vtss_acl_port_counter_t; 

/* ACE rule type */
typedef enum _vtss_ace_rule_t {
	VTSS_ACE_RULE_PORT,    /* Port rule */
	VTSS_ACE_RULE_POLICY,  /* Policy rule */
	VTSS_ACE_RULE_SWITCH   /* Switch rule */
} vtss_ace_rule_t;

/* ACE frame type */
typedef enum _vtss_ace_type_t {
	VTSS_ACE_TYPE_ANY,     /* Any frame type */
	VTSS_ACE_TYPE_ETYPE,   /* Ethernet Type */
	VTSS_ACE_TYPE_LLC,     /* LLC */
	VTSS_ACE_TYPE_SNAP,    /* SNAP */
	VTSS_ACE_TYPE_ARP,     /* ARP/RARP */
	VTSS_ACE_TYPE_IPV4,    /* IPv4 */
	VTSS_ACE_TYPE_IPV6     /* IPv6 */
} vtss_ace_type_t;

/* ACE 1 bit */
typedef enum _vtss_ace_bit_t {
	VTSS_ACE_BIT_ANY, /* Value 0 or 1 */
	VTSS_ACE_BIT_0,   /* Value 0 */
	VTSS_ACE_BIT_1    /* Value 1 */
} vtss_ace_bit_t;

/* ACE 8 bit value and mask */
typedef struct _vtss_ace_uchar_t {
	uchar value; /* Value */
	uchar mask;  /* Mask, cleared bits are wildcards */
} vtss_ace_uchar_t;

/* ACE 16 bit value and mask */
typedef struct _vtss_ace_uchar2_t {
	uchar value[2]; /* Value */
	uchar mask[2];  /* Mask, cleared bits are wildcards */
} vtss_ace_uchar2_t;

/* ACE 32 bit value and mask */
typedef struct _vtss_ace_uchar4_t {
	uchar value[4]; /* Value */
	uchar mask[4];  /* Mask, cleared bits are wildcards */
} vtss_ace_uchar4_t;

/* ACE 40 bit value and mask */
typedef struct _vtss_ace_uchar5_t {
	uchar value[5]; /* Value */
	uchar mask[5];  /* Mask, cleared bits are wildcards */
} vtss_ace_uchar5_t;

/* ACE 48 bit value and mask */
typedef struct _vtss_ace_uchar6_t {
	uchar value[6]; /* Value */
	uchar mask[6];  /* Mask, cleared bits are wildcards */
} vtss_ace_uchar6_t;

/* ACE 128 bit value and mask */
typedef struct _vtss_ace_uchar16_t {
	uchar value[16]; /* Value */
	uchar mask[16];  /* Mask, cleared bits are wildcards */
} vtss_ace_uchar16_t;

/* ACE VLAN ID value and mask */
typedef struct _vtss_ace_vid_t {
	ushort value; /* Value */
	ushort mask;  /* Mask, cleared bits are wildcards */
} vtss_ace_vid_t;

/* ACE IP address value and mask */
typedef struct _vtss_ace_ip_t {
	vtss_ip_t value; /* Value */
	vtss_ip_t mask;  /* Mask, cleared bits are wildcards */
} vtss_ace_ip_t; 

/* ACE UDP/TCP port range */
typedef struct _vtss_ace_udp_tcp_t {
	BOOL           in_range; /* Port in range match */
	vtss_udp_tcp_t low;      /* Port low value */
	vtss_udp_tcp_t high;     /* Port high value */
} vtss_ace_udp_tcp_t;

/* ACL Action */
typedef struct _vtss_acl_action_t {
	BOOL                  forward;      /* Allow forwarding */
	BOOL                  learn;        /* Allow learning */
	BOOL                  cpu;          /* Forward to CPU */
	BOOL                  cpu_once;     /* Only first frame forwarded to CPU */
	vtss_cpu_rx_queue_t   cpu_queue;    /* CPU queue */
	BOOL                  police;       /* Enable policer */
	vtss_acl_policer_no_t policer_no;   /* Policer number */
	BOOL                  port_forward; /* Forward to specific port */
	vtss_port_no_t        port_no;      /* Specified port */
} vtss_acl_action_t;

/* Access Control Entry */
typedef struct _vtss_ace_t {
	vtss_ace_id_t        id;        /* ACE ID */
	vtss_ace_rule_t      rule;      /* ACE rule type */
	vtss_poag_no_t       port_no;   /* Port number: VTSS_ACL_RULE_PORT */
	vtss_acl_policy_no_t policy_no; /* Policy number: VTSS_ACL_RULE_POLICY */
	vtss_ace_type_t      type;      /* ACE frame type */
	vtss_acl_action_t    action;    /* ACE action */
	
	/* DMAC information */
	vtss_ace_bit_t      dmac_mc;  /* Multicast DMAC */
	vtss_ace_bit_t      dmac_bc;  /* Broadcast DMAC */
	
	/* VLAN Tag */
	struct {
		vtss_ace_vid_t   vid;      /* VLAN ID (12 bit) */
		vtss_ace_uchar_t usr_prio; /* User priority (3 bit) */
		vtss_ace_bit_t   cfi;      /* CFI */
	} vlan;
	
	/* Frame type specific data */
	union {
		/* VTSS_ACE_TYPE_ANY: No specific fields */
		
		/* VTSS_ACE_TYPE_ETYPE */
		struct {
			vtss_ace_uchar6_t dmac;  /* DMAC */
			vtss_ace_uchar6_t smac;  /* SMAC */
			vtss_ace_uchar2_t etype; /* Ethernet Type value */
			vtss_ace_uchar2_t data;  /* MAC data */ 
		} etype;
		
		/* VTSS_ACE_TYPE_LLC */
		struct {
			vtss_ace_uchar6_t dmac; /* DMAC */
			vtss_ace_uchar6_t smac; /* SMAC */
			vtss_ace_uchar4_t llc;  /* LLC */
		} llc;
		
		/* VTSS_ACE_TYPE_SNAP */
		struct {
			vtss_ace_uchar6_t dmac; /* DMAC */
			vtss_ace_uchar6_t smac; /* SMAC */
			vtss_ace_uchar5_t snap; /* SNAP */
		} snap;
		
		/* VTSS_ACE_TYPE_ARP */
		struct {
			vtss_ace_uchar6_t smac;       /* SMAC */
			vtss_ace_bit_t    arp;        /* Opcode ARP/RARP */
			vtss_ace_bit_t    req;        /* Opcode request/reply */
			vtss_ace_bit_t    unknown;    /* Opcode unknown */
			vtss_ace_bit_t    smac_match; /* Sender MAC matches SMAC */
			vtss_ace_bit_t    dmac_match; /* Target MAC matches DMAC */
			vtss_ace_bit_t    length;     /* Protocol addr. length 4, hardware length 6 */
			vtss_ace_bit_t    ip;         /* Protocol address type IP */
			vtss_ace_bit_t    ethernet;   /* Hardware address type Ethernet */
			vtss_ace_ip_t     sip;        /* Sender IP address */
			vtss_ace_ip_t     dip;        /* Target IP address */
		} arp;
		
		/* IPv4: Type VTSS_ACE_TYPE_IPV4 */
		struct {
			vtss_ace_bit_t     ttl;       /* TTL zero */
			vtss_ace_bit_t     fragment;  /* Fragment */
			vtss_ace_bit_t     options;   /* Header options */
			vtss_ace_uchar_t   ds;        /* DS field */
			vtss_ace_uchar_t   proto;     /* Protocol */
			vtss_ace_ip_t      sip;       /* Source IP address */
			vtss_ace_ip_t      dip;       /* Destination IP address */
			vtss_ace_uchar6_t  data;      /* Not UDP/TCP: IP data */
			vtss_ace_udp_tcp_t sport;     /* UDP/TCP: Source port */
			vtss_ace_udp_tcp_t dport;     /* UDP/TCP: Destination port */
			vtss_ace_bit_t     tcp_fin;   /* TCP FIN */
			vtss_ace_bit_t     tcp_syn;   /* TCP SYN */
			vtss_ace_bit_t     tcp_rst;   /* TCP RST */
			vtss_ace_bit_t     tcp_psh;   /* TCP PSH */
			vtss_ace_bit_t     tcp_ack;   /* TCP ACK */
			vtss_ace_bit_t     tcp_urg;   /* TCP URG */
		} ipv4;
		
		/* IPv6: Type VTSS_ACE_TYPE_IPV6 */
		struct {
			vtss_ace_uchar_t   proto; /* IPv6 protocol */
			vtss_ace_uchar16_t sip;   /* IPv6 source address */
		} ipv6;
	} frame;
} vtss_ace_t;

typedef struct vtss_acl_entry_t {
	struct vtss_acl_entry_t *next;   /* Next in list */
	vtss_ace_t              ace;     /* ACE data */
	ulong                   counter; /* Hit counter */
} vtss_acl_entry_t;

typedef struct {
	vtss_ace_udp_tcp_t port;  /* UDP/TCP range information */
	BOOL               sport; /* Source or destination port */
	uint               count; /* Reference count */
} vtss_acl_range_t;

typedef struct vtss_acl_state_t {
	vtss_acl_entry_t         *acl_list; /* List of active ACEs */
	vtss_acl_entry_t         *acl_free; /* List of free ACEs */
	vtss_acl_entry_t         acl[VTSS_ACES];
	vtss_acl_range_t         acl_range[VTSS_ACL_RANGES];
} vtss_acl_state_t;

/* set policer rate */
vtss_rc vtss_ll_acl_policer_rate_set(vtss_acl_policer_no_t policer_no,
                                     vtss_packet_rate_t rate);
/* set default action for port */
vtss_rc vtss_ll_acl_port_action_set(vtss_port_no_t port_no,
                                    const vtss_acl_action_t * action);
/* get default action counter for port */
vtss_rc vtss_ll_acl_port_counter_get(const vtss_port_no_t port_no,
                                     vtss_acl_port_counter_t * const counter);
/* clear default action counter for port */
vtss_rc vtss_ll_acl_port_counter_clear(const vtss_port_no_t port_no);
/* set policy number for port */
vtss_rc vtss_ll_acl_policy_no_set(vtss_port_no_t port_no,
                                  vtss_acl_policy_no_t policy_no);
/* add ACE */
vtss_rc vtss_ll_ace_add(vtss_ace_id_t ace_id,
                        const vtss_ace_t * ace);
/* delete ACE */
vtss_rc vtss_ll_ace_del(vtss_ace_id_t ace_id);
/* get ACE counter */
vtss_rc vtss_ll_ace_counter_get(vtss_ace_id_t ace_id,
                                vtss_ace_counter_t * counter);
/* clear ACE counter */
vtss_rc vtss_ll_ace_counter_clear(vtss_ace_id_t ace_id);


/******************************************************************************
 * Description: Set rate for ACL policer.
 *
 * \param policer_no (input): ACL policer number.
 * \param rate (input)      : ACL policer rate.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_acl_policer_rate_set(const vtss_acl_policer_no_t policer_no,
                                  const vtss_packet_rate_t    rate);

/******************************************************************************
 * Description: Set default ACL action for port.
 *
 * \param port_no (input): Port number.
 * \param action (input) : Default action for port.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_acl_port_action_set(const vtss_port_no_t            port_no,
                                 const vtss_acl_action_t * const action);

/******************************************************************************
 * Description: Get default action counter for port.
 *
 * \param port_no (input) : Port number.
 * \param counter (output): Default action counter for port.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_acl_port_counter_get(const vtss_port_no_t            port_no,
                                  vtss_acl_port_counter_t * const counter);

/******************************************************************************
 * Description: Clear default action counter for port.
 *
 * \param port_no (input): Port number.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_acl_port_counter_clear(const vtss_port_no_t port_no);


/******************************************************************************
 * Description: Setup policy number for port.
 *
 * \param port_no (input)  : Port number.
 * \param policy_no (input): Policy number.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_acl_policy_no_set(const vtss_port_no_t       port_no,
                               const vtss_acl_policy_no_t policy_no);

/******************************************************************************
 * Description: Initialize ACE to default values.
 *
 * \param type (input): ACE type.
 * \param ace (output): ACE structure.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_ace_init(const vtss_ace_type_t type,
                      vtss_ace_t * const    ace);

/******************************************************************************
 * Description: Add/modify ACE.
 *
 * \param ace_id (input): ACE ID. The ACE will be added after the entry with 
 *                        this ID. VTSS_ACE_ID_LAST is reserved for inserting 
 *                        last.
 * \param ace (input)   : ACE structure.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_ace_add(const vtss_ace_id_t      ace_id,
                     const vtss_ace_t * const ace);

/******************************************************************************
 * Description: Delete ACE.
 *
 * \param ace_id (input): ACE ID.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_ace_del(const vtss_ace_id_t ace_id);

/******************************************************************************
 * Description: Get ACE counter.
 *
 * \param ace_id (input)  : ACE ID.
 * \param counter (output): ACE counter.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_ace_counter_get(const vtss_ace_id_t        ace_id,
                             vtss_ace_counter_t * const counter);

/******************************************************************************
 * Description: Clear ACE counter.
 *
 * \param ace_id (input): ACE ID.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc vtss_ace_counter_clear(const vtss_ace_id_t ace_id);

#ifdef CONFIG_VTSS_ACL
void vtss_acl_reset_port(vtss_port_no_t port_no);
void vtss_acl_init_state(void);
vtss_rc vtss_acl_enable(void);
#else
static inline void vtss_acl_reset_port(vtss_port_no_t port_no) {}
static inline void vtss_acl_init_state(void) {}
static inline vtss_rc vtss_acl_enable(void) { return VTSS_OK; }
#endif

#endif /* __VTSS_ACL_H_INCLUDE__ */
