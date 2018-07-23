/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "delay.h"
#include "key.h"
#include "hw_config.h"
#include "usb_lib.h"
#include "usb_desc.h"
#include "platform_config.h"
#include "usb_pwr.h"
#include "FAT16.h"
#include "cartio.h"

/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

char game_title[4][17];
uint32_t rom_size[4];
uint32_t ram_size[4];
const uint32_t ram_size_lut[6] = {0, 2048, 8192, 32768, 131072, 65536};
bool game_valid[4];
bool flash_valid;
bool cfi_valid;
bool cgb_game[4];
bool has_ram;
mbc_t mbc_type; 
const char *manuf_string;
const char *model_string;
bool flash_empty;
bool multicart_mode;
char manuf_id_string[5];
char dev_id_string[5];
const uint32_t addr_lut[4] = {1*1048576, 2*1048576, 4*1048576, 6*1048576};
    
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */

bool check_flash_id(uint8_t manuf_id, uint8_t dev_id) {
    bool valid = FALSE;
    manuf_string = manuf_id_string;
    model_string = dev_id_string;
    sprintf(manuf_id_string, "0x%2X", manuf_id);
    sprintf(dev_id_string, "0x%2X", dev_id);
    if (manuf_id == 0x52) {
        valid = TRUE;
        manuf_string = "Alliance";
        if      (dev_id == 0x34) { model_string = "AS29F002B"; }
        else if (dev_id == 0xB0) { model_string = "AS29F002T"; }
        else if (dev_id == 0x04) { model_string = "AS29F010"; }
        else if (dev_id == 0xA4) { model_string = "AS29F040"; }
        else if (dev_id == 0x57) { model_string = "AS29F200B"; }
        else if (dev_id == 0x51) { model_string = "AS29F200T"; }
        else if (dev_id == 0x49) { model_string = "AS29LV160B"; }
        else if (dev_id == 0xCA) { model_string = "AS29LV160T"; }
        else if (dev_id == 0xBA) { model_string = "AS29LV400B"; }
        else if (dev_id == 0xB9) { model_string = "AS29LV400T"; }
        else if (dev_id == 0x5B) { model_string = "AS29LV800B"; }
        else if (dev_id == 0xDA) { model_string = "AS29LV800T"; }
        else { valid = FALSE; }
    }
    else if (manuf_id == 0x01) {
        valid = TRUE;
        manuf_string = "AMD";
        if      (dev_id == 0x0C) { model_string = "AM29DL400BT"; }
        else if (dev_id == 0x0F) { model_string = "AM29DL400BB"; }
        else if (dev_id == 0x4A) { model_string = "AM29DL800BT"; }
        else if (dev_id == 0xCB) { model_string = "AM29DL800BB"; }
        else if (dev_id == 0x34) { model_string = "AM29F002BB"; }
        else if (dev_id == 0xB0) { model_string = "AM29F002BT"; }
        else if (dev_id == 0x7B) { model_string = "AM29F004BB"; }
        else if (dev_id == 0x77) { model_string = "AM29F004BT"; }
        else if (dev_id == 0xAD) { model_string = "AM29F016D"; }
        else if (dev_id == 0x20) { model_string = "AM29F010"; }
        else if (dev_id == 0xA4) { model_string = "AM29F040"; }
        else if (dev_id == 0xD5) { model_string = "AM29F080"; }
        else if (dev_id == 0x57) { model_string = "AM29F200BB"; }
        else if (dev_id == 0x51) { model_string = "AM29F200BT"; }
        else if (dev_id == 0xAB) { model_string = "AM29F400BB"; }
        else if (dev_id == 0x23) { model_string = "AM29F400BT"; }
        else if (dev_id == 0x58) { model_string = "AM29F800BB"; }
        else if (dev_id == 0xD6) { model_string = "AM29F800BT"; }
        else if (dev_id == 0x6D) { model_string = "AM29LV001BB"; }
        else if (dev_id == 0xED) { model_string = "AM29LV001BT"; }
        else if (dev_id == 0x6E) { model_string = "AM29LV010B"; }
        else if (dev_id == 0xC2) { model_string = "AM29LV002BB"; }
        else if (dev_id == 0x40) { model_string = "AM29LV002BT"; }
        else if (dev_id == 0xB6) { model_string = "AM29LV004BB"; }
        else if (dev_id == 0xB5) { model_string = "AM29LV004BT"; }
        else if (dev_id == 0x37) { model_string = "AM29LV008BB"; }
        else if (dev_id == 0x3E) { model_string = "AM29LV008BT"; }
        else if (dev_id == 0x4F) { model_string = "AM29LV040B"; }
        else if (dev_id == 0x38) { model_string = "AM29LV080B"; }
        else if (dev_id == 0xBF) { model_string = "AM29LV200BB"; }
        else if (dev_id == 0x3B) { model_string = "AM29LV200BT"; }
        else if (dev_id == 0x5B) { model_string = "AM29LV800BB"; }
        else if (dev_id == 0xB9) { model_string = "AM29LV400BT"; }
        else if (dev_id == 0xBA) { model_string = "AM29LV400BB"; }
        else if (dev_id == 0xDA) { model_string = "AM29LV800BT"; }
        else { valid = FALSE; }
    }
    else if (manuf_id == 0x37) {
        valid = TRUE;
        manuf_string = "AMIC";
        if      (dev_id == 0x0D) { model_string = "A29002B"; }
        else if (dev_id == 0x8C) { model_string = "A29002T"; }
        else if (dev_id == 0x86) { model_string = "A29040B"; }
        else if (dev_id == 0xB0) { model_string = "A29400T"; }
        else if (dev_id == 0x31) { model_string = "A29400U"; }
        else if (dev_id == 0x0E) { model_string = "A29800T"; }
        else if (dev_id == 0x8F) { model_string = "A29800U"; }
        else if (dev_id == 0x34) { model_string = "A29L004T"; }
        else if (dev_id == 0xB5) { model_string = "A29L004U"; }
        else if (dev_id == 0x1A) { model_string = "A29L008T"; }
        else if (dev_id == 0x9B) { model_string = "A29L008U"; }
        else if (dev_id == 0x92) { model_string = "A29L040"; }
        else if (dev_id == 0x9D) { model_string = "A29LF040A"; }
        else { valid = FALSE; }
    }
    else if (manuf_id == 0x1F) {
        valid = TRUE;
        manuf_string = "ATMEL";
        if      (dev_id == 0x3D) { model_string = "AT29LV512"; }
        else if (dev_id == 0x35) { model_string = "AT29LV010A"; }
        else if (dev_id == 0xBA) { model_string = "AT29LV020"; }
        else if (dev_id == 0xC4) { model_string = "AT29BV040A"; }
        else if (dev_id == 0xA4) { model_string = "AT29C040A"; }
        else if (dev_id == 0xD5) { model_string = "AT29C010A"; }
        else if (dev_id == 0xDA) { model_string = "AT29C020"; }
        else if (dev_id == 0x5D) { model_string = "AT29C512"; }
        else if (dev_id == 0x03) { model_string = "AT49BV512"; }
        else if (dev_id == 0x05) { model_string = "AT49F001N"; }
        else if (dev_id == 0x04) { model_string = "AT49F001NT"; }
        else if (dev_id == 0x0B) { model_string = "AT49F020"; }
        else if (dev_id == 0x07) { model_string = "AT49F002N"; }
        else if (dev_id == 0x08) { model_string = "AT49F002NT"; }
        else if (dev_id == 0xE9) { model_string = "AT49LH002"; }
        else if (dev_id == 0xED) { model_string = "AT49LH00B4"; }
        else if (dev_id == 0xEE) { model_string = "AT49LH004"; }
        else if (dev_id == 0x08) { model_string = "AT49F002NT"; }
        else if (dev_id == 0x17) { model_string = "AT49F010"; }
        else if (dev_id == 0x0B) { model_string = "AT49F020"; }
        else if (dev_id == 0x13) { model_string = "AT49F040"; }
        else if (dev_id == 0x23) { model_string = "AT49F080"; }
        else if (dev_id == 0x27) { model_string = "AT49F080T"; }
        else { valid = FALSE; }
    }
    else if (manuf_id == 0x1C) {
        valid = TRUE;
        manuf_string = "EON";
        if      (dev_id == 0x20) { model_string = "EN29F010"; }
        else if (dev_id == 0x04) { model_string = "EN29F040"; }
        else if (dev_id == 0x6E) { model_string = "EN29LV010"; }
        else if (dev_id == 0x4F) { model_string = "EN29LV040"; }
        else if (dev_id == 0xCB) { model_string = "EN29LV640B"; }
        else if (dev_id == 0xC9) { model_string = "EN29LV640T"; }
        else if (dev_id == 0x7E) { model_string = "EN29LV640U"; }
        else if (dev_id == 0x92) { model_string = "EN29F002T"; }
        else if (dev_id == 0x97) { model_string = "EN29F002B"; }
        else { valid = FALSE; }
    }
    else if (manuf_id == 0x04) {
        valid = TRUE;
        manuf_string = "Fujitsu";
        if      (dev_id == 0x0F) { model_string = "MBM29DL400BC"; }  
        else if (dev_id == 0x0C) { model_string = "MBM29DL400TC"; }  
        else if (dev_id == 0xCB) { model_string = "MBM29DL800BA"; }  
        else if (dev_id == 0x4A) { model_string = "MBM29DL800TA"; }  
        else if (dev_id == 0x34) { model_string = "MBM29F002BC"; }  
        else if (dev_id == 0xB0) { model_string = "MBM29F002TC"; }  
        else if (dev_id == 0x7B) { model_string = "MBM29F004BC"; }  
        else if (dev_id == 0x77) { model_string = "MBM29F004TC"; }  
        else if (dev_id == 0xA4) { model_string = "MBM29F040C"; }  
        else if (dev_id == 0xD5) { model_string = "MBM29F080A"; }  
        else if (dev_id == 0x57) { model_string = "MBM29F200BC"; }  
        else if (dev_id == 0x51) { model_string = "MBM29F200TC"; }  
        else if (dev_id == 0xAB) { model_string = "MBM29F400BC"; }  
        else if (dev_id == 0x23) { model_string = "MBM29F400TC"; }  
        else if (dev_id == 0x58) { model_string = "MBM29F800BA"; }  
        else if (dev_id == 0xD6) { model_string = "MBM29F800TA"; }  
        else if (dev_id == 0xC2) { model_string = "MBM29LV002BC"; }  
        else if (dev_id == 0x40) { model_string = "MBM29LV002TC"; }  
        else if (dev_id == 0xB6) { model_string = "MBM29LV004BC"; }  
        else if (dev_id == 0xB5) { model_string = "MBM29LV004TC"; }  
        else if (dev_id == 0x37) { model_string = "MBM29LV008BA"; }  
        else if (dev_id == 0x3E) { model_string = "MBM29LV008TA"; }  
        else if (dev_id == 0x38) { model_string = "MBM29LV080A"; }  
        else if (dev_id == 0xBF) { model_string = "MBM29LV200BC"; }  
        else if (dev_id == 0x3B) { model_string = "MBM29LV200TC"; }  
        else if (dev_id == 0xBA) { model_string = "MBM29LV400BC"; }  
        else if (dev_id == 0xB9) { model_string = "MBM29LV400TC"; }  
        else if (dev_id == 0x5B) { model_string = "MBM29LV800BA"; }  
        else if (dev_id == 0xDA) { model_string = "MBM29LV800TA"; }  
        else if (dev_id == 0x49) { model_string = "MBM29LV160BE"; }  
        else if (dev_id == 0x0C) { model_string = "MBM29LV160TE"; }  
        else { valid = FALSE; }
    }
    else if (manuf_id == 0xAD) {
        valid = TRUE;
        manuf_string = "Hyundai";
        if      (dev_id == 0x23) { model_string = "HY29F400T"; }    
        else if (dev_id == 0x58) { model_string = "HY29F800B"; }    
        else if (dev_id == 0x5B) { model_string = "HY29LV800B"; }    
        else if (dev_id == 0xA4) { model_string = "HY29F040A"; }    
        else if (dev_id == 0xAB) { model_string = "HY29F400B"; }    
        else if (dev_id == 0x34) { model_string = "HY29F002B"; }    
        else if (dev_id == 0xB0) { model_string = "HY29F002T"; }    
        else if (dev_id == 0xB9) { model_string = "HY29LV400T"; }    
        else if (dev_id == 0xBA) { model_string = "HY29LV400B"; }    
        else if (dev_id == 0xD5) { model_string = "HY29F080"; }    
        else if (dev_id == 0xD6) { model_string = "HY29F800T"; }    
        else if (dev_id == 0xDA) { model_string = "HY29LV800T"; }   
        else { valid = FALSE; }
    }
    else if (manuf_id == 0x89) {
        valid = TRUE;
        manuf_string = "Intel";
        if      (dev_id == 0x14) { model_string = "i28F320J5"; }   
        else if (dev_id == 0x15) { model_string = "i28F640J5"; }   
        else if (dev_id == 0x16) { model_string = "i28F320J3"; }   
        else if (dev_id == 0x17) { model_string = "i28F640J3"; }   
        else if (dev_id == 0x18) { model_string = "i28F128J3"; }   
        else if (dev_id == 0x1D) { model_string = "i28F256J3"; }   
        else if (dev_id == 0x70) { model_string = "i28F400T"; }     
        else if (dev_id == 0x71) { model_string = "i28F400B"; }     
        else if (dev_id == 0x74) { model_string = "i28F200T"; }     
        else if (dev_id == 0x75) { model_string = "i28F200B"; }     
        else if (dev_id == 0x78) { model_string = "i28F004T"; }     
        else if (dev_id == 0x79) { model_string = "i28F004B"; }     
        else if (dev_id == 0x7C) { model_string = "i28F002T"; }     
        else if (dev_id == 0x7D) { model_string = "i28F002B"; }     
        else if (dev_id == 0x94) { model_string = "i28F001T"; }     
        else if (dev_id == 0x95) { model_string = "i28F001B"; }     
        else if (dev_id == 0x98) { model_string = "i28F008T"; }     
        else if (dev_id == 0x99) { model_string = "i28F008B"; }     
        else if (dev_id == 0x9C) { model_string = "i28F800T"; }     
        else if (dev_id == 0x9D) { model_string = "i28F800B"; }     
        else if (dev_id == 0xA0) { model_string = "i28F016SV"; }    
        else if (dev_id == 0xA2) { model_string = "i28F008SA"; }   
        else if (dev_id == 0xA6) { model_string = "i28F008S3"; }    
        else if (dev_id == 0xA7) { model_string = "i28F004S3"; }    
        else if (dev_id == 0xA8) { model_string = "i28F016XS"; }   
        else if (dev_id == 0xAA) { model_string = "i28F016S3"; }    
        else if (dev_id == 0xAC) { model_string = "i82802AC"; }   
        else if (dev_id == 0xAD) { model_string = "i82802AB"; }   
        else if (dev_id == 0xB4) { model_string = "i28F010"; }   
        else if (dev_id == 0xB8) { model_string = "i28F512"; }   
        else if (dev_id == 0xB9) { model_string = "i28F256A"; }   
        else if (dev_id == 0xBD) { model_string = "i28F020"; }   
        else if (dev_id == 0xD0) { model_string = "i28F016B3T"; } 
        else if (dev_id == 0xD1) { model_string = "i28F016B3B"; }  
        else if (dev_id == 0xD2) { model_string = "i28F008B3T"; }  
        else if (dev_id == 0xD3) { model_string = "i28F008B3B"; }  
        else if (dev_id == 0xD4) { model_string = "i28F004B3T"; }  
        else if (dev_id == 0xD5) { model_string = "i28F004B3B"; }  
        else { valid = FALSE; }
    }
    else if (manuf_id == 0xC2) {
        valid = TRUE;
        manuf_string = "Macronix";
        if      (dev_id == 0x19) { model_string = "MX29F001B"; }  
        else if (dev_id == 0x18) { model_string = "MX29F001T"; }  
        else if (dev_id == 0x34) { model_string = "MX29F002B"; }   
        else if (dev_id == 0xB0) { model_string = "MX29F002T"; }   
        else if (dev_id == 0x46) { model_string = "MX29F004B"; }  
        else if (dev_id == 0x45) { model_string = "MX29F004T"; }  
        else if (dev_id == 0x37) { model_string = "MX29F022B"; }   
        else if (dev_id == 0x36) { model_string = "MX29F022T"; }   
        else if (dev_id == 0xA4) { model_string = "MX29F040"; }   
        else if (dev_id == 0xD5) { model_string = "MX29F080"; }  
        else if (dev_id == 0x57) { model_string = "MX29F200B"; }   
        else if (dev_id == 0x51) { model_string = "MX29F200T"; }   
        else if (dev_id == 0xAB) { model_string = "MX29F400B"; }   
        else if (dev_id == 0x23) { model_string = "MX29F400T"; }   
        else if (dev_id == 0x58) { model_string = "MX29F800B"; }  
        else if (dev_id == 0xD6) { model_string = "MX29F800T"; }  
        else if (dev_id == 0x7E) { model_string = "MX29GL Series"; }
        else if (dev_id == 0x5A) { model_string = "MX29LV002CB"; }  
        else if (dev_id == 0x59) { model_string = "MX29LV002CT"; }  
        else if (dev_id == 0xB6) { model_string = "MX29LV004B"; }   
        else if (dev_id == 0xB5) { model_string = "MX29LV004T"; }   
        else if (dev_id == 0x37) { model_string = "MX29LV008B"; }   
        else if (dev_id == 0x3E) { model_string = "MX29LV008T"; }   
        else if (dev_id == 0x4F) { model_string = "MX29LV040"; }   
        else if (dev_id == 0x38) { model_string = "MX29LV081"; }  
        else if (dev_id == 0x7A) { model_string = "MX29LV128DB"; }  
        else if (dev_id == 0x49) { model_string = "MX29LV160DB"; }   
        else if (dev_id == 0xC4) { model_string = "MX29LV160DT"; }   
        else if (dev_id == 0xA8) { model_string = "MX29LV320DB"; }   
        else if (dev_id == 0xA7) { model_string = "MX29LV320DT"; }   
        else if (dev_id == 0xBA) { model_string = "MX29LV400B"; }   
        else if (dev_id == 0xB9) { model_string = "MX29LV400T"; }   
        else if (dev_id == 0xCB) { model_string = "MX29LV640DB"; }   
        else if (dev_id == 0xC9) { model_string = "MX29LV640DT"; }   
        else if (dev_id == 0x5B) { model_string = "MX29LV800B"; }   
        else if (dev_id == 0xDA) { model_string = "MX29LV800T"; }   
        else if (dev_id == 0xF1) { model_string = "MX29SL402CB"; }  
        else if (dev_id == 0x70) { model_string = "MX29SL402CT"; }  
        else if (dev_id == 0x6B) { model_string = "MX29SL800CB"; }   
        else if (dev_id == 0xEA) { model_string = "MX29SL800CT"; }   
        else { valid = FALSE; }
    }
    else if (manuf_id == 0x01) {
        valid = TRUE;
        manuf_string = "SPANSION";
        if      (dev_id == 0xC4) { model_string = "S29GL016"; }  
        else if (dev_id == 0x49) { model_string = "S29GL016"; }  
        else if (dev_id == 0x7E) { model_string = "S29GL Series"; }
        else { valid = FALSE; }
    }
    else if (manuf_id == 0x20) {
        valid = TRUE;
        manuf_string = "ST";
        if      (dev_id == 0xB0) { model_string = "M29F002T"; }     
        else if (dev_id == 0xE2) { model_string = "M29F040B"; }     
        else if (dev_id == 0xF1) { model_string = "M29F080"; }    
        else if (dev_id == 0xD3) { model_string = "M29F200BT"; }    
        else if (dev_id == 0xD4) { model_string = "M29F200BB"; }    
        else if (dev_id == 0xD5) { model_string = "M29F400BT"; }     
        else if (dev_id == 0xD6) { model_string = "M29F400BB"; }     
        else if (dev_id == 0x58) { model_string = "M29F800DB"; }    
        else if (dev_id == 0xEC) { model_string = "M29F800DT"; }    
        else if (dev_id == 0x23) { model_string = "M29W010B"; }    
        else if (dev_id == 0xE3) { model_string = "M29W040B"; }    
        else if (dev_id == 0x27) { model_string = "M29W512B"; }    
        else if (dev_id == 0x88) { model_string = "M28W Series"; }  
        else if (dev_id == 0x7E) { model_string = "M29W Series"; }  
        else if (dev_id == 0x00) { model_string = "M29W Series"; }  
        else if (dev_id == 0x22) { model_string = "M29W Series"; }  
        else { valid = FALSE; }
    }
    flash_valid = valid;
    return valid;
}

