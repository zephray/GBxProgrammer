/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2013 Andrzej Surowiec <emeryth@gmail.com>
 * Copyright (C) 2013 Pavol Rusnak <stick@gk2.sk>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include "ramdisk.h"
#include "fat16.h"

#define WBVAL(x) ((x) & 0xFF), (((x) >> 8) & 0xFF)
#define QBVAL(x) ((x) & 0xFF), (((x) >> 8) & 0xFF),\
         (((x) >> 16) & 0xFF), (((x) >> 24) & 0xFF)

// Each cluster is 16KB
// 80MB total
// Each FAT can hold 256 entries, 4 MB
// Need 20 FATs (Not preserved, generated on the fly)
#define SECTOR_COUNT              163840
#define SECTOR_SIZE               512
#define BYTES_PER_SECTOR          512
#define SECTORS_PER_CLUSTER       32
#define RESERVED_SECTORS          2
#define FAT_COPIES                2
#define SECTOR_PER_FAT            20
#define ROOT_ENTRIES              512
#define ROOT_ENTRY_LENGTH         32
#define FILEDATA_START_CLUSTER    3
#define ROOT_ENTRY_SECTOR     (RESERVED_SECTORS + FAT_COPIES * SECTOR_PER_FAT)
#define DATA_REGION_SECTOR    (ROOT_ENTRY_SECTOR + \
            (ROOT_ENTRIES * ROOT_ENTRY_LENGTH) / BYTES_PER_SECTOR)
#define FILEDATA_START_SECTOR    (DATA_REGION_SECTOR + \
            (FILEDATA_START_CLUSTER - 2) * SECTORS_PER_CLUSTER)

// filesize is 16kB (32 * SECTOR_SIZE)
#define FILEDATA_SECTOR_COUNT    32

uint8_t BootSector[] = {
    0xEB, 0x3C, 0x90,                    // code to jump to the bootstrap code
    'm', 'k', 'd', 'o', 's', 'f', 's', 0x00,        // OEM ID
    WBVAL(BYTES_PER_SECTOR),                 // bytes per sector
    SECTORS_PER_CLUSTER,                     // sectors per cluster
    WBVAL(RESERVED_SECTORS),                 // # of reserved sectors 
    FAT_COPIES,                              // FAT copies (2)
    WBVAL(ROOT_ENTRIES),                     // root entries (512)
    0x00, 0x00,                              // total number of sectors 16
    0xF8,                                    // media descriptor (0xF8 = Fixed disk)
    WBVAL(SECTOR_PER_FAT),                   // sectors per FAT
    WBVAL(0x003F),                           // sectors per track 
    WBVAL(0x00FF),                           // number of heads 
    0x00, 0x00, 0x00, 0x00,                  // hidden sectors (0)
    QBVAL(SECTOR_COUNT),                     // total number of sectors 32 
    0x80,                                    // drive number (0)
    0x00,                                    // reserved
    0x29,                                    // extended boot signature
    0xC0, 0xFF, 0x12, 0x45,                  // volume serial number
    'G', 'B', 'X', 'P', 'R', 'O', 'G', ' ', ' ', ' ', ' ',    // volume label
    'F', 'A', 'T', '1', '6', ' ', ' ', ' '            // filesystem type
};

const uint8_t DirSector[]= 
{
    'G','B','x','P','r','o','g',' ',' ',' ',' ',  // Volume label
    0x08,           // File attribute = Volume label 
    0x00,           // Reserved 
    0x00,           // Create Time Tenth 
    WBVAL(0x0000),  // Create Time 
    WBVAL(0x0000),  // Create Date 
    WBVAL(0x0000),  // Last Access Date 
    0x00, 0x00,     // Not used in FAT16 
    WBVAL(0x429D),  // Write Time 
    WBVAL(0x3892),  // Write Date 
    WBVAL(0x0000),  // Cluster Low
    WBVAL(0x0000),  // File Size
    'I','N','F','O',' ',' ',' ',' ','T','X','T',  // File name
    0x08,           // File attribute = Archive
    0x00,           // Reserved 
    0x4B,           // Create Time Tenth 
    WBVAL(0x429C),  // Create Time 
    WBVAL(0x3892),  // Create Date 
    WBVAL(0x3892),  // Last Access Date 
    0x00, 0x00,     // Not used in FAT16 
    WBVAL(0x429D),  // Write Time 
    WBVAL(0x3892),  // Write Date 
    WBVAL(0x0000),  // Cluster Low
    WBVAL(0x0000),  // File Size
};

int ramdisk_init(void)
{
    uint32_t i = 0;

    return 0;
}

int ramdisk_read(uint32_t lba, uint8_t *copy_to)
{
    memset(copy_to, 0, SECTOR_SIZE);
    switch (lba) {
        case 0: // sector 0 is the boot sector
            memcpy(copy_to, BootSector, sizeof(BootSector));
            copy_to[512 - 2] = 0x55;
            copy_to[512 - 1] = 0xAA;
            break;
        case ROOT_ENTRY_SECTOR: // sector 3 is the directory entry
            memcpy(copy_to, DirSector, sizeof(DirSector));
            break;
        default:
            if (lba < ROOT_ENTRY_SECTOR) {
                // FAT entries
                uint32_t fat_seek = lba - RESERVED_SECTORS;
                if (fat_seek > SECTOR_PER_FAT) fat_seek -= SECTOR_PER_FAT;
            }
            break;
    }
    return 0;
}

int ramdisk_write(uint32_t lba, const uint8_t *copy_from)
{
    (void)lba;
    (void)copy_from;
    // ignore writes
    return 0;
}

int ramdisk_blocks(void)
{
    return SECTOR_COUNT;
}
