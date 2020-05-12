#!/bin/sh
export PATH="/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/sbin:/usr/local/bin"
export LD_LIBRARY_PATH="/lib:/usr/local/lib"
ifconfig wlan0 up
iwpriv wlan0 enc 1
iwpriv wlan0 auth 1
iwconfig wlan0 essid "jupiter"
sleep 5
