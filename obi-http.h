#ifndef OBI_HTTP_H
#define OBI_HTTP_H

#include <ESP8266WebServer.h>

extern ESP8266WebServer http_server;
extern const long tbl_serial_baud_rate[15];
extern const long tbl_serial_bits[4];
extern const char *tbl_serial_parity[3];
extern const long tbl_serial_stopbits[2];

extern void http_POST_config (void);
extern void http_GET_status (void);

#endif /* OBI_HTTP_H  */
