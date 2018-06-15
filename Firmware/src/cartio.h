/**
  ******************************************************************************
  * @file           : cartio.h
  * @brief          : Header for cartio.c file.
  ******************************************************************************
  */
  
#ifndef __CARTIO_H__
#define __CARTIO_H__

typedef enum cap {
  CAP_NONE = 0, 
  CAP_512B, 
  CAP_8K, 
  CAP_32K, 
  CAP_128K,
  CAP_256K,
  CAP_512K,
  CAP_1M,
  CAP_2M,
  CAP_4M,
  CAP_8M} cap_t;

void gb_delay(void);
void cart_reset(void);
// GameBoy related functions
void cart_set_gb_mode(void);
uint8_t cart_gb_read(uint16_t addr, bool ram_access);
void cart_gb_write(uint16_t addr, uint8_t data, bool ram_access);
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

#endif