#ifndef __FAT16_H__
#define __FAT16_H__

#include <stdint.h>

#define WBVAL(x) ((x) & 0xFF), (((x) >> 8) & 0xFF)
#define QBVAL(x) ((x) & 0xFF), (((x) >> 8) & 0xFF),\
         (((x) >> 16) & 0xFF), (((x) >> 24) & 0xFF)

#define DATE(y, m, d) (((y & 0x7F) << 9) | ((m & 0xF) << 5) | (d & 0x1F))
#define TIME(h, m, s) (((h & 0xF) << 11) | ((m & 0x3F) << 5) | ((s & 0x1F)))
          // Unit: 2s
           
// All the definitions are meant to help understand the code, but not to be modified
            
// Each cluster is 8KB
// 100MB total
// Each sector can hold 1024 FAT entries, which can manage 8MB of data
// 13 FATS can manage 104MB of data, with last 4MB unused
#define SECTOR_COUNT              51200
#define BYTES_PER_SECTOR          2048   // Do not change, need to be the same as the USB driver
#define SECTORS_PER_CLUSTER       4
#define BYTES_PER_CLUSTER         (BYTES_PER_SECTOR * SECTORS_PER_CLUSTER)
#define RESERVED_SECTORS          2
#define FAT_COPIES                2
#define CLUSTERS_IN_A_FAT_SECTOR  (BYTES_PER_SECTOR / 2) // 1024
#define SECTOR_PER_FAT            13
#define SECTOR_PER_TRACK          63
#define NUM_HEADS                 255
#define ROOT_ENTRIES              512
#define ROOT_ENTRY_LENGTH         32
#define FAT_ENTRY_SECTOR(x)   (RESERVED_SECTORS + x * SECTOR_PER_FAT)
#define ROOT_ENTRY_SECTOR     (RESERVED_SECTORS + FAT_COPIES * SECTOR_PER_FAT) // 2+ 2*13 = 28
#define DATA_REGION_SECTOR    (ROOT_ENTRY_SECTOR + \
            (ROOT_ENTRIES * ROOT_ENTRY_LENGTH) / BYTES_PER_SECTOR) // 28 + 512 * 32 / 2048 = 36
#define FILEDATA_START_SECTOR(x)   (DATA_REGION_SECTOR + \
            (x - 2) * SECTORS_PER_CLUSTER)

#define INFO_FILE_START_CLUSTER  2  // First avaliable cluster
#define ROM_FILE_START_CLUSTER   CLUSTERS_IN_A_FAT_SECTOR // Info file should only occupy 1 cluster, but things are better to be 8MB aligned
#define SLOT0_FILE_START_CLUSTER (ROM_FILE_START_CLUSTER + (1*1024*1024 / BYTES_PER_CLUSTER))
#define SLOT1_FILE_START_CLUSTER (ROM_FILE_START_CLUSTER + (2*1024*1024 / BYTES_PER_CLUSTER))
#define SLOT2_FILE_START_CLUSTER (ROM_FILE_START_CLUSTER + (4*1024*1024 / BYTES_PER_CLUSTER))
#define SLOT3_FILE_START_CLUSTER (ROM_FILE_START_CLUSTER + (6*1024*1024 / BYTES_PER_CLUSTER))
#define RAM_FILE_START_CLUSTER   (ROM_FILE_START_CLUSTER + (32*1024*1024 / BYTES_PER_CLUSTER)) // ROM file can be as large as 32MB, that's the largest possible value
#define SAVE0_FILE_START_CLUSTER (RAM_FILE_START_CLUSTER)
#define SAVE1_FILE_START_CLUSTER (RAM_FILE_START_CLUSTER + (1*32*1024 / BYTES_PER_CLUSTER))
#define SAVE2_FILE_START_CLUSTER (RAM_FILE_START_CLUSTER + (2*32*1024 / BYTES_PER_CLUSTER))
#define SAVE3_FILE_START_CLUSTER (RAM_FILE_START_CLUSTER + (3*32*1024 / BYTES_PER_CLUSTER))
#define USER_FILE_START_CLUSTER  (RAM_FILE_START_CLUSTER + (8*1024*1024 / BYTES_PER_CLUSTER))  // Allocate 8 MB for RAM file

// Files in the root folder
#define FILE_NO_INFO    0
#define FILE_NO_ROM     1
#define FILE_NO_RAM     2
#define FILE_NO_ROM_SLOT(x) (3+x)
#define FILE_NO_RAM_SLOT(x) ((x == 0) ? (2) : (7+x))
#define FILE_COUNT      10  // 3 default + 4 multicart + 3 ram muliticart
#define WRITABLE_COUNT  8   // Reserve more for LFN entries writen by the OS
           
#define DIR_LABEL_SIZE (32)
#define DIR_FILE_SIZE  (32 * FILE_COUNT)
#define DIR_FILE_ATTR  ((8+3+6) * FILE_COUNT)
#define DIR_WRITABLE_SIZE (32 * WRITABLE_COUNT)
#define FILE_INFO_MAX_SIZE 256

extern uint8_t info_file_content[];

void fat_init();

void fat_set_filesize(uint32_t file_no, uint32_t file_size);
void fat_set_filename(uint32_t file_no, char *file_name);
void fat_set_fileext(uint32_t file_no, char *file_ext);

uint32_t fat_read_lba(uint32_t lba, uint8_t* data);
uint32_t fat_write_lba(uint32_t lba, uint8_t* data);

#endif // __FAT16_H
