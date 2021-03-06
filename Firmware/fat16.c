// FAT Emulation Layer

#include "string.h"
#include "stdlib.h"
#include "fat16.h"
#include "cartio.h"

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

uint8_t dir_template[32 - 8 - 3 - 6] =
{
    0x20,           // File attribute = Archive
    0x00,           // Reserved 
    0x00,           // Create Time Tenth 
    WBVAL(TIME(11, 49, 0)),  // Create Time 
    WBVAL(DATE(38, 7, 1)),  // Create Date 
    WBVAL(DATE(38, 7, 1)),   // Last Access Date 
    0x00, 0x00,     // Not used in FAT16 
    WBVAL(TIME(11, 49, 0)),  // Write Time 
    WBVAL(DATE(38, 7, 1)),  // Write Date 
};

uint8_t dir_sector_file[DIR_FILE_ATTR]=
{
    'I','N','F','O',' ',' ',' ',' ','T','X','T',  // File name
    WBVAL(INFO_FILE_START_CLUSTER),  // Cluster Low
    QBVAL(0x0000),  // File Size
    
    'R','O','M',' ',' ',' ',' ',' ','G','B',' ',  // File name
    WBVAL(ROM_FILE_START_CLUSTER),  // Cluster Low
    QBVAL(32768),  // File Size
    
    'R','A','M',' ',' ',' ',' ',' ','S','A','V',  // File name
    WBVAL(RAM_FILE_START_CLUSTER),  // Cluster Low
    QBVAL(8192),  // File Size  
    
    'S','L','O','T','1',' ',' ',' ','G','B',' ',  // File name
    WBVAL(SLOT0_FILE_START_CLUSTER),  // Cluster Low
    QBVAL(32768),  // File Size
    
    'S','L','O','T','2',' ',' ',' ','G','B',' ',  // File name
    WBVAL(SLOT1_FILE_START_CLUSTER),  // Cluster Low
    QBVAL(32768),  // File Size

    'S','L','O','T','3',' ',' ',' ','G','B',' ',  // File name
    WBVAL(SLOT2_FILE_START_CLUSTER),  // Cluster Low
    QBVAL(32768),  // File Size
    
    'S','L','O','T','4',' ',' ',' ','G','B',' ',  // File name
    WBVAL(SLOT3_FILE_START_CLUSTER),  // Cluster Low
    QBVAL(32768),  // File Size
    
    'S','L','O','T','2',' ',' ',' ','S','A','V',  // File name
    WBVAL(SAVE1_FILE_START_CLUSTER),  // Cluster Low
    QBVAL(32768),  // File Size

    'S','L','O','T','3',' ',' ',' ','S','A','V',  // File name
    WBVAL(SAVE2_FILE_START_CLUSTER),  // Cluster Low
    QBVAL(32768),  // File Size
    
    'S','L','O','T','4',' ',' ',' ','S','A','V',  // File name
    WBVAL(SAVE3_FILE_START_CLUSTER),  // Cluster Low
    QBVAL(32768),  // File Size
};

uint8_t dir_sector_free[DIR_WRITABLE_SIZE];

const char info_file_fixed[] = "** GBxProgrammer V0.9.1 **\r\nPlease do not overwrite any file!\r\n�벻Ҫ�����ļ���\r\nSee gbxprog.zephray.me for information about MultiCart support.\r\n\r\n";

bool writing_rom = TRUE;
bool writing_valid = FALSE;
uint32_t writing_target;
uint16_t writing_start_cluster;
extern bool flash_empty;
extern bool cfi_valid;
extern bool game_valid[4];
extern bool flash_valid;
extern uint32_t rom_size[4];
extern uint32_t ram_size[4];
extern bool multicart_mode;
bool writing_loader;
uint32_t dir_file_size;
extern const uint32_t addr_lut[4];
volatile bool writing_done;
uint32_t writing_size;

uint8_t info_file_content[FILE_INFO_MAX_SIZE];

void fat_init() {
    memset(dir_sector_free, DIR_WRITABLE_SIZE, 0x00);
    writing_valid = FALSE;
    writing_done = FALSE;
}

