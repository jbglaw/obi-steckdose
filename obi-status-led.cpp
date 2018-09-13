#include "obi-status-led.h"
#include "obi-common.h"

static bool wifi_led_enabled_p = false;
static unsigned long last_action = 0;

static unsigned long last_test_millis = 0;
static unsigned long test_counter = 0;

void
status_led_handle (void)
{
	switch (state) {
		case st_config:
			if (last_action + 500 < millis ()) {
				wifi_led_enabled_p = ! wifi_led_enabled_p;
				digitalWrite (pin_led_wifi, wifi_led_enabled_p);
				last_action = millis ();
			}
			break;

		case st_connecting:
			if (last_action + 100 < millis ()) {
				wifi_led_enabled_p = ! wifi_led_enabled_p;
				digitalWrite (pin_led_wifi, wifi_led_enabled_p);
				last_action = millis ();
			}
			break;

		case st_flashing:
			last_action = millis ();
			digitalWrite (pin_led_wifi, true);
			break;

		case st_running:
			if (relay_get_state ()) {
				if (wifi_led_enabled_p) {
					if (last_action + 150 < millis ()) {
						wifi_led_enabled_p = ! wifi_led_enabled_p;
						digitalWrite (pin_led_wifi, wifi_led_enabled_p);
						last_action = millis ();
					}
				} else {
					if (last_action + 850 < millis ()) {
						wifi_led_enabled_p = ! wifi_led_enabled_p;
						digitalWrite (pin_led_wifi, wifi_led_enabled_p);
						last_action = millis ();
					}
				}
			} else {
				if (wifi_led_enabled_p) {
					if (last_action + 10 < millis ()) {
						wifi_led_enabled_p = ! wifi_led_enabled_p;
						digitalWrite (pin_led_wifi, wifi_led_enabled_p);
						last_action = millis ();
					}
				} else {
					if (last_action + 990 < millis ()) {
						wifi_led_enabled_p = ! wifi_led_enabled_p;
						digitalWrite (pin_led_wifi, wifi_led_enabled_p);
						last_action = millis ();
					}
				}
			}
			break;
	}

	return;
}
