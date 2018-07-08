#include "main.h"
#include "cartio.h"

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

mbc_t cart_mbc;

// GB    GBA
// D7-0  ADDR23-16 RAM_D7-0
// A15-8 ADDR15-8  ROM_D15-8
// A7-0  ADDR7-0   ROM_D7-0

__attribute__((always_inline)) inline void gb_res_cs2_high(void) {
    GPIO_SetBits(CART_GB_RES_CS2_PORT, CART_GB_RES_CS2_PIN);
}

__attribute__((always_inline)) inline void gb_res_cs2_low(void) {
    GPIO_ResetBits(CART_GB_RES_CS2_PORT, CART_GB_RES_CS2_PIN);
}

__attribute__((always_inline)) inline void gb_ain_irq_high(void) {
    GPIO_SetBits(CART_GB_AIN_IRQ_PORT, CART_GB_AIN_IRQ_PIN);
}

__attribute__((always_inline)) inline void gb_ain_irq_low(void) {
    GPIO_ResetBits(CART_GB_AIN_IRQ_PORT, CART_GB_AIN_IRQ_PIN);
}

__attribute__((always_inline)) inline void gb_wr_high(void) {
    GPIO_SetBits(CART_GB_WR_PORT, CART_GB_WR_PIN);
}

__attribute__((always_inline)) inline void gb_wr_low(void) {
    GPIO_ResetBits(CART_GB_WR_PORT, CART_GB_WR_PIN);
}

__attribute__((always_inline)) inline void gb_rd_high(void) {
    GPIO_SetBits(CART_GB_RD_PORT, CART_GB_RD_PIN);
}

__attribute__((always_inline)) inline void gb_rd_low(void) {
    GPIO_ResetBits(CART_GB_RD_PORT, CART_GB_RD_PIN);
}

__attribute__((always_inline)) inline void gb_cs_high(void) {
    GPIO_SetBits(CART_GB_CS_PORT, CART_GB_CS_PIN);
}

__attribute__((always_inline)) inline void gb_cs_low(void) {
    GPIO_ResetBits(CART_GB_CS_PORT, CART_GB_CS_PIN);
}

#ifdef HW_R2
//GBA Support start from Hardware Revision 2.0
__attribute__((always_inline)) inline void gba_d0_high(void) {
    GPIO_SetBits(CART_GBA_D0_PORT, CART_GBA_D0_PIN);
}

__attribute__((always_inline)) inline void gba_d0_low(void) {
    GPIO_ResetBits(CART_GBA_D0_PORT, CART_GBA_D0_PIN);
}

__attribute__((always_inline)) inline void gb_a_set_input(void) {
    GPIO_SetBits_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO_ALL);
    GPIO_ResetBits(CART_GB_A_DIR_PORT, CART_GB_A_DIR_PIN);
}

__attribute__((always_inline)) inline void gb_a_set_output(void) {
    GPIO_SetBits(CART_GB_A_DIR_PORT, CART_GB_A_DIR_PIN);
    GPIO_SetBits_mode(GPIOB, GPIO_MODE_OUTPUT_10_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO_ALL);
}

__attribute__((always_inline)) inline uint16_t gb_a_read(void) {
    return gpio_port_read(GPIOB);
}
#endif

__attribute__((always_inline)) inline void gb_a_write(uint16_t addr) {
    GPIO_Write(GPIOB, addr);
}

__attribute__((always_inline)) inline void gb_d_set_input(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = 0xFF; // GPIO0-7
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;

    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_ResetBits(CART_GB_D_DIR_PORT, CART_GB_D_DIR_PIN);
}

__attribute__((always_inline)) inline void gb_d_set_output(void) {
    GPIO_SetBits(CART_GB_D_DIR_PORT, CART_GB_D_DIR_PIN);
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = 0xFF; // GPIO0-7
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

__attribute__((always_inline)) inline uint8_t gb_d_read(void) {
    return (GPIO_ReadInputData(GPIOA) & 0xFF);
}

__attribute__((always_inline)) inline void gb_d_write(uint8_t data) {
    GPIOA->ODR = ((GPIOA->ODR & 0xFF00) | (data & 0xFF));
}

#ifdef HW_R2
__attribute__((always_inline)) inline void gba_a_write(uint32_t addr) {
    gb_d_write(addr >> 16);
    gb_a_write(addr & 0xFFFF);
}
#endif

void gb_delay(void) {
    volatile size_t i = 2;
    while (i--);
}

void cart_io_init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    
    // Set default state
    gb_res_cs2_high();
    gb_ain_irq_high();
    gb_wr_high();
    gb_rd_high();
    gb_cs_high();

    // Initialize Control Pins
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

    GPIO_InitStructure.GPIO_Pin = CART_GB_RES_CS2_PIN;
    GPIO_Init(CART_GB_RES_CS2_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = CART_GB_AIN_IRQ_PIN;
    GPIO_Init(CART_GB_AIN_IRQ_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = CART_GB_WR_PIN;
    GPIO_Init(CART_GB_WR_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = CART_GB_RD_PIN;
    GPIO_Init(CART_GB_RD_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = CART_GB_CS_PIN;
    GPIO_Init(CART_GB_CS_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = CART_GB_D_DIR_PIN;
    GPIO_Init(CART_GB_D_DIR_PORT, &GPIO_InitStructure);
    
#ifdef HW_R2
    
    GPIO_InitStructure.GPIO_Pin = CART_GB_D_DIR_PIN;
    GPIO_Init(CART_GB_D_DIR_PORT, &GPIO_InitStructure);
    
#endif
    
    // Initialize Address Pins
    GPIO_InitStructure.GPIO_Pin = 0xFFFF;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void cart_set_mbc_type(mbc_t new_type) {
    cart_mbc = new_type;
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


// More application specific function calls
void cart_gb_read_bulk(uint8_t *buffer, uint16_t addr, uint16_t size, bool ram_access) {
  for (size_t i = 0; i < size; i++) {
    buffer[i] = cart_gb_read(addr + i, ram_access);
  }
}

void cart_gb_enable_sram(void) {
    cart_gb_write(0x0000, 0x0A, FALSE);
}

void cart_gb_disable_sram(void) {
    cart_gb_write(0x0000, 0x00, FALSE);
}

void cart_gb_set_mbc1_model(uint8_t model) {
    cart_gb_write(0x6000, model, FALSE);
}

void cart_gb_switch_rom_bank(uint16_t bank) {
    cart_gb_write(0x3000, (bank >> 8)&0xff, FALSE);
    cart_gb_write(0x2000, bank & 0xff, FALSE);
    if (cart_mbc == MBC1)
        cart_gb_write(0x4000, ((bank & 0xff) >> 5), FALSE);
}

void cart_gb_switch_ram_bank(uint8_t bank) {
    cart_gb_write(0x4000, bank, FALSE);
}

void cart_gb_rom_read_bulk(uint8_t *buffer, uint32_t addr, uint16_t size) {
    // it has to be aligned.
    if (((addr & 0x3fff) + size) <= 0x4000) {
        if (addr < 0x3fff) {
            // read from lower bank
            cart_gb_read_bulk(buffer, addr, size, FALSE);
        }
        else {
            // read from higher bank
            cart_gb_switch_rom_bank(addr >> 14);
            gb_d_set_input();
            cart_gb_read_bulk(buffer, 0x4000 | addr & 0x3fff, size, FALSE);
        }
    }
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