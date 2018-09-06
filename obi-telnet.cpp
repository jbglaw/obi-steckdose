#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "obi-common.h"
#include "obi-misc.h"
#include "obi-config.h"
#include "obi-telnet.h"

#define UART0_DEFAULT_RxX_PIN	3
#define UART0_DEFAULT_TxD_PIN	1
#define UART0_UART_FUNCTION	FUNCTION_0
#define UART0_GPIO_FUNCTION	FUNCTION_3

enum telnet_state {
	TN_init,
	TN_transparent,

	TN_normal,
	TN_iac,
	TN_will,
	TN_start,
	TN_end,
	TN_comPort,
	TN_setControl,
	TN_setBaud,
	TN_setDataSize,
	TN_setParity,
	TN_setStopSize,
	TN_purgeData,
};

static byte buf_rx[1024];
static byte buf_tx[1024];
static size_t buf_rx_len = 0;
static size_t buf_tx_len = 0;
static unsigned long last_rx_send_millis = 0;
static unsigned long last_tx_send_millis = 0;

static bool break_active_p = false;

static struct telnet_client {
	bool active_p;
	enum telnet_state state;
	WiFiClient client;
} telnet_client[OBI_MAX_TELNET_CLIENTS] = {
	{
		.active_p = false,
	}, {
		.active_p = false,
	}, {
		.active_p = false,
	},
};

WiFiServer telnet_server (OBI_TELNET_PORT_DEFAULT);

static void
syslog_format_buf (char *outbuf, size_t outbuf_len, char *asciibuf, size_t asciibuf_len, uint8_t *inbuf, size_t inbuf_len)
{
	char hexbuf[10];

	outbuf[0] = '\0';
	for (size_t i = 0; i < inbuf_len; i++) {		// XXX overflow
		sprintf (hexbuf, " %02x", inbuf[i]);
		strcat (outbuf, hexbuf);
		if (isprint (inbuf[i])) {
			asciibuf[i]   = inbuf[i];
			asciibuf[i+1] = '\0';
		} else {
			asciibuf[i]   = ' ';
			asciibuf[i+1] = '\0';
		}
	}

	return;
}

static void
syslog_send_buffer (const char *prefix, byte *inbuf, size_t inbuf_len)
{
#define BYTES_PER_LINE	32
	size_t done = 0;
	size_t max;
	char hexline[BYTES_PER_LINE * 3 + 5];
	char asciiline[BYTES_PER_LINE + 5];

	while (done < inbuf_len) {
		max = inbuf_len - done;
		if (max > BYTES_PER_LINE)
			max = BYTES_PER_LINE;

		syslog_format_buf (hexline, sizeof (hexline), asciiline, sizeof (asciiline), inbuf + done, max);
		syslog.logf (LOG_INFO, "%s %s%s |%s|", cfg.dev_mqtt_name, prefix, hexline, asciiline);

		done += max;
	}

	return;
}

static void
syslog_send_rx_data (bool force_p)
{
	if (buf_rx_len == 0)
		return;

	if (force_p || last_rx_send_millis + 200 < millis ()) {
		syslog_send_buffer ("<", buf_rx, buf_rx_len);
		last_rx_send_millis = millis ();
		buf_rx_len = 0;
	}

	return;
}

static void
syslog_send_tx_data (bool force_p)
{
	if (buf_tx_len == 0)
		return;

	if (force_p || last_tx_send_millis + 200 < millis ()) {
		syslog_send_buffer (">", buf_tx, buf_tx_len);
		last_tx_send_millis = millis ();
		buf_tx_len = 0;
	}

	return;
}

void
telnet_begin (void)
{
	//  syslog.logf(LOG_INFO, "This is info message no. %d", iteration);
	//  syslog.log(LOG_INFO, F("End loop"));

	telnet_server.begin (atoi (cfg.telnet_port));

	return;
}

static void
telnet_break_start (void)
{
	if (! break_active_p) {
		pinMode (UART0_DEFAULT_TxD_PIN, UART0_GPIO_FUNCTION);
		digitalWrite (UART0_DEFAULT_TxD_PIN, 0);
		break_active_p = true;
	}

	return;
}

static void
telnet_break_stop (void)
{
	if (break_active_p) {
		digitalWrite (UART0_DEFAULT_TxD_PIN, 1);
		pinMode (UART0_DEFAULT_TxD_PIN, UART0_UART_FUNCTION);
		break_active_p = false;
	}

	return;
}

