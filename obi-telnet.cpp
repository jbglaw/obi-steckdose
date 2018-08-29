#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "Syslog.h"
#include "obi-common.h"
#include "obi-config.h"
#include "obi-telnet.h"

static WiFiUDP udpClient;
static Syslog syslog (udpClient);
static bool have_syslog_p = false;

static struct telnet_client {
	bool active_p;
	WiFiClient client;
} telnet_client[OBI_MAX_TELNET_CLIENTS] = {
	{
		.active_p = false,
	}, {
		.active_p = false,
	}, {
		.active_p = false,
	},
};

WiFiServer telnet_server (OBI_TELNET_PORT);

static void
syslog_format_buf (char *outbuf, size_t outbuf_len, uint8_t *inbuf, size_t inbuf_len)
{
	char hexbuf[10];

	outbuf[0] = '\0';
	for (size_t i = 0; i < inbuf_len; i++) {		// XXX overflow
		sprintf (hexbuf, " %02x", inbuf[i]);
		strcat (outbuf, hexbuf);
	}
}

void
telnet_begin (void)
{
	if (strlen (cfg.syslog_ip) > 0) {
		syslog.server (cfg.syslog_ip, OBI_SYSLOG_PORT);
		if (strlen (cfg.dev_mqtt_name) > 0)
			syslog.deviceHostname (cfg.dev_mqtt_name);
		syslog.appName ("obi-steckdose");
		have_syslog_p = true;
	}

	//  syslog.logf(LOG_INFO, "This is info message no. %d", iteration);
	//  syslog.log(LOG_INFO, F("End loop"));

	telnet_server.begin ();

	return;
}

void
telnet_handle (void)
{
	uint8_t ser_buf[16];
	ssize_t read_len;
	char syslog_msg[100];
	WiFiClient new_client = telnet_server.available ();

	/* Check for new clients.  */
	if (new_client) {
		bool found_slot_p = false;

		for (size_t i = 0; i < ARRAY_SIZE (telnet_client); i++) {
			if (! telnet_client[i].active_p) {
				telnet_client[i].client = new_client;
				telnet_client[i].active_p = true;
				found_slot_p = true;
			}
		}

		/* No free slot, kill the client. 8-x  */
		if (! found_slot_p)
			new_client.stop ();
	}

	/* Check for clients gone.  */
	for (size_t i = 0; i < ARRAY_SIZE (telnet_client); i++) {
		if (telnet_client[i].active_p && ! telnet_client[i].client.connected ()) {
			telnet_client[i].active_p = false;
		}
	}

	/* Wifi -> Serial.  */
	for (size_t i = 0; i < ARRAY_SIZE (telnet_client); i++) {
		if (telnet_client[i].active_p
		    && telnet_client[i].client.available () > 0) {
			read_len = telnet_client[i].client.read (ser_buf, sizeof (ser_buf));
			if (read_len > 0) {
				if (cfg.syslog_sent_to_serial_p) {
					syslog_format_buf (syslog_msg, sizeof (syslog_msg), ser_buf, read_len);
					syslog.logf (LOG_INFO, "%s Wifi->serial:%s", cfg.dev_mqtt_name, syslog_msg);
				}
				Serial.write ((char *) ser_buf, read_len);
			}
		}
	}

	/* Serial -> Wifi.  */
	if (Serial.available () > 0) {
		read_len = Serial.readBytes (ser_buf, sizeof (ser_buf));
		if (read_len > 0) {
			if (cfg.syslog_recv_from_serial_p) {
				syslog_format_buf (syslog_msg, sizeof (syslog_msg), ser_buf, read_len);
				syslog.logf (LOG_INFO, "%s Serial->Wifi:%s", cfg.dev_mqtt_name, syslog_msg);
			}

			for (size_t i = 0; i < ARRAY_SIZE (telnet_client); i++) {
				if (telnet_client[i].active_p) {
					telnet_client[i].client.write ((char *) ser_buf, read_len);
				}
			}
		}
	}

	return;
}
