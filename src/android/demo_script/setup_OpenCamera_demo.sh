#!/system/bin/sh

# Argument check
USAGE="setup_demo.sh <MNTPOINT> <mount device>"
EXAMPLE_USAGE="setup_demo.sh /sdcard/gray_nbd /dev/nbd1\n\
setup_demo.sh /sdcard/gray_nbd /dev/block/loop5"

if [ $# -ne 2 ] 
then
	echo $USAGE
	echo Example:
	echo $EXAMPLE_USAGE
	exit
fi
MNTPOINT=$1
DEVICE=$2
FOLDER_NAME=`echo $MNTPOINT | rev | cut -d'/' -f 1 | rev` 

set -x
mkdir $MNTPOINT

# Mount device
if [[ $DEVICE != *loop* ]]
then
	# Mounting
	read
    busybox mount $DEVICE $MNTPOINT
else
    # Setting up loop device $DEVICE
    read
    cp /sdcard/blank.ima loop_gray.ima
    busybox losetup -o 1048576 $DEVICE loop_gray.ima #Offset manually extracted from `par

    # Mounting
    read
    busybox mount $DEVICE $MNTPOINT 
fi

# Prove it's mounted
mount | grep $FOLDER_NAME  

#Make this MNTPOINT available
read
chown root:sdcard_rw $MNTPOINT
chmod 775 $MNTPOINT


