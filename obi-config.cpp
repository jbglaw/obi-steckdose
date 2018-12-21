#include <ArduinoJson.h>
#include <CRC32.h>
#include <EEPROM.h>
#include "obi-common.h"
#include "obi-config.h"
#include "obi-misc.h"
#include "obi-http.h"
#include "obi-telnet.h"
#include "obi-mqtt.h"

/* Globally used.  */
struct obi_config cfg;

bool
long_in_table (uint32_t lng, const uint32_t *tbl, size_t num)
{
	for (size_t i = 0; i < num; i++)
		if (tbl[i] == lng)
			return true;

	return false;
}

int
config_load (void)
{
	int ret = -1;
	byte magic[] = OBI_CONFIG_FLASH_MAGIC;
	uint32_t crc32sum;
	struct obi_config_flash_header flash_header;
	byte *ptr = (byte *) &flash_header;
	byte *json;
	DynamicJsonBuffer jsonBuffer (2048);

	obi_println ("Config: Load");

	/* Clear config and apply sane defaults.  */
	memset (&cfg, 0x00, sizeof (cfg));
	cfg.serial_speed = 9600;
	cfg.serial_bits = 8;
	cfg.serial_parity[0] = 'N';
	cfg.serial_stopbits = 1;
	cfg.relay_on_after_boot_p = false;
	cfg.syslog_port = OBI_SYSLOG_PORT_DEFAULT;
	cfg.syslog_sent_to_serial_p = false;
	cfg.syslog_recv_from_serial_p = false;
	cfg.mqtt_port = OBI_MQTT_PORT_DEFAULT;
	cfg.telnet_port = OBI_TELNET_PORT_DEFAULT;
	cfg.telnet_enable_proto_p = true;
	cfg.suppress_led_blinking_p = false;

	/* Read flash header.  */
	EEPROM.begin (sizeof (flash_header));
	for (size_t i = 0; i < sizeof (flash_header); i++)
		ptr[i] = EEPROM.read (i);
	EEPROM.end ();

	/* Check flash header.  */
	if (memcmp (magic, flash_header.magic, sizeof (magic)) == 0
	    && flash_header.json_len < 4096) {

		/* Flash header has correct MAGIC, so continue.  */
		obi_println ("Found valid header, will check checksum.");

		json = (byte *) malloc (flash_header.json_len + 1);
		if (json) {
			/* Load JSON from flash.  */
			memset (json, '\0', flash_header.json_len + 1);
			EEPROM.begin (sizeof (flash_header) + flash_header.json_len);
			for (size_t i = sizeof (flash_header); i < sizeof (flash_header) + flash_header.json_len; i++)
				json[i - sizeof (flash_header)] = EEPROM.read (i);
			EEPROM.end ();

			/* Check checksum.  */
			crc32sum = CRC32::calculate (json, flash_header.json_len);
			if (crc32sum == flash_header.crc32sum) {
				JsonObject &root = jsonBuffer.parseObject (json);

				obi_println ("Found valid header checksum.");

				if (root.success ()) {
					obi_println ("JSON parsed successfully, applying values.");

					/* Device.  */
					if (root.containsKey ("dev_descr") && root["dev_descr"].is<char *> () && strlen (root["dev_descr"]) < sizeof (cfg.dev_descr))
						strncpy (cfg.dev_descr, root["dev_descr"], sizeof (cfg.dev_descr));
					/* Wifi.  */
					if (root.containsKey ("wifi_ssid") && root["wifi_ssid"].is<char *> () && strlen (root["wifi_ssid"]) < sizeof (cfg.wifi_ssid))
						strncpy (cfg.wifi_ssid, root["wifi_ssid"], sizeof (cfg.wifi_ssid));
					if (root.containsKey ("wifi_psk") && root["wifi_psk"].is<char *> () && strlen (root["wifi_psk"]) < sizeof (cfg.wifi_psk))
						strncpy (cfg.wifi_psk, root["wifi_psk"], sizeof (cfg.wifi_psk));
					/* Serial.  */
					if (root.containsKey ("serial_speed") && root["serial_speed"].is<uint32_t> () && long_in_table (root["serial_speed"], tbl_serial_baud_rate, ARRAY_SIZE (tbl_serial_baud_rate)))
						cfg.serial_speed = root["serial_speed"];
					if (root.containsKey ("serial_bits") && root["serial_bits"].is<uint32_t> () && long_in_table (root["serial_bits"], tbl_serial_bits, ARRAY_SIZE (tbl_serial_bits)))
						cfg.serial_bits = root["serial_bits"];
					if (root.containsKey ("serial_parity") && root["serial_parity"].is<char *> () && strlen (root["serial_parity"]) < sizeof (cfg.serial_parity))
						strncpy (cfg.serial_parity, root["serial_parity"], sizeof (cfg.serial_parity));
					if (root.containsKey ("serial_stopbits") && root["serial_stopbits"].is<uint32_t> () && long_in_table (root["serial_stopbits"], tbl_serial_stopbits, ARRAY_SIZE (tbl_serial_stopbits)))
						cfg.serial_stopbits = root["serial_stopbits"];
					/* Relay.  */
					if (root.containsKey ("relay_on_after_boot_p") && root["relay_on_after_boot_p"].is<bool> ())
						cfg.relay_on_after_boot_p = root["relay_on_after_boot_p"];
					/* Syslog.  */
					if (root.containsKey ("syslog_host") && root["syslog_host"].is<char *> () && strlen (root["syslog_host"]) < sizeof (cfg.syslog_host))
						strncpy (cfg.syslog_host, root["syslog_host"], sizeof (cfg.syslog_host));
					if (root.containsKey ("syslog_port") && root["syslog_port"].is<uint16_t> ())
						cfg.syslog_port = root["syslog_port"];
					if (root.containsKey ("syslog_sent_to_serial_p") && root["syslog_sent_to_serial_p"].is<bool> ())
						cfg.syslog_sent_to_serial_p = root["syslog_sent_to_serial_p"];
					if (root.containsKey ("syslog_recv_from_serial_p") && root["syslog_recv_from_serial_p"].is<bool> ())
						cfg.syslog_recv_from_serial_p = root["syslog_recv_from_serial_p"];
					/* MQTT.  */
					if (root.containsKey ("mqtt_name") && root["mqtt_name"].is<char *> () && strlen (root["mqtt_name"]) < sizeof (cfg.mqtt_name))
						strncpy (cfg.mqtt_name, root["mqtt_name"], sizeof (cfg.mqtt_name));
					if (root.containsKey ("mqtt_host") && root["mqtt_host"].is<char *> () && strlen (root["mqtt_host"]) < sizeof (cfg.mqtt_host))
						strncpy (cfg.mqtt_host, root["mqtt_host"], sizeof (cfg.mqtt_host));
					if (root.containsKey ("mqtt_port") && root["mqtt_port"].is<uint16_t> ())
						cfg.mqtt_port = root["mqtt_port"];
					/* Telnet.  */
					if (root.containsKey ("telnet_port") && root["telnet_port"].is<uint16_t> ())
						cfg.telnet_port = root["telnet_port"];
					if (root.containsKey ("telnet_enable_proto_p") && root["telnet_enable_proto_p"].is<bool> ())
						cfg.telnet_enable_proto_p = root["telnet_enable_proto_p"];
					/* Misc.  */
					if (root.containsKey ("suppress_led_blinking_p") && root["suppress_led_blinking_p"].is<bool> ())
						cfg.suppress_led_blinking_p = root["suppress_led_blinking_p"];
					ret = 0;
				} else {
					obi_println ("JSON didn't parse.");
				}
			} else {
				obi_println ("Header checksum mismatch.");
			}

			free (json);
		}
	} else {
		obi_println ("Did not find a valid config header magic.");
	}

	return ret;
}

