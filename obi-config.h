#ifndef OBI_CONIFG_H
#define OBI_CONIFG_H

struct obi_config {
	uint8_t _checksum;

	char wifi_ssid[64];
	char wifi_psk[64];

	char dev_mqtt_name[128];
	char dev_descr[128];

	long serial_speed;
	long serial_bits;
	char serial_parity[2];
	long serial_stopbits;

	bool relay_on_after_boot_p;

	char syslog_host[128];
	char syslog_port[6];
	char mqtt_server_host[128];
	char mqtt_server_port[6];
	char telnet_port[6];
	bool enable_telnet_negotiation_p;
	bool syslog_sent_to_serial_p;
	bool syslog_recv_from_serial_p;
};

extern struct obi_config cfg;

extern int config_load (struct obi_config *cfg);
extern int config_save (struct obi_config *cfg);
extern bool long_in_table (long lng, const long *tbl, size_t num);

#endif /* OBI_CONFIG_H  */
