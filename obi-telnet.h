#ifndef OBI_TELNET_H
#define OBI_TELNET_H

#define OBI_MAX_TELNET_CLIENTS	3
#define OBI_TELNET_PORT_DEFAULT	23
#define OBI_SYSLOG_PORT_DEFAULT	514

#define T_SE	240	/* Subnegotiation parameters End.  */
#define T_NOP	241	/* No OPeration.  */
#define T_BREAK	243
#define T_IP	244	/* Interrupt Process.  */
#define T_AO	245	/* Abort Output.  */
#define T_GO	249	/* Go Ahead.  */
#define T_SB	250	/* Subnegotiation Begin.  */
#define T_WILL	251
#define T_WONT	252
#define T_DO	253
#define T_DONT	254
#define T_IAC	255	/* Interpret As Command.  */

#define T_OPT_COMPORT			44
#define T_SUB_COMPORT_SETBAUD		1
#define T_SUB_COMPORT_SETDATASIZE	2
#define T_SUB_COMPORT_SETPARITY		3
#define T_SUB_COMPORT_SETSTOPSIZE	4
#define T_SUB_COMPORT_SETCONTROL	5
#define T_SUB_COMPORT_PURGEDATA		12
#define T_SUB_PURGE_TX			2
#define T_SUB_BRK_REQ			4
#define T_SUB_BRK_ON			5
#define T_SUB_BRK_OFF			6


extern void telnet_begin (void);
extern void telnet_handle (void);

#endif /* OBI_TELNET_H  */
