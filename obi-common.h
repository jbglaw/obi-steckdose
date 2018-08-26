#ifndef OBI_COMMON_H
#define OBI_COMMON_H

#define WITH_OBI_SERIAL_DEBUGGING
#ifdef WITH_OBI_SERIAL_DEBUGGING
#  define obi_print(...)	Serial.print(__VA_ARGS__)
#  define obi_printf(...)	Serial.printf(__VA_ARGS__)
#  define obi_println(...)	Serial.println(__VA_ARGS__)
#else
#  define obi_print(...)
#  define obi_printf(...)
#  define obi_println(...)
#endif

#define ARRAY_SIZE(x)	(sizeof(x)/sizeof(x[0]))

extern void relay_set (bool on_p);

extern const int pin_relay_on;
extern const int pin_relay_off;
extern const int pin_led_wifi;
extern const int pin_btn;

enum state {
	st_config,
	st_connecting,
	st_running
};
extern enum state state;
extern bool relay_on_p;

#endif /* OBI_COMMON_H  */
