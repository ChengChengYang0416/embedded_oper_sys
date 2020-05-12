#!/bin/sh
#
# makedev.sh - creates device files for a busybox boot floppy image

# we do our work in the dev/ directory
#if [ -z "$1" ]; then
#	echo "usage: `basename $0` path/to/dev/dir"
#	exit 1
#fi
pwdstring=$PWD
cd /dev


# Miscellaneous one-of-a-kind stuff
#mknod console c 5 1
if [ -e "full" ];then 
   echo "exist,don't create device"
   cd  $pwdstring
   exit 0 
fi
mknod full c 0 0
mknod kmem c 1 2
mknod mem c 1 1
#mknod null c 1 3
mknod port c 1 4
mknod random c 1 8
mknod urandom c 1 9
mknod zero c 1 5
mknod ptmx c 5 2
mknod rtc c 10 135
mknod fb0 c 29 0
mknod fb0autodetect c 29 1
mknod fb0current c 29 0
mknod fb1 c 29 32
mknod fb1autodetect c 29 32
mknod fb1current c 29 32
mknod video0 c 81 0
mknod cmos c 110 0
mknod lcd  c 120 0
mknod codec c 121 0
mknod gprs c  122 0
mknod mtdsp c  123 0
mknod fpga c  124 0
mknod fpga_a2d c  125 0

ln -s /dev/video0 /dev/video
ln -s /proc/kcore core

# IDE HD devs
# note: not going to bother creating all concievable partitions; you can do
# that yourself as you need 'em.
mknod hda b 3 0
for i in `seq 1 19`; do
	mknod hda$i b 3 $i
done

mknod hdb b 3 64
for i in `seq 1 19`; do
	mknod hdb$i b 3 `expr $i + 64`
done

mknod hdc b 22 0
for i in `seq 1 19`; do
	mknod hdc$i b 22 $i
done

mknod hdd b 22 64

#scsi hard disks
mknod sda b 8 0
for i in `seq 1 9`; do
	mknod sda$i b 8 $i
done

mknod sdb b 8 16
for i in `seq 1 9`; do
	mknod sdb$i b 8 `expr $i + 16`
done

mknod sdc b 8 32
for i in `seq 1 9`; do
	mknod sdc$i b 8 `expr $i + 32`
done


# loop devs
for i in `seq 0 7`; do
	mknod loop$i b 7 $i
done

# ram devs
for i in `seq 0 9`; do
	mknod ram$i b 1 $i
done
ln -s ram1 ram

# ttys
mknod tty c 5 0
for i in `seq 0 9`; do
	mknod tty$i c 4 $i
done

# ttySs
for i in `seq 0 3`; do
	mknod ttyS$i c 4 `expr $i + 64`
done

# ttySAs
for i in `seq 0 3`; do
	mknod ttySA$i c 204 `expr $i + 5`
done

# ttyUSBs
for i in `seq 0 1`; do
	mknod ttyUSB$i c 188 $i
done

# ptys
for i in `seq 0 9`; do
	mknod ptya$i c 2 `expr $i + 176`
done

# virtual console screen devs
for i in `seq 0 9`; do
	mknod vcs$i b 7 $i
done
ln -s vcs0 vcs

# mtds
for i in `seq 0 7`; do
	mknod mtd$i c 90 `expr $i \* 2`
done

# mtdblocks
for i in `seq 0 7`; do
	mknod mtdblock$i b 31 $i
done

# mixers
#mknod mixer c 14 0
#for i in `seq 1 3`; do
#	mknod mixer$i c 14 `expr $i \* 16`
#done

ln -s vcsa0 vcsa
#
for i in `seq 0 9`; do
	mknod vcsa$i b 7 $i
done

#audio
#mknod audio  c 14   4
#mknod audio0 c 14   4
#mknod audio1 c 14  20
#mknod audio2 c 14  36
#mknod audio3 c 14  52

#dsp
#mknod dsp  c 14  3
#mknod dsp0 c 14  3
#mknod dsp1 c 14 19
#mknod dsp2 c 14 35
#mknod dsp3 c 14 51
if [ -f /etc/init.d/snddevices ]
then
/etc/init.d/snddevices
fi

#mmc
mknod mmcda  b 60 0
mknod mmcda1 b 60 1
mknod mmcda2 b 60 2

#i2c 
mknod i2c0 c 89 0
for i in `seq 0 2`; do
	mknod i2c-$i b 89 $i
done

#input
mkdir input
mknod input/event0  c 13  64
mknod input/event1  c 13  65
mknod input/js0     c 13  0
mknod input/js1     c 13  1
mknod input/mice    c 13  63
mknod input/mouse0  c 13  32
mknod input/mouse1  c 13  33
mknod input/ts0     c 13  128
mknod input/ts1     c 13  129
ln -s input/event0  ts

#bluetooth
if [ -f /etc/init.d/create_dev ]
then
/etc/init.d/create_dev
fi
cd  $pwdstring

