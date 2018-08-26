#ifndef OBI_HTTP_H
#define OBI_HTTP_H

#include <ESP8266WebServer.h>

extern ESP8266WebServer http_server;

extern void http_POST_config (void);
extern void http_GET_status (void);

#endif /* OBI_HTTP_H  */
