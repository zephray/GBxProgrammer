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

#endif