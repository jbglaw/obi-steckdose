 Building
~~~~~~~~~~
# Once:
git submodule init
git submodule update
cd Esp8266-Arduino-Makefile &&
# Always:
make
make upload	# Serial port in Makefile

 LED Blinking
~~~~~~~~~~~~~~
Config Mode: 500/500
Connecting in STA: 100/100
Running: 100/900 / 900/100 (depends on Relay on/off)

 Missing stuff
~~~~~~~~~~~~~~~
[ ] MQTT (server:port config, endpoint implementation)
[ ] Telnet<->serial proxy
[ ] Telnet serial extensions
[ ] Telnet <-> syslog
[ ] Delayed device power-on after boot
+       char syslog_ip[16];
+       char mqtt_server_ip[16];
+       int mqtt_server_port;

