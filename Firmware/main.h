#ifndef __MAIN_H__
#define __MAIN_H__

#include "stm32f10x.h"
#include <stdio.h>

#define LED_OFF() GPIOC->BRR  = GPIO_Pin_14
#define LED_ON()  GPIOC->BSRR = GPIO_Pin_14

typedef enum
{
  FALSE = 0, TRUE  = !FALSE
}
bool;

void sys_reset();

#endif
