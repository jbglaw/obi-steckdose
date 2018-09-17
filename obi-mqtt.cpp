#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "obi-common.h"
#include "obi-config.h"
#include "obi-mqtt.h"

static bool reset_relay_active_p = false;
static unsigned long reset_relay_on_millis = 0;
static unsigned long last_reconnect_millis = 0;
static bool mqtt_active_p = false;
static WiFiClient espClient;
static PubSubClient mqtt_client (espClient);


static void
mqtt_callback (char *_topic, uint8_t *_payload, unsigned int len)
{
	char *payload = (char *) _payload;
	char *slash;
	char *topic;

	obi_printf ("Message arrived [%s], len = %u\r\n", _topic, len);
	for (unsigned int i = 0; i < len; i++)
		obi_print (payload[i]);
	obi_println ("");

	/* Find real topic. It's <devname>/topic, so find the slash and
	   continue after it.  */
	slash = strchr (_topic, '/');
	if (! slash)
		return;
	topic = &slash[1];

	/* Handle "<devname>/device=on" and "<devname>/device=off".  */
	if (strcmp (topic, OBI_MQTT_SUBSCRIBE_RELAY) == 0) {
		if (len == 2 && strncmp (payload, "on", strlen ("on")) == 0)
			relay_set (true);
		else if (len == 3 && strncmp (payload, "off", strlen ("off")) == 0)
			relay_set (false);
	}

	/* Handle "<devname>/reset=trigger".  */
	if (strcmp (topic, OBI_MQTT_SUBSCRIBE_RESET) == 0)
		if (len == 7 && strncmp (payload, "trigger", strlen ("trigger")) == 0)
			mqtt_trigger_reset ();

	return;
}

static void
mqtt_reconnect (void)
{
	String mqtt_sub_relay;
	String mqtt_sub_reset;

	if (! mqtt_active_p)
		return;

	if (mqtt_client.connect (cfg.mqtt_name)) {
		mqtt_sub_relay = cfg.mqtt_name;
		mqtt_sub_relay += "/";
		mqtt_sub_relay += OBI_MQTT_SUBSCRIBE_RELAY;

		mqtt_sub_reset = cfg.mqtt_name;
		mqtt_sub_reset += "/";
		mqtt_sub_reset += OBI_MQTT_SUBSCRIBE_RESET;

		mqtt_client.subscribe (mqtt_sub_relay.c_str ());
		mqtt_client.subscribe (mqtt_sub_reset.c_str ());
		mqtt_client.setCallback (&mqtt_callback);
	}

	return;
}

void
mqtt_begin (void)
{
	if (strlen (cfg.mqtt_host) > 0
	    && cfg.mqtt_port != 0) {

		mqtt_client.setServer (cfg.mqtt_host, cfg.mqtt_port);
		mqtt_active_p = true;


		mqtt_reconnect ();
	}

	return;
}

void
mqtt_publish (const char *_topic, const char *value)
{
	if (mqtt_active_p) {
		String topic (cfg.mqtt_name);

		topic += "/";
		topic += _topic;

		mqtt_client.publish (topic.c_str (), value);
	}

	return;
}

void
mqtt_trigger_reset (void)
{
	reset_relay_active_p = true;
	reset_relay_on_millis = millis ();
	// XXX Switch on RESET relay, aka. Wifi LED.
	syslog.logf (LOG_CRIT, "Triggering RESET");

	return;
}

void
mqtt_handle (void)
{
	if (reset_relay_active_p
	    && reset_relay_on_millis + 250 < millis ()) {

		// XXX Switch off RESET relay, aka. Wifi LED.
		syslog.logf (LOG_CRIT, "Releasing RESET");
		reset_relay_active_p = false;
	}

	if (mqtt_active_p) {
		if (! mqtt_client.connected ()
		    && last_reconnect_millis + 10000 < millis ()) {

			last_reconnect_millis = millis ();
			mqtt_reconnect ();
		}

		if (mqtt_client.connected ())
			mqtt_client.loop ();
	}

	return;
}
