#pragma once
#ifndef _BSP_FLASH_H
#define _BSP_FLASH_H

#include "stm32f10x.h"
#include <stdio.h>

#ifdef _cplusplus
extern "c"{
#endif

typedef void (*pFunction)(void);
#define FLASH_SIZE          ((uint32_t)0x10000)
#define PAGE_SIZE           ((uint32_t)1024)
#define ApplicationAddress  ((uint32_t)0x08003000)
#define AppImageSize        ((uint32_t)(FLASH_SIZE - (ApplicationAddress - 0x08000000)))
#define OtaFlag             ((uint32_t)0x08002FA0)


/**
  * @brief      rite data to flash
  * 
  * @param[in]  addr   the offest of flash
  * @param[in]  dst    the source of data
  * @param[in]  len    thr length of source(2n)
  * 
  * @retval     
  *     - 0 successful
  */
uint32_t FLASH_Write1kData(uint32_t addr, uint8_t* dst, size_t len);

/**
  * @brief  get ota flag
  *  
  * @retval 0xAAAAAAAA ota work
  */
uint32_t FLASH_GetOtaFlag(void);

/**
  * @brief jump to app
  */
void jump_to_application(void);

#ifdef _cplusplus
}
#endif
#endif
