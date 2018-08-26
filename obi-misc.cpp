#include "obi-misc.h"
#include "obi-common.h"
#include "obi-config.h"

/*
 * Misc. helper functions.
 */
static struct serial_map {
	SerialConfig ser;
	int bits;
	const char *parity;
	int stopbits;
} serial_map[] = {
	{ SERIAL_5N1,	5,	"N",	1, },
	{ SERIAL_5N2,	5,	"N",	2, },
	{ SERIAL_5E1,	5,	"E",	1, },
	{ SERIAL_5E2,	5,	"E",	2, },
	{ SERIAL_5O1,	5,	"O",	1, },
	{ SERIAL_5O2,	5,	"O",	2, },

	{ SERIAL_6N1,	6,	"N",	1, },
	{ SERIAL_6N2,	6,	"N",	2, },
	{ SERIAL_6E1,	6,	"E",	1, },
	{ SERIAL_6E2,	6,	"E",	2, },
	{ SERIAL_6O1,	6,	"O",	1, },
	{ SERIAL_6O2,	6,	"O",	2, },

	{ SERIAL_7N1,	7,	"N",	1, },
	{ SERIAL_7N2,	7,	"N",	2, },
	{ SERIAL_7E1,	7,	"E",	1, },
	{ SERIAL_7E2,	7,	"E",	2, },
	{ SERIAL_7O1,	7,	"O",	1, },
	{ SERIAL_7O2,	7,	"O",	2, },

	{ SERIAL_8N1,	8,	"N",	1, },
	{ SERIAL_8N2,	8,	"N",	2, },
	{ SERIAL_8E1,	8,	"E",	1, },
	{ SERIAL_8E2,	8,	"E",	2, },
	{ SERIAL_8O1,	8,	"O",	1, },
	{ SERIAL_8O2,	8,	"O",	2, },
};

SerialConfig
serial_framing (void)
{
	SerialConfig ret = SERIAL_8N1;

	for (size_t i = 0; i < ARRAY_SIZE (serial_map); i++) {
		if (cfg.serial_bits == serial_map[i].bits
		    && cfg.serial_stopbits == serial_map[i].stopbits
		    && strcmp (cfg.serial_parity, serial_map[i].parity) == 0) {
			ret = serial_map[i].ser;
			break;
		}
	}

	return ret;
}
