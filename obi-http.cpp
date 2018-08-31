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



void
http_POST_config (void)
{
	bool need_config_save_p = false;
	bool need_reboot_p = false;
	bool need_serial_change_p = false;

	/* Dump arguments.  */
	for (int i = 0; i < http_server.args (); i++)
		obi_printf ("%s=%s\r\n", http_server.argName(i).c_str (), http_server.arg(i).c_str ());

	if (http_server.hasArg ("wifi_ssid")
	    && http_server.arg("wifi_ssid").length () < sizeof (cfg.wifi_ssid) - 1) {
		if (strcmp (http_server.arg("wifi_ssid").c_str (), cfg.wifi_ssid) != 0) {
			strncpy (cfg.wifi_ssid, http_server.arg("wifi_ssid").c_str (), sizeof (cfg.wifi_ssid));
			need_config_save_p = true;
			need_reboot_p = true;
		}
	}

	if (http_server.hasArg ("wifi_psk")
	    && http_server.arg("wifi_psk").length () < sizeof (cfg.wifi_psk) - 1) {
		if (strcmp (http_server.arg("wifi_psk").c_str (), cfg.wifi_psk) != 0) {
			strncpy (cfg.wifi_psk, http_server.arg("wifi_psk").c_str (), sizeof (cfg.wifi_psk));
			need_config_save_p = true;
			need_reboot_p = true;
		}
	}

	if (http_server.hasArg ("dev_mqtt_name")
	    && http_server.arg("dev_mqtt_name").length () < sizeof (cfg.dev_mqtt_name) - 1) {
		if (strcmp (http_server.arg("dev_mqtt_name").c_str (), cfg.dev_mqtt_name) != 0) {
			strncpy (cfg.dev_mqtt_name, http_server.arg("dev_mqtt_name").c_str (), sizeof (cfg.dev_mqtt_name));
			need_config_save_p = true;
			need_reboot_p = true;
		}
	}

	if (http_server.hasArg ("dev_title")
	    && http_server.arg("dev_title").length () < sizeof (cfg.dev_title) - 1) {
		if (strcmp (http_server.arg("dev_title").c_str (), cfg.dev_title) != 0) {
			strncpy (cfg.dev_title, http_server.arg("dev_title").c_str (), sizeof (cfg.dev_title));
			need_config_save_p = true;
		}
	}

	if (http_server.hasArg ("serial_speed")) {
		long serial_speed = atol (http_server.arg("serial_speed").c_str ());
		bool speed_okay_p = false;

		for (size_t i = 0; i < ARRAY_SIZE (tbl_serial_baud_rate); i++)
			if (tbl_serial_baud_rate[i] == serial_speed)
				speed_okay_p = true;

		if (speed_okay_p && serial_speed != cfg.serial_speed) {
			cfg.serial_speed = serial_speed;
			need_config_save_p = true;
			need_serial_change_p = true;
		}
	}

	if (http_server.hasArg ("serial_bits")) {
		int this_serial_bits = atoi (http_server.arg("serial_bits").c_str ());
		bool bits_okay_p = false;

		for (size_t i = 0; i < ARRAY_SIZE (tbl_serial_bits); i++)
			if (tbl_serial_bits[i] == this_serial_bits)
				bits_okay_p = true;

		if (bits_okay_p && this_serial_bits != cfg.serial_bits) {
			cfg.serial_bits = this_serial_bits;
			need_config_save_p = true;
			need_serial_change_p = true;
		}
	}

	if (http_server.hasArg ("serial_parity")) {
		bool parity_okay_p = false;

		for (size_t i = 0; i < ARRAY_SIZE (tbl_serial_parity); i++)
			if (strcmp (tbl_serial_parity[i], http_server.arg ("serial_parity").c_str ()) == 0)
				parity_okay_p = true;

		if (parity_okay_p && strcmp (cfg.serial_parity, http_server.arg("serial_parity").c_str ()) != 0) {
			strncpy (cfg.serial_parity, http_server.arg("serial_parity").c_str (), sizeof (cfg.serial_parity));
			need_config_save_p = true;
			need_serial_change_p = true;
		}
	}

	if (http_server.hasArg ("serial_stopbits")) {
		int this_serial_stopbits = atol (http_server.arg ("serial_stopbits").c_str ());
		bool stopbits_okay_p = false;

		for (size_t i = 0; i < ARRAY_SIZE (tbl_serial_stopbits); i++)
			if (tbl_serial_stopbits[i] == this_serial_stopbits)
				stopbits_okay_p = true;

		if (stopbits_okay_p && this_serial_stopbits != cfg.serial_stopbits) {
			cfg.serial_stopbits = this_serial_stopbits;
			need_config_save_p = true;
			need_serial_change_p = true;
		}
	}

	if (http_server.hasArg ("relay_on_after_boot_p")) {
		bool relay_on_after_boot_p = false;

		if (strcmp (http_server.arg ("relay_on_after_boot_p").c_str (), "on") == 0)
			relay_on_after_boot_p = true;

		if (relay_on_after_boot_p != cfg.relay_on_after_boot_p) {
			cfg.relay_on_after_boot_p = relay_on_after_boot_p;
			need_config_save_p = true;
		}
	}

	if (http_server.hasArg ("relay")) {
		bool new_relay_on_p = false;

		if (strcmp (http_server.arg ("relay").c_str (), "on") == 0)
			new_relay_on_p = true;

		relay_set (new_relay_on_p);
	}

	if (http_server.hasArg ("syslog_ip")
	    && http_server.arg("syslog_ip").length () < sizeof (cfg.syslog_ip) - 1) {
		if (strcmp (http_server.arg("syslog_ip").c_str (), cfg.syslog_ip) != 0) {
			strncpy (cfg.syslog_ip, http_server.arg("syslog_ip").c_str (), sizeof (cfg.syslog_ip));
			need_config_save_p = true;
			need_reboot_p = true;
		}
	}

	if (http_server.hasArg ("mqtt_server_ip")
	    && http_server.arg("mqtt_server_ip").length () < sizeof (cfg.mqtt_server_ip) - 1) {
		if (strcmp (http_server.arg("mqtt_server_ip").c_str (), cfg.mqtt_server_ip) != 0) {
			strncpy (cfg.mqtt_server_ip, http_server.arg("mqtt_server_ip").c_str (), sizeof (cfg.mqtt_server_ip));
			need_config_save_p = true;
			need_reboot_p = true;
		}
	}

	if (http_server.hasArg ("mqtt_server_port")) {
		const char *mqtt_server_port = http_server.arg ("mqtt_server_port").c_str ();

		if (strcmp (mqtt_server_port, cfg.mqtt_server_port) != 0
		    && atol (mqtt_server_port) > 1
		    && atol (mqtt_server_port) < 6536) {
			memcpy (cfg.mqtt_server_port, http_server.arg("mqtt_server_port").c_str (), sizeof (cfg.mqtt_server_port));
			need_config_save_p = true;
			need_reboot_p = true;
		}
	}

	if (http_server.hasArg ("reset")) {
		if (strcmp (http_server.arg ("reset").c_str (), "yes") == 0)
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
	html += gen_string_input  ("Wifi SSID",           "wifi_ssid",        sizeof (cfg.wifi_ssid),        cfg.wifi_ssid);
	html += gen_string_input  ("Wifi PSK",            "wifi_psk",         sizeof (cfg.wifi_psk),         cfg.wifi_psk);
	html += gen_string_input  ("Device Description",  "dev_title",        sizeof (cfg.dev_title),        cfg.dev_title);
	html += gen_string_input  ("MQTT/mDNS Name",      "dev_mqtt_name",    sizeof (cfg.dev_mqtt_name),    cfg.dev_mqtt_name);
	html += gen_string_input  ("MQTT Broker IP",      "mqtt_server_ip",   sizeof (cfg.mqtt_server_ip),   cfg.mqtt_server_ip);
	html += gen_string_input  ("MQTT Broker Port",    "mqtt_server_port", sizeof (cfg.mqtt_server_port), cfg.mqtt_server_port);
	html += gen_string_input  ("Syslog Server IP",    "syslog_ip",        sizeof (cfg.syslog_ip),        cfg.syslog_ip);
	html += gen_long_choice   ("Serial Speed",        "serial_speed",    tbl_serial_baud_rate, ARRAY_SIZE (tbl_serial_baud_rate), 9600, cfg.serial_speed);
	html += gen_long_choice   ("Serial Bits",         "serial_bits",     tbl_serial_bits,      ARRAY_SIZE (tbl_serial_bits),      8,    cfg.serial_bits);
	html += gen_string_choice ("Serial Parity",       "serial_parity",   tbl_serial_parity,    ARRAY_SIZE (tbl_serial_parity),    "N",  cfg.serial_parity);
	html += gen_long_choice   ("Serial Stopbits",     "serial_stopbits", tbl_serial_stopbits,  ARRAY_SIZE (tbl_serial_stopbits),   1,   cfg.serial_stopbits);
	html += gen_bool_choice   ("Boot-Up Relay state", "relay_on_after_boot_p", "ON", "OFF", cfg.relay_on_after_boot_p);
	html += gen_bool_choice   ("Current Relay State", "relay",                 "ON", "OFF", relay_get_state ());
	html += "<tr><th>Wifi MAC:</th><td>" + String (mac_formatted) + "</td></tr>";
	html += "<tr><th>Wifi IP:</th><td>" + my_ip.toString () + "</td></tr>";
	html += "<tr><th>Mode:</th><td>" + (state == st_config? String ("Config-Only"): String ("Production")) + "</td></tr>";
	html += gen_bool_choice   ("Trigger RESET",       "reset",                 "ON", "OFF", false);
	html += "<tr><th>GIT Commit:</th><td>";
	html += OBI_GIT_COMMIT;
	html += "</td></tr>";
	html += "<tr><th>Build timestamp:</th><td>";
	html += OBI_BUILD_DATE;
	html += "</td></tr>";

	html += "</table>";
	html += "<input type=\"submit\" value=\"Save\">";
	html += "</form></body></html>";

	http_server.send (200, "text/html", html.c_str ());
}
