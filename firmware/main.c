#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "usbdrv.h"

#include "board.h"
#include "requests.h"
#include "jiffies.h"
#include "io.h"
#include "events.h"

static struct usb_status status;

static void reset_cpu(void)
{
	wdt_disable();

	wdt_enable(WDTO_15MS);

	for (;;);
}

usbMsgLen_t usbFunctionSetup(uint8_t data[8])
{
	struct usbRequest *rq = (void *)data;
	struct event ev;

	switch (rq->bRequest) {
	case CUSTOM_RQ_STATUS:
		status.fail = (read_fault(CHANNEL_USB1) ? 1 : 0) |
			(read_fault(CHANNEL_USB2) ? 2 : 0);
		usbMsgPtr = (uint8_t *)&status;
		return sizeof(status);
		break;
	case CUSTOM_RQ_EV_CLEAR:
		ev_reset();
		break;
	case CUSTOM_RQ_EV_PUSH:
		if (ev_count() == 0) {
			/* start from current time */
			ev.ts = jiffies + rq->wValue.word;
		} else {
			/* start from last event */
			struct event *tmp = ev_last();
			ev.ts = tmp->ts + rq->wValue.word;
		}
		ev.mask = rq->wIndex.bytes[1];
		ev.value = rq->wIndex.bytes[0];
		ev_push(&ev);
		break;
	case CUSTOM_RQ_RESET:
		reset_cpu();
		break;
	default:
		/* do nothing */
		;
	}

	return 0;
}

static void check_io(struct event *ev, uint8_t mask, uint8_t ch)
{
	if (ev->mask & mask) {
		if (ev->value & mask) {
			set_led(ch, 1);
			set_en(ch, 1);
		} else {
			set_en(ch, 0);
			set_led(ch, 0);
		}
	}
}

#define time_after(a, b) ((int16_t)b - (int16_t)a > 0)
static void event_poll(void)
{
	static uint16_t holdoff = 0;
	struct event *ev;

	/* status LEDs */
	if (ev_count()) {
		led_a_off();
		if (time_after(jiffies, holdoff))
			led_b_off();
		else
			led_b_on();
	} else {
		led_a_on();
		led_b_off();
	}

	/* event processing */
	while (ev_count()) {
		ev = ev_first();

		if (time_after(jiffies, ev->ts))
			return;

		check_io(ev, PM_CH_USB1, CHANNEL_USB1);
		check_io(ev, PM_CH_USB2, CHANNEL_USB2);
		check_io(ev, PM_CH_POWER, CHANNEL_POWER);
		check_io(ev, PM_CH_IO1, CHANNEL_IO1);
		check_io(ev, PM_CH_IO2, CHANNEL_IO2);

		ev_drop_first();

		holdoff = jiffies + 5;
	}
}

static void hello(void)
{
	uint8_t i;

	for (i = 0; i < 8; i++) {
		led_a_toggle();
		led_b_toggle();
		_delay_ms(50);
	}
}

int __attribute__((noreturn)) main(void)
{
	uint8_t i;

	led_init();
	led_a_on();
	led_b_off();

	io_init();
	ev_reset();

	wdt_enable(WDTO_1S);

	hello();

	jiffies_init();

	usbInit();
	usbDeviceDisconnect();

	i = 0;
	while (--i) {
		wdt_reset();
		_delay_ms(1);
	}

	usbDeviceConnect();

	sei();
	for (;;) {
		wdt_reset();
		usbPoll();
		event_poll();
	}
}
