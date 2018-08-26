#ifndef OBI_MISC_H
#define OBI_MISC_H

#if 1
#  define obi_print(...)	Serial.print(__VA_ARGS__)
#  define obi_println(...)	Serial.println(__VA_ARGS__)
#else
#  define obi_print(...)
#  define obi_println(...)
#endif

extern char nibble2hex (int nibble);
extern SerialConfig serial_framing (void);

#endif /* OBI_MISC_H  */
