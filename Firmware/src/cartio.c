/**
    ******************************************************************************
    * @file           : cartio.c
    * @brief          : Cartridge input and output functions
    ******************************************************************************
    */
#include <stdlib.h>
#include <libopencm3/stm32/gpio.h>

#include "cartio.h"

#define CART_GB_RES_CS2_PORT    GPIOC
#define CART_GB_RES_CS2_PIN     GPIO13
#define CART_GB_AIN_IRQ_PORT    GPIOA
#define CART_GB_AIN_IRQ_PIN     GPIO15
#define CART_GB_WR_PORT         GPIOA
#define CART_GB_WR_PIN          GPIO8
#define CART_GB_RD_PORT         GPIOA
#define CART_GB_RD_PIN          GPIO9
#define CART_GB_CS_PORT         GPIOA
#define CART_GB_CS_PIN          GPIO10
#ifdef HW_R2
#define CART_GBA_D0_PORT        GPIOB
#define CART_GBA_D0_PIN         GPIO0
#define CART_GB_A_DIR_PORT      GPIOC
#define CART_GB_A_DIR_PIN       GPIO15
#endif
#define CART_GB_D_DIR_PORT      GPIOC
#define CART_GB_D_DIR_PIN       GPIO14

// GB    GBA
// D7-0  ADDR23-16 RAM_D7-0
// A15-8 ADDR15-8  ROM_D15-8
// A7-0  ADDR7-0   ROM_D7-0

__attribute__((always_inline)) inline void gb_res_cs2_high(void) {
    gpio_set(CART_GB_RES_CS2_PORT, CART_GB_RES_CS2_PIN);
}

__attribute__((always_inline)) inline void gb_res_cs2_low(void) {
    gpio_clear(CART_GB_RES_CS2_PORT, CART_GB_RES_CS2_PIN);
}

__attribute__((always_inline)) inline void gb_ain_irq_high(void) {
    gpio_set(CART_GB_AIN_IRQ_PORT, CART_GB_AIN_IRQ_PIN);
}

__attribute__((always_inline)) inline void gb_ain_irq_low(void) {
    gpio_clear(CART_GB_AIN_IRQ_PORT, CART_GB_AIN_IRQ_PIN);
}

__attribute__((always_inline)) inline void gb_wr_high(void) {
    gpio_set(CART_GB_WR_PORT, CART_GB_WR_PIN);
}

__attribute__((always_inline)) inline void gb_wr_low(void) {
    gpio_clear(CART_GB_WR_PORT, CART_GB_WR_PIN);
}

__attribute__((always_inline)) inline void gb_rd_high(void) {
    gpio_set(CART_GB_RD_PORT, CART_GB_RD_PIN);
}

__attribute__((always_inline)) inline void gb_rd_low(void) {
    gpio_clear(CART_GB_RD_PORT, CART_GB_RD_PIN);
}

__attribute__((always_inline)) inline void gb_cs_high(void) {
    gpio_set(CART_GB_CS_PORT, CART_GB_CS_PIN);
}

__attribute__((always_inline)) inline void gb_cs_low(void) {
    gpio_clear(CART_GB_CS_PORT, CART_GB_CS_PIN);
}

#ifdef HW_R2
//GBA Support start from Hardware Revision 2.0
__attribute__((always_inline)) inline void gba_d0_high(void) {
    gpio_set(CART_GBA_D0_PORT, CART_GBA_D0_PIN);
}

__attribute__((always_inline)) inline void gba_d0_low(void) {
    gpio_clear(CART_GBA_D0_PORT, CART_GBA_D0_PIN);
}

__attribute__((always_inline)) inline void gb_a_set_input(void) {
    gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO_ALL);
    gpio_clear(CART_GB_A_DIR_PORT, CART_GB_A_DIR_PIN);
}

__attribute__((always_inline)) inline void gb_a_set_output(void) {
    gpio_set(CART_GB_A_DIR_PORT, CART_GB_A_DIR_PIN);
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_10_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO_ALL);
}

__attribute__((always_inline)) inline uint16_t gb_a_read(void) {
    return gpio_port_read(GPIOB);
}
#endif

__attribute__((always_inline)) inline void gb_a_write(uint16_t addr) {
    gpio_port_write(GPIOB, addr);
}

__attribute__((always_inline)) inline void gb_d_set_input(void) {
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, 0x00FF);// GPIO0-7
    gpio_clear(CART_GB_D_DIR_PORT, CART_GB_D_DIR_PIN);
}

__attribute__((always_inline)) inline void gb_d_set_output(void) {
    gpio_set(CART_GB_D_DIR_PORT, CART_GB_D_DIR_PIN);
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_10_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, 0x00FF);
}

__attribute__((always_inline)) inline uint8_t gb_d_read(void) {
    return (gpio_port_read(GPIOA) & 0xFF);
}

__attribute__((always_inline)) inline void gb_d_write(uint8_t data) {
    GPIOA_ODR = ((GPIOA_ODR & 0xFF00) | (data & 0xFF));
}

