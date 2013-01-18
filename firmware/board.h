/* LEDs */

#define LED_A_PORT    PORTB
#define LED_A_DDR     DDRB
#define LED_A         PB4

#define LED_B_PORT    PORTB
#define LED_B_DDR     DDRB
#define LED_B         PB5

#define led_a_on()     LED_A_PORT |=  _BV(LED_A)
#define led_a_off()    LED_A_PORT &= ~_BV(LED_A)
#define led_a_toggle() LED_A_PORT ^=  _BV(LED_A)

#define led_b_on()     LED_B_PORT |=  _BV(LED_B)
#define led_b_off()    LED_B_PORT &= ~_BV(LED_B)
#define led_b_toggle() LED_B_PORT ^=  _BV(LED_B)

#define led_init()				\
	do {					\
		LED_A_DDR |= _BV(LED_A);	\
		LED_B_DDR |= _BV(LED_B);	\
	} while (0);

/* ADC */

#define USB_PARTITOR_H		330	/* 33k */
#define USB_PARTITOR_L		56	/* 5k6 */
#define POWER_PARTITOR_H	1800	/* 180k */
#define POWER_PARTITOR_L	56	/* 5k6 */

#define USB_N	(USB_PARTITOR_L + USB_PARTITOR_H)
#define USB_D	(USB_PARTITOR_L)
#define POWER_N	(POWER_PARTITOR_H + POWER_PARTITOR_L)
#define POWER_D	(POWER_PARTITOR_L)
