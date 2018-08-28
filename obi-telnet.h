#ifndef OBI_TELNET_H
#define OBI_TELNET_H

#define OBI_MAX_TELNET_CLIENTS	3
#define OBI_TELNET_PORT		23
#define OBI_SYSLOG_PORT		514

extern void telnet_begin (void);
extern void telnet_handle (void);

#endif /* OBI_TELNET_H  */
