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

uint8_t ev_full(void)
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
		return - 1;

	queue.first = (queue.first + 1) % EV_COUNT;
	queue.count--;

	return 0;
}

int8_t ev_push(struct event *evt_in)
{
	struct event *evt;
	
	if (ev_full())
		return -1;

	queue.last = (queue.last + 1) % EV_COUNT;
	queue.count++;

	evt = ev_last();
	memcpy(evt, evt_in, sizeof(*evt));

	return 0;
}

void ev_reset(void)
{
	memset(&queue, 0, sizeof(queue));
	queue.last = EV_COUNT - 1;
}
