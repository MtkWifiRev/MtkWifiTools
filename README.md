MtkWifiTools
==========================

this repository contains a list of tools which have been used for analyzing the Mediatek Wifi firmware in order to get a better understanding of how it works.


MASS
==================

Python scraper which permits to download automatically every WIFI_RAM_CODE found in the https://dumps.tadiphone.dev/ repository.

mtk_real_cmd
==================

simple tool which permits to associate for every CID it's real ID inside the RAM firmware hardcoded table, useful for finding the entry of the table in the fw.

android_fw_log
==================

simple tools which permits to set the log level and enable the wifi's log shell in gen4m based phones, do "strings /dev/fw_log_wifi" for reading them.

ghidra/nds32.sinc
==================

NDS32 ghidra plugin with the definitions for the unknown opcodes for the inlining function extension (still not working).

patch_ram_merger.c
==================

Given the rom patch and the ram, this tool will toggle the first region of the ram and it will append it into the last region of the ROM patch.
