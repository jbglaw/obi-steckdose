ARDUINO_VARIANT = generic
SERIAL_PORT = /dev/ttyUSB0
FLASH_PARTITION=1M0
ESP8266_VERSION=-2.4.1
#USER_DEFINE=-DOBI_GIT_COMMIT=\""`git rev-parse HEAD`"\" -DOBI_BUILD_DATE=\""`date`"\" -DDEBUG_ESP_HTTP_UPDATE=yes -DDEBUG_ESP_PORT=Serial
USER_DEFINE=-DOBI_GIT_COMMIT=\""`git rev-parse HEAD`"\" -DOBI_BUILD_DATE=\""`date`"\"
include Esp8266-Arduino-Makefile/espXArduino.mk
