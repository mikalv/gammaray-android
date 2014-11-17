#!/bin/sh

adb push bin/gray-crawler /sdcard/gray/
adb shell su -c 'cp /sdcard/gray/gray-crawler /data/gray/gray-crawler'
