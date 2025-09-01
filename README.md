MtkWifiTools
==========================

this repository contains a list of tools which have been used for analyzing the Mediatek Wifi firmware in order to get a better understanding of how it works.


MASS
==================ù

```
usage:
python3 Mass.py startpage=<x> ("define the start page for the https://dumps.tadiphone.dev/dumps page)
```

Python scraper which permits to download automatically every WIFI_RAM_CODE found in the https://dumps.tadiphone.dev/ repository.

mtk_real_cmd / cmd_to_realid
==================

```
usage:
./mtk_real_cmd
```

simple tool which permits to associate for every CID it's real ID inside the RAM firmware hardcoded table, useful for finding the entry of the table in the fw.

android_fw_log
==================

```
usage:
./fw_log_controller [on|off|set_level=<x>]
```

simple tools which permits to set the log level and enable the wifi's log shell in gen4m based phones, do "strings /dev/fw_log_wifi" for reading them.

patch_ram_merger.c
==================

```
usage:
./patch_ram_merger <wifi_ram_code> <rom_patch>
NOTE: the order isn't important, you can put the name of the rom_patch before the ram fw and it will work as usual, just remember to use the correct name ("WIFI_RAM_CODE" and "_patch_mcu_")
```

Given the rom patch and the ram, this tool will toggle the first region of the ram and it will append it into the last region of the ROM patch.

MT7981Merge.c
==================

```
usage:
./MT7981Merge <fw1.bin> <fw2.bin> <wf_rom.axf> ...
```

Specific tool used for converting the ram + rom patch of the mt7981 (valid also for other firmwares like the mt7986, mt7916, mt7915 ecc..) into an ELF file.

MT7961Merge.c
==================

```
usage:
./MT7961Merge <fw1.bin> <fw2.bin> <wf_rom.axf> ...
```

Specific tool used for converting the ram + rom patch of the mt7921 (aka mt7961) hardware into an elf file, the difference if compared to the MT7981Merge, is that there is an initial padding of 1kb in the data region (otherwise the strings are mismatched).

PatchIncorporator.c
=================

```
usage:
./PatchIncorporator <wifi_ram_code> <rom_patch>
NOTE: the order isn't important, you can put the name of the rom_patch before the ram fw and it will work as usual, just remember to use the correct name ("WIFI_RAM_CODE" and "_patch_mcu_")
```

Manipulate the format of the ROM patch by merging it with the RAM firmware for bypassing the ROM crc32 integrity check

mapperremapper
=================

```
usage:
./mapperremapper <device-number> <hardware-model> <mode>
where:
<device-number> is the phyX number in /sys/kernel/debug/ieee80211/phyX/mt76/
<hardware-model> can be mt7921 or mt7925 (note that mt7961 is used for dumping data from the mt7921 usb adapter!)
<mode> can be --interactive or --automatic
- interactive means that the user can write the desired address to dump and see the results
- automatic means that every memory area defined in the hardware's dependent struct are dumped
```

Used for dumping every address range present in the PCIe mapped memory table declared in mt76 into an ELF file. Works with mt7915, mt7916, mt7981, mt7986, mt7921, mt7922 and mt7925.
It helped a lot for breaking mt7922's AES encryption.

auto_detect
==================

```
usage:
./simple_detect
(this is just an explanatory code!)
```

Code example which shows how is possible to automatically get the mediatek wifi entry under sysfs, commonly used in other projects

ramreader
==================

```
usage:
sudo ./ramreader
insert the address at runtime
```
RamReader permits to read the disassembled NDS32 instructions in the live RAM by abusing the regidx/regval feature, this permits to understand where the code is relocated for certain firmwares and to read directly the assembly code in the ROM without dumping it
( valid for some hardwares )


barepatcher
==================

```
usage:
write the C code at './code_output/nds32_code.c'
sudo ./barepatcher
insert the patching address at runtime
```

RAM live patcher tool, it works by using the NDS32 gcc compiler under the hood, it compiles a program, extract its bytes and then it applies them by abusing the regidx/regval trick for a specific memory area, tested with mt7922 and permitted to bypass the firmware encryption.


ghidra/nds32.sinc
==================

NDS32 ghidra plugin with the definitions for the unknown opcodes for the inlining function extension (still not working, the next step is to implement the Pcode injection for handling also the function inlining extension).
