#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include "obi-common.h"
#include "obi-config.h"
#include "obi-http.h"
#include "obi-misc.h"
#include "obi-mqtt.h"

const long tbl_serial_baud_rate[] = {
	75,
	110,
	150,
	300,
	1200,
	2400,
	4800,
	9600,
	19200,
	38400,
	57600,
	115200,
	230400,
	460800,
	921600,
};
const long tbl_serial_bits[] = {
	8,
	7,
	6,
	5,
};
const char *tbl_serial_parity[] = {
	"N",
	"E",
	"O",
};
const long tbl_serial_stopbits[] = {
	1,
	2,
};


static String
gen_bool_choice (const char *title, const char *cfg_option, const char *yes, const char *no, bool curr_on_p)
{
	String ret;

	ret += "<tr><th>";
	ret += title;
	ret += "</th><td>";
	ret += "<input type=\"radio\" name=\"";
	ret += cfg_option;
	ret += "\" value=\"yes\"";
	ret += curr_on_p? " checked": "";
	ret += "><label for=\"yes\">";
	ret += yes;
	ret +="</label>";
	ret += "<input type=\"radio\" name=\"";
	ret += cfg_option;
	ret += "\" value=\"no\"";
	ret += curr_on_p? "": " checked";
	ret += "><label for=\"no\">";
	ret += no;
	ret += "</label>";
	ret += "</td></tr>";

	return ret;
}

static String
gen_long_choice (const char *title, const char *cfg_option, const long *data, size_t num, long dflt, long curr)
{
	bool value_found_p = false;
	String ret;

	for (size_t i = 0; i < num; i++) {
		if (data[i] == curr) {
			value_found_p = true;
			break;
		}
	}

	ret += "<tr><th>";
	ret += title;
	ret += "</th><td><select name=\"";
	ret += cfg_option;
	ret += "\">";
	for (size_t i = 0; i < num; i++)
		ret += "<option value=\"" + String (data[i]) + "\""
		       + (curr == data[i]
		          || (! value_found_p
		              && data[i] == dflt)? " selected": "")
		       + ">" + String (data[i]) + "</option>";
	ret += "</select></td></tr>";

	return ret;
}

static String
gen_string_choice (const char *title, const char *cfg_option, const char **data, size_t num, const char *dflt, const char *curr)
{
	bool value_found_p = false;
	String ret;

	for (size_t i = 0; i < num; i++) {
		if (strcmp (data[i], curr) == 0) {
			value_found_p = true;
			break;
		}
	}

	ret += "<tr><th>";
	ret += title;
	ret += "</th><td><select name=\"";
	ret += cfg_option;
	ret += "\">";
	for (size_t i = 0; i < num; i++) {
		ret += "<option value=\"";
		ret += data[i];
		ret += "\"";
		ret += (strcmp (curr, data[i]) == 0
		          || (! value_found_p
		              && strcmp (data[i], dflt) == 0)? " selected": "");
		ret += ">";
		ret += data[i];
		ret += "</option>";
	}
	ret += "</select></td></tr>";

	return ret;
}

static String
gen_string_input (const char *title, const char *cfg_option, size_t buflen, const char *curr)
{
	String ret;

	ret += "<tr><th>";
	ret += title;
	ret += ":</th><td><input type=\"text\" name=\"";
	ret += cfg_option;
	ret += "\" maxlength=\"" + String (buflen - 1)+ "\" value=\"" + String (curr)+ "\"></td></tr>";

	return ret;
}

static bool
parse_bool (const char *cfg_option, bool *dst)
{
	bool ret_p = false;
	bool new_value_p;

	if (http_server.hasArg (cfg_option)
	    && (strcmp (http_server.arg (cfg_option).c_str (), "yes") == 0
	        || strcmp (http_server.arg (cfg_option).c_str (), "no") == 0)) {

		new_value_p = strcmp (http_server.arg (cfg_option).c_str (), "yes") == 0;
		if (*dst != new_value_p) {
			*dst = new_value_p;
			ret_p = true;
		}
	}

	return ret_p;
}

