#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "obi-common.h"
#include "obi-config.h"
#include "obi-mqtt.h"

static bool reset_relay_active_p = false;
static unsigned long reset_relay_on_millis = 0;
static bool mqtt_active_p = false;
static WiFiClient espClient;
static PubSubClient mqtt_client (espClient);

static void
mqtt_callback (char *topic, byte *payload, unsigned int len)
{
	obi_printf ("Message arrived [%s], len = %li\n", topic, len);
	for (int i = 0; i < len; i++)
		obi_print ((char) payload[i]);
	obi_println ();

	/* Handle "device=0" and "device=1".  */
	if (strcmp (topic, MQTT_SUBSCRIBE_RELAIS) == 0) {
		if (len >= 1 && payload[0] == '1')
			relay_set (true);
		else if (len >= 1 && payload[0] == '0')
			relay_set (false);
	}

	/* Handle "reset=1".  */
	if (strcmp (topic, MQTT_SUBSCRIBE_RESET) == 0)
		if (len >= 1 && payload[0] == '1')
			mqtt_trigger_reset ();
}

static void
mqtt_reconnect (void)
{
	if (mqtt_client.connect (cfg.dev_mqtt_name)) {
		mqtt_client.subscribe (MQTT_SUBSCRIBE_RELAIS);
		mqtt_client.subscribe (MQTT_SUBSCRIBE_RESET);
	}
}

void
mqtt_begin (void)
{
	if (strlen (cfg.mqtt_server_ip) > 0
	    && strlen (cfg.dev_mqtt_name) > 0
	    && strlen (cfg.mqtt_server_port) > 0
	    && atoi (cfg.mqtt_server_port) > 0
	    && atoi (cfg.mqtt_server_port) < 65536) {

		mqtt_client.setServer (cfg.mqtt_server_ip, atoi (cfg.mqtt_server_port));
		mqtt_client.setCallback (&mqtt_callback);
		mqtt_active_p = true;

		mqtt_reconnect ();
	}
}

void
mqtt_publish (const char *topic, const char *value)
{
	if (mqtt_active_p)
		mqtt_client.publish (topic, value);

	return;
}

void
mqtt_trigger_reset (void)
{
	reset_relay_active_p = true;
	reset_relay_on_millis = millis ();
	// XXX Switch on RESET relay, aka. Wifi LED.

	return;
}

void
mqtt_handle (void)
{
	if (reset_relay_active_p
	    && reset_relay_on_millis + 250 < millis ()) {

		// XXX Switch off RESET relay, aka. Wifi LED.
		reset_relay_active_p = false;
	}

	if (mqtt_active_p) {
		if (! mqtt_client.connected ())
			mqtt_reconnect ();

		mqtt_client.loop ();
	}

	return;
}
