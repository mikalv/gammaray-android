#!/system/bin/sh

# Argument check
USAGE="cleanup_demo.sh <folder>"
EXAMPLE_USAGE="cleanup_demo.sh /data/media/0/DCIM/Camera"
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
rm -rf $FOLDER-Backup
 
# Restore data
# automaticly restored by umount
# mv $FOLDER-Backup $FOLDER
