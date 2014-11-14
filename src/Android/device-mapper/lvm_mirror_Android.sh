#!/system/bin/sh
export LVM_SYSTEM_DIR=/data/gray/lvm/

if [$# - 2]; then 
	echo "Example usage: lvm_mirror_Android.sh ram0 ram1"
	echo "Example usage: lvm_mirror_Android.sh loop0 loop1"
fi

lvm pvcreate /dev/block/$1                
lvm pvcreate /dev/block/$2 
lvm lvmdiskscan

lvm vgcreate vg1 /dev/block/$1 /dev/block/$2
lvm vgscan
lvm vgdisplay vg1 

lvm lvcreate -L 4M -m1 --mirrorlog core -n mirrorlv vg1 /dev/block/$1 /dev/block/$2
ls /dev/vg1/mirrorlv

mkfs.ext2 /dev/vg1/mirrorlv 
mount -t ext2 /dev/vg1/mirrorlv mnttest/
cd mnttest/