static bool
parse_long_choice (const char *cfg_option, const long *data, size_t num, long *dst)
{
	bool ret_p = false;
	long new_value;

	if (http_server.hasArg (cfg_option)) {
		new_value = atol (http_server.arg (cfg_option).c_str ());

		for (size_t i = 0; i < num; i++) {
			if (data[i] == new_value
			    && *dst != new_value) {
				*dst = new_value;
				ret_p = true;
				break;
			}
		}
	}

	return ret_p;
}

static bool
parse_string_choice (const char *cfg_option, const char **data, size_t num, char *dst, size_t dst_len)
{
	bool ret_p = false;
	long new_value;

	if (http_server.hasArg (cfg_option)
	    && strlen (http_server.arg (cfg_option).c_str ()) < dst_len) {

		for (size_t i = 0; i < num; i++) {
			if (strcmp (data[i], http_server.arg (cfg_option).c_str ()) == 0) {
				if (strcmp (dst, http_server.arg (cfg_option).c_str ()) != 0) {
					strncpy (dst, http_server.arg (cfg_option).c_str (), dst_len);
					ret_p = true;
				}
				break;
			}
		}
	}

	return ret_p;
}

static bool
parse_string_input (const char *cfg_option, char *dst, size_t dst_len)
{
	bool ret_p = false;

	if (http_server.hasArg (cfg_option)
	    && strlen (http_server.arg (cfg_option).c_str ()) < dst_len
	    && strcmp (http_server.arg (cfg_option).c_str (), dst) != 0) {

		strncpy (dst, http_server.arg (cfg_option).c_str (), dst_len);
		ret_p = true;
	}

	return ret_p;
}


