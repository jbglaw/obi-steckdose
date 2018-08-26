#ifndef OBI_TELNET_H
#define OBI_TELNET_H

#include <ESP8266WiFi.h>

#define OBI_MAX_TELNET_CLIENTS	3
#define OBI_TELNET_PORT		23

class ObiTelnetClient {
	public:
		WiFiClient client;
};
class ObiTelnet {
	public:
		ObiTelnet ();
		void begin (void);
		void handle (void);

	private:
		WiFiServer *server;
		ObiTelnetClient client[OBI_MAX_TELNET_CLIENTS];
};

#endif /* OBI_TELNET_H  */