int
config_save (void)
{
	int ret = -1;
	byte magic[] = OBI_CONFIG_FLASH_MAGIC;
	struct obi_config_flash_header flash_header;
	byte *ptr = (byte *) &flash_header;
	DynamicJsonBuffer jsonBuffer (2048);
	String json;

	obi_println ("Config: Save");

	/* Generate JSON object.  */
	JsonObject &root = jsonBuffer.createObject ();
	root["dev_descr"]                 = cfg.dev_descr;
	root["wifi_ssid"]                 = cfg.wifi_ssid;
	root["wifi_psk"]                  = cfg.wifi_psk;
	root["serial_speed"]              = cfg.serial_speed;
	root["serial_bits"]               = cfg.serial_bits;
	root["serial_parity"]             = cfg.serial_parity;
	root["serial_stopbits"]           = cfg.serial_stopbits;
	root["relay_on_after_boot_p"]     = cfg.relay_on_after_boot_p;
	root["syslog_host"]               = cfg.syslog_host;
	root["syslog_port"]               = cfg.syslog_port;
	root["syslog_sent_to_serial_p"]   = cfg.syslog_sent_to_serial_p;
	root["syslog_recv_from_serial_p"] = cfg.syslog_recv_from_serial_p;
	root["mqtt_name"]                 = cfg.mqtt_name;
	root["mqtt_host"]                 = cfg.mqtt_host;
	root["mqtt_port"]                 = cfg.mqtt_port;
	root["telnet_port"]               = cfg.telnet_port;
	root["telnet_enable_proto_p"]     = cfg.telnet_enable_proto_p;
	root["suppress_led_blinking_p"]   = cfg.suppress_led_blinking_p;
	root.printTo (json);

	/* Prepare flash header.  */
	memset (&flash_header, 0x00, sizeof (flash_header));
	memcpy (flash_header.magic, magic, sizeof (flash_header.magic));
	flash_header.json_len = json.length ();
	flash_header.crc32sum = CRC32::calculate (json.c_str (), json.length ());

	/* Write flash.  */
	EEPROM.begin (sizeof (flash_header) + json.length ());
	for (size_t i = 0; i < sizeof (flash_header); i++)
		EEPROM.write (i, ptr[i]);
	for (size_t i = 0; i < json.length (); i++)
		EEPROM.write (sizeof (flash_header) + i, json.c_str()[i]);
	ret = EEPROM.commit ();
	EEPROM.end ();

	obi_println (json.c_str ());
	obi_printf ("Config: Saved checksum is 0x%lu\r\n", flash_header.crc32sum);

	return ret;
}
