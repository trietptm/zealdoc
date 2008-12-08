#ifndef __VTSS_CONF_H_INCLUDE__
#define __VTSS_CONF_H_INCLUDE__

/* ================================================================= *
   Introduction:
   The API consists of a number of components. Each component may
   have its own source files, API and compile options. This file 
   describes each component briefly. Compile options for switch/PHY 
   API and the application component are also included.
   For other components, please refer to the API header files listed.
 * ================================================================= */

#ifdef CONFIG_VTSS_ROUTER_VLAN_LAN
#define VTSS_ROUTER_VLAN_LAN	CONFIG_VTSS_ROUTER_VLAN_LAN
#else
#define VTSS_ROUTER_VLAN_LAN	1
#endif

#ifdef CONFIG_VTSS_ROUTER_PORT_WAN
#define VTSS_ROUTER_PORT_WAN	CONFIG_VTSS_ROUTER_PORT_WAN
#else
#define VTSS_ROUTER_PORT_WAN	0
#endif

#ifdef CONFIG_VTSS_ROUTER_VLAN_WAN
#define VTSS_ROUTER_VLAN_WAN	CONFIG_VTSS_ROUTER_VLAN_WAN
#else
#define VTSS_ROUTER_VLAN_WAN	2
#endif

#ifdef CONFIG_VTSS_ROUTER_PORT_5_IF
#define VTSS_ROUTER_PORT_5_IF	CONFIG_VTSS_ROUTER_PORT_5_IF
#else
#define VTSS_ROUTER_PORT_5_IF	0
#endif

/* ================================================================= *
 *  Switch/PHY API options
 * ================================================================= */

/* Single ended clock design */
#ifndef CONFIG_VTSS_REF_CLK_SINGLE_ENDED
#ifdef CONFIG_VTSS_ARCH_SPARX_28
#define CONFIG_VTSS_REF_CLK_SINGLE_ENDED 1
#else
#undef CONFIG_VTSS_REF_CLK_SINGLE_ENDED
#endif
#endif

/* Number of updated MAC table entries for get next traversal. 
   Must be a multiplum of 4. The value zero means disabled */
#ifndef CONFIG_VTSS_MAC_NEXT_MAX
#define CONFIG_VTSS_MAC_NEXT_MAX 1024
#endif

#endif /* __VTSS_CONF_H_INCLUDE__ */
