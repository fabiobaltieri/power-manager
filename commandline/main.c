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
