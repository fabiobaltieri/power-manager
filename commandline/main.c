#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <endian.h>

#include <usb.h>

#include "opendevice.h"

#include "../firmware/requests.h"

#define PRODUCT "power-manager"

static void send_reset(usb_dev_handle *handle)
{
	int ret;

	ret = usb_control_msg(handle,
			      USB_TYPE_VENDOR | USB_RECIP_DEVICE |
			      USB_ENDPOINT_IN,
			      CUSTOM_RQ_RESET,
			      0, 0, NULL, 0, 1000);

	if (ret < 0) {
		printf("usb_control_msg: %s\n", usb_strerror());
		exit(1);
	}
}

static void status(usb_dev_handle *handle)
{
	int ret;
	struct usb_status status;

	ret = usb_control_msg(handle,
			      USB_TYPE_VENDOR | USB_RECIP_DEVICE |
			      USB_ENDPOINT_IN,
			      CUSTOM_RQ_STATUS,
			      0, 0, (char *)&status, sizeof(status), 1000);

	if (ret < 0) {
		printf("usb_control_msg: %s\n", usb_strerror());
		exit(1);
	}

	/* print status */
	printf("               USB1   USB2  POWER\n");
	printf(" voltage_in: %6hd %6hd %6hd\n",
	       status.voltage_in[0],
	       status.voltage_in[1],
	       status.voltage_in[2]);
	printf("voltage_out: %6hd %6hd %6hd\n",
	       status.voltage_out[0],
	       status.voltage_out[1],
	       status.voltage_out[2]);
	printf("    current: %6hd %6hd %6hd\n",
	       status.current[0],
	       status.current[1],
	       status.current[2]);
	printf("      power: %6hd %6hd %6hd\n",
	       status.power[0],
	       status.power[1],
	       status.power[2]);
	printf("       fail: %02x\n", status.fail);
}

static void usage(char *name)
{
	fprintf(stderr, "Usage: %s -h\n", name);
	fprintf(stderr, "       %s -R\n", name);
	fprintf(stderr, "       %s [options]\n", name);
	fprintf(stderr, "options:\n"
			"  -h         this help\n"
			"  -R         reset device\n"
			);
	exit(1);
}

int main(int argc, char **argv)
{
	usb_dev_handle *handle = NULL;
	int opt;
	int reset = 0;

	usb_init();

	while ((opt = getopt(argc, argv, "hR")) != -1) {
		switch (opt) {
		case 'h':
			usage(argv[0]);
			break;
		case 'R':
			reset = 1;
			break;
		default:
			usage(argv[0]);
		}
	}

	if (usbOpenDevice(&handle, 0, NULL, 0, PRODUCT, NULL, NULL, NULL)) {
		fprintf(stderr, "error: could not find USB device \"%s\"\n", PRODUCT);
		exit(1);
	}

	if (reset) {
		send_reset(handle);
		return 0;
	}

	/* TODO: do stuff */

	usb_close(handle);

	return 0;
}
