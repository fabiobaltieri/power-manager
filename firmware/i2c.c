/*
 * Copyright 2013 Fabio Baltieri (fabio.baltieri@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include <avr/io.h>
#include <util/twi.h>
#include <stdio.h>

#include "i2c.h"

static int8_t i2c_rdwr(uint8_t addr,
		       uint8_t *out_buf, uint8_t out_len,
		       uint8_t *in_buf, uint8_t in_len)
{
	uint8_t i;

	/* start condition */
	TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
	loop_until_bit_is_set(TWCR, TWINT);

	if (TW_STATUS != TW_START)
		goto out;

	/* device address */
	TWDR = (addr << 1) | TW_WRITE;
	TWCR = _BV(TWINT) | _BV(TWEN);
	loop_until_bit_is_set(TWCR, TWINT);

	if (TW_STATUS != TW_MT_SLA_ACK)
		goto out;

	/* data out */
	for (i = 0; i < out_len; i++) {
		TWDR = out_buf[i];
		TWCR = _BV(TWINT) | _BV(TWEN);
		loop_until_bit_is_set(TWCR, TWINT);

		if (TW_STATUS != TW_MT_DATA_ACK)
			goto out;
	}

	if (in_len == 0)
		goto stop;

	/* repeated start condition */
	TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
	loop_until_bit_is_set(TWCR, TWINT);

	if (TW_STATUS != TW_REP_START)
		goto out;

	/* device address */
	TWDR = (addr << 1) | TW_READ;
	TWCR = _BV(TWINT) | _BV(TWEN);
	loop_until_bit_is_set(TWCR, TWINT);

	if (TW_STATUS != TW_MR_SLA_ACK)
		goto out;

	/* read data */
	for (i = 0; i < in_len; i++) {
		if (i + 1 < in_len)
			TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN);
		else
			/* last byte for us, send NACK */
			TWCR = _BV(TWINT) | _BV(TWEN);
		loop_until_bit_is_set(TWCR, TWINT);

		in_buf[i] = TWDR;

		if (TW_STATUS != TW_MR_DATA_ACK)
			goto out;
	}

stop:
	/* stop condition */
	TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);

	return 0;

out:
	TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);

	return -1;
}

uint16_t i2c_read_word(uint8_t addr, uint8_t reg)
{
	uint8_t out[2];

	i2c_rdwr(addr, &reg, 1, out, sizeof(out));

	return (out[0] << 8) | out[1];
}

void i2c_write_word(uint8_t addr, uint8_t reg, uint16_t val)
{
	uint8_t out[3];

	out[0] = reg;
	out[1] = val >> 8;
	out[2] = val & 0xff;

	i2c_rdwr(addr, out, sizeof(out), NULL, 0);
}

void i2c_init(uint32_t freq)
{
	TWBR = ((F_CPU / freq) - 16) / 2;
	TWCR = _BV(TWEN);
}
