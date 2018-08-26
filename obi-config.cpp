#include <EEPROM.h>
#include "obi-common.h"
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
		obi_printf ("Config: Checksum okay (0x%02x)\r\n", checksum);
	} else {
		obi_printf ("Config: Checksum does not match: 0x%02x calculated vs. 0x%02x in flash\r\n", checksum, cfg->_checksum);
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

	obi_printf ("Config: Saved checksum is 0x%02x\r\n", cfg->_checksum);

	return ret;
}
