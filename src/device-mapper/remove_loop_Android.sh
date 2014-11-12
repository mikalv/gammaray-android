#!/system/bin/sh
echo Disassociating loop device /dev/block/loop$1
busybox losetup -d /dev/block/loop$1 