void fat_set_filesize(uint32_t file_no, uint32_t file_size) {
    if (file_no == FILE_NO_INFO)
        file_size += (sizeof(info_file_fixed) - 1);
    dir_sector_file[17*file_no + 13] = file_size & 0xff;
    dir_sector_file[17*file_no + 14] = (file_size >> 8) & 0xff;
    dir_sector_file[17*file_no + 15] = (file_size >> 16) & 0xff;
    dir_sector_file[17*file_no + 16] = (file_size >> 24) & 0xff;
}

uint32_t fat_get_filesize(uint32_t file_no) {
    uint32_t file_size;
    file_size  = dir_sector_file[17*file_no + 13];
    file_size |= dir_sector_file[17*file_no + 14] << 8;
    file_size |= dir_sector_file[17*file_no + 15] << 16;
    file_size |= dir_sector_file[17*file_no + 16] << 24;
    return file_size;
}

// Set file name without extension (8.3)
void fat_set_filename(uint32_t file_no, char *file_name) {
    memcpy(dir_sector_file + 17*file_no, file_name, (strlen(file_name) > 8) ? 8 : strlen(file_name));
}

// Set file extension (8.3)
void fat_set_fileext(uint32_t file_no, char *file_ext) {
    memcpy(dir_sector_file + 17*file_no + 8, file_ext, 3);
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
        cart_gb_ram_read_bulk(data, target * BYTES_PER_SECTOR, BYTES_PER_SECTOR);
    }
    else if (lba >= FILEDATA_START_SECTOR(ROM_FILE_START_CLUSTER)) {
        target = lba - FILEDATA_START_SECTOR(ROM_FILE_START_CLUSTER);
        cart_gb_rom_read_bulk(data, target * BYTES_PER_SECTOR, BYTES_PER_SECTOR);
    }
    else if (lba >= FILEDATA_START_SECTOR(INFO_FILE_START_CLUSTER)) {
        target = lba - FILEDATA_START_SECTOR(INFO_FILE_START_CLUSTER);
        if (target == 0) {
            memcpy(data, info_file_fixed, sizeof(info_file_fixed) - 1);
            memcpy(data + sizeof(info_file_fixed) - 1, info_file_content, sizeof(info_file_content));
        }
    }
    else if (lba >= ROOT_ENTRY_SECTOR) {
        target = lba - ROOT_ENTRY_SECTOR;
        if (target == 0) {
            memcpy(data, dir_sector_label, DIR_LABEL_SIZE);
            //memcpy(data + DIR_LABEL_SIZE, dir_sector_file, DIR_FILE_SIZE);
            uint32_t file_count;
            file_count = (multicart_mode) ? (10) : (3);
            for (uint32_t i = 0; i < FILE_COUNT; i++) {
                memcpy(data + DIR_LABEL_SIZE + i*32, dir_sector_file + i*17, 11); // file name
                memcpy(data + DIR_LABEL_SIZE + i*32 + 11,  dir_template, 15); // file info
                memcpy(data + DIR_LABEL_SIZE + i*32 + 26,  dir_sector_file + i*17 + 11, 6); // file size
            }
            dir_file_size = file_count * 32;
            memcpy(data + DIR_LABEL_SIZE + dir_file_size, dir_sector_free, DIR_WRITABLE_SIZE);
            
            if (multicart_mode) {
                for (uint32_t i = 0; i < 4; i++) {
                    if (!game_valid[i]) {
                        data[DIR_LABEL_SIZE + FILE_NO_ROM_SLOT(i)*32] = 0xE5;
                    }
                }
            }
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
            bool is_ram;
          
            if (target < (RAM_FILE_START_CLUSTER / CLUSTERS_IN_A_FAT_SECTOR)) {
                // this is a ROM file
                target -= (ROM_FILE_START_CLUSTER / CLUSTERS_IN_A_FAT_SECTOR);
                file_size = fat_get_filesize(FILE_NO_ROM);
                if (multicart_mode) file_size = 8*1024*1024;
                start_cluster = ROM_FILE_START_CLUSTER;
                is_ram = FALSE;
            }
            else {
                // this is a RAM file
                target -= (RAM_FILE_START_CLUSTER / CLUSTERS_IN_A_FAT_SECTOR);
                file_size = fat_get_filesize(FILE_NO_RAM);
                if (multicart_mode) file_size = 128*1024;
                start_cluster = RAM_FILE_START_CLUSTER;
                is_ram = TRUE;
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
            
            if (multicart_mode) {
                // Ugly fix for multicart mode
                if (is_ram) {
                    data[6] = 0xFF;  data[7] = 0xFF;
                    data[14] = 0xFF; data[15] = 0xFF;
                    data[22] = 0xFF; data[23] = 0xFF;
                    data[30] = 0xFF; data[31] = 0xFF;
                }
                else {
                    data[254] = 0xFF;  data[255] = 0xFF;
                    data[510] = 0xFF;  data[511] = 0xFF;
                    data[1022] = 0xFF; data[1023] = 0xFF;
                    data[2046] = 0xFF; data[2047] = 0xFF;
                }
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
    uint32_t target;
    
    if (lba >= FILEDATA_START_SECTOR(USER_FILE_START_CLUSTER)) {
        //target = lba - FILEDATA_START_SECTOR(USER_FILE_START_CLUSTER);
        if ((writing_valid)&&(lba >= FILEDATA_START_SECTOR(writing_start_cluster))) {
            target = lba - FILEDATA_START_SECTOR(writing_start_cluster);
            uint32_t offset;
            if ((multicart_mode)&&(!writing_loader)) {
                offset = (writing_rom) ? (addr_lut[writing_target]) : (32 * 1024 * writing_target);
            }
            else {
                offset = 0;
            }
            if (writing_rom)
                cart_gb_rom_program_bulk(data, offset + target * BYTES_PER_SECTOR, BYTES_PER_SECTOR, ((!flash_empty)||(multicart_mode)));
            else
                cart_gb_ram_write_bulk(data, offset + target * BYTES_PER_SECTOR, BYTES_PER_SECTOR);
            if (((target+1)*BYTES_PER_SECTOR) >= writing_size) {
                //finished, but not reset yet.
                writing_done = TRUE;
            }
        }
    }
    else if (lba >= FILEDATA_START_SECTOR(INFO_FILE_START_CLUSTER)) {
        ;
        // Good bye
    }
    else if (lba >= ROOT_ENTRY_SECTOR) {
        target = lba - ROOT_ENTRY_SECTOR;
        if (target == 0) {
            // Old files might get deleted
            uint32_t file_count;
            file_count = (multicart_mode) ? (10) : (3);
            for (uint32_t i = 0; i < FILE_COUNT; i++) {
                memcpy(dir_sector_file + i*17, data + DIR_LABEL_SIZE + i*32, 8+3); // file name
                memcpy(dir_sector_file + i*17 + 11, data + DIR_LABEL_SIZE + i*32 + 28, 4); // file size
            }
            dir_file_size = file_count * 32;
            if ((*(dir_sector_file + 1 * 17) == 0xE5)&&((!flash_empty)||(multicart_mode))) {
                // ROM file or loader file (multicart mode) get deleted, issue a full cart erase.
                cart_erase_flash();
                flash_empty = TRUE;
                sys_reset();
            }
            if ((*(dir_sector_file + 2 * 17) == 0xE5)&&(ram_size[0] != 0)) {
                // RAM��file get deleted, fill the RAM with 0xFF
                uint8_t *empty;
                empty = malloc(1024);
                memset(empty, 1024, 0xFF);
                for (uint32_t i = 0; i < (ram_size[0] / 1024); i++) {
                    cart_gb_ram_write_bulk(empty, i * 1024, 1024);
                }
                free(empty);
                sys_reset();
            }
            if ((multicart_mode)) {
                for (uint32_t i = 0; i < 4; i++) {
                    if ((*(dir_sector_file + FILE_NO_ROM_SLOT(i) * 17) == 0xE5)&&(game_valid[i])) {
                        cart_erase_sector(addr_lut[i]); //only erase the first sector
                        game_valid[i] = FALSE;
                        sys_reset();
                    }
                    if (*(dir_sector_file + FILE_NO_RAM_SLOT(i) * 17) == 0xE5) {
                        uint8_t *empty;
                        empty = malloc(1024);
                        memset(empty, 1024, 0xFF);
                        for (uint32_t j = 0; j < (ram_size[0] / 1024); j++) {
                            cart_gb_ram_write_bulk(empty, 32 * 1024 * i + 1024 * j, 1024);
                        }
                        free(empty);
                        sys_reset();
                    }
                }
                    
            }
            
            // Process new files
            memcpy(dir_sector_free, data + DIR_LABEL_SIZE + dir_file_size, DIR_WRITABLE_SIZE);
            // Scan the written data
            writing_valid = FALSE;
            for (uint32_t i = 0; i < WRITABLE_COUNT; i++) {
                if (dir_sector_free[i * 32 + 0x0B] != 0x0F) {
                    // this is not a LFN entry
                    if (memcmp(dir_sector_free + i * 32 + 8, "GB", 2) == 0) {
                        if (multicart_mode) {
                            if (memcmp(dir_sector_free + i * 32, "MULTIC", 6) == 0) {
                                // updating the loader
                                writing_valid = TRUE;
                                writing_loader = TRUE;
                            }
                            else {
                                writing_valid = FALSE;
                                writing_loader = FALSE;
                                for (uint32_t i = 0; i < 4; i++) {
                                    if (game_valid[i] == FALSE) {
                                        writing_target = i;
                                        writing_valid = TRUE;
                                        break;
                                    }
                                }
                            }
                        }
                        else
                            writing_valid = TRUE;
                        writing_done = FALSE;
                        writing_rom = TRUE;
                        writing_start_cluster = (uint16_t)dir_sector_free[i * 32 + 27] << 8;
                        writing_start_cluster |=  dir_sector_free[i * 32 + 26];
                        if ((!cfi_valid)&&(!flash_valid)&&(!game_valid[0])) {
                            // This means there was probably no cartridge when powered on, try to re-detect
                            cart_probe_cart();
                        }
                        if ((!cfi_valid)&&(!flash_empty)) {
                            cart_erase_flash();
                            flash_empty = TRUE;
                        }
                        writing_size  = (uint32_t)dir_sector_free[i * 32 + 28];
                        writing_size |= (uint32_t)dir_sector_free[i * 32 + 29] << 8;
                        writing_size |= (uint32_t)dir_sector_free[i * 32 + 30] << 16;
                        writing_size |= (uint32_t)dir_sector_free[i * 32 + 31] << 24;
                    }
                    else if (memcmp(dir_sector_free + i * 32 + 8, "SAV", 3) == 0) {
                        if (multicart_mode) {
                            writing_valid = FALSE;
                            for (uint32_t i = 0; i < 4; i++) {
                                if (*(dir_sector_free + i * 32) == ('1' + i)) {
                                    writing_target = i;
                                    writing_valid = TRUE;
                                    break;
                                }
                            }
                        }
                        else
                            writing_valid = TRUE;
                        writing_done = FALSE;
                        writing_rom = FALSE;
                        writing_start_cluster = (uint16_t)dir_sector_free[i * 32 + 27] << 8;
                        writing_start_cluster |=  dir_sector_free[i * 32 + 26];
                        writing_size  = (uint32_t)dir_sector_free[i * 32 + 28];
                        writing_size |= (uint32_t)dir_sector_free[i * 32 + 29] << 8;
                        writing_size |= (uint32_t)dir_sector_free[i * 32 + 30] << 16;
                        writing_size |= (uint32_t)dir_sector_free[i * 32 + 31] << 24;
                    }
                }
            }
        }
    }
    else if (lba >= FAT_ENTRY_SECTOR(0)) {
        ;
        // I ~ ~ g ~ o ~ t ~ ~ y ~ o ~ u ~ !
    }
    else if (lba == 0) {
        ;
        // WHY??
    }
    
    return BYTES_PER_SECTOR;
}