#ifdef HW_R2
__attribute__((always_inline)) inline void gba_a_write(uint32_t addr) {
    gb_d_write(addr >> 16);
    gb_a_write(addr & 0xFFFF);
}
#endif

void gb_delay(void) {
    volatile size_t i = 5;
    while (i--);
}

void cart_reset(void) {
    gb_cs_high();
    gb_rd_high();
    gb_wr_high();
    gb_res_cs2_high();
    gb_ain_irq_high();
}

// GameBoy related functions
void cart_set_gb_mode(void) {
    gb_d_set_input();
#ifdef HW_R2
    gb_a_set_output();
#endif
}

uint8_t cart_gb_read(uint16_t addr, bool ram_access) {
    gb_a_write(addr);
    if (ram_access) gb_cs_low();
    gb_rd_low();
    gb_delay();
    uint8_t data = gb_d_read();
    gb_rd_high();
    if (ram_access) gb_cs_high();
    return data;
}

void cart_gb_write(uint16_t addr, uint8_t data, bool ram_access) {
    gb_d_set_output();
    gb_a_write(addr);
    gb_d_write(data);
    if (ram_access) gb_cs_low();
    gb_wr_low();
    gb_ain_irq_low();
    gb_delay();
    gb_wr_high();
    gb_ain_irq_high();
    if (ram_access) gb_cs_high();
}

#ifdef HW_R2
void cart_set_gba_mode(void) {
    gba_a_write(0x000000);
    gb_d_set_output();
    gb_a_set_output();
}

uint16_t cart_gba_read(uint32_t addr) {
    gb_a_set_output();
    gb_d_set_output();
    gba_a_write(addr);
    gb_cs_low();
    gb_a_set_input();
    gb_rd_low();
    gb_delay();
    uint16_t data = gb_a_read();
    gb_rd_high();
    gb_cs_high();
    return data;
}

uint8_t cart_gba_ram_read(uint16_t addr) {
    gb_a_set_output();
    gb_d_set_input();
    gb_a_write(addr);
    gb_rd_low();
    gb_res_cs2_low();
    gb_delay();
    uint8_t data = gb_d_read();
    gb_res_cs2_high();
    gb_rd_high();
    return data;
}

void cart_gba_ram_write(uint16_t addr, uint8_t data) {
    gb_a_set_output();
    gb_d_set_output();
    gb_a_write(addr);
    gb_d_write(data);
    gb_wr_low();
    gb_res_cs2_low();
    gb_delay();
    gb_res_cs2_high();
    gb_wr_high();
    gb_d_set_input();
}

void cart_set_eeprom_mode() {
    gba_a_write(0xffff80);
    gb_a_set_output();
    gb_d_set_output();
}

void cart_eeprom_a_write(uint16_t addr, bool write, cap_t cap) {
    gb_cs_low();
    int8_t addr_bit = 0;

    if (cap == CAP_512B)
        addr_bit = 7;
    else if (cap == CAP_8K)
        addr_bit = 15;
    else
        return;

    addr |= (1 << addr_bit);
    if (!write) addr |= (1 << (addr_bit - 1));

    for (size_t i = 0; i <= addr_bit; ++i) {
        if (addr & (1 << addr_bit))
            gba_d0_high();
        else
            gba_d0_low();
        addr = addr << 1;
        gb_wr_low();
        gb_delay();
        gb_wr_high();
        gb_delay();
    }

    // if in read mode, send stop bit
    // if in writing mode, this is not end of the frame
    if (!write) {
        gba_d0_low();
        gb_delay();
        gb_wr_low();
        gb_delay();
        gb_wr_high();
        gb_cs_high();
    }
}

void cart_eeprom_read(uint16_t addr, uint8_t * buffer, cap_t cap) {
    cart_eeprom_a_write(addr, false, cap);
    gb_a_set_input();
    gb_cs_low();

    // Ignore 4 bits
    for (size_t bit = 0; bit < 4; ++bit) {
        gb_rd_low();
        gb_delay();
        gb_rd_high();
        gb_delay();
    }

    // Read 64 bits
    for (size_t byte = 0; byte < 8; ++byte) {
        uint8_t data = 0;
        for (size_t bit = 0; bit < 8; ++bit) {
            gb_rd_low();
            gb_delay();
            gb_rd_high();
            if (gb_a_read() & 0x01) data |= 0x01;
            data = data << 1;
        }
        buffer[byte] = data;
    }

    gb_cs_high();
    gb_a_set_output();
}

void cart_eeprom_write(uint16_t addr, uint8_t *buffer, cap_t cap) {
    cart_eeprom_a_write(addr, true, cap);

    // Write 64 bits
    for (size_t byte = 0; byte < 8; ++byte) {
        uint8_t data = buffer[byte];
        for (size_t bit = 0; bit < 8; ++bit) {
            if (data & 0x80)
                gba_d0_high();
            else
                gba_d0_low();
            gb_wr_low();
            gb_delay();
            gb_wr_high();
            gb_delay();
            data = data << 1;
        }
    }

    gba_d0_low();
    gb_wr_low();
    gb_delay();
    gb_wr_high();
    gb_delay();

    gb_cs_high();
}
#endif