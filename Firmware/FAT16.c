// FAT Emulation Layer

#include "string.h"
#include "stdlib.h"
#include "FAT16.h"
#include "main.h"
#include "cartio.h"
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
#define RAM_FILE_START_CLUSTER   (ROM_FILE_START_CLUSTER + (32*1024*1024 / BYTES_PER_CLUSTER)) // ROM file can be as large as 32MB, that's the largest possible value
#define USER_FILE_START_CLUSTER  (RAM_FILE_START_CLUSTER + (8*1024*1024 / BYTES_PER_CLUSTER))  // Allocate 8 MB for RAM file

// ROM start sector = 36 + (1024 - 2) * 4 = 4124 

uint8_t boot_sector[] = {
    0xEB, 0x3C, 0x90,                        // code to jump to the bootstrap code
    'M', 'S', 'D', 'O', 'S', '5', '.', '0',  // OEM ID
    WBVAL(BYTES_PER_SECTOR),                 // bytes per sector
    SECTORS_PER_CLUSTER,                     // sectors per cluster
    WBVAL(RESERVED_SECTORS),                 // # of reserved sectors 
    FAT_COPIES,                              // FAT copies (2)
    WBVAL(ROOT_ENTRIES),                     // root entries (512)
    WBVAL(0x00),                             // total number of sectors 16
    0xF8,                                    // media descriptor (0xF8 = Fixed disk)
    WBVAL(SECTOR_PER_FAT),                   // sectors per FAT
    WBVAL(SECTOR_PER_TRACK),                 // sectors per track 
    WBVAL(NUM_HEADS),                        // number of heads 
    0x00, 0x00, 0x00, 0x00,                  // hidden sectors (0)
    QBVAL(SECTOR_COUNT),                     // total number of sectors 32 
    0x80,                                    // drive number (0)
    0x00,                                    // reserved
    0x29,                                    // extended boot signature
    0xC0, 0xFF, 0x12, 0x45,                  // volume serial number
    'G', 'B', 'X', 'P', 'R', 'O', 'G', ' ', ' ', ' ', ' ',    // volume label
    'F', 'A', 'T', '1', '6', ' ', ' ', ' '            // filesystem type
};

const uint8_t dir_sector_label[DIR_LABEL_SIZE]= 
{
    'G','B','X','P','R','O','G',' ',' ',' ',' ',  // Volume label
    0x08,           // File attribute = Volume label 
    0x00,           // Reserved 
    0x00,           // Create Time Tenth 
    WBVAL(0x0000),  // Create Time 
    WBVAL(0x0000),  // Create Date 
    WBVAL(0x0000),  // Last Access Date 
    0x00, 0x00,     // Not used in FAT16 
    WBVAL(TIME(11, 49, 0)),  // Write Time 
    WBVAL(DATE(38, 7, 1)),  // Write Date 
    WBVAL(0x0000),  // Cluster Low
    QBVAL(0x0000),  // File Size
};

uint8_t dir_sector_file[DIR_FILE_SIZE]=
{
    'I','N','F','O',' ',' ',' ',' ','T','X','T',  // File name
    0x20,           // File attribute = Archive
    0x00,           // Reserved 
    0x00,           // Create Time Tenth 
    WBVAL(TIME(11, 49, 0)),  // Create Time 
    WBVAL(DATE(38, 7, 1)),  // Create Date 
    WBVAL(DATE(38, 7, 1)),   // Last Access Date 
    0x00, 0x00,     // Not used in FAT16 
    WBVAL(TIME(11, 49, 0)),  // Write Time 
    WBVAL(DATE(38, 7, 1)),  // Write Date 
    WBVAL(INFO_FILE_START_CLUSTER),  // Cluster Low
    QBVAL(0x0000),  // File Size
    
    'R','O','M',' ',' ',' ',' ',' ','G','B',' ',  // File name
    0x20,           // File attribute = Archive
    0x00,           // Reserved 
    0x00,           // Create Time Tenth 
    WBVAL(TIME(11, 49, 0)),  // Create Time 
    WBVAL(DATE(38, 7, 1)),  // Create Date 
    WBVAL(DATE(38, 7, 1)),   // Last Access Date 
    0x00, 0x00,     // Not used in FAT16 
    WBVAL(TIME(11, 49, 0)),  // Write Time 
    WBVAL(DATE(38, 7, 1)),  // Write Date 
    WBVAL(ROM_FILE_START_CLUSTER),  // Cluster Low
    QBVAL(32768),  // File Size
    
    'R','A','M',' ',' ',' ',' ',' ','S','A','V',  // File name
    0x20,           // File attribute = Archive
    0x00,           // Reserved 
    0x00,           // Create Time Tenth 
    WBVAL(TIME(11, 49, 0)),  // Create Time 
    WBVAL(DATE(38, 7, 1)),  // Create Date 
    WBVAL(DATE(38, 7, 1)),   // Last Access Date 
    0x00, 0x00,     // Not used in FAT16 
    WBVAL(TIME(11, 49, 0)),  // Write Time 
    WBVAL(DATE(38, 7, 1)),  // Write Date 
    WBVAL(RAM_FILE_START_CLUSTER),  // Cluster Low
    QBVAL(8192),  // File Size  
};

uint8_t InfoFileContent[FILE_INFO_MAX_SIZE];

void fat_set_filesize(uint32_t file_no, uint32_t file_size) {
    dir_sector_file[32*file_no + 28] = file_size & 0xff;
    dir_sector_file[32*file_no + 29] = (file_size >> 8) & 0xff;
    dir_sector_file[32*file_no + 30] = (file_size >> 16) & 0xff;
    dir_sector_file[32*file_no + 31] = (file_size >> 24) & 0xff;
}

