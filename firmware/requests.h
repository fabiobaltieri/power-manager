/* frame formats */

struct usb_status {
	uint8_t	switches;
	uint16_t voltage[3];	/* mV */
	uint16_t current[3];	/* mA */
};

#define PM_CH_USB1	0x01
#define PM_CH_USB2	0x02
#define PM_CH_POWER	0x04
#define PM_CH_IO1	0x08
#define PM_CH_IO2	0x10

/* requests */

#define CUSTOM_RQ_STATUS	0x00
#define CUSTOM_RQ_EV_CLEAR	0x01
#define CUSTOM_RQ_EV_PUSH	0x02

#define CUSTOM_RQ_RESET		0xff
