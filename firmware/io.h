enum {
	CHANNEL_USB1,
	CHANNEL_USB2,
	CHANNEL_POWER,
	CHANNEL_IO1,
	CHANNEL_IO2,
	CHANNEL_NR,
};

uint8_t get_addr(uint8_t chan);
uint8_t get_adc_ch(uint8_t chan);
uint8_t read_fault(uint8_t chan);
void set_en(uint8_t chan, uint8_t val);
void set_led(uint8_t chan, uint8_t val);
void io_init(void);