uint32_t fat_get_filesize(uint32_t file_no) {
    uint32_t file_size;
    file_size  = dir_sector_file[32*file_no + 31] << 24;
    file_size |= dir_sector_file[32*file_no + 30] << 16;
    file_size |= dir_sector_file[32*file_no + 29] << 8;
    file_size |= dir_sector_file[32*file_no + 28];
    return file_size;
}

// Set file name without extension (8.3)
void fat_set_filename(uint32_t file_no, char *file_name) {
    memcpy(dir_sector_file + 32*file_no, file_name, (strlen(file_name) > 8) ? 8 : strlen(file_name));
}

// Set file extension (8.3)
void fat_set_fileext(uint32_t file_no, char *file_ext) {
    memcpy(dir_sector_file + 32*file_no + 8, file_ext, 3);
}

uint32_t fat_read_lba(uint32_t lba, uint8_t* data)
{
    uint32_t target;
    memset(data, 0, BYTES_PER_SECTOR);
    
    if (lba >= FILEDATA_START_SECTOR(USER_FILE_START_CLUSTER)) {
        target = lba - FILEDATA_START_SECTOR(USER_FILE_START_CLUSTER);
    }
    else if (lba >= FILEDATA_START_SECTOR(RAM_FILE_START_CLUSTER)) {
        target = lba - FILEDATA_START_SECTOR(RAM_FILE_START_CLUSTER);
    }
    else if (lba >= FILEDATA_START_SECTOR(ROM_FILE_START_CLUSTER)) {
        target = lba - FILEDATA_START_SECTOR(ROM_FILE_START_CLUSTER);
        cart_gb_rom_read_bulk(data, target * BYTES_PER_SECTOR, BYTES_PER_SECTOR);
    }
    else if (lba >= FILEDATA_START_SECTOR(INFO_FILE_START_CLUSTER)) {
        target = lba - FILEDATA_START_SECTOR(INFO_FILE_START_CLUSTER);
        if (target == 0)
          memcpy(data, InfoFileContent, sizeof(InfoFileContent));
    }
    else if (lba >= ROOT_ENTRY_SECTOR) {
        target = lba - ROOT_ENTRY_SECTOR;
        if (target == 0) {
            memcpy(data, dir_sector_label, DIR_LABEL_SIZE);
            memcpy(data + DIR_LABEL_SIZE, dir_sector_file, DIR_FILE_SIZE);
        }
    }
    else if (lba >= FAT_ENTRY_SECTOR(0)) {
        target = lba - FAT_ENTRY_SECTOR(0);
        target = target % SECTOR_PER_FAT;
        if (target == 0) {
            // First 2 clusters are fixed
            data[0] = 0xF8;
            data[1] = 0xFF;
            data[2] = 0xFF;
            data[3] = 0xFF;
            // Cluster 2 is the info file
            data[4] = 0xFF;
            data[5] = 0xFF;
            // Mark all others as unusable
            for (uint32_t i = 0; i < (CLUSTERS_IN_A_FAT_SECTOR - 3); i++) {
                data[6 + i*2 + 0] = 0xF7;
                data[6 + i*2 + 1] = 0xFF;
            }
        }
        else if (target < (USER_FILE_START_CLUSTER / CLUSTERS_IN_A_FAT_SECTOR)){
            // ROM or RAM file region
            
            uint32_t file_size;
            uint16_t start_cluster;
          
            if (target < (RAM_FILE_START_CLUSTER / CLUSTERS_IN_A_FAT_SECTOR)) {
                // this is a ROM file
                target -= (ROM_FILE_START_CLUSTER / CLUSTERS_IN_A_FAT_SECTOR);
                file_size = fat_get_filesize(FILE_NO_ROM);
                start_cluster = ROM_FILE_START_CLUSTER;
            }
            else {
                // this is a RAM file
                target -= (RAM_FILE_START_CLUSTER / CLUSTERS_IN_A_FAT_SECTOR);
                file_size = fat_get_filesize(FILE_NO_RAM);
                start_cluster = RAM_FILE_START_CLUSTER;
            }

            file_size /= BYTES_PER_CLUSTER; // 2KB cluster
            uint32_t ptr = 0;
            
            if ((target * CLUSTERS_IN_A_FAT_SECTOR) < file_size) {
                bool reach_end_of_file = FALSE;
                
                if ((file_size) > ((target + 1) * CLUSTERS_IN_A_FAT_SECTOR)) 
                    file_size = ((target + 1) * CLUSTERS_IN_A_FAT_SECTOR);
                else
                    reach_end_of_file = TRUE;
                
                for (uint32_t i = target * CLUSTERS_IN_A_FAT_SECTOR; i < file_size; i++) {
                    uint16_t next_cluster = start_cluster + i + 1;
                    data[ptr++] = next_cluster & 0xFF;
                    data[ptr++] = next_cluster >> 8;
                }
                
                if (reach_end_of_file) {
                    ptr -= 2;
                    data[ptr++] = 0xFF;
                    data[ptr++] = 0xFF;
                }
            }
            
            // See how many clusters need to be filled.
            for (uint32_t i = file_size; i < ((target + 1) * CLUSTERS_IN_A_FAT_SECTOR); i++) {
                data[ptr++] = 0xF7;
                data[ptr++] = 0xFF;
            }
        }
    }
    else if (lba == 0) {
        memcpy(data, boot_sector, sizeof(boot_sector));
        data[0x1FE] = 0x55;
        data[0x1FF] = 0xAA;
    }
    
    return BYTES_PER_SECTOR;
} /* EndBody */

uint32_t fat_write_lba(uint32_t lba, uint8_t* data)
{
    /*switch(FAT_LBA)
    {
       break; 
    }*/
    
    return BYTES_PER_SECTOR;
}
