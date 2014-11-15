#!/bin/bash

source consts.sh
sudo modprobe nbd
set -x -e

[[ -e /dev/nbd0 ]] || sudo mknod /dev/nbd0 b 43 0
sudo nbd-client -N test_ba $NBD_SERVER $NBD_PORT /dev/nbd0
PART=$(sudo kpartx -av /dev/nbd0 | sed -e 's/add map \(\S*\) .*/\1/g' )
sudo mkdir -p /tmp/mpc
sudo mount /dev/mapper/$PART /tmp/mpc
sudo su -c 'echo newfileline >> /tmp/mpc/testfile'
sudo su -c 'echo newfile >> /tmp/mpc/newfile'
sudo umount /tmp/mpc
sudo rm -rf /tmp/mpc
sudo kpartx -dv /dev/nbd0
sudo nbd-client -d /dev/nbd0
