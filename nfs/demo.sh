#!/bin/sh

set -x

rmmod -f mydev
insmod mydev.ko

./writer 9 &
./reader 192.168.0.114 4444 /dev/mydev
