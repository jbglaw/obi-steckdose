#include "obi-misc.h"
#include "obi-config.h"

/*
 * Misc. helper functions.
 */

char
nibble2hex (int nibble)
{
	if (nibble >= 0 && nibble <= 9)
		return '0' + nibble;
	if (nibble >= 10 && nibble <= 15)
		return 'a' + nibble - 10;
	return 'x';
}

SerialConfig
serial_framing (void)
{
	SerialConfig ret = SERIAL_8N1;

	if (cfg.serial_bits == 5) {
		if (strcmp (cfg.serial_parity, "N") == 0) {
			if (cfg.serial_stopbits == 1)
				ret = SERIAL_5N1;
			else if (cfg.serial_stopbits == 2)
				ret = SERIAL_5N2;
		} else if (strcmp (cfg.serial_parity, "E") == 0) {
			if (cfg.serial_stopbits == 1)
				ret = SERIAL_5E1;
			else if (cfg.serial_stopbits == 2)
				ret = SERIAL_5E2;
		} else if (strcmp (cfg.serial_parity, "O") == 0) {
			if (cfg.serial_stopbits == 1)
				ret = SERIAL_5O1;
			else if (cfg.serial_stopbits == 2)
				ret = SERIAL_5O2;
		}
	} else if (cfg.serial_bits == 6) {
		if (strcmp (cfg.serial_parity, "N") == 0) {
			if (cfg.serial_stopbits == 1)
				ret = SERIAL_6N1;
			else if (cfg.serial_stopbits == 2)
				ret = SERIAL_6N2;
		} else if (strcmp (cfg.serial_parity, "E") == 0) {
			if (cfg.serial_stopbits == 1)
				ret = SERIAL_6E1;
			else if (cfg.serial_stopbits == 2)
				ret = SERIAL_6E2;
		} else if (strcmp (cfg.serial_parity, "O") == 0) {
			if (cfg.serial_stopbits == 1)
				ret = SERIAL_6O1;
			else if (cfg.serial_stopbits == 2)
				ret = SERIAL_6O2;
		}
	} else if (cfg.serial_bits == 7) {
		if (strcmp (cfg.serial_parity, "N") == 0) {
			if (cfg.serial_stopbits == 1)
				ret = SERIAL_7N1;
			else if (cfg.serial_stopbits == 2)
				ret = SERIAL_7N2;
		} else if (strcmp (cfg.serial_parity, "E") == 0) {
			if (cfg.serial_stopbits == 1)
				ret = SERIAL_7E1;
			else if (cfg.serial_stopbits == 2)
				ret = SERIAL_7E2;
		} else if (strcmp (cfg.serial_parity, "O") == 0) {
			if (cfg.serial_stopbits == 1)
				ret = SERIAL_7O1;
			else if (cfg.serial_stopbits == 2)
				ret = SERIAL_7O2;
		}
	} else if (cfg.serial_bits == 8) {
		if (strcmp (cfg.serial_parity, "N") == 0) {
			if (cfg.serial_stopbits == 1)
				ret = SERIAL_8N1;
			else if (cfg.serial_stopbits == 2)
				ret = SERIAL_8N2;
		} else if (strcmp (cfg.serial_parity, "E") == 0) {
			if (cfg.serial_stopbits == 1)
				ret = SERIAL_8E1;
			else if (cfg.serial_stopbits == 2)
				ret = SERIAL_8E2;
		} else if (strcmp (cfg.serial_parity, "O") == 0) {
			if (cfg.serial_stopbits == 1)
				ret = SERIAL_8O1;
			else if (cfg.serial_stopbits == 2)
				ret = SERIAL_8O2;
		}
	}

	return ret;
}
