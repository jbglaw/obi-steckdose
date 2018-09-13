#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include "obi-button.h"
#include "obi-common.h"
#include "obi-config.h"
#include "obi-http.h"
#include "obi-mqtt.h"
#include "obi-misc.h"
#include "obi-telnet.h"
#include "obi-status-led.h"
#include "obi-extra-pins.h"

/* Hard-wired pins on PCB.  */
const int pin_relay_on = 12;
const int pin_relay_off = 5;
const int pin_led_wifi = 4;
const int pin_btn = 14;
ADC_MODE (ADC_VCC);

static bool relay_on_p = false;
enum state state = st_running;

static WiFiUDP udpClient;
Syslog syslog (udpClient);
bool have_syslog_p = false;
ESP8266WebServer http_server (80);
/* Keep these two in obi-steckdose.ino so that they're up-to-date for every build.  */
const char obi_git_commit[] = OBI_GIT_COMMIT;
const char obi_build_timestamp[] = OBI_BUILD_TIMESTAMP;

/*
 * Relay handling.
 */
void
relay_set (bool on_p)
{
	int toggle_pin = on_p? pin_relay_on: pin_relay_off;
	char relay_state[10];

	relay_on_p = on_p;

	/* Put both pins HIGH.  */
	digitalWrite (pin_relay_on,  1);
	digitalWrite (pin_relay_off, 1);

	/* Toggle the ON or OFF pin.  */
	digitalWrite (toggle_pin, 0);
	delay (50);
	digitalWrite (toggle_pin, 1);

	snprintf (relay_state, sizeof (relay_state), "%i", (relay_on_p? "on": "off"));
	mqtt_publish (OBI_MQTT_SUBSCRIBE_RELAY, relay_state);

	obi_printf ("Setting relay to %s\r\n", (relay_on_p? "ON": "OFF"));
	syslog.logf (LOG_CRIT, "Setting Relay to %s", (relay_on_p? "ON": "OFF"));

	return;
}

bool
relay_get_state (void)
{
	return relay_on_p;
}


void
setup (void)
{
	Serial.begin (115200, SERIAL_8N1);

	pinMode (pin_btn, INPUT);
	pinMode (pin_led_wifi, OUTPUT);
	pinMode (pin_relay_on, OUTPUT);
	pinMode (pin_relay_off, OUTPUT);

	/* Set to initial off.  */
	relay_set (false);

	http_server.on ("/",       HTTP_GET,  &http_GET_slash);
	http_server.on ("/config", HTTP_POST, &http_POST_config);
	http_server.on ("/on",     HTTP_GET,  &http_GET_on);
	http_server.on ("/off",    HTTP_GET,  &http_GET_off);
	http_server.on ("/toggle", HTTP_GET,  &http_GET_toggle);
	http_server.on ("/status", HTTP_GET,  &http_GET_status);
	http_server.onNotFound (&http_X_not_found);
	http_server.begin ();

	/* Enable config-only mode?  */
	if (config_load (&cfg) != 0
	    || button_pressed_p ()
	    || strlen (cfg.wifi_ssid) == 0
	    || strlen (cfg.wifi_psk) == 0) {

		uint8_t mac[6];
		char default_ssid_name[64];

		WiFi.softAPmacAddress (mac);
		snprintf (default_ssid_name, sizeof (default_ssid_name), "OBI-%02x%02x%02x",
		          mac[3], mac[4], mac[5]);

		state = st_config;

		Serial.printf ("Starting AP wifi with %s", default_ssid_name);
		WiFi.mode (WIFI_AP);
		WiFi.softAP (default_ssid_name);
		MDNS.begin (default_ssid_name);

		while (1) {
			http_server.handleClient ();
			status_led_handle ();
		}
	}

	/* After configuration, set to proper state as early as possible.  */
	relay_set (cfg.relay_on_after_boot_p);

	Serial.printf ("Starting STA wifi with %s / %s", cfg.wifi_ssid, cfg.wifi_psk);
	WiFi.mode (WIFI_STA);
	if (strlen (cfg.dev_mqtt_name) > 0)
		WiFi.hostname (cfg.dev_mqtt_name);
	WiFi.begin (cfg.wifi_ssid, cfg.wifi_psk);
	state = st_connecting;
	while (WiFi.status () != WL_CONNECTED) {
		delay (10);
		status_led_handle ();
	}
	WiFi.setAutoReconnect (true);

	/* Init syslog.  */
	if (strlen (cfg.syslog_host) > 0
	    && strlen (cfg.syslog_port) > 0
	    && atoi (cfg.syslog_port) > 0
	    && atoi (cfg.syslog_port) < 6536) {

		syslog.server (cfg.syslog_host, atoi (cfg.syslog_port));
		if (strlen (cfg.dev_mqtt_name) > 0)
			syslog.deviceHostname (cfg.dev_mqtt_name);
		syslog.appName ("firmware");
		have_syslog_p = true;
		syslog.logf (LOG_CRIT, "OBI-Steckdose `%s' starting up in STA mode using GIT=%s built at %s",
		             cfg.dev_mqtt_name, obi_git_commit, obi_build_timestamp);
	}

	/* Bring up all the stuff.  */
	Serial.begin (cfg.serial_speed, serial_framing ());
	MDNS.begin (cfg.dev_mqtt_name);
	http_server.begin ();
	telnet_begin ();
	mqtt_begin ();
	extra_pins_begin ();

	/* Publish initial relais state after MQTT server should be up.  */
	relay_set (cfg.relay_on_after_boot_p);

	state = st_running;
}

void
loop (void)
{
	http_server.handleClient ();
	telnet_handle ();
	mqtt_handle ();
	extra_pins_handle ();
	status_led_handle ();
	button_handle ();
}
