#ifndef __DELAY_H__
#define __DELAY_H__
#include "stm32f10x.h"
/*--------------------------------- 
�� ��������1��Ϊ1ms��1000��Ϊ1s��ֻ�м�
us����
-----------------------------------*/
extern void Delayms(__IO uint32_t nTime);

/*---------------------------------
�� ��������1��Ϊ1us��1000��Ϊ1ms��ֻ�м�
us����
-----------------------------------*/
extern void Delayus(__IO uint32_t nTime);

#endif