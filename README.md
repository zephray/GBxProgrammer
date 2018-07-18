# GBxProgrammer

Game Boy Cartridge Programmer. Works with Microsoft(R) Windows(R) and Linux operating system.

# Features

* Dump (Backup) games ROM to your PC
* Backup save data to your PC
* Restore save data to game cartridge
* Write ROMs to supported Flash carts

# Compatibilty

The programmer should be able to support ROM and RAM readout on all MBC1-5 cartridges and GameBoy Camera. The reprogramming function should work with majority flash carts, but it has not been widely tested. It supports these 3 common wiring diagrams with or without an MBC:

* 8-bit NOR Flash with write enable connected to AIN
* 8-bit NOR Flash with write enable connected to WR
* 16-bit NOR Flash under 8-bit mode with write enable connected to WR

Due to the nature of FAT filesystem emulation, there is no guarentee that it would work under all OSes. Ubuntu 16.04 LTS and Windows 7 are tested to be working. Please be aware that many linux distros have writing cache for external devices default enabled (which does not make sense for a programmer), you probably want to use 'sync' manually to ensure data has been written to the device.

# Speed

This should beat all other programmers. (Though probably not important)

* 1 MByte ROM Read: 3.9s
* 32 KByte SRAM Read or Write: 1s
* 32 KByte Flash Cart Write: 3s
* 1 MByte Flash Cart Write: 39s

# Usage

The device would emulate itself as a 100MB Flash Drive with FAT16 file system. There would be three files in the root directory:

* INFO.TXT
* ROM.GB
* RAM.SAV

Note if you have a cartridge with valid game inserted, GB and SAV file would be renamed to the game name recorded in the cartridge header. 

INFO.TXT would contain some information about the cartridge, either from cartridge header, NOR command query or CFI query.

Please be aware, since the OS would always assume the device would not change the file themselves and the device has no way to notify the OS the file or directory has been changed, you should ALWAYS unplug and replug the USB cable after each write operation.

## Erase Flash/ RAM

Generally one do not need to erase the flash or RAM manually. The Flash would be automatically erased during reprogramming(write) process if needed, and the RAM would be simply overwritten during reprogramming(write) process.

In order to do an erase operation, simply delete the corresponding file, wait till the LED light up steadily, unplug and replug the programmer. A Flash erase process would generally takes anything between 1 second to 1 minute depending on your Flash capacity. Note Windows might show some error, this is normal, the full chip erase is just too slow for Windows to wait. A RAM erase process should finish within 1 second. 

## Dump ROM/ Backup Savefile

Just copy the file from the programmer drive to your hard drive or other places. You do not need to unplug and replug after the reading process.

## Reprogramming Flash/ Restore Savefile

Copy the file to the programmer. The Flash image should have a file extension of GB or GBC, and the save file should have a file extension of SAV. 

Please do not overwrite any file that is already listed in the programmer's drive, means you should use a different name other than anything already in the drive for new image file. For example, you already have a POKEMON.SAV in the drive, please avoid copy another POKEMON.SAV or pokemon.sav into the drive. Rename it into something like pokemon_save.sav before copying.

If it failed, these are possible reasons:

* The new file overwrites an existing file
* The filename is too long, try a filename shorter than 40 chars
* The flash cart is not fully compatible with the programmer, try again or manual erase before reprogramming.

Here are some detailed explanations.

If the Flash support CFI (it should), the programmer would use 'Sector Erase' rather than 'Full Chip Erase' to increase the speed. But if somehow this does not work with your cartridge, the copying process might fail the first time (because it might fallback to the 'Full Chip Erase' mode and it is SLOW, cause timeout error while erasing), try to unplug, replug, and try again. 

LFN is supported in 'back-compatible' mode, means you can have either lowercase or uppercase or mixed in the filename, but it should not be too long due to the limited programmer internal RAM. (Longer name is just garbage to the programmer, but it do consume internal RAM capacity.)

# Status

Should work as explained in this page. New functions may be added in future versions. This device is still under development.

# Acknowledge

This project reused codes from several other projects. A great thanks to their efforts!

* https://github.com/Tauwasser/GBCartFlasher
* https://www.flashrom.org/Flashrom

# Legalese

I'm not affiliated with Nintendo in any way.

Game Boy® is a registered trademark by Nintendo. Nintendo® is a registered trademark. All other trademarks are property of their respective owner.

# License

Software codes except low level hardware libraries provided by silicon vendors are licensed under GPLv3.

All other documents are licensed under CC BY-SA 4.0

Copyright 2018 Wenting Zhang
