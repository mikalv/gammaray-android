#!/bin/bash

source consts.sh

sudo modprobe nbd

pathadd() {
  [ -d "$1" ] && [[ ":$PATH:" != *":$1:"* ]] && PATH="${PATH:+"$PATH:"}$1"
}
pathadd $HOME/repos/gammaray-android/bin
pathadd $HOME/repos/gammaray-android/bin/test

set -x -e

redis-cli FLUSHALL

dd of=disk.raw seek=$((1024*1024*1024*5)) count=0 bs=1
parted -s disk.raw mklabel msdos
parted -s disk.raw mkpart primary ext4 1 $((1024*5))
PART=$(sudo kpartx -av disk.raw | sed -e 's/add map \(\S*\) .*/\1/g' )
sudo mkfs.ext4 -F /dev/mapper/$PART
sudo mkdir -p /tmp/mps
sudo mount /dev/mapper/$PART /tmp/mps
#read
sudo su -c 'echo filecontents > /tmp/mps/testfile'
sudo umount /tmp/mps
sudo rm -rf /tmp/mps
sudo kpartx -dv disk.raw

sudo pkill gray || true
sudo pkill nbd-queuer-test || true
sudo fusermount -u /tmp/gray-fs || true

sudo pkill nbd || true
nbd-queuer-test test_ba disk.raw $REDIS_SERVER $REDIS_PORT $REDIS_DB \
                5368709120 $NBD_SERVER $NBD_PORT_1 y &> queuer-test.log &
sleep 1

gray-crawler disk.raw disk.bson &> crawler.log
gray-inferencer disk.bson $REDIS_DB disk_test_instance &> inferencer.log &
sleep 1
pkill gray-inferencer

mkdir -p /tmp/gray-fs || true
gray-fs /tmp/gray-fs -d -s disk.raw & &> gray-fs.log

sudo nbd-client localhost $NBD_PORT_1 /dev/nbd0 &> nbd-client-1.log &
sleep 1
PART=$(sudo kpartx -av /dev/nbd0 | sed -e 's/add map \(\S*\) .*/\1/g' )
sudo qemu-nbd -p $NBD_PORT_2 /dev/mapper/$PART &

{ cd /tmp/gray-fs; python2 -m SimpleHTTPServer; } &

set +e

while true; do
  redis-cli flushall
  gray-crawler disk.raw disk.bson &> crawler.log
  gray-inferencer disk.bson $REDIS_DB disk_test_instance &> inferencer.log &
  sleep 20
  pkill gray-inferencer
done
