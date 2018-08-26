#include <WiFiUdp.h>
#include "Syslog.h"
#include "obi-common.h"
#include "obi-config.h"
#include "obi-telnet.h"

static WiFiUDP udpClient;
static Syslog syslog (udpClient);
static bool have_syslog_p = false;

// #define OBI_MAX_TELNET_CLIENTS	3
// class ObiTelnetClient {
// 	public:
// 		WiFiClient client;
// };
// class ObiTelnet {
// 	public:
// 		Telnet ();
// 		begin ();
// 		handle ();
// 
// 	private:
// 		ObiTelnetClient client[OBI_MAX_TELNET_CLIENTS];
// };

ObiTelnet::ObiTelnet (void)
{
	return;
}

void
ObiTelnet::begin (void)
{
	server = new WiFiServer (OBI_TELNET_PORT);

	if (strlen (cfg.syslog_ip) > 0) {
		syslog.server (cfg.syslog_ip, 514);
		if (strlen (cfg.dev_mqtt_name) > 0)
			syslog.deviceHostname (cfg.dev_mqtt_name);
		syslog.appName ("obi-steckdose");
		have_syslog_p = true;
	}

	//  syslog.logf(LOG_INFO, "This is info message no. %d", iteration);
	//   syslog.log(LOG_INFO, F("End loop"));

	return;
}

void
ObiTelnet::handle ()
{
	/* XXX Check for new connections.  */
	/* XXX Check for data received from Clients to be forwarded to serial (+ logging.)  */
	/* XXX Check for data received from Serial to be forwarded to clients (+ logging.)  */

	char buf[100];
	size_t read_len;

	/* Wifi -> Serial.  */
	for (size_t i = 0; i < ARRAY_SIZE (client); i++) {
		if (client[i].connected ()
		    && client[i].available () > 0) {
			read_len = client[i].read (buf, sizeof (buf) - 1);
			if (read_len > 0) {
				buf[read_len] = '\0';
				if (cfg.syslog_sent_to_serial)
					syslog.logf (LOG_INFO, "Wifi->serial: %s", buf);
				Serial.write (buf, read_len);	// XXX
			}
		}
	}

	/* Serial -> Wifi.  */
	if (Serial.available () > 0) {
		read_len = Serial.read (buf, sizeof (buf) - 1);
		if (read_len > 0) {
			buf[read_len] = '\0';
			if (cfg.syslog_recv_from_serial_p)
				syslog.logf (LOG_INFO, "Serial->Wifi: %s", buf);

			for (size_t i = 0; i < ARRAY_SIZE (client); i++) {
				if (client[i].connected ()) {
					client[i].write (buf, read_len);	// XXX
				}
			}
		}
	}

	return;
}
