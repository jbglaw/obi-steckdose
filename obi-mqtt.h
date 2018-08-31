#ifndef OBI_MQTT_H
#define OBI_MQTT_H

#define MQTT_SUBSCRIBE_RELAIS	"device"
#define MQTT_SUBSCRIBE_RESET	"reset"

extern void mqtt_begin (void);
extern void mqtt_handle (void);
extern void mqtt_publish (const char *topic, const char *value);
extern void mqtt_trigger_reset (void);

#endif /* OBI_MQTT_H  */
