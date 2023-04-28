#include "bsp_systick.h"

static uint32_t SysTickCount = 0;
 
void systick_init(void)
{   
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SysTick->LOAD  = (uint32_t)(9000 - 1UL);                         /* set reload register */
    NVIC_SetPriority (SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL); /* set Priority for Systick Interrupt */
    SysTick->VAL   = 0UL;                                             /* Load the SysTick Counter Value */
    SysTick->CTRL  = SysTick_CLKSource_HCLK_Div8 |
                     SysTick_CTRL_TICKINT_Msk   |
                        SysTick_CTRL_ENABLE_Msk;                         /* Enable SysTick IRQ and SysTick Timer */
}

uint32_t SysGetTickCount(void)
{
    return SysTickCount;
}

/**
  * @brief  : This function handles SysTick Handler.
  * @param  : count 49.71 DAYS
  * @retval None
  */
void SysTick_Handler(void)
{
    if (SysTickCount < 0xFFFFFFFF) {
        SysTickCount ++;
    }else SysTickCount = 0;
    
}