static void
push_tx_data (byte *buf, size_t len)
{
	size_t max_len;
	size_t done = 0;

	/* First drain Rx data to keep proper order in Syslog.  */
	syslog_send_rx_data (true);

	/* Push new data into buffer, possibly forcing it out to Syslog.  */
	while (len > 0) {
		max_len = len;
		if (max_len > sizeof (buf_tx) - buf_tx_len)
			max_len = sizeof (buf_tx) - buf_tx_len;
		memcpy (buf_tx + buf_tx_len, buf + done, max_len);

		done += max_len;
		len -= max_len;

		if (buf_tx_len == sizeof (buf_tx))
			syslog_send_tx_data (true);
	}

	return;
}

static void
push_rx_data (byte *buf, size_t len)
{
	size_t max_len;
	size_t done = 0;

	/* First drain Tx data to keep proper order in Syslog.  */
	syslog_send_tx_data (true);

	/* Push new data into buffer, possibly forcing it out to Syslog.  */
	while (len > 0) {
		max_len = len;
		if (max_len > sizeof (buf_rx) - buf_rx_len)
			max_len = sizeof (buf_rx) - buf_rx_len;
		memcpy (buf_rx + buf_rx_len, buf + done, max_len);

		done += max_len;
		len -= max_len;

		if (buf_rx_len == sizeof (buf_rx))
			syslog_send_rx_data (true);
	}

	return;
}

static unsigned long tn_baud;
static size_t tn_baudCnt;

