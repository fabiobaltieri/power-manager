/*
 * Copyright 2013 Fabio Baltieri (fabio.baltieri@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

#include "adc.h"

void adc_init(void)
{
	ADCSRA = ( (1 << ADEN)  | /* enable           */
		   (0 << ADSC)  | /* start conversion */
		   (0 << ADATE) | /* free running     */
		   (1 << ADIF)  | /* clear interrupts */
		   (0 << ADIE)  | /* interrupt enable */
		   (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) );

	/* 12Mhz / 128 = 94kHz */

	/* turn on bandgap */
	ADMUX = _BV(REFS1) | _BV(REFS0);
}

void adc_stop(void)
{
	ADCSRA = 0x00; /* ADC disable */
}

void adc_set_channel(uint8_t channel)
{
	/* always use 1.1 Vref */
	ADMUX = _BV(REFS1) | _BV(REFS0) | (channel & 0xff);
}

uint16_t adc_get_value(void)
{
	ADCSRA |= _BV(ADSC);

	loop_until_bit_is_clear(ADCSRA, ADSC);

	return ADCW;
}
