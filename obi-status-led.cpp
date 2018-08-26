#include "obi-status-led.h"
#include "obi-common.h"

static bool wifi_led_enabled_p = false;
static long last_action = 0;

void
handle_status_led (void)
{

	switch (state) {
		case st_config:
			if (last_action + 500 < millis ()) {
				wifi_led_enabled_p = ! wifi_led_enabled_p;
				digitalWrite (pin_led_wifi, wifi_led_enabled_p);
				relay_set (wifi_led_enabled_p);
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

		case st_running:
			if (relay_on_p) {
				if (wifi_led_enabled_p) {
					if (last_action + 900 < millis ()) {
						wifi_led_enabled_p = ! wifi_led_enabled_p;
						digitalWrite (pin_led_wifi, wifi_led_enabled_p);
						last_action = millis ();
					}
				} else {
					if (last_action + 100 < millis ()) {
						wifi_led_enabled_p = ! wifi_led_enabled_p;
						digitalWrite (pin_led_wifi, wifi_led_enabled_p);
						last_action = millis ();
					}
				}
			} else {
				if (wifi_led_enabled_p) {
					if (last_action + 100 < millis ()) {
						wifi_led_enabled_p = ! wifi_led_enabled_p;
						digitalWrite (pin_led_wifi, wifi_led_enabled_p);
						last_action = millis ();
					}
				} else {
					if (last_action + 900 < millis ()) {
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
