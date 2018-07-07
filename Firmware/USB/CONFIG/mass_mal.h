/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : mass_mal.h
* Author             : MCD Application Team
* Version            : V2.2.0
* Date               : 06/13/2008
* Description        : Header for mass_mal.c file.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MASS_MAL_H
#define __MASS_MAL_H

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define MAL_OK   0
#define MAL_FAIL 1
#define MAX_LUN  0

#define MAL_SIZE              (100*1024*1024)  // 100M
#define MAL_PAGE_SIZE         (2*1024)         // 2k Bytes per page
#define MAL_WAIT_TIMEOUT      100000

extern uint32_t Mass_Memory_Size[1];
extern uint32_t Mass_Block_Size[1];
extern uint32_t Mass_Block_Count[1];

u16 MAL_Init (u8 lun);
u16 MAL_GetStatus (u8 lun);
u16 MAL_Read(u8 lun, u32 Memory_Offset, u32 *Readbuff);
u16 MAL_Write(u8 lun, u32 Memory_Offset, u32 *Writebuff);
#endif /* __MASS_MAL_H */

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
