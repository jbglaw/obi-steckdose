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
[x] Telnet<->serial proxy
[ ] Telnet serial extensions
[x] Telnet <-> syslog
[ ] OTA: https://tttapa.github.io/ESP8266/Chap13%20-%20OTA.html
[x] Proper flash config checking
