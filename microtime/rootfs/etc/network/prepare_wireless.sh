#!/bin/sh
export PATH="/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/sbin:/usr/local/bin"
export LD_LIBRARY_PATH="/lib:/usr/local/lib"
ifconfig rausb0 up
iwpriv rausb0 enc 1
iwpriv rausb0 auth 1
iwconfig rausb0 essid "jupiter"
sleep 5
