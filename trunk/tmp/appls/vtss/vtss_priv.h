#ifndef __VTSS_PRIV_H_INCLUDE__
#define __VTSS_PRIV_H_INCLUDE__

#include <logger.h>
#include <vtss.h>
#include <module.h>
#include <netsvc.h>
#include <udev.h>
#include <service.h>
#include <uiserv.h>
#include <panic.h>
#include "vtss_switch.h"
#include "vtss_phy.h"

/* Standard headers */
#ifdef CONFIG_VTSS_VITGENIO
#include <string.h>
#include <linux/vitgenio.h>
#include <linux/ioctl.h> /* ioctl() */
#endif

#define VTSS_LOG_CRIT		LOG_EMERG
#define VTSS_LOG_FAIL		LOG_CRIT
#define VTSS_LOG_ERR		LOG_ERR
#define VTSS_LOG_WARN		LOG_WARNING
#define VTSS_LOG_INFO		LOG_INFO
#define VTSS_LOG_DEBUG		LOG_DEBUG

void vtss_log(int level, const char *format, ...);

#include "vtss_cil.h"

/* API private headers */
#ifdef VTSS_IO_HEATHROW
#include "vtss_sparx_reg.h"
#endif
#include "vtss_heathrow.h"
#include "vtss_acl.h"

extern vtss_config_t vtss_main_config;

int vtss_appl_start(void);
void vtss_mac_age_start(void);
void vtss_mac_age_stop(void);

extern vtss_mac_state_t vtss_mac_state;
extern vtss_mac_io_state_t vtss_io_state;
extern vtss_phy_state_t vtss_phy_state;
extern vtss_phy_io_state_t vtss_phy_io_state;

vtss_rc vtss_mac_table_update(void);
BOOL vtss_pgid_member(vtss_pgid_no_t pgid_no, vtss_port_no_t port);

int __init vtss_grocx_init(void);
void __exit vtss_grocx_exit(void);

#endif /* __VTSS_PRIV_H_INCLUDE__ */
