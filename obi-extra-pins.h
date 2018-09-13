#ifndef OBI_EXTRA_PINS_H
#define OBI_EXTRA_PINS_H

/* Pins available for extra fun.  */
extern const int pin_extra1 = 0;	/* Pulled up with 10k, at boot-up: Flashing enabled if connected to GND.  */
extern const int pin_extra2 = 2;	/* Pulled up with 10k, used for flash mode during boot-up.  */
extern const int pin_extra3 = 15;	/* Pulled down with 10k, used for flash mode during boot-up.  */
extern const int pin_extra4 = 16;	/* Seems NC.  */

extern void extra_pins_begin (void);
extern void extra_pins_handle (void);

#endif /* OBI_EXTRA_PINS_H  */
