#ifndef OBI_COMMON_H
#define OBI_COMMON_H

#include "Syslog.h"

#if 1
#  define obi_print(...)	do { Serial.print(__VA_ARGS__); Serial.flush (); } while (0)
#  define obi_printf(...)	do { Serial.printf(__VA_ARGS__); Serial.flush (); } while (0)
#  define obi_println(...)	do { Serial.println(__VA_ARGS__); Serial.flush (); } while (0)
#else
#  define obi_print(...)
#  define obi_printf(...)
#  define obi_println(...)
#endif

#define ARRAY_SIZE(x)	(sizeof(x)/sizeof(x[0]))

extern void relay_set (bool on_p);
extern bool relay_get_state (void);

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

extern Syslog syslog;
extern bool have_syslog_p;
extern const char obi_git_commit[];
extern const char obi_build_timestamp[];

#endif /* OBI_COMMON_H  */
