#ifndef __BSP_MASTER_USART_H
#define	__BSP_MASTER_USART_H

#include "stm32f10x.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#ifdef _cplusplus
extern "c"{
#endif

#define  MASTER_USARTx                      USART1
#define  MASTER_USART_TX_GPIO_PORT          GPIOA
#define  MASTER_USART_TX_GPIO_PIN           GPIO_Pin_9
#define  MASTER_USART_RX_GPIO_PORT          GPIOA
#define  MASTER_USART_RX_GPIO_PIN           GPIO_Pin_10
#define  MASTER_USART_BAUDRATE              115200
#define  MASTER_USART_CLK                   RCC_APB2Periph_USART1
#define  MASTER_USART_APBxClkCmd            RCC_APB2PeriphClockCmd
#define  MASTER_USART_GPIO_CLK              RCC_APB2Periph_GPIOA
#define  MASTER_USART_GPIO_APBxClkCmd       RCC_APB2PeriphClockCmd    
#define  MASTER_USART_IRQ                   USART1_IRQn
#define  MASTER_USART_IRQHandler            USART1_IRQHandler

#define  MODEM_USARTx                       USART2
#define  MODEM_USART_TX_GPIO_PORT           GPIOA
#define  MODEM_USART_TX_GPIO_PIN            GPIO_Pin_2
#define  MODEM_USART_RX_GPIO_PORT           GPIOA
#define  MODEM_USART_RX_GPIO_PIN            GPIO_Pin_3
#define  MODEM_USART_BAUDRATE               115200
#define  MODEM_USART_CLK                    RCC_APB1Periph_USART2
#define  MODEM_USART_APBxClkCmd             RCC_APB1PeriphClockCmd
#define  MODEM_USART_GPIO_CLK               RCC_APB2Periph_GPIOA
#define  MODEM_USART_GPIO_APBxClkCmd        RCC_APB2PeriphClockCmd    
#define  MODEM_USART_IRQ                    USART2_IRQn
#define  MODEM_USART_IRQHandler             USART2_IRQHandler

/* USART Control Block */
typedef struct usart_control_block{
    void* self;                             /* 指向usart tcb 本身 */
    uint8_t data[1024+128];                 /* 指示数据本体 */
    uint32_t len;                           /* 指示接收或发送数据长度 */
    bool flag;                              /* 指示数据发送或接收就位 */
}usart_t;

void Usart_SendByte(USART_TypeDef* pUSARTx, uint8_t ch);
uint32_t Usart_SendArray( USART_TypeDef * pUSARTx, uint8_t *array, uint32_t num);
void Usart_SendString(USART_TypeDef* pUSARTx, char* str);

void MasterUsart_Config(void);
usart_t* GetMasterUsartRecvFlag(void);
void ClearMasterUsartFlag(void);
usart_t* GetModemUsartRecvFlag(void);
void ClearModemUsartFlag(void);

#ifdef _cplusplus
}
#endif
#endif /* __USARTDMA_H */



