#include "obi-common.h"
#include "obi-config.h"
#include "obi-http.h"
#include "obi-misc.h"

static long serial_baud_rate[] = {
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
	921600
};
static int serial_bits[] = {
	8,
	7,
	6,
	5,
};
static const char *serial_parity[] = {
	"N",
	"E",
	"O",
};
static int serial_stopbits[] = {
	1,
	2,
};

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

		for (size_t i = 0; i < ARRAY_SIZE (serial_baud_rate); i++)
			if (serial_baud_rate[i] == serial_speed)
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

		for (size_t i = 0; i < ARRAY_SIZE (serial_bits); i++)
			if (serial_bits[i] == this_serial_bits)
				bits_okay_p = true;

		if (bits_okay_p && this_serial_bits != cfg.serial_bits) {
			cfg.serial_bits = this_serial_bits;
			need_config_save_p = true;
			need_serial_change_p = true;
		}
	}

	if (http_server.hasArg ("serial_parity")) {
		bool parity_okay_p = false;

		for (size_t i = 0; i < ARRAY_SIZE (serial_parity); i++)
			if (strcmp (serial_parity[i], http_server.arg ("serial_parity").c_str ()) == 0)
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

		for (size_t i = 0; i < ARRAY_SIZE (serial_stopbits); i++)
			if (serial_stopbits[i] == this_serial_stopbits)
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

	if (http_server.hasArg ("relay_delay_seconds")) {
		int relay_delay_seconds = atoi (http_server.arg("relay_delay_seconds").c_str ());

		if (relay_delay_seconds != cfg.relay_delay_seconds) {
			cfg.relay_delay_seconds = relay_delay_seconds;
			need_config_save_p = true;
		}
	}

	if (http_server.hasArg ("relay_randomize_delay_p")) {
		bool relay_randomize_delay_p = false;

		if (strcmp (http_server.arg ("relay_randomize_delay_p").c_str (), "on") == 0)
			relay_randomize_delay_p = true;

		if (relay_randomize_delay_p != cfg.relay_randomize_delay_p) {
			cfg.relay_randomize_delay_p = relay_randomize_delay_p;
			need_config_save_p = true;
		}
	}

	if (http_server.hasArg ("relay")) {
		bool new_relay_on_p = false;
		relay_on_p;

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
		long mqtt_server_port = atol (http_server.arg("mqtt_server_port").c_str ());

		if (mqtt_server_port != cfg.mqtt_server_port
		    && mqtt_server_port > 1
		    && mqtt_server_port < 6536) {
			cfg.mqtt_server_port = mqtt_server_port;
			need_config_save_p = true;
			need_reboot_p = true;
		}
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
	bool valid_serial_speed_p = false;
	bool valid_serial_bits_p = false;
	bool valid_serial_parity_p = false;
	bool valid_serial_stopbits_p = false;
	uint8_t mac[6];
	char mac_formatted[18];
	IPAddress my_ip;

	/* Check if we already have valid values.  */
	for (size_t i = 0; i < ARRAY_SIZE (serial_baud_rate); i++) {
		if (cfg.serial_speed == serial_baud_rate[i]) {
			valid_serial_speed_p = true;
			break;
		}
	}
	for (size_t i = 0; i < ARRAY_SIZE (serial_bits); i++) {
		if (cfg.serial_bits == serial_bits[i]) {
			valid_serial_bits_p = true;
			break;
		}
	}
	for (size_t i = 0; i < ARRAY_SIZE (serial_parity); i++) {
		if (strcmp (cfg.serial_parity, serial_parity[i]) == 0) {
			valid_serial_parity_p = true;
			break;
		}
	}
	for (size_t i = 0; i < ARRAY_SIZE (serial_stopbits); i++) {
		if (cfg.serial_stopbits == serial_stopbits[i]) {
			valid_serial_stopbits_p = true;
			break;
		}
	}

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
	html += "<tr><th>Wifi SSID:</th><td><input type=\"text\" name=\"wifi_ssid\" maxlength=\"" + String (sizeof (cfg.wifi_ssid) - 1)+ "\" value=\"" + String (cfg.wifi_ssid)+ "\"></td></tr>";
	html += "<tr><th>Wifi PSK:</th><td><input type=\"text\" name=\"wifi_psk\" maxlength=\"" + String (sizeof (cfg.wifi_psk) - 1) + "\" value=\"" + String (cfg.wifi_psk) + "\"></td></tr>";
	html += "<tr><th>Title:</th><td><input type=\"text\" name=\"dev_title\" maxlength=\"" + String (cfg.dev_title) + "\" value=\"" + String (cfg.dev_title) + "\"></td></tr>";
	html += "<tr><th>MQTT/mDNS Name:</th><td><input type=\"text\" name=\"dev_mqtt_name\" maxlength=\"" + String (sizeof (cfg.dev_mqtt_name) - 1)+ "\" value=\"" + String (cfg.dev_mqtt_name)+ "\"></td></tr>";
	html += "<tr><th>MQTT Server IP:</th><td><input type=\"text\" name=\"mqtt_server_ip\" maxlength=\"" + String (sizeof (cfg.mqtt_server_ip) - 1)+ "\" value=\"" + String (cfg.mqtt_server_ip)+ "\"></td></tr>";
	html += "<tr><th>MQTT Server Port:</th><td><input type=\"text\" name=\"mqtt_server_port\" maxlength=\"\" value=\"" + String (cfg.mqtt_server_port)+ "\"></td></tr>";
	html += "<tr><th>Syslog Server IP:</th><td><input type=\"text\" name=\"syslog_ip\" maxlength=\"" + String (sizeof (cfg.syslog_ip) - 1)+ "\" value=\"" + String (cfg.syslog_ip)+ "\"></td></tr>";
	html += "<tr><th>Serial Speed:</th><td><select name=\"serial_speed\">";
		for (size_t i = 0; i < ARRAY_SIZE (serial_baud_rate); i++)
			html += "<option value=\"" + String (serial_baud_rate[i]) + "\""
			        + ((cfg.serial_speed == serial_baud_rate[i] || ! valid_serial_speed_p)? " selected": "") + ">"
			        + String (serial_baud_rate[i]) + "</option>";
	html += "</select></td></tr>";
	html += "<tr><th>Serial Bitsize:</th><td><select name=\"serial_bits\">";
		for (size_t i = 0; i < ARRAY_SIZE (serial_bits); i++)
			html += "<option value=\"" + String (serial_bits[i]) + "\""
			        + ((cfg.serial_bits == serial_bits[i] || ! valid_serial_bits_p)? " selected": "") + ">"
			        + String (serial_bits[i]) + "</option>";
	html += "</select></td></tr>";
	html += "<tr><th>Serial Parity:</th><td><select name=\"serial_parity\">";
		for (size_t i = 0; i < ARRAY_SIZE (serial_parity); i++)
			html += "<option value=\"" + String (serial_parity[i]) + "\""
			        + ((strcmp (cfg.serial_parity, serial_parity[i]) == 0 || ! valid_serial_parity_p)? " selected": "") + ">"
			        + String (serial_parity[i]) + "</option>";
	html += "</select></td></tr>";
	html += "<tr><th>Serial Stopbits:</th><td><select name=\"serial_stopbits\">";
		for (size_t i = 0; i < ARRAY_SIZE (serial_stopbits); i++)
			html += "<option value=\"" + String (serial_stopbits[i]) + "\""
			        + ((cfg.serial_stopbits == serial_stopbits[i] || ! valid_serial_stopbits_p)? " selected": "") + ">"
			        + String (serial_stopbits[i]) + "</option>";
	html += "</select></td></tr>";
	html += "<tr><th>Relay state after OBI boot:</th><td>";
		html += "<input type=\"radio\" name=\"relay_on_after_boot_p\" value=\"on\"";
		html += cfg.relay_on_after_boot_p? " checked": "";
		html += "><label for=\"on\">on</label>";
		html += "<input type=\"radio\" name=\"relay_on_after_boot_p\" value=\"off\"";
		html += (! cfg.relay_on_after_boot_p)? " checked": "";
		html += "><label for=\"off\">off</label>";
	html += "<tr><th>After-boot Relay switch-on delay (sec):</th><td><input type=\"text\" name=\"relay_delay_seconds\" value=\"" + String (cfg.relay_delay_seconds) + "\"/></td></tr>";
	html += "<tr><th>Randomize after-boot delay:</th><td>";
		html += "<input type=\"radio\" name=\"relay_randomize_delay_p\" value=\"on\"";
		html += cfg.relay_randomize_delay_p? " checked": "";
		html += "><label for=\"on\">yes</label>";
		html += "<input type=\"radio\" name=\"relay_randomize_delay_p\" value=\"off\"";
		html += (! cfg.relay_randomize_delay_p)? " checked": "";
		html += "><label for=\"off\">no</label>";
	html += "<tr><th>Current relay state:</th><td>";
		html += "<input type=\"radio\" name=\"relay\" value=\"on\"";
		html += relay_on_p? " checked": "";
		html += "><label for=\"on\">on</label>";
		html += "<input type=\"radio\" name=\"relay\" value=\"off\"";
		html += relay_on_p? "": " checked";
		html += "><label for=\"off\">off</label>";
		html += "</td></tr>";
	html += "<tr><th>Wifi MAC:</th><td>" + String (mac_formatted) + "</td></tr>";
	html += "<tr><th>Wifi IP:</th><td>" + my_ip.toString () + "</td></tr>";
	html += "<tr><th>Mode:</th><td>" + (state == st_config? String ("Config-Only"): String ("Production")) + "</td></tr>";

	html += "</table>";
	html += "<input type=\"submit\" value=\"Save\">";
	html += "</form></body></html>";

	http_server.send (200, "text/html", html.c_str ());
}
