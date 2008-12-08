#include "vtss_priv.h"

log_source_t vtss_log_source = {
	"vtss",
};

int vtss_logger = 0;
notify_t vtss_udev_notify;
vtss_config_t vtss_main_config = {
	VTSS_MAC_AGE_DEFAULT,
};

/* Return code interpretation */
const char *vtss_error_txt(vtss_rc rc)
{
	const char *txt;
	switch (rc) {
	case VTSS_OK:
		txt = "VTSS_OK";
		break;
	case VTSS_WARNING:
		txt = "VTSS_WARNING";
		break;
	case VTSS_INCOMPLETE:
		txt = "VTSS_INCOMPLETE";
		break;
	case VTSS_UNSPECIFIED_ERROR:
		txt = "VTSS_UNSPECIFIED_ERROR";
		break;
	case VTSS_NOT_IMPLEMENTED:
		txt = "VTSS_NOT_IMPLEMENTED";
		break;
	case VTSS_INVALID_PARAMETER:
		txt = "VTSS_INVALID_PARAMETER";
		break;
	case VTSS_DATA_NOT_READY:
		txt = "VTSS_DATA_NOT_READY";
		break;
	case VTSS_ENTRY_NOT_FOUND:
		txt = "VTSS_ENTRY_NOT_FOUND";
		break;
	case VTSS_TIMEOUT_RETRYLATER:
		txt = "VTSS_TIMEOUT_RETRYLATER";
		break;
	case VTSS_FATAL_ERROR:
		txt = "VTSS_FATAL_ERROR";
		break;
	case VTSS_PHY_NOT_MAPPED:
		txt = "VTSS_PHY_NOT_MAPPED";
		break;
	case VTSS_PHY_READ_ERROR:
		txt = "VTSS_PHY_READ_ERROR";
		break;
	case VTSS_PHY_TIMEOUT:
		txt = "VTSS_PHY_TIMEOUT";
		break;
	case VTSS_PACKET_BUF_SMALL:
		txt = "VTSS_PACKET_BUF_SMALL";
		break;
	case VTSS_IO_READ_ERROR:
		txt = "VTSS_IO_READ_ERROR";
		break;
	case VTSS_IO_WRITE_ERROR:
		txt = "VTSS_IO_WRITE_ERROR";
		break;
	default:
		txt = "VTSS_?";
		break;
	}
	return txt;
}

void vtss_log(int level, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	loggingv(vtss_logger, level, format, ap);
	va_end(ap);
}

static int vtss_udev_event(notify_t *nb,
			   unsigned long event, void *data)
{
	udev_node_t *node = (udev_node_t *)data;

	if (strcmp(node->filename, _PATH_VITGENIO) != 0)
		return 0;
	switch (event) {
	case UDEV_CREATE:
		vtss_appl_start();
		break;
	case UDEV_DELETE:
		break;
	}
	return 0;
}

static int vtss_switch_port_status(switch_port_t *port)
{
	vtss_port_status_t status;
	vtss_rc rc = vtss_port_status_get(port->port_no, &status);

	if (rc == VTSS_OK) {
		port->full_duplex = status.fdx;
		port->link_state = status.link ? SWITCH_LINK_UP : SWITCH_LINK_DOWN;
		port->port_speed = status.speed;
	}
	return 0;
}

static int vtss_switch_next_mac(switch_mac_t *mac, switch_mac_t *next)
{
	vtss_vid_mac_t search_mac;
	vtss_mac_table_entry_t return_mac;
	vtss_rc rc;

	memset(&search_mac, 0, sizeof(search_mac));
	if (mac) {
		memcpy(search_mac.mac.addr, mac->mac, sizeof (search_mac.mac));
		search_mac.vid = mac->vid;
	}

	rc = vtss_mac_table_get_next(&search_mac, &return_mac);
	/* end iteration */
	if (rc != VTSS_OK)
		return TRUE;

	memcpy(next->mac, return_mac.vid_mac.mac.addr, sizeof (next->mac));
	next->vid = return_mac.vid_mac.vid;
	if (return_mac.aged)
		next->state = SWITCH_MAC_AGED;
	else
		next->state = SWITCH_MAC_LEARNED;
	return FALSE;
}

static void vtss_switch_mac_aging(unsigned long timeout)
{
	vtss_main_config.mac_aging = timeout;
	if (vtss_main_config.mac_aging)
		vtss_mac_age_start();
	else
		vtss_mac_age_stop();
}

static int vtss_start(void)
{
	vtss_udev_notify.call = vtss_udev_event;
	vtss_udev_notify.next = NULL;
	vtss_udev_notify.priority = 9;
	udev_register_notify(&vtss_udev_notify);
	return 0;
}

static void vtss_stop(void)
{
	udev_unregister_notify(&vtss_udev_notify);
}

switch_t vtss_switch = {
	vtss_start,
	vtss_stop,
	vtss_switch_port_status,
	vtss_switch_next_mac,
	vtss_switch_mac_aging,
};

modlinkage int __init vtss_init(void)
{
	vtss_logger = log_register_source(&vtss_log_source);
	if (!vtss_logger)
		return -1;

	/* vtss switch depends on /dev/vitgenio */
	service_register_depend(SWITCH_SERVICE_NAME, UDEV_SERVICE_NAME);
	net_register_switch(&vtss_switch);

#ifdef CONFIG_VTSS_GROCX
	vtss_grocx_init();
#endif
	return 0;
}

modlinkage void __exit vtss_exit(void)
{
#ifdef CONFIG_VTSS_GROCX
	vtss_grocx_exit();
#endif
	net_unregister_switch(&vtss_switch);
	log_unregister_source(vtss_logger);
}

module_init(vtss_init);
module_exit(vtss_exit);
