#include"delay.h"

static __IO uint32_t TimingDelay;

void delay_ms(__IO uint32_t nTime) 
{ 
    while(SysTick_Config(SystemCoreClock/1000)); 
    TimingDelay = nTime;
    while(TimingDelay != 0);
    SysTick->CTRL=0x00; 
    SysTick->VAL =0X00;
}

void delay_us(__IO uint32_t nTime)
{ 
    while(SysTick_Config(SystemCoreClock/1000000)); 
    TimingDelay = nTime;
    while(TimingDelay != 0);
    SysTick->CTRL=0x00;
    SysTick->VAL =0X00;
}

void TimingDelay_Decrement(void)
{
    if (TimingDelay != 0x00)
    { 
        TimingDelay--;
    }
}

void SysTick_Handler(void)
{
    TimingDelay_Decrement();
}