void
http_POST_config (void)
{
	bool need_reboot_p = false;
	bool need_config_save_p = false;
	bool need_serial_change_p = false;
	bool ret_p;

	/* Dump arguments.  */
	for (int i = 0; i < http_server.args (); i++)
		obi_printf ("%s=%s\r\n", http_server.argName(i).c_str (), http_server.arg(i).c_str ());

	/*
	 * Parse all config-related values.
	 */
	ret_p = parse_string_input (HTTP_ARG_WIFI_SSID, cfg.wifi_ssid, sizeof (cfg.wifi_ssid));
	obi_printf ("%s: %s\r\n", HTTP_ARG_WIFI_SSID, (ret_p? "yes": "no"));
	need_config_save_p |= ret_p;
	need_reboot_p |= ret_p;

	ret_p = parse_string_input (HTTP_ARG_WIFI_PSK, cfg.wifi_psk, sizeof (cfg.wifi_psk));
	obi_printf ("%s: %s\r\n", HTTP_ARG_WIFI_PSK, (ret_p? "yes": "no"));
	need_config_save_p |= ret_p;
	need_reboot_p |= ret_p;

	ret_p = parse_string_input (HTTP_ARG_MQTT_NAME, cfg.dev_mqtt_name, sizeof (cfg.dev_mqtt_name));
	obi_printf ("%s: %s\r\n", HTTP_ARG_MQTT_NAME, (ret_p? "yes": "no"));
	need_config_save_p |= ret_p;
	need_reboot_p |= ret_p;

	ret_p = parse_string_input (HTTP_ARG_DEV_DESCR, cfg.dev_descr, sizeof (cfg.dev_descr));
	obi_printf ("%s: %s\r\n", HTTP_ARG_DEV_DESCR, (ret_p? "yes": "no"));
	need_config_save_p |= ret_p;

	ret_p = parse_long_choice (HTTP_ARG_SERIAL_SPEED, tbl_serial_baud_rate, ARRAY_SIZE (tbl_serial_baud_rate), &cfg.serial_speed);
	obi_printf ("%s: %s\r\n", HTTP_ARG_SERIAL_SPEED, (ret_p? "yes": "no"));
	need_config_save_p |= ret_p;
	need_serial_change_p |= ret_p;

	ret_p = parse_long_choice (HTTP_ARG_SERIAL_BITS, tbl_serial_bits, ARRAY_SIZE (tbl_serial_bits), &cfg.serial_bits);
	obi_printf ("%s: %s\r\n", HTTP_ARG_SERIAL_BITS, (ret_p? "yes": "no"));
	need_config_save_p |= ret_p;
	need_serial_change_p |= ret_p;

	ret_p = parse_string_choice (HTTP_ARG_SERIAL_PARITY, tbl_serial_parity, ARRAY_SIZE (tbl_serial_parity), cfg.serial_parity, sizeof (cfg.serial_parity));
	obi_printf ("%s: %s\r\n", HTTP_ARG_SERIAL_PARITY, (ret_p? "yes": "no"));
	need_config_save_p |= ret_p;
	need_serial_change_p |= ret_p;

	ret_p = parse_long_choice (HTTP_ARG_SERIAL_STOPBITS, tbl_serial_stopbits, ARRAY_SIZE (tbl_serial_stopbits), &cfg.serial_stopbits);
	obi_printf ("%s: %s\r\n", HTTP_ARG_SERIAL_STOPBITS, (ret_p? "yes": "no"));
	need_config_save_p |= ret_p;
	need_serial_change_p |= ret_p;

	ret_p = parse_bool (HTTP_ARG_RELAY_BOOT_STATE, &cfg.relay_on_after_boot_p);
	obi_printf ("%s: %s\r\n", HTTP_ARG_RELAY_BOOT_STATE, (ret_p? "yes": "no"));
	need_config_save_p |= ret_p;

	ret_p = parse_string_input (HTTP_ARG_SYSLOG_HOST, cfg.syslog_host, sizeof (cfg.syslog_host));
	obi_printf ("%s: %s\r\n", HTTP_ARG_SYSLOG_HOST, (ret_p? "yes": "no"));
	need_config_save_p |= ret_p;
	need_reboot_p |= ret_p;

	ret_p = parse_string_input (HTTP_ARG_SYSLOG_PORT, cfg.syslog_port, sizeof (cfg.syslog_port));
	obi_printf ("%s: %s\r\n", HTTP_ARG_SYSLOG_PORT, (ret_p? "yes": "no"));
	need_config_save_p |= ret_p;
	need_reboot_p |= ret_p;

	ret_p = parse_string_input (HTTP_ARG_MQTT_SERVER_HOST, cfg.mqtt_server_host, sizeof (cfg.mqtt_server_host));
	obi_printf ("%s: %s\r\n", HTTP_ARG_MQTT_SERVER_HOST, (ret_p? "yes": "no"));
	need_config_save_p |= ret_p;
	need_reboot_p |= ret_p;

	ret_p = parse_string_input (HTTP_ARG_MQTT_SERVER_PORT, cfg.mqtt_server_port, sizeof (cfg.mqtt_server_port));
	obi_printf ("%s: %s\r\n", HTTP_ARG_MQTT_SERVER_PORT, (ret_p? "yes": "no"));
	need_config_save_p |= ret_p;
	need_reboot_p |= ret_p;

	ret_p = parse_string_input (HTTP_ARG_TELNET_PORT, cfg.telnet_port, sizeof (cfg.telnet_port));
	obi_printf ("%s: %s\r\n", HTTP_ARG_TELNET_PORT, (ret_p? "yes": "no"));
	need_config_save_p |= ret_p;
	need_reboot_p |= ret_p;

	ret_p = parse_bool (HTTP_ARG_ENABLE_TELNET_PROTO, &cfg.enable_telnet_negotiation_p);
	obi_printf ("%s: %s\r\n", HTTP_ARG_ENABLE_TELNET_PROTO, (ret_p? "yes": "no"));
	need_config_save_p |= ret_p;

	ret_p = parse_bool (HTTP_ARG_SYSLOG_IP_TO_SERIAL, &cfg.syslog_sent_to_serial_p);
	obi_printf ("%s: %s\r\n", HTTP_ARG_SYSLOG_IP_TO_SERIAL, (ret_p? "yes": "no"));
	need_config_save_p |= ret_p;

	ret_p = parse_bool (HTTP_ARG_SYSLOG_SERIAL_TO_IP, &cfg.syslog_recv_from_serial_p);
	obi_printf ("%s: %s\r\n", HTTP_ARG_SYSLOG_SERIAL_TO_IP, (ret_p? "yes": "no"));
	need_config_save_p |= ret_p;

	/*
	 * Act upon config changes.
	 */
	if (need_config_save_p)
		config_save (&cfg);
	if (need_reboot_p)
		ESP.restart ();
	if (need_serial_change_p)
		Serial.begin (cfg.serial_speed, serial_framing ());

	/*
	 * Handle special calls.
	 */

	/* OTA Update URL.  */
	if (http_server.hasArg (HTTP_ARG_OTA_UPDATE_URL) && http_server.arg(HTTP_ARG_OTA_UPDATE_URL).length () > 0) {
		syslog.logf (LOG_CRIT, "Trying to update firmware from %s, current GIT=%s built at %s",     http_server.arg(HTTP_ARG_OTA_UPDATE_URL).c_str (), obi_git_commit, obi_build_timestamp);
		obi_printf (           "Trying to update firmware from %s, current GIT=%s built at %s\r\n", http_server.arg(HTTP_ARG_OTA_UPDATE_URL).c_str (), obi_git_commit, obi_build_timestamp);
		ESPhttpUpdate.rebootOnUpdate (false);
		t_httpUpdate_return ret = ESPhttpUpdate.update (http_server.arg (HTTP_ARG_OTA_UPDATE_URL), OBI_GIT_COMMIT);
		syslog.logf (LOG_CRIT, "After OTA update from %s, ret = %i",     http_server.arg(HTTP_ARG_OTA_UPDATE_URL).c_str (), (int) ret);
		obi_printf (           "After OTA update from %s, ret = %i\r\n", http_server.arg(HTTP_ARG_OTA_UPDATE_URL).c_str (), (int) ret);
		if (ret == HTTP_UPDATE_OK) {
			syslog.logf (LOG_CRIT, "Update successful, rebooting...");
			obi_printf (           "Update successful, rebooting...\r\n");
			/* Allow network and Serial to drain.  */
			delay (2000);
			ESP.restart ();
		} else {
			syslog.logf (LOG_CRIT, "Update failed, will not reboot.");
			obi_printf (           "Update failed, will not reboot.\r\n");
		}
	}

	/* New relay state.  */
	{
		bool new_relay_on_p = relay_get_state ();

		ret_p = parse_bool (HTTP_ARG_NEW_RELAY_STATE, &new_relay_on_p);
		if (ret_p)
			relay_set (new_relay_on_p);
	}

	/* Reset trigger.  */
	{
		bool trigger_reset_p = false;

		ret_p = parse_bool (HTTP_ARG_TRIGGER_RESET, &trigger_reset_p);
		if (ret_p && trigger_reset_p)
			mqtt_trigger_reset ();
	}

	http_server.sendHeader ("Location", "/status");
	http_server.send (302, "text/html", "<html><head><title>Redirect</title></head><body>Please go to <a href=\"/status\">status</a>.</body></html>");

	if (need_config_save_p)
		config_save (&cfg);
	if (need_reboot_p)
		ESP.restart ();
	if (need_serial_change_p)
		Serial.begin (cfg.serial_speed, serial_framing ());

	return;
}