void cart_probe_cart() {
    
    // Read Cartridge Header
    uint8_t *buf;
    
    buf = malloc(335);
    cart_set_gb_mode();
    cart_gb_read_bulk(buf, 0x100, 335, FALSE);
    
    // Read various parameters
    memcpy(game_title[0], buf + 0x34, 16);
    game_title[0][16] = 0x00;
    
    cgb_game[0] = ((buf[0x43] == 0x80) || (buf[0x43] == 0xC0)) ? TRUE : FALSE;
    
    uint8_t cart_type = buf[0x47];
    if ((cart_type == 0x00) || (cart_type == 0x08)|| (cart_type == 0x09)) { mbc_type = NONE;} 
    else if ((cart_type >= 0x01) && (cart_type <= 0x03)) { mbc_type = MBC1; }
    else if ((cart_type >= 0x05) && (cart_type <= 0x06)) { mbc_type = MBC2; }
    else if ((cart_type >= 0x0B) && (cart_type <= 0x0D)) { mbc_type = MMM01; }
    else if ((cart_type >= 0x0F) && (cart_type <= 0x13)) { mbc_type = MBC3; }
    else if ((cart_type >= 0x19) && (cart_type <= 0x1E)) { mbc_type = MBC5; }
    else if ((cart_type == 0x20)) { mbc_type = MBC6; }
    else if ((cart_type == 0x22)) { mbc_type = MBC7; }
    else if ((cart_type == 0xFC)) { mbc_type = POCKET_CAMERA; }
    else if ((cart_type == 0xFD)) { mbc_type = TAMA5; }
    else if ((cart_type == 0xFE)) { mbc_type = HUC3; }
    else if ((cart_type == 0xFF)) { mbc_type = HUC1; }
    
    has_ram = (((cart_type >= 0x02)&&(cart_type <= 0x03))||
        ((cart_type >= 0x08)&&(cart_type <= 0x09))||
        ((cart_type >= 0x0C)&&(cart_type <= 0x10))||
        ((cart_type >= 0x12)&&(cart_type <= 0x13))||
        ((cart_type >= 0x1A)&&(cart_type <= 0x1B))||
        ((cart_type >= 0x1D)&&(cart_type <= 0x1E))||
        (cart_type == 0x22)||(cart_type == 0xFC)||
        (cart_type == 0xFF)) ? TRUE : FALSE;
        
    const char *mbc_type_string;
    switch (mbc_type) {
    case NONE: mbc_type_string = "None"; break;
    case MBC1: mbc_type_string = "MBC1"; break;
    case MBC2: mbc_type_string = "MBC2"; break;
    case MBC3: mbc_type_string = "MBC3"; break;
    case MBC5: mbc_type_string = "MBC5"; break;
    case MBC6: mbc_type_string = "MBC6"; break;
    case MBC7: mbc_type_string = "MBC7"; break;
    case MMM01: mbc_type_string = "MMM01"; break;
    case POCKET_CAMERA: mbc_type_string = "Pocket Camera"; break;
    case TAMA5: mbc_type_string = "TAMA5"; break;
    case HUC3: mbc_type_string = "HUC1"; break;
    case HUC1: mbc_type_string = "HUC1"; break;
    }
        
    rom_size[0] = buf[0x48];
    rom_size[0] = (rom_size[0] > 0x50) ? (1024 + (32 << (rom_size[0] - 0x50))) : (32 << (rom_size[0]));
    rom_size[0] = rom_size[0] * 1024;
    ram_size[0] = buf[0x49];
    ram_size[0] = (ram_size[0] < 6) ? ram_size_lut[ram_size[0]] : 0;
    if (!has_ram) ram_size[0] = 0;
    if (mbc_type == 2) ram_size[0] = 256; // MBC2 has internal 256B RAM. 
    
    uint8_t checksum = 0;
    for (size_t i = 0x34; i <= 0x4c; i++) checksum = checksum - buf[i] - 1;
    game_valid[0] = (checksum == buf[0x4d]) ? TRUE : FALSE;
    
    // Check FLASH ID
    uint32_t seq_type = 2;
    uint8_t manuf_id;
    uint8_t dev_id;
    uint8_t protect;
    do {
        cart_set_seq_type((seq_type_t)seq_type);
        cart_enter_product_id_mode();
        manuf_id = cart_gb_read(ADDR_FLASH_MANUF[seq_type], FALSE);
        dev_id = cart_gb_read(ADDR_FLASH_ID[seq_type], FALSE);
        protect = cart_gb_read(ADDR_FLASH_PROTECT[seq_type], FALSE);
        cart_leave_product_id_mode();
        seq_type ++;
        check_flash_id(manuf_id, dev_id);
        cart_gb_cfi_query();
        flash_valid |= cfi_valid;
    } while ((!flash_valid) && (seq_type < 3));
    seq_type --;
    
    flash_empty = FALSE;
    cart_gb_cfi_query(); // check again
    if ((flash_valid)&&(!game_valid[0]))
    {
        flash_empty = TRUE;
        for (uint32_t i = 0; i < 335; i++) {
            if (buf[i] != 0xFF)
                flash_empty = FALSE;
        }
    }
    
    if (memcmp(game_title, "MULTICARTLDR", 12) == 0)
    {
        multicart_mode = TRUE;
        cart_set_mbc_type(MBC5);
        fat_set_filename(FILE_NO_RAM_SLOT(0), "SLOT1");
        for (uint32_t i = 0; i < 4; i++) {
            cart_gb_rom_read_bulk(buf, addr_lut[i] + 0x100, 335);
            memcpy(game_title[i], buf + 0x34, 16);
            game_title[i][16] = 0x00;
            rom_size[i] = buf[0x48];
            rom_size[i] = (rom_size[i] > 0x50) ? (1024 + (32 << (rom_size[i] - 0x50))) : (32 << (rom_size[i]));
            rom_size[i] = rom_size[i] * 1024;
            ram_size[i] = buf[0x49];
            ram_size[i] = (ram_size[i] < 6) ? ram_size_lut[ram_size[i]] : 0;
            if (!has_ram) ram_size[i] = 0;
            uint8_t cart_type = buf[0x47];
            if ((cart_type == 0x05) || (cart_type == 0x06)) ram_size[i] = 256; // MBC2 has internal 256B RAM. 
            cgb_game[i] = ((buf[0x43] == 0x80) || (buf[0x43] == 0xC0)) ? TRUE : FALSE;
            uint8_t checksum = 0;
            for (size_t i = 0x34; i <= 0x4c; i++) checksum = checksum - buf[i] - 1;
            game_valid[i] = (checksum == buf[0x4d]) ? TRUE : FALSE;
            
            if (game_valid[i]) {
                fat_set_filesize(FILE_NO_ROM_SLOT(i), rom_size[i]);
                fat_set_filesize(FILE_NO_RAM_SLOT(i), ram_size[i]);
                if (cgb_game[i]) {
                    fat_set_fileext(FILE_NO_ROM_SLOT(i), "GBC");
                }
                else {
                    fat_set_fileext(FILE_NO_ROM_SLOT(i), "GB ");
                }
            }
            
            size_t length = sprintf(
                (char *)info_file_content, 
                "MultiCart Mode\r\nGame1: %s \r\nGame2: %s \r\nGame3: %s \r\nGame4: %s \r\nFlash Manufacturer: %s\r\nFlash Model: %s\r\nCart Type: %s\r\nFlash Valid: %s\r\n", 
                game_title[0], 
                game_title[1], 
                game_title[2], 
                game_title[3], 
                manuf_string,
                model_string,
                ((flash_valid) ? ((seq_type == 0) ? "AIN Flash Cartridge" : "WR Flash Cartridge") : ((flash_empty) ? "Bad or No Cartridge" : "Normal Cartridge")),
                ((flash_valid) ? ((protect == 0x00) ? "No" : "Yes") : "N/A"),
                ((flash_empty) ? "Yes" : "No"));
            fat_set_filesize(FILE_NO_INFO, length);
        }
    }
    else {
        multicart_mode = FALSE;
        // Write into summary file
        size_t length = sprintf(
            (char *)info_file_content, 
            "Game: %s\r\nROM: %dKB\r\nRAM: %dKB\r\nMBC: %s\r\nFlash Manufacturer: %s\r\nFlash Model: %s\r\nCart Type: %s\r\nProtect: %s\r\nCFI Valid: %s\r\nEmpty: %s\r\nMulitCart Mode: %s\r\nGame Valid: %s\r\n", 
            game_title[0], 
            rom_size[0] / 1024, 
            ram_size[0] / 1024, 
            mbc_type_string,
            manuf_string,
            model_string,
            ((flash_valid) ? ((seq_type == 0) ? "AIN Flash Cartridge" : "WR Flash Cartridge") : ((flash_empty) ? "Bad or No Cartridge" : "Normal Cartridge")),
            ((flash_valid) ? ((protect == 0x00) ? "No" : "Yes") : "N/A"),
            ((cfi_valid)? "Yes" : "No"),
            ((flash_empty) ? "Yes" : "No"),
            ((multicart_mode) ? "Yes" : "No"),
            ((game_valid[0])? "Yes" : "No"));
        fat_set_filesize(FILE_NO_INFO, length);
    
        // Set pre-set files
        if (game_valid[0]) {
            fat_set_filename(FILE_NO_ROM, game_title[0]);
            fat_set_filename(FILE_NO_RAM, game_title[0]);
            fat_set_filesize(FILE_NO_ROM, rom_size[0]);
            fat_set_filesize(FILE_NO_RAM, ram_size[0]);
            if (cgb_game[0]) {
                fat_set_fileext(FILE_NO_ROM, "GBC");
            }
            else {
                fat_set_fileext(FILE_NO_ROM, "GB ");
            }
            cart_set_mbc_type(mbc_type);
        }
        else {
            cart_set_mbc_type(MBC5); // Could be a empty flash cart...?
        }
    }
    
    free(buf);
}

