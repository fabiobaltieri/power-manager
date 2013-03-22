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

#include "jiffies.h"

volatile uint16_t jiffies;

void jiffies_init(void)
{
	jiffies = 0;

	TIMSK1 = (1 << OCIE1A);

	TCCR1A = ((0 << WGM11) | (0 << WGM10)); /* CTC -> OCR1A*/
	TCCR1B = ((0 << WGM13) | (1 << WGM12) |
			(0 << CS12) | (1 << CS11) | (1 << CS10)); /* /64 */

	OCR1A = F_CPU / 64 / HZ;
}

ISR(TIMER1_COMPA_vect)
{
	jiffies++;
}