void
http_GET_status (void)
{
	String html;
	uint8_t mac[6];
	char mac_formatted[18];
	IPAddress my_ip;

	/* Get MAC + IP address.  */
	if (state == st_config) {
		/* AP mode.  */
		WiFi.softAPmacAddress (mac);
		my_ip = WiFi.softAPIP ();
	} else {
		/* STA mode.  */
		WiFi.macAddress (mac);
		my_ip = WiFi.localIP ();
	}
	snprintf (mac_formatted, sizeof (mac_formatted), "%02X:%02X:%02X:%02X:%02X:%02X",
	          mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	/* Prepare HTML.  */
	html += "<html><head><title>System Status</title></head><body><form action=\"/config\" method=\"post\"><table>";
	html += gen_string_input  ("Wifi SSID",              HTTP_ARG_WIFI_SSID,           sizeof (cfg.wifi_ssid),        cfg.wifi_ssid);
	html += gen_string_input  ("Wifi PSK",               HTTP_ARG_WIFI_PSK,            sizeof (cfg.wifi_psk),         cfg.wifi_psk);
	html += gen_string_input  ("Device Description",     HTTP_ARG_DEV_DESCR,           sizeof (cfg.dev_descr),        cfg.dev_descr);
	html += gen_string_input  ("MQTT/mDNS Name",         HTTP_ARG_MQTT_NAME,           sizeof (cfg.dev_mqtt_name),    cfg.dev_mqtt_name);
	html += gen_string_input  ("MQTT Broker IP",         HTTP_ARG_MQTT_SERVER_HOST,    sizeof (cfg.mqtt_server_host), cfg.mqtt_server_host);
	html += gen_string_input  ("MQTT Broker Port",       HTTP_ARG_MQTT_SERVER_PORT,    sizeof (cfg.mqtt_server_port), cfg.mqtt_server_port);
	html += gen_string_input  ("Syslog Server IP/Name",  HTTP_ARG_SYSLOG_HOST,         sizeof (cfg.syslog_host),      cfg.syslog_host);
	html += gen_string_input  ("Syslog Server Port",     HTTP_ARG_SYSLOG_PORT,         sizeof (cfg.syslog_port),      cfg.syslog_port);
	html += gen_bool_choice   ("Syslog IP->Serial",      HTTP_ARG_SYSLOG_IP_TO_SERIAL, "Yes",  "No",                  cfg.syslog_sent_to_serial_p);
	html += gen_bool_choice   ("Syslog Serial->IP",      HTTP_ARG_SYSLOG_SERIAL_TO_IP, "Yes",  "No",                  cfg.syslog_recv_from_serial_p);
	html += gen_long_choice   ("Serial Speed",           HTTP_ARG_SERIAL_SPEED,        tbl_serial_baud_rate,          ARRAY_SIZE (tbl_serial_baud_rate), 9600, cfg.serial_speed);
	html += gen_long_choice   ("Serial Bits",            HTTP_ARG_SERIAL_BITS,         tbl_serial_bits,               ARRAY_SIZE (tbl_serial_bits),      8,    cfg.serial_bits);
	html += gen_string_choice ("Serial Parity",          HTTP_ARG_SERIAL_PARITY,       tbl_serial_parity,             ARRAY_SIZE (tbl_serial_parity),    "N",  cfg.serial_parity);
	html += gen_long_choice   ("Serial Stopbits",        HTTP_ARG_SERIAL_STOPBITS,     tbl_serial_stopbits,           ARRAY_SIZE (tbl_serial_stopbits),  1,    cfg.serial_stopbits);
	html += gen_string_input  ("Raw/Telnet Port",        HTTP_ARG_TELNET_PORT,         sizeof (cfg.telnet_port),      cfg.telnet_port);
	html += gen_bool_choice   ("Enable TELNET Protocol", HTTP_ARG_ENABLE_TELNET_PROTO, "Yes",    "No",                cfg.enable_telnet_negotiation_p);
	html += gen_bool_choice   ("Boot-Up Relay state",    HTTP_ARG_RELAY_BOOT_STATE,    "On",     "Off",               cfg.relay_on_after_boot_p);

	html += gen_bool_choice   ("Current Relay State",    HTTP_ARG_NEW_RELAY_STATE,     "On",     "Off",               relay_get_state ());
	html += gen_bool_choice   ("Trigger RESET",          HTTP_ARG_TRIGGER_RESET,       "Reset!", "Keep running",      false);
	html += gen_string_input  ("Load OTA Update from URL",HTTP_ARG_OTA_UPDATE_URL,     256,                           "");
	html += "<tr><th>Wifi MAC:</th><td>" + String (mac_formatted) + "</td></tr>";
	html += "<tr><th>Wifi IP:</th><td>" + my_ip.toString () + "</td></tr>";
	html += "<tr><th>Mode:</th><td>" + (state == st_config? String ("Config-Only / AP"): String ("Production / STA")) + "</td></tr>";
	html += "<tr><th>GIT Commit:</th><td>";
	html += obi_git_commit;
	html += "</td></tr>";
	html += "<tr><th>Build timestamp:</th><td>";
	html += obi_build_timestamp;
	html += "</td></tr>";

	html += "</table>";
	html += "<input type=\"submit\" value=\"Save\">";
	html += "</form></body></html>";

	http_server.send (200, "text/html", html.c_str ());
}

void
http_X_not_found (void)
{
	http_server.sendHeader ("Location", "/status");
	http_server.send (302, "text/html", "<html><head><title>Redirect</title></head><body>Please go to <a href=\"/status\">status</a>.</body></html>");

	return;
}
