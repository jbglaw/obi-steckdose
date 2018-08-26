#include "obi-button.h"
#include "obi-common.h"

static long button_press_detected_millis = 0;
static bool button_was_pressed_p = false;
static bool button_action_done_p = false;

bool
button_pressed_p (void)
{
	return ! digitalRead (pin_btn);
}

void
handle_button (void)
{
	if (button_pressed_p ()) {
		if (! button_was_pressed_p) {
			button_press_detected_millis = millis ();
			button_was_pressed_p = true;
		}
	} else {
		button_was_pressed_p = false;
		button_action_done_p = false;
	}

	if (button_was_pressed_p
	    && ! button_action_done_p
	    && millis () > button_press_detected_millis + 500) {

		obi_printf ("Setting relay from %s to %s\r\n",
		            (relay_on_p? "on": "off"),
		            (relay_on_p? "off": "on"));

		relay_set (! relay_on_p);
		button_action_done_p = true;
	}
}
