#ifndef __FAT16_H__
#define __FAT16_H__

#include <stdint.h>

// Files in the root folder
#define FILE_NO_INFO    0
#define FILE_NO_ROM     1
#define FILE_NO_RAM     2
#define FILE_COUNT      3
           
#define DIR_LABEL_SIZE (32)
#define DIR_FILE_SIZE  (32 * FILE_COUNT)
#define FILE_INFO_MAX_SIZE 128

extern uint8_t InfoFileContent[];

void fat_set_filesize(uint32_t file_no, uint32_t file_size);
void fat_set_filename(uint32_t file_no, char *file_name);
void fat_set_fileext(uint32_t file_no, char *file_ext);

uint32_t fat_read_lba(uint32_t lba, uint8_t* data);
uint32_t fat_write_lba(uint32_t lba, uint8_t* data);

#endif // __FAT16_H
