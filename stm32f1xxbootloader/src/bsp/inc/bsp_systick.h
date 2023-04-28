#ifndef _BSP_SYSTICK_H_
#define _BSP_SYSTICK_H_

#include "stm32f10x.h"

#ifdef _cplusplus
extern "c"{
#endif
	
void systick_init(void);
uint32_t SysGetTickCount(void);

#ifdef _cplusplus
}
#endif
#endif
