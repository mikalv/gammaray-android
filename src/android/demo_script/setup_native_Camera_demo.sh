#!/system/bin/sh

# Argument check
USAGE="setup_demo.sh <folder> <mount device>"
EXAMPLE_USAGE="setup_demo.sh /data/media/0/DCIM/Camera /dev/nbd1"
if [ $# -ne 2 ] 
then
	echo $USAGE
	echo Example:
	echo $EXAMPLE_USAGE
	exit
fi
FOLDER=$1
DEVICE=$2

set -x
# Backup data
# mv $FOLDER $FOLDER-Backup

##########Option 1################
#####ln -s doens't work in /sdcard

# Mount device
# read
# MOUNTPOINT=/sdcard/gray-device/
# mkdir $MOUNTPOINT
# busybox mount $DEVICE $MOUNTPOINT

# Fake the folder with a simlink
# read
# ln -s $MOUNTPOINT $FOLDER

###########Option 2################

# Mount device
read
if [[ $DEVICE != *loop* ]]
then
    busybox mount $DEVICE $FOLDER
else
    echo Setting up loop device $DEVICE
    cp /sdcard/blank.ima loop_gray.ima
    busybox losetup -o 1048576 $DEVICE loop_gray.ima #Offset manually extracted from `par
    busybox mount $DEVICE $FOLDER 
fi
##########End of Option 2##########
# Restore the data
#read
#cp $FOLDER-Backup/* $FOLDER/

