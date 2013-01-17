#include <avr/io.h>
#include <util/delay.h>

#include "board.h"
#include "io.h"

enum {
	CH_PB,
	CH_PC,
	CH_PD,
	CH_NA,
};

struct channel {
	uint8_t  en_port:3,
		 en_nr:4,
		 en_inv:1;
	uint8_t  fault_port:4,
		 fault_nr:4;
	uint8_t  led_port:4,
		 led_nr:4;
	uint8_t  adc_ch;
	uint8_t  i2c_addr;
};

static struct channel channels[CHANNEL_NR] = {
	[CHANNEL_USB1]  = {CH_PD, 0, 1, CH_PD, 5, CH_PC, 1,    0, 0x40},
	[CHANNEL_USB2]  = {CH_PD, 4, 1, CH_PD, 3, CH_PC, 2,    7, 0x41},
	[CHANNEL_POWER] = {CH_PD, 7, 0, CH_NA, 0, CH_PD, 6,    6, 0x42},
	[CHANNEL_IO1]   = {CH_PB, 1, 0, CH_NA, 0, CH_PB, 2, 0xff, 0xff},
	[CHANNEL_IO2]   = {CH_PB, 0, 0, CH_NA, 0, CH_PC, 3, 0xff, 0xff},
};

static void set_output(uint8_t port, uint8_t bit)
{
	switch (port) {
		case CH_PB:
			DDRB |= 1 << bit;
			break;
		case CH_PC:
			DDRC |= 1 << bit;
			break;
		case CH_PD:
			DDRD |= 1 << bit;
			break;
	}
}

static void set_value(uint8_t port, uint8_t bit, uint8_t value)
{
	switch (port) {
		case CH_PB:
			if (value)
				PORTB |= (1 << bit);
			else
				PORTB &= ~(1 << bit);
			break;
		case CH_PC:
			if (value)
				PORTC |= (1 << bit);
			else
				PORTC &= ~(1 << bit);
			break;
		case CH_PD:
			if (value)
				PORTD |= (1 << bit);
			else
				PORTD &= ~(1 << bit);
			break;
	}
}

static uint8_t read_value(uint8_t port, uint8_t bit)
{
	switch (port) {
		case CH_PB:
			return !!(PINB &= 1 << bit);
		case CH_PC:
			return !!(PINC &= 1 << bit);
		case CH_PD:
			return !!(PIND &= 1 << bit);
	}

	return 0;
}

uint8_t get_addr(uint8_t chan)
{
	struct channel *ch = &channels[chan];

	if (chan >= CHANNEL_NR)
		return 0;

	return ch->i2c_addr;
}

uint8_t read_fault(uint8_t chan)
{
	struct channel *ch = &channels[chan];

	if (chan >= CHANNEL_NR)
		return 0;

	if (ch->fault_port != CH_NA)
		return 0;

	return !read_value(ch->fault_port, ch->fault_nr);
}

void set_en(uint8_t chan, uint8_t val)
{
	struct channel *ch = &channels[chan];

	if (chan >= CHANNEL_NR)
		return;

	if (ch->en_inv)
		val = !val;

	set_value(ch->en_port, ch->en_nr, val);
}

void set_led(uint8_t chan, uint8_t val)
{
	struct channel *ch = &channels[chan];

	if (chan >= CHANNEL_NR)
		return;

	set_value(ch->led_port, ch->led_nr, val);
}

void io_init(void)
{
	uint8_t i;
	struct channel *ch;

	for (i = 0; i < CHANNEL_NR; i++) {
		ch = &channels[i];

		/* EN output */
		set_output(ch->en_port, ch->en_nr);
		set_en(i, 0);

		/* LED output */
		set_output(ch->led_port, ch->led_nr);
		set_led(i, 0);

		/* FAULT pull-up */
		if (ch->fault_port != CH_NA)
			set_value(ch->fault_port, ch->fault_nr, 1);
	}
}

