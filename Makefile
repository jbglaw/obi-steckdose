ARDUINO_VARIANT = generic
SERIAL_PORT = /dev/ttyUSB0
FLASH_PARTITION=1M0
ESP8266_VERSION=2.4.2
#ESP8266_VERSION=git
#USER_DEFINE=-DOBI_GIT_COMMIT=\""`git rev-parse HEAD`"\" -DOBI_BUILD_TIMESTAMP=\""`date`"\" -DDEBUG_ESP_HTTP_UPDATE=yes -DDEBUG_ESP_PORT=Serial
USER_DEFINE=-DOBI_GIT_COMMIT=\""`git rev-parse HEAD`"\" -DOBI_BUILD_TIMESTAMP=\""`date`"\"
include Esp8266-Arduino-Makefile/espXArduino.mk
