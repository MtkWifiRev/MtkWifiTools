#!/usr/bin/sh

aarch64-linux-gnu-gcc --static fw_log_controller.c -o fw_log_controller

### use ADB for PC-TO-ANDROID file transfer

adb push fw_log_controller /storage/self/primary/DCIM
