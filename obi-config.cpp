#include <EEPROM.h>
#include "obi-config.h"
#include "obi-misc.h"

/* Globally used.  */
struct obi_config cfg;

int
config_load (struct obi_config *cfg)
{
	uint8_t checksum = 0;
	int ret = -1;
	byte *ptr = (byte *) cfg;

	obi_println ("Config: Load");

	/* Zap config.  */
	memset (cfg, 0x00, sizeof (*cfg));

	/* Read config.  */
	EEPROM.begin (sizeof (*cfg));
	for (int i = 0; i < sizeof (*cfg); i++)
		ptr[i] = EEPROM.read (i);
	EEPROM.end ();

	/* Check checksum.  */
	for (int i = sizeof (cfg->_checksum); i < sizeof (*cfg); i++)
		checksum += ptr[i];

	if (checksum == cfg->_checksum) {
		ret = 0;
		obi_print ("Config: Checksum okay (0x");
		obi_print (checksum, HEX);
		obi_println (")");
	} else {
		obi_print ("Config: Checksum does not match: 0x");
		obi_print (checksum, HEX);
		obi_print ("calculated vs. 0x");
		obi_print (cfg->_checksum, HEX);
		obi_println (" in flash");
		memset (cfg, 0x00, sizeof (*cfg));
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

	obi_print ("Config: Saved checksum is 0x");
	obi_println (cfg->_checksum, HEX);

	return ret;
}
