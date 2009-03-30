#include <usb.h>
#include <stdio.h>

#include "ccid.h"

/* smartcard reader handle */
usb_dev_handle *g_dev = NULL; 
#define EP_IN 	0x84
#define EP_OUT 	0x05

typedef unsigned char u8;
char output[64];
char input[64];

#define ASSERT_TRUE(fun)	\
	do {			\
		int rv;		\
		rv = fun();	\
		if (!rv) {	\
			printf("assert fun err\r\n");	\
			exit(1);			\
		}					\
	} while (0)


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
int get_usb_dev(void) 
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
	/* found */
	if (g_dev)
		return 1;
	return 0;
}

void print_bin2hex(unsigned char *tmp_bin, int len)
{
	int i;

	if (len * 3 > 64) {
		printf("default output len is samll\r\n");
		return;
	}
	memset(output, 0, sizeof (output));

	printf("recv len=%d\n", len);
	for (i = 0; i < len; i++)
		sprintf(output + 3 * i, "%02x ", tmp_bin[i]);
	printf("%s\n", output);
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
	return 0;
}

int warm_up(void)
{
	int i = 10;
	int ret;

	while (i--) {
		char cmd[2];
		cmd[1] = 0x02;
		cmd[0] = 0x20;

		ret = usb_bulk_write(g_dev, EP_OUT, cmd, sizeof (cmd), 50);
		if (ret < 0) {
			printf("GetSlotStatus, err=%s\r\n", strerror(errno));
			continue;
		}
		printf("GetSlotStatus write succ\r\n");
		ret = usb_bulk_read(g_dev, EP_IN, cmd, sizeof (cmd), 50);
		if (ret < 0) {
			printf("read bulk err=%s\r\n", strerror(errno));
		} else {
			return 1;
		}
	}
	return 0;
}

int test_init(void)
{
	ASSERT_TRUE(get_usb_dev);
	ASSERT_TRUE(warm_up);

	return 0;
}
int main(void)
{
	test_init();
	if (g_dev) {
		do_GetSlotStatus(0, 0);
		do_PowerOn(0, 0);
		usb_release_interface(g_dev, 0);
		usb_close(g_dev);
		g_dev = NULL;
	}
	return 0;
}
