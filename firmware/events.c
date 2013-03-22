/*
 * Copyright 2013 Fabio Baltieri (fabio.baltieri@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 */

#include <stdint.h>
#include <string.h>

#include "events.h"

static struct {
	struct event events[EV_COUNT];
	uint8_t first;
	uint8_t last;
	uint8_t count;
} queue;

uint8_t ev_count(void)
{
	return queue.count;
}

static uint8_t ev_full(void)
{
	return (queue.count == EV_COUNT);
}

struct event *ev_last(void)
{
	return &queue.events[queue.last];
}

struct event *ev_first(void)
{
	return &queue.events[queue.first];
}

int8_t ev_drop_first(void)
{
	if (ev_count() == 0)
		return -1;

	queue.first = (queue.first + 1) % EV_COUNT;
	queue.count--;

	return 0;
}

int8_t ev_push(struct event *ev_in)
{
	struct event *ev;
	
	if (ev_full())
		return -1;

	queue.last = (queue.last + 1) % EV_COUNT;
	queue.count++;

	ev = ev_last();
	memcpy(ev, ev_in, sizeof(*ev));

	return 0;
}

void ev_reset(void)
{
	memset(&queue, 0, sizeof(queue));
	queue.last = EV_COUNT - 1;
}
