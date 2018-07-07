/*---------------------------------
��ʱģ�麯��
˵����ֻ���ڹ����м���delay.c��delay.h
�ļ��������� Delayms(__IO uint32_t nTime)��
Delayus(__IO uint32_t nTime)
-----------------------------------*/
#include"delay.h"
static __IO uint32_t TimingDelay;
/* Private function prototypes -----------------------------------------------*/
/*---------------------------------
��������ms��ʱ���� 
�� ��������1��Ϊ1ms��1000��Ϊ1s��ֻ�м�
us����
-----------------------------------*/
void Delayms(__IO uint32_t nTime) 
{ 
while(SysTick_Config(SystemCoreClock/1000)); 
TimingDelay = nTime;
while(TimingDelay != 0);
SysTick->CTRL=0x00; //�رռ����� 
SysTick->VAL =0X00; //��ռ����� 
}
/*---------------------------------
��������us��ʱ���� 
�� ��������1��Ϊ1us��1000��Ϊ1ms��ֻ�м�
us����
-----------------------------------*/
void Delayus(__IO uint32_t nTime)
{ 
while(SysTick_Config(SystemCoreClock/1000000)); 
TimingDelay = nTime;
while(TimingDelay != 0);
SysTick->CTRL=0x00; //�رռ����� 
SysTick->VAL =0X00; //��ռ����� 
}
/*---------------------------------
����������ʱ�������� 
�� ���� 
-----------------------------------*/
void TimingDelay_Decrement(void)
{
if (TimingDelay != 0x00)
{ 
TimingDelay--;
}
}
/*---------------------------------
��������systick���жϺ��� 
�� ��������1��Ϊ1us��1000��Ϊ1ms��ֻ�м�
us����
-----------------------------------*/
void SysTick_Handler(void)
{
TimingDelay_Decrement();
}