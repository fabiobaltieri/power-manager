/*
 * Copyright 2013 Fabio Baltieri (fabio.baltieri@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 */
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

#include "../firmware/requests.h"

#define VENDOR_ID 0x1d50
#define PRODUCT_ID 0x6061

static int usb_open_device(usb_dev_handle **device,
		int vendor_id, int product_id)
{
	struct usb_bus *bus;
	struct usb_device *dev;
	usb_dev_handle *handle;

	usb_find_busses();
	usb_find_devices();

	for (bus = usb_get_busses(); bus; bus = bus->next) {
		for (dev = bus->devices; dev; dev = dev->next) {
			if (dev->descriptor.idVendor == vendor_id &&
					dev->descriptor.idProduct == product_id) {
				handle = usb_open(dev);
				if (!handle)
					continue;

				*device = handle;
				return 0;
			}
		}
	}

	return -1;
}

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

static void send_clear(usb_dev_handle *handle)
{
	int ret;

	ret = usb_control_msg(handle,
			      USB_TYPE_VENDOR | USB_RECIP_DEVICE |
			      USB_ENDPOINT_IN,
			      CUSTOM_RQ_EV_CLEAR,
			      0, 0, NULL,0, 1000);

	if (ret < 0) {
		printf("usb_control_msg: %s\n", usb_strerror());
		exit(1);
	}
}

static void show_status(usb_dev_handle *handle)
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
	printf("              USB1%c  USB2%c POWER%c   IO1%c   IO2%c\n",
	       (status.state & PM_CH_USB1) ? '*' : ' ',
	       (status.state & PM_CH_USB2) ? '*' : ' ',
	       (status.state & PM_CH_POWER) ? '*' : ' ',
	       (status.state & PM_CH_IO1) ? '*' : ' ',
	       (status.state & PM_CH_IO2) ? '*' : ' ');
	printf(" voltage_in: %6hd %6hd %6hd\n",
	       le16toh(status.voltage_in[0]),
	       le16toh(status.voltage_in[1]),
	       le16toh(status.voltage_in[2]));
	printf("voltage_out: %6hd %6hd %6hd\n",
	       le16toh(status.voltage_out[0]),
	       le16toh(status.voltage_out[1]),
	       le16toh(status.voltage_out[2]));
	printf("    current: %6hd %6hd %6hd\n",
	       le16toh(status.current[0]),
	       le16toh(status.current[1]),
	       le16toh(status.current[2]));
	printf("      power: %6hd %6hd %6hd\n",
	       le16toh(status.power[0]),
	       le16toh(status.power[1]),
	       le16toh(status.power[2]));
	printf("       fail: %02x\n", status.fail);
}

static void send_event(usb_dev_handle *handle,
		       int delay, int mask, int value)
{
	int ret;
	uint16_t usb_index;
	uint16_t usb_value;

	usb_index = delay & 0xffff;
	usb_value = ((mask & 0xff) << 8) | (value & 0xff);

	ret = usb_control_msg(handle,
			      USB_TYPE_VENDOR | USB_RECIP_DEVICE |
			      USB_ENDPOINT_OUT,
			      CUSTOM_RQ_EV_PUSH,
			      usb_index, usb_value,
			      NULL, 0, 1000);

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
			"  -s         show status\n"
			"  -w         wait delay in tenth of seconds\n"
			"  -C         clear event queue\n"
			"  -e list    enable listed i/o (comma separated)\n"
			"  -d list    disable listed i/o (comma separated)\n"
			"\n"
			"valid i/o: usb1,usb2,power,io1,io2,all\n"
			);
	exit(1);
}

static int parse_io(char *str)
{
	int ret = 0;
	char *tk;

	while ((tk = strtok(str, ",;:")) != NULL) {
		if (strcmp(tk, "usb1") == 0) {
			ret |= PM_CH_USB1;
		} else if (strcmp(tk, "usb2") == 0) {
			ret |= PM_CH_USB2;
		} else if (strcmp(tk, "power") == 0) {
			ret |= PM_CH_POWER;
		} else if (strcmp(tk, "io1") == 0) {
			ret |= PM_CH_IO1;
		} else if (strcmp(tk, "io2") == 0) {
			ret |= PM_CH_IO2;
		} else if (strcmp(tk, "all") == 0) {
			ret |= PM_CH_USB1 | PM_CH_USB2 |
				PM_CH_POWER |
				PM_CH_IO1 | PM_CH_IO2;
		} else {
			printf("unknown i/o: %s\n", tk);
		}
		str = NULL;
	}

	return ret;
}

int main(int argc, char **argv)
{
	usb_dev_handle *handle = NULL;
	int opt;
	int reset = 0;
	int status = 0;
	int mask = 0;
	int value = 0;
	int delay = 0;
	int clear = 0;
	int tmp;

	usb_init();

	while ((opt = getopt(argc, argv, "hRCse:d:w:")) != -1) {
		switch (opt) {
		case 'h':
			usage(argv[0]);
			break;
		case 'R':
			reset = 1;
			break;
		case 'C':
			clear = 1;
			break;
		case 's':
			status = 1;
			break;
		case 'w':
			delay = strtol(optarg, NULL, 0);
			break;
		case 'e':
			tmp = parse_io(optarg);
			value |= tmp;
			mask |= tmp;
			break;
		case 'd':
			tmp = parse_io(optarg);
			value &= ~tmp;
			mask |= tmp;
			break;
		default:
			usage(argv[0]);
		}
	}

	if (usb_open_device(&handle, VENDOR_ID, PRODUCT_ID)) {
		fprintf(stderr, "error: could not find USB device %04x:%04x\n",
				VENDOR_ID, PRODUCT_ID);
		exit(1);
	}

	if (reset) {
		send_reset(handle);
		return 0;
	}

	if (clear)
		send_clear(handle);

	if (status)
		show_status(handle);

	if (mask)
		send_event(handle, delay, mask, value);

	usb_close(handle);

	return 0;
}
