 Building
~~~~~~~~~~
# Once:
git submodule init
git submodule update
( cd Esp8266-Arduino-Makefile && bash ./esp8266-install.sh )

# Always:
make
make upload	# Serial port in Makefile

 LED Blinking
~~~~~~~~~~~~~~
Config Mode (as AP): 500/500
OTA Flashing: solid on
Connecting in STA mode: 100/100
Running (in STA mode): 150/850 / 990/10 (depends on Relay on/off)

 Missing stuff
~~~~~~~~~~~~~~~
[x] MQTT (server:port config, endpoint implementation)
[x] network<->serial proxy
[ ] Telnet serial extensions
[x] Telnet <-> syslog
[x] OTA: https://tttapa.github.io/ESP8266/Chap13%20-%20OTA.html (tested)
[x] Proper flash config checking (tested)
[x] mDNS (tested: ping ${MQTT/mDNS Name}.local.)
[x] Syslog (tested)
