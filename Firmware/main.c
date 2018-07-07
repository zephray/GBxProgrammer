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

char game_title[12];
uint32_t rom_size;
uint32_t ram_size;
const uint32_t ram_size_lut[6] = {0, 2048, 8192, 32768, 131072, 65536};
bool game_valid;
bool flash_valid;
bool is_cgb_game;
bool has_ram;
mbc_t mbc_type; 
    
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

void cart_probe_cart() {
    uint8_t *buf;
    
    buf = malloc(335);
    cart_set_gb_mode();
    cart_gb_read_bulk(buf, 0x100, 335, FALSE);
    
    memcpy(game_title, buf + 0x34, 11);
    game_title[11] = 0x00;
    
    is_cgb_game = ((buf[0x43] == 0x80) || (buf[0x43] == 0xC0)) ? TRUE : FALSE;
    
    uint8_t cart_type = buf[0x47];
    if ((cart_type == 0x00) || (cart_type == 0x08)|| (cart_type == 0x09)) { mbc_type = NONE;} 
    else if ((cart_type >= 0x01) && (cart_type <= 0x03)) { mbc_type = MBC1; }
    else if ((cart_type >= 0x05) && (cart_type <= 0x06)) { mbc_type = MBC2; }
    else if ((cart_type >= 0x0B) && (cart_type <= 0x1D)) { mbc_type = MMM01; }
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
        (cart_type == 0x22)||
        (cart_type == 0xFF)) ? TRUE : FALSE;
        
    rom_size = buf[0x48];
    rom_size = (rom_size > 0x50) ? (1024 + (32 << (rom_size - 0x50))) : (32 << (rom_size));
    rom_size = rom_size * 1024;
    ram_size = buf[0x49];
    ram_size = (ram_size < 6) ? ram_size_lut[ram_size] : 0;
    if (!has_ram) ram_size = 0;
    if (mbc_type == 2) ram_size = 256; // MBC2 has internal 256B RAM. 
    
    uint8_t checksum = 0;
    for (size_t i = 0x34; i <= 0x4c; i++) checksum = checksum - buf[i] - 1;
    game_valid = (checksum == buf[0x4d]) ? TRUE : FALSE;
    
    size_t length = sprintf((char *)InfoFileContent, 
        "Game: %s\r\nROM: %dKB\r\nRAM: %dKB\r\nValid: %s\r\n", 
        game_title, rom_size / 1024, ram_size / 1024, (game_valid == TRUE)? "Yes" : "No");
    fat_set_filesize(FILE_NO_INFO, length);
    if (game_valid) {
        fat_set_filename(FILE_NO_ROM, game_title);
        fat_set_filename(FILE_NO_RAM, game_title);
        fat_set_filesize(FILE_NO_ROM, rom_size);
        fat_set_filesize(FILE_NO_RAM, ram_size);
        if (is_cgb_game) {
            fat_set_fileext(FILE_NO_ROM, "GBC");
        }
        else {
            fat_set_fileext(FILE_NO_ROM, "GB");
        }
        cart_set_mbc_type(mbc_type);
    }
    else {
        cart_set_mbc_type(MBC5); // Could be a empty flash cart
    }
    free(buf);
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
    
    cart_probe_cart();
      
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    USB_Interrupts_Config();
    Set_USBClock();
    USB_Init();

    while (1)
    {
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