void
telnet_handle (void)
{
	uint8_t ser_buf[16];
	ssize_t read_len;
	char syslog_msg[100];
	WiFiClient new_client = telnet_server.available ();

	/* Check for new clients.  */
	if (new_client) {
		bool found_slot_p = false;

		for (size_t i = 0; i < ARRAY_SIZE (telnet_client); i++) {
			if (! telnet_client[i].active_p) {
				telnet_client[i].client = new_client;
				telnet_client[i].state = cfg.enable_telnet_negotiation_p? TN_init: TN_transparent;
				telnet_client[i].active_p = true;
				found_slot_p = true;
			}
		}

		/* No free slot, kill the client. 8-x  */
		if (! found_slot_p)
			new_client.stop ();
	}

	/* Check for clients gone.  */
	for (size_t i = 0; i < ARRAY_SIZE (telnet_client); i++) {
		if (telnet_client[i].active_p && ! telnet_client[i].client.connected ()) {
			telnet_client[i].active_p = false;
		}
	}

	/* Wifi -> Serial.  */
	for (size_t i = 0; i < ARRAY_SIZE (telnet_client); i++) {
		if (telnet_client[i].active_p
		    && telnet_client[i].client.available () > 0) {
			read_len = telnet_client[i].client.read (ser_buf, sizeof (ser_buf));
			if (read_len > 0) {
				/* Detect TELNET protocol initiated by client.  */
				if (telnet_client[i].state == TN_init
				    && read_len >= 2
				    && ser_buf[0] == T_IAC
				    && (ser_buf[1] == T_WILL || ser_buf[1] == T_DO))
					telnet_client[i].state = TN_normal;
				/* Not detected? Then it's plain pass-through.  */
				if (telnet_client[i].state == TN_init)
					telnet_client[i].state = TN_transparent;

				if (telnet_client[i].state == TN_transparent) {
					if (cfg.syslog_sent_to_serial_p)
						push_tx_data (ser_buf, read_len);
					Serial.write (ser_buf, read_len);
				} else {
					for (size_t bufidx = 0; bufidx < read_len; bufidx++) {
						switch (telnet_client[i].state) {
							case TN_normal:
								if (ser_buf[bufidx] == T_IAC)
									telnet_client[i].state = TN_iac;
								else {
									if (cfg.syslog_sent_to_serial_p)
										push_tx_data (&ser_buf[bufidx], 1);
									Serial.write (ser_buf[bufidx]);
								}
								break;

							case TN_iac:
								switch (ser_buf[bufidx]) {
									case T_IAC:
										if (cfg.syslog_sent_to_serial_p)
											push_tx_data (&ser_buf[bufidx], 1);
										Serial.write (ser_buf[bufidx]);
										telnet_client[i].state = TN_normal;
										break;

									case T_WILL:
										telnet_client[i].state = TN_will;
										break;

									case T_SB:
										telnet_client[i].state = TN_start;
										break;

									case T_SE:
										telnet_client[i].state = TN_normal;
										break;

									default:
										/* ??? Pass-through... */
										if (cfg.syslog_sent_to_serial_p) {
											byte data[1] = { T_IAC};
											push_tx_data (data, 1);
											push_tx_data (&ser_buf[bufidx], 1);
										}
										Serial.write (T_IAC);
										Serial.write (ser_buf[bufidx]);
										break;
								}

							case TN_will: {
								byte respBuf[3] = {T_IAC, T_DONT, ser_buf[bufidx]};
								if (ser_buf[bufidx] == T_OPT_COMPORT)
									respBuf[1] = T_DO;
								telnet_client[i].client.write (respBuf, sizeof (respBuf));
								telnet_client[i].state = TN_normal;
								break;
							}

							case TN_start:
								if (ser_buf[bufidx] == T_OPT_COMPORT)
									telnet_client[i].state = TN_comPort;
								else
									telnet_client[i].state = TN_end;
								break;

							case TN_end:
								if (ser_buf[bufidx] == T_IAC)
									telnet_client[i].state = TN_iac;
								break;

							case TN_comPort:
								switch (ser_buf[bufidx]) {
									case T_SUB_COMPORT_SETCONTROL:
										telnet_client[i].state = TN_setControl;
										break;

									case T_SUB_COMPORT_SETDATASIZE:
										telnet_client[i].state = TN_setDataSize;
										break;

									case T_SUB_COMPORT_SETPARITY:
										telnet_client[i].state = TN_setParity;
										break;

									case T_SUB_COMPORT_SETSTOPSIZE:
										telnet_client[i].state = TN_setStopSize;
										break;

									case T_SUB_COMPORT_SETBAUD:
										telnet_client[i].state = TN_setBaud;
										tn_baudCnt = 0;
										tn_baud = 0;
										break;

									case T_SUB_COMPORT_PURGEDATA:
										telnet_client[i].state = TN_purgeData;
										break;

									default:
										telnet_client[i].state = TN_end;
										break;
								}
								break;

							case TN_purgeData:
								switch (ser_buf[bufidx]) {
									case T_SUB_PURGE_TX:
										Serial.flush ();
										break;
								}
								telnet_client[i].state = TN_end;
								break;

							case TN_setControl:
								switch (ser_buf[bufidx]) {
									case T_SUB_BRK_REQ: {
										char respBuf[7] = { T_IAC, T_SB, T_OPT_COMPORT, T_SUB_COMPORT_SETCONTROL, (char) (break_active_p? 1: 0), T_IAC, T_SE };
										telnet_client[i].client.write (respBuf, sizeof (respBuf));
										break;
									}

									case T_SUB_BRK_ON:
										telnet_break_start ();
										break;

									case T_SUB_BRK_OFF:
										telnet_break_stop ();
										break;
								}
								telnet_client[i].state = TN_end;
								break;

							case TN_setDataSize:
								if (ser_buf[bufidx] >= 5
								    && ser_buf[bufidx] <= 8) {
									cfg.serial_bits = ser_buf[bufidx];
									Serial.begin (cfg.serial_speed, serial_framing ());
									config_save (&cfg);
								} else if (ser_buf[bufidx] == 0) {
									char respBuf[7] = { T_IAC, T_SB, T_OPT_COMPORT, T_SUB_COMPORT_SETDATASIZE, (char) cfg.serial_bits, T_IAC, T_SE };
									telnet_client[i].client.write (respBuf, sizeof (respBuf));
								}
								telnet_client[i].state = TN_end;
								break;

							case TN_setBaud:
								/* XXX */
								telnet_client[i].state = TN_end;
								break;

							case TN_setParity:
								/* XXX */
								telnet_client[i].state = TN_end;
								break;

							case TN_setStopSize:
								/* XXX */
								telnet_client[i].state = TN_end;
								break;
						}
					} /* for each ser_buf[byte]  */
				} /* if (not TN_transparent)  */
			} /* if (read_len > 0)  */
		} /* if (client_active && available ()  */
	} /* for (each client)  */

	/* Serial -> Wifi.  */
	if (Serial.available () > 0) {
		read_len = Serial.readBytes (ser_buf, sizeof (ser_buf));
		if (read_len > 0) {
			if (cfg.syslog_recv_from_serial_p)
				push_rx_data (ser_buf, read_len);

			for (size_t i = 0; i < ARRAY_SIZE (telnet_client); i++) {
				if (telnet_client[i].active_p) {
					telnet_client[i].client.write ((char *) ser_buf, read_len);
				}
			}
		}
	}

	return;
}
