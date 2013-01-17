struct event {
	uint16_t ts;
	uint8_t mask;
	uint8_t value;
};

#define EV_COUNT 32

uint8_t ev_count(void);
struct event *ev_last(void);
struct event *ev_first(void);
int8_t ev_drop_first(void);
int8_t ev_push(struct event *evt_in);
void ev_reset(void);
