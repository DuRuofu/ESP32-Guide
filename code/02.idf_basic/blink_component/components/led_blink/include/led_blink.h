#ifndef LED_BLINK_H
#define LED_BLINK_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

	// Function declarations
	void blink_led(uint8_t s_led_state);
	void configure_led(void);

#ifdef __cplusplus
}
#endif

#endif /* LED_BLINK_H */
