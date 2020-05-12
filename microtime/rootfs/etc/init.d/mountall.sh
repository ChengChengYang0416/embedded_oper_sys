#
# mountall.sh	Mount all filesystems.
#
# Version:	@(#)mountall.sh  2.78  05-Jun-2000  miquels@cistron.nl
#
# chkconfig: S 35 0
#

. /etc/default/rcS

#
# Mount local file systems in /etc/fstab.
#
[ "$VERBOSE" != no ] && echo "Mounting local filesystems..."
#mount -avt nonfs,noproc,nosmbfs
mount -n -t proc /proc /proc
mount -n -t sysfs /sys /sys
mount    -t devpts null /dev/pts
mount -a
#mkdir /var/run
#mkdir /var/log
#mkdir /var/lock

#
# We might have mounted something over /dev, see if /dev/initctl is there.
#
if [ ! -p /dev/initctl ]
then
	rm -f /dev/initctl
	mknod -m 600 /dev/initctl p
fi
# by pyliu kill -USR1 1

# We don't enable swap here.. we do that in the mountswap.sh
