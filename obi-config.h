#ifndef OBI_CONIFG_H
#define OBI_CONIFG_H

struct obi_config {
	uint8_t _checksum;

        char wifi_ssid[64];
        char wifi_psk[64];

        char dev_mqtt_name[128];
        char dev_title[128];

        long serial_speed;
        int  serial_bits;
        char serial_parity[2];
        int  serial_stopbits;

        bool relay_on_after_boot_p;
	int  relay_delay_seconds;
	bool relay_randomize_delay_p;
};

extern struct obi_config cfg;

extern int  config_load (struct obi_config *cfg);
extern int  config_save (struct obi_config *cfg);

#endif /* OBI_CONFIG_H  */
