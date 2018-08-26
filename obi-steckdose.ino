#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include "obi-button.h"
#include "obi-common.h"
#include "obi-config.h"
#include "obi-http.h"
#include "obi-misc.h"
#include "obi-telnet.h"
#include "obi-status-led.h"

const int pin_relay_on = 12;
const int pin_relay_off = 5;
const int pin_led_wifi = 4;
const int pin_btn = 14;

bool relay_on_p = false;
enum state state = st_running;

ESP8266WebServer	http_server (80);
static ObiTelnet	telnet_server;

/*
 * Relay handling.
 */
void
relay_set (bool on_p)
{
	int toggle_pin = on_p? pin_relay_on: pin_relay_off;
	relay_on_p = on_p;

	/* Put both pins HIGH.  */
	digitalWrite (pin_relay_on,  1);
	digitalWrite (pin_relay_off, 1);

	/* Toggle the ON or OFF pin.  */
	digitalWrite (toggle_pin, 0);
	delay (50);
	digitalWrite (toggle_pin, 1);

	obi_printf ("Setting relay to %s\r\n", (relay_on_p? "ON": "OFF"));

	return;
}

void
setup (void)
{
	Serial.begin (115200, SERIAL_8N1);

	pinMode (pin_btn, INPUT);
	pinMode (pin_led_wifi, OUTPUT);
	pinMode (pin_relay_on, OUTPUT);
	pinMode (pin_relay_off, OUTPUT);

	relay_set (false);

	http_server.on ("/config", HTTP_POST, &http_POST_config);
	http_server.on ("/status", HTTP_GET,  &http_GET_status);
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
			handle_status_led ();
		}
	}

	relay_set (cfg.relay_on_after_boot_p);	// XXX Delay
	Serial.printf ("Starting STA wifi with %s / %s", cfg.wifi_ssid, cfg.wifi_psk);
	WiFi.mode (WIFI_STA);
	WiFi.begin (cfg.wifi_ssid, cfg.wifi_psk);

	state = st_connecting;
	while (WiFi.status () != WL_CONNECTED) {
		delay (10);
		handle_status_led ();
	}
	WiFi.setAutoReconnect (true);

	/* Bring up all the stuff.  */
	Serial.begin (cfg.serial_speed, serial_framing ());
	http_server.begin ();
	telnet_server.begin ();
	MDNS.begin (cfg.dev_mqtt_name);
	// XXX MQTT

	state = st_running;
}

void
loop (void)
{
	http_server.handleClient ();
	telnet_server.handle ();
	handle_status_led ();
	handle_button ();
}