#if defined (__ICCARM__)
__intrinsic void __DSB(void) {
    asm("dsb");
}
#endif

void sys_reset() {
    NVIC_SystemReset();
}

int main(void)
{
    /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f10x_xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f10x.c file
     */
    // Set Vector Table to application start
    SCB->VTOR = 0x08004000;
    
    // Disable JTAG DP
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
  
    //Key_GPIO_Config();
    cart_io_init();
    cart_set_wait_mode(MODE_POLL);
    fat_init();
    
    // First Probe
    cart_probe_cart();
    // If failed, try if it is a GameBoy Camera!
    if ((!game_valid)&&(!flash_valid)) {
        cart_allow_ain_clock(TRUE);
        cart_probe_cart();
        if (mbc_type != POCKET_CAMERA)
            cart_allow_ain_clock(FALSE);
    } 
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    USB_Interrupts_Config();
    Set_USBClock();
    USB_Init();

    while (1)
    {
        // this has to be handled outside of fat emulation layer, since the
        // USB operation need to be done before it reset
        if (writing_done) {
            delay_ms(1000);
            sys_reset();
        }
    }
}

#ifdef USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\\r\\n", file, line) */
    printf("\\033[1;40;41mERROR:Wrong parameters value: file %s on line %d\\033[0m\\r\\n", file, line)
        /* Infinite loop */
        while (1)
    {
    }
}
#endif

/**
  * @}
  */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
