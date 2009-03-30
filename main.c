#include <usb.h>
#include <stdio.h>
#include <errno.h>
#include "ccid_priv.h"

/* smartcard reader handle */
usb_dev_handle *g_dev = NULL; 
#define EP_IN 	0x84
#define EP_OUT 	0x05

int find_interclass(struct usb_config_descriptor *config, int cla)
{
	int j, i;
	struct usb_interface *intfc;


	for (i = 0; i < config->bNumInterfaces; i++)
		intfc = &config->interface[i];
		for (j = 0; j < intfc->num_altsetting; j++) {
			if (intfc->altsetting[j].bInterfaceClass == cla)
				return 1;
	}
	return 0;
}


int is_needed(struct usb_device *dev)
{
	int i;
	for (i = 0; i < dev->descriptor.bNumConfigurations; i++) 
		if (find_interclass(&dev->config[i], 0x0b))
			return 1;
	return 0;
}

/* init usb bus and get set g_dev handle */
int test_usb_init(void) 
{
	struct usb_bus *bus; 
	struct usb_device *dev; 
	usb_dev_handle *udev; 

	usb_init(); 
	usb_find_busses(); 
	usb_find_devices(); 
	usb_set_debug(255);

	printf("bus/device idVendor/idProduct\n"); 

	for (bus = usb_busses; bus; bus = bus->next) { 
		for (dev = bus->devices; dev; dev = dev->next) { 
			int ret, i; 
			char string[256]; 
			udev = usb_open(dev);
			if (udev) {
				if (is_needed(dev)) {
					printf("%s/%s %04X/%04X\n", bus->dirname, dev->filename, 
					dev->descriptor.idVendor, dev->descriptor.idProduct); 

#if 0
					if (usb_set_configuration(udev, 1) < 0) {
						printf("set_config err\r\n");
						usb_close(udev);
						break;
					}

					if (usb_set_altinterface(udev, 0) < 0) {
						printf("altset err\r\n");
						usb_close(udev);
						break;
					}

#endif
					if (usb_claim_interface(udev, 0) < 0) {
						printf("claim err\r\n");
						usb_close(udev);
						break;
					}
					g_dev = udev;
					break;
				}
				usb_close(udev);
			}
		}
	}
	return 0;
}

void print_bin2hex(unsigned char *bin, int len)
{
	int i;
	char buff[3*len];

	memset(buff, 0, sizeof (buff));

	printf("recv len=%d\n", len);
	for (i = 0; i < len; i++)
		sprintf(buff + 3 * i, "%02x ", bin[i]);
	printf("%s\n", buff);
}

int do_PowerOn(int bSlot, int bSeq)
{
	unsigned char cmd[10], buff[10];
	int ret, i = 3;

	cmd[0] = PC_TO_RDR_ICCPOWERON;
	cmd[1] = 0x00;
	cmd[2] = 0x00;
	cmd[3] = 0x00;
	cmd[4] = 0x00;
	cmd[5] = bSlot;
	cmd[6] = bSeq;
	cmd[7] = 0x01;	/* v=5V */
	cmd[8] = 0x00;
	cmd[9] = 0x00;

	while (i--) {
		ret = usb_bulk_write(g_dev, EP_OUT, cmd, 10, 50);
		if (ret < 0) {
			printf("GetSlotStatus, err=%s\r\n", strerror(errno));
			continue;
		}
		printf("GetSlotStatus write succ\r\n");
		ret = usb_bulk_read(g_dev, EP_IN, buff, 10, 50);
		if (ret < 0) {
			printf("read bulk err=%s\r\n", strerror(errno));
		} else {
			print_bin2hex(buff, ret);
			break;
		}
	}
	return 0;
}

int do_GetSlotStatus(int bSlot, int bSeq)
{
	unsigned char cmd[10], buff[64];
	int i, ret;

	cmd[0] = PC_TO_RDR_GETSLOTSTATUS;
	cmd[1] = 0x00;
	cmd[2] = 0x00;
	cmd[3] = 0x00;
	cmd[4] = 0x00;
	cmd[5] = bSlot;
	cmd[6] = 1;
	cmd[7] = 0x00;
	cmd[8] = 0x00;
	cmd[9] = 0x00;

	/* warm up */
	i = 3;

	while (i--) {
		ret = usb_bulk_write(g_dev, EP_OUT, cmd, 10, 50);
		if (ret < 0) {
			printf("GetSlotStatus, err=%s\r\n", strerror(errno));
			continue;
		}
		printf("GetSlotStatus write succ\r\n");
		ret = usb_bulk_read(g_dev, EP_IN, buff, 10, 50);
		if (ret < 0) {
			printf("read bulk err=%s\r\n", strerror(errno));
		} else {
			print_bin2hex(buff, ret);
			break;
		}
	}
out:
	return 0;
}

int main(void)
{
	test_usb_init();
	if (g_dev) {
		do_GetSlotStatus(0, 0);
		do_PowerOn(0, 0);
		usb_release_interface(g_dev, 0);
		usb_close(g_dev);
		g_dev = NULL;
	}
	return 0;
}
