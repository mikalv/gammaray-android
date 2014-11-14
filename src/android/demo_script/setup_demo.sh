#!/system/bin/sh

# Argument check
USAGE="setup_demo.sh <folder> <mount device>"
EXAMPLE_USAGE="setup_demo.sh /sdcard/DCIM/Camera /dev/nbd1"
if [ $# -ne 2 ] 
then
	echo $USAGE
	echo Example:
	echo $EXAMPLE_USAGE
fi
FOLDER=$1
DEVICE=$2

set -x
# Backup data
mv $FOLDER $FOLDER-Backup

# Mount device
read
MOUNTPOINT= /data/gray-device/
mkdir $MOUNTPOINT
mount $DEVICE $MOUNTPOINT

# Fake the folder with a simlink
read
ln -s $MOUNTPOINT $FOLDER

# Restore the data
read
cp $FOLDER-Backup/* $FOLDER

