#include <EEPROM.h>
#include "obi-common.h"
#include "obi-config.h"
#include "obi-misc.h"
#include "obi-http.h"

/* Globally used.  */
struct obi_config cfg;

static bool
string_terminated_p (const char *start, size_t len) {
	return memchr (start, '\0', len) != NULL;
}

static bool
is_ip_address_p (const char *start, size_t len)
{
	return string_terminated_p (start, len);	// XXX harder check?
}

static bool
ip_ip_port_p (const char *start, size_t len)
{
	long lng;

	if (! string_terminated_p (start, len))
		return false;

	lng = atol (start);
	if (lng < 1 || lng > 65535)
		return false;

	return true;
}

static bool
long_in_table (long lng, const long *tbl, size_t num)
{
	for (size_t i = 0; i < num; i++)
		if (tbl[i] == lng)
			return true;

	return false;
}

static bool
string_in_table (const char *start, size_t len, const char **tbl, size_t num)
{
	if (! string_terminated_p (start, len))
		return false;

	for (size_t i = 0; i < num; i++)
		if (strcmp (tbl[i], start) == 0)
			return true;

	return false;
}

int
config_load (struct obi_config *cfg)
{
	uint8_t checksum = 0;
	int ret = -1;
	struct obi_config tmp;
	byte *ptr = (byte *) &tmp;
	bool config_good_p = true;

	obi_println ("Config: Load");

	/* Read config.  */
	EEPROM.begin (sizeof (*cfg));
	for (int i = 0; i < sizeof (*cfg); i++)
		ptr[i] = EEPROM.read (i);
	EEPROM.end ();

	/* Check checksum.  */
	for (int i = sizeof (cfg->_checksum); i < sizeof (*cfg); i++)
		checksum += ptr[i];
	if (checksum == tmp._checksum
	    && string_terminated_p (tmp.wifi_ssid,     sizeof (tmp.wifi_ssid))
	    && string_terminated_p (tmp.wifi_psk,      sizeof (tmp.wifi_psk))
	    && string_terminated_p (tmp.dev_mqtt_name, sizeof (tmp.dev_mqtt_name))
	    && string_terminated_p (tmp.dev_title,     sizeof (tmp.dev_title))
	    && long_in_table (tmp.serial_speed, tbl_serial_baud_rate, ARRAY_SIZE (tbl_serial_baud_rate))
	    && long_in_table (tmp.serial_bits,  tbl_serial_bits,      ARRAY_SIZE (tbl_serial_bits))
	    && string_in_table (tmp.serial_parity, sizeof (tmp.serial_parity), tbl_serial_parity, ARRAY_SIZE (tbl_serial_parity))
	    && long_in_table (tmp.serial_stopbits, tbl_serial_stopbits, ARRAY_SIZE (tbl_serial_stopbits))) {

		ret = 0;
		obi_printf ("Config: Checksum okay (0x%02x)\r\n", checksum);
		memcpy (cfg, &tmp, sizeof (*cfg));
	} else {
		obi_printf ("Config: Checksum does not match: 0x%02x calculated vs. 0x%02x in flash\r\n", checksum, cfg->_checksum);
		memset (cfg, 0x00, sizeof (*cfg));
		cfg->serial_speed = 9600;
		cfg->serial_bits = 8;
		cfg->serial_parity[0] = 'N';
		cfg->serial_stopbits = 2;
	}

	return ret;
}

int
config_save (struct obi_config *cfg)
{
	byte *ptr = (byte *) cfg;
	int ret;

	obi_println ("Config: Save");

	/* Calculate checksum.  */
	cfg->_checksum = 0;
	for (int i = sizeof (cfg->_checksum); i < sizeof (*cfg); i++)
		cfg->_checksum += ptr[i];

	/* Write flash.  */
	EEPROM.begin (sizeof (*cfg));
	for (int i = 0; i < sizeof (*cfg); i++)
		EEPROM.write (i, ptr[i]);
	ret = EEPROM.commit ();
	EEPROM.end ();

	obi_printf ("Config: Saved checksum is 0x%02x\r\n", cfg->_checksum);

	return ret;
}
