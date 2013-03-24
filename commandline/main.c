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

#include <libusb.h>

#include "../firmware/requests.h"

#define VENDOR_ID 0x1d50
#define PRODUCT_ID 0x6061

static void send_reset(libusb_device_handle *usbdev)
{
	int ret;

	ret = libusb_control_transfer(usbdev,
			LIBUSB_REQUEST_TYPE_VENDOR |
			LIBUSB_RECIPIENT_DEVICE |
			LIBUSB_ENDPOINT_IN,
			CUSTOM_RQ_RESET,
			0, 0, NULL, 0, 1000);

	if (ret < 0) {
		printf("libusb_control_transfer: %s\n", libusb_error_name(ret));
		exit(1);
	}
}

static void send_clear(libusb_device_handle *usbdev)
{
	int ret;

	ret = libusb_control_transfer(usbdev,
			LIBUSB_REQUEST_TYPE_VENDOR |
			LIBUSB_RECIPIENT_DEVICE |
			LIBUSB_ENDPOINT_IN,
			CUSTOM_RQ_EV_CLEAR,
			0, 0, NULL,0, 1000);

	if (ret < 0) {
		printf("libusb_control_transfer: %s\n", libusb_error_name(ret));
		exit(1);
	}
}

static void show_status(libusb_device_handle *usbdev)
{
	int ret;
	struct usb_status status;

	ret = libusb_control_transfer(usbdev,
			      LIBUSB_REQUEST_TYPE_VENDOR |
			      LIBUSB_RECIPIENT_DEVICE |
			      LIBUSB_ENDPOINT_IN,
			      CUSTOM_RQ_STATUS,
			      0, 0,
			      (unsigned char *)&status, sizeof(status), 1000);

	if (ret < 0) {
		printf("libusb_control_transfer: %s\n", libusb_error_name(ret));
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

static void send_event(libusb_device_handle *usbdev,
		       int delay, int mask, int value)
{
	int ret;
	uint16_t usb_index;
	uint16_t usb_value;

	usb_index = delay & 0xffff;
	usb_value = ((mask & 0xff) << 8) | (value & 0xff);

	ret = libusb_control_transfer(usbdev,
			LIBUSB_REQUEST_TYPE_VENDOR |
			LIBUSB_RECIPIENT_DEVICE |
			LIBUSB_ENDPOINT_IN,
			CUSTOM_RQ_EV_PUSH,
			usb_index, usb_value,
			NULL, 0, 1000);

	if (ret < 0) {
		printf("libusb_control_transfer: %s\n", libusb_error_name(ret));
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
	libusb_context *ctx;
	libusb_device_handle *usbdev;
	int opt;
	int reset = 0;
	int status = 0;
	int mask = 0;
	int value = 0;
	int delay = 0;
	int clear = 0;
	int tmp;

	libusb_init(&ctx);

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

	usbdev = libusb_open_device_with_vid_pid(ctx, VENDOR_ID, PRODUCT_ID);
	if (!usbdev) {
		fprintf(stderr, "error: could not find USB device %04x:%04x\n",
				VENDOR_ID, PRODUCT_ID);
		exit(1);
	}

	if (reset) {
		send_reset(usbdev);
		return 0;
	}

	if (clear)
		send_clear(usbdev);

	if (status)
		show_status(usbdev);

	if (mask)
		send_event(usbdev, delay, mask, value);

	libusb_close(usbdev);

	libusb_exit(ctx);

	return 0;
}
