#ifndef OBI_CONIFG_H
#define OBI_CONIFG_H

/* Cf. obi-http.h, obi-config.c .  */
struct obi_config {
	char	 dev_descr[128];

	char	 wifi_ssid[64];
	char	 wifi_psk[64];

	uint32_t serial_speed;
	uint32_t serial_bits;
	char	 serial_parity[2];
	uint32_t serial_stopbits;

	bool	 relay_on_after_boot_p;

	char	 syslog_host[128];
	uint32_t syslog_port;
	bool	 syslog_sent_to_serial_p;
	bool	 syslog_recv_from_serial_p;

	char	 mqtt_name[128];
	char	 mqtt_host[128];
	uint32_t mqtt_port;

	uint32_t telnet_port;
	bool	 telnet_enable_proto_p;

	bool	 suppress_led_blinking_p;
};
extern struct obi_config cfg;

struct obi_config_flash_header {
	char magic[8];
#define OBI_CONFIG_FLASH_MAGIC	{ 0x0c, 0x7d, 0xfd, 0xc2, 0xf9, 0x4d, 0xda, 0x9d, }
	uint32_t crc32sum;
	uint16_t json_len;
};

extern int config_load (void);
extern int config_save (void);
extern bool long_in_table (uint32_t lng, const uint32_t *tbl, size_t num);

#endif /* OBI_CONFIG_H  */
