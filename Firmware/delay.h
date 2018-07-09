#ifndef __DELAY_H__
#define __DELAY_H__

#include "stm32f10x.h"

void delay_ms(__IO uint32_t nTime);
void delay_us(__IO uint32_t nTime);

#endif