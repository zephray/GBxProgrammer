/**
  ******************************************************************************
  * @file           : cartio.h
  * @brief          : Header for cartio.c file.
  ******************************************************************************
  */
  
#ifndef __CARTIO_H__
#define __CARTIO_H__

#define CART_GB_RES_CS2_PORT    GPIOC
#define CART_GB_RES_CS2_PIN     GPIO_Pin_13
#define CART_GB_AIN_IRQ_PORT    GPIOA
#define CART_GB_AIN_IRQ_PIN     GPIO_Pin_15
#define CART_GB_WR_PORT         GPIOA
#define CART_GB_WR_PIN          GPIO_Pin_8
#define CART_GB_RD_PORT         GPIOA
#define CART_GB_RD_PIN          GPIO_Pin_9
#define CART_GB_CS_PORT         GPIOA
#define CART_GB_CS_PIN          GPIO_Pin_10
#ifdef HW_R2
#define CART_GBA_D0_PORT        GPIOB
#define CART_GBA_D0_PIN         GPIO0
#define CART_GB_A_DIR_PORT      GPIOC
#define CART_GB_A_DIR_PIN       GPIO_Pin_15
#endif
#define CART_GB_D_DIR_PORT      GPIOC
#define CART_GB_D_DIR_PIN       GPIO_Pin_14

#define PRGM_TIMEOUT 10000
#define DEF_PRGM_DELAY 55
#define DEF_ERASE_DELAY 5000
#define LONG_PRGM_DELAY 110
#define LONG_ERASE_DELAY 10000
      
typedef enum cap {
    CAP_NONE = 0, 
    CAP_512B, 
    CAP_8K, 
    CAP_32K, 
    CAP_64K,
    CAP_128K,
    CAP_256K,
    CAP_512K,
    CAP_1M,
    CAP_2M,
    CAP_4M,
    CAP_8M
} cap_t;
  
typedef enum mbc {
    NONE = 0,
    MBC1 = 1,
    MBC2 = 2,
    MBC3 = 3,
    MBC5 = 5,
    MBC6 = 6,
    MBC7 = 7,
    MMM01,
    POCKET_CAMERA,
    TAMA5,
    HUC3,
    HUC1
} mbc_t;

typedef enum seq_type {
    TYPE_AIN = 0,
    TYPE_N8 = 1,
    TYPE_N16 = 2
} seq_type_t;

typedef enum wait_mode {
    MODE_LONGER = 0,
    MODE_DEFAULT,
    MODE_POLL,
    MODE_TOGGLE
} wait_mode_t;

typedef enum mbc1_model {
    MODEL_16_8 = 0,
    MODEL_4_32 = 1
} mbc1_model_t;

typedef enum wait_opt {
    OPT_PROGRAM,
    OPT_ERASE
} wait_opt_t;

extern const uint8_t ADDR_FLASH_MANUF[];
extern const uint8_t ADDR_FLASH_ID[];
extern const uint8_t ADDR_FLASH_PROTECT[];

void gb_delay(void);
void cart_reset(void);
void cart_io_init(void);
void cart_set_mbc_type(mbc_t new_type);
// GameBoy related functions
void cart_set_gb_mode(void);
uint8_t cart_gb_read(uint16_t addr, bool ram_access);
void cart_gb_write(uint16_t addr, uint8_t data, bool ram_access);
void cart_gb_read_bulk(uint8_t *buffer, uint16_t addr, uint16_t size, bool ram_access);
void cart_gb_rom_read_bulk(uint8_t *buffer, uint32_t addr, uint16_t size);
#ifdef HW_R2
void cart_set_gba_mode(void);
uint16_t cart_gba_read(uint32_t addr);
uint8_t cart_gba_ram_read(uint16_t addr);
void cart_gba_ram_write(uint16_t addr, uint8_t data);
void cart_set_eeprom_mode(void);
void cart_eeprom_a_write(uint16_t addr, bool write, cap_t cap);
void cart_eeprom_read(uint16_t addr, uint8_t * buffer, cap_t cap);
void cart_eeprom_write(uint16_t addr, uint8_t *buffer, cap_t cap);
#endif
void cart_set_seq_type(seq_type_t new_type);
void cart_set_wait_mode(wait_mode_t new_mode);
void cart_erase_flash();
void cart_program_byte(uint32_t addr, uint8_t dat);
void cart_enter_product_id_mode();
void cart_leave_product_id_mode();
void cart_gb_rom_program_bulk(uint8_t *buffer, uint32_t addr, uint16_t size);

#endif