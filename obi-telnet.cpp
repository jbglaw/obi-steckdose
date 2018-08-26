#include "obi-telnet.h"

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

	return;
}

void
ObiTelnet::handle ()
{
	/* XXX Check for new connections.  */
	/* XXX Check for data received from Clients to be forwarded to serial (+ logging.)  */
	/* XXX Check for data received from Serial to be forwarded to clients (+ logging.)  */

	return;
}
