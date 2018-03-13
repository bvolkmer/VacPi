#!/usr/bin/env bash

echo "Installing dependencies needed to build the sources and tests..."

ARDUINO_BASENAME="arduino-1.8.5"
ARDUINO_FILE="$ARDUINO_BASENAME-linux64.tar.xz"
ARDUINO_URL="http://arduino.cc/download.php?f=/$ARDUINO_FILE"

#AVR_GCC_BASENAME="arduino-1.5.8"
#AVR_GCC_FILE="$AVR_GCC_BASENAME-linux64.tgz"
AVR_GCC_BASENAME="$ARDUINO_BASENAME"
AVR_GCC_FILE="$ARDUINO_FILE"
AVR_GCC_URL="http://arduino.cc/download.php?f=/$AVR_GCC_FILE"

echo "Installing Arduino 1.8.5..."

wget "$ARDUINO_URL" -O "$ARDUINO_FILE"
tar -xJf "$ARDUINO_FILE"
sudo mv "$ARDUINO_BASENAME/" "$ARDUINO"

echo "Installing avr-gcc from Arduino 1.5.5..."

wget "$AVR_GCC_URL" -O "$AVR_GCC_FILE"
tar -xJf "$AVR_GCC_FILE"
sudo mv "$AVR_GCC_BASENAME/hardware/tools/avr" "$AVR_GCC"

echo "Installation of dependencies is complete, we are now going to run some tests..."

source "$SCRIPTS_DIR/runtests.sh"
