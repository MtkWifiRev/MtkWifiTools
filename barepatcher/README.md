Simple live RAM patcher for Mediatek wifi based cards.

supports for now:
* mt7921
* mt7981
* mt7922


WARNING: for mt7921, remember to avoid writing the first segment region (0x915000), seems that is an unwritable region, the last one luckily can be written and is still executable. 
