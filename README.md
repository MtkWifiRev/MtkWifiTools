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

NDS32 ghidra plugin with the definitions for the unknown opcodes for the inlining function extension (still not working, the next step is to implement the Pcode injection for handling also the function inlining extension).

patch_ram_merger.c
==================

Given the rom patch and the ram, this tool will toggle the first region of the ram and it will append it into the last region of the ROM patch.

MT7981Merge.c
==================

Specific tool used for converting the ram + rom patch of the mt7981 (valid also for other firmwares like the mt7986, mt7916, mt7915 ecc..) into an ELF file.

MT7961Merge.c
==================

Specific tool used for converting the ram + rom patch of the mt7921 (aka mt7961) hardware into an elf file, the difference if compared to the MT7981Merge, is that there is an initial padding of 1kb in the data region (otherwise the strings are mismatched).

PatchIncorporator.c
=================

Manipulate the format of the ROM patch by merging it with the RAM firmware for bypassing the ROM crc32 integrity check

mapperremapper
=================

Used for dumping every address range present in the PCIe mapped memory table declared in mt76 into an ELF file. Works with mt7915, mt7916, mt7981, mt7986, mt7921, mt7922 and mt7925.
It helped a lot for breaking mt7922's AES encryption.

auto_detect
==================

Code example which shows how is possible to automatically get the mediatek wifi entry under sysfs, commonly used in other projects

ramreader
==================

RamReader permits to read the disassembled NDS32 instructions in the live RAM by abusing the regidx/regval feature, this permits to understand where the code is relocated for certain firmwares and to read directly the assembly code in the ROM without dumping it
( valid for some hardwares )

cmd_to_realid
==================

Simple tool used for displaying the real CID of a command, useful when is needed to search the callback associated to a specific CID in the hardcoded table in the firmware RAM

barepatcher
==================

RAM live patcher tool, it works by using the NDS32 gcc compiler under the hood, it compiles a program, extract its bytes and then it applies them by abusing the regidx/regval trick for a specific memory area, tested with mt7922 and permitted to bypass the firmware encryption.
