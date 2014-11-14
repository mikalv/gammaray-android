#!/system/bin/sh
echo Setting up loop device /dev/block/loop$1
cp /sdcard/blank.ima loop$1.ima
busybox losetup -o 1048576 /dev/block/loop$1 loop$1.ima #Offset manually extracted from `parted`
mkdir /data/dloop$1
busybox mount /dev/block/loop$1 /data/dloop$1
mount | grep loop
busybox umount /dev/block/loop$1
mount | grep loop
