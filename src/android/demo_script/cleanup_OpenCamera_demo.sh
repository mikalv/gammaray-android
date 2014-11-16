#!/system/bin/sh

# Argument check
USAGE="cleanup_demo.sh <folder>"
EXAMPLE_USAGE="cleanup_OpenCamera_demo.sh /sdcard/gray_nbd"
if [ $# -ne 1 ] 
then
	echo $USAGE
	echo Example:
	echo $EXAMPLE_USAGE
	exit
fi
FOLDER=$1

set -x

# Mount device
busybox umount $FOLDER
rm -rf $FOLDER
 
