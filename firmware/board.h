/* LEDs */

#define LED_A_PORT    PORTB
#define LED_A_DDR     DDRB
#define LED_A         PB4

#define LED_B_PORT    PORTB
#define LED_B_DDR     DDRB
#define LED_B         PB5

#define led_a_on()     LED_A_PORT &= ~_BV(LED_A);
#define led_a_off()    LED_A_PORT |=  _BV(LED_A);
#define led_a_toggle() LED_A_PORT ^=  _BV(LED_A);

#define led_b_on()     LED_B_PORT &= ~_BV(LED_B);
#define led_b_off()    LED_B_PORT |=  _BV(LED_B);
#define led_b_toggle() LED_B_PORT ^=  _BV(LED_B);

#define led_init()				\
	do {					\
		LED_A_DDR |= _BV(LED_A);	\
		LED_B_DDR |= _BV(LED_B);	\
	} while (0);
