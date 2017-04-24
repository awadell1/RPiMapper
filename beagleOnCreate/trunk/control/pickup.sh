#!/bin/bash

# From http://wh1t3s.com/2009/05/14/reading-beagleboard-gpio/

# Orginally /usr/bin/readgpio, Modified by Mark A. Yoder 20-Jul-2011

#
# Toggle a GPIO input

echo 131 > /sys/class/gpio/export
echo 130 > /sys/class/gpio/export
echo "out" > /sys/class/gpio/gpio130/direction
echo "out" > /sys/class/gpio/gpio131/direction

echo 15:off >/dev/servodrive0

echo 17:off >/dev/servodrive0


echo 15:100 > /dev/servodrive0

sleep 1
echo 15:off > /dev/servodrive0

echo 17:100 > /dev/servodrive0

sleep 1

#Switch solenoid valve and push pump on

echo '1' > /sys/class/gpio/gpio130/value
sleep 2




echo 17:-100 >/dev/servodrive0

sleep 1
#Switch solenoid valve and push pump off
echo '0' > /sys/class/gpio/gpio130/value
#Turn suction pump on

echo '1' > /sys/class/gpio/gpio131/value
sleep 3

echo 17:100 > /dev/servodrive0

sleep 3
echo 15:-100 > /dev/servodrive0

sleep 1
echo 15:off > /dev/servodrive0

echo 17:off > /dev/servodrive0
