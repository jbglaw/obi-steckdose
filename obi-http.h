#ifndef OBI_HTTP_H
#define OBI_HTTP_H

#include <ESP8266WebServer.h>

extern ESP8266WebServer http_server;
extern const uint32_t tbl_serial_baud_rate[15];
extern const uint32_t tbl_serial_bits[4];
extern const char *tbl_serial_parity[3];
extern const uint32_t tbl_serial_stopbits[2];

extern void http_POST_config (void);
extern void http_GET_slash (void);
extern void http_X_not_found (void);

/* Pimatic compatibility locations.  */
extern void http_GET_on (void);
extern void http_GET_off (void);
extern void http_GET_status (void);
extern void http_GET_toggle (void);

#define HTTP_ARG_DEV_DESCR		"dev_descr"
#define HTTP_ARG_WIFI_SSID		"wifi_ssid"
#define HTTP_ARG_WIFI_PSK		"wifi_psk"
#define HTTP_ARG_SERIAL_SPEED		"serial_speed"
#define HTTP_ARG_SERIAL_BITS		"serial_bits"
#define HTTP_ARG_SERIAL_PARITY		"serial_parity"
#define HTTP_ARG_SERIAL_STOPBITS	"serial_stopbits"
#define HTTP_ARG_RELAY_BOOT_STATE	"relay_on_after_boot_p"
#define HTTP_ARG_SYSLOG_HOST		"syslog_host"
#define HTTP_ARG_SYSLOG_PORT		"syslog_port"
#define HTTP_ARG_SYSLOG_IP_TO_SERIAL	"syslog_sent_to_serial_p"
#define HTTP_ARG_SYSLOG_SERIAL_TO_IP	"syslog_recv_from_serial_p"
#define HTTP_ARG_MQTT_NAME		"mqtt_name"
#define HTTP_ARG_MQTT_SERVER_HOST	"mqtt_host"
#define HTTP_ARG_MQTT_SERVER_PORT	"mqtt_port"
#define HTTP_ARG_TELNET_PORT		"telnet_port"
#define HTTP_ARG_ENABLE_TELNET_PROTO	"telnet_enable_proto_p"

#define HTTP_ARG_NEW_RELAY_STATE	"relay"
#define HTTP_ARG_TRIGGER_RESET		"reset"
#define HTTP_ARG_OTA_UPDATE_URL		"ota_url"


#endif /* OBI_HTTP_H  */
