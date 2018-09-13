#include "obi-status-led.h"
#include "obi-common.h"

/* Pins available for extra fun.  */
const int pin_extra1 =  0;	/* Pulled up with 10k, at boot-up: Flashing enabled if connected to GND.  */
const int pin_extra2 =  2;	/* Pulled up with 10k, used for flash mode during boot-up.  */
const int pin_extra3 = 15;	/* Pulled down with 10k, used for flash mode during boot-up.  */
const int pin_extra4 = 16;	/* No pull-up/down connected.  */

static unsigned long last_test_millis = 0;
static unsigned long test_counter = 0;

void
extra_pins_begin (void)
{
	pinMode (pin_extra1, OUTPUT);
	pinMode (pin_extra2, OUTPUT);
	pinMode (pin_extra3, OUTPUT);
	pinMode (pin_extra4, OUTPUT);
}

void
extra_pins_handle (void)
{
	if (last_test_millis + 1500 < millis ()) {
		last_test_millis = millis ();
		test_counter++;

		/* Just toggle these pins around to allow measurements with multimeter.  */
		digitalWrite (pin_extra1, (test_counter & 0x01)? HIGH: LOW);
		digitalWrite (pin_extra2, (test_counter & 0x02)? HIGH: LOW);
		digitalWrite (pin_extra3, (test_counter & 0x04)? HIGH: LOW);
		digitalWrite (pin_extra4, (test_counter & 0x08)? HIGH: LOW);
	}

	return;
}
