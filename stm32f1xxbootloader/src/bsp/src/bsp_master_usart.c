#include "bsp_master_usart.h"

/* USART Control Block */
usart_t master_usart;
static usart_t modem_usart;

/**
  * @brief  master_usart 配置,工作参数配置
  * @param  
  * @retval 
  */
void MasterUsart_Config(void)
{   
    /*************master usart init**********/
    master_usart.self = &master_usart;
    master_usart.flag = 0;
    master_usart.len = 0;

	MASTER_USART_GPIO_APBxClkCmd(MASTER_USART_GPIO_CLK, ENABLE);/* open uart gpio clock */	
	MASTER_USART_APBxClkCmd(MASTER_USART_CLK, ENABLE);/* open uart clock */

    GPIO_InitTypeDef GPIO_Master = {
        .GPIO_Pin = MASTER_USART_TX_GPIO_PIN,
        .GPIO_Mode = GPIO_Mode_AF_PP,
        .GPIO_Speed = GPIO_Speed_50MHz
    };
	GPIO_Init(MASTER_USART_TX_GPIO_PORT, &GPIO_Master);

	GPIO_Master.GPIO_Pin = MASTER_USART_RX_GPIO_PIN;
	GPIO_Master.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(MASTER_USART_RX_GPIO_PORT, &GPIO_Master);

    USART_InitTypeDef USART_Master = {
        .USART_BaudRate = MASTER_USART_BAUDRATE,
        .USART_WordLength = USART_WordLength_8b,
        .USART_StopBits = USART_StopBits_1,
        .USART_Parity = USART_Parity_No,
        .USART_HardwareFlowControl = USART_HardwareFlowControl_None,
        .USART_Mode = USART_Mode_Rx | USART_Mode_Tx
    };
	USART_Init(MASTER_USARTx, &USART_Master);	

    NVIC_InitTypeDef NVIC_Master = {
        .NVIC_IRQChannel = MASTER_USART_IRQ,
        .NVIC_IRQChannelPreemptionPriority = 1,
        .NVIC_IRQChannelSubPriority = 0,
        .NVIC_IRQChannelCmd = ENABLE
    };
    NVIC_Init(&NVIC_Master);
	USART_ITConfig(MASTER_USARTx,USART_IT_RXNE,ENABLE);/* 此函数不能或与设置 */
    USART_ITConfig(MASTER_USARTx,USART_IT_IDLE,ENABLE);
	USART_Cmd(MASTER_USARTx, ENABLE);
    /*************master usart init**********/

    /*************modem usart init**********/
    modem_usart.self = &modem_usart;
    modem_usart.flag = 0;
    modem_usart.len = 0;

	MODEM_USART_GPIO_APBxClkCmd(MODEM_USART_GPIO_CLK, ENABLE);/* open uart gpio clock */	
	MODEM_USART_APBxClkCmd(MODEM_USART_CLK, ENABLE);/* open uart clock */

    GPIO_InitTypeDef GPIO_Modem = {
        .GPIO_Pin = MODEM_USART_TX_GPIO_PIN,
        .GPIO_Mode = GPIO_Mode_AF_PP,
        .GPIO_Speed = GPIO_Speed_50MHz
    };
	GPIO_Init(MODEM_USART_TX_GPIO_PORT, &GPIO_Modem);

	GPIO_Modem.GPIO_Pin = MODEM_USART_RX_GPIO_PIN;
	GPIO_Modem.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(MODEM_USART_RX_GPIO_PORT, &GPIO_Modem);

    USART_InitTypeDef USART_Modem = {
        .USART_BaudRate = MODEM_USART_BAUDRATE,
        .USART_WordLength = USART_WordLength_8b,
        .USART_StopBits = USART_StopBits_1,
        .USART_Parity = USART_Parity_No,
        .USART_HardwareFlowControl = USART_HardwareFlowControl_None,
        .USART_Mode = USART_Mode_Rx | USART_Mode_Tx
    };
	USART_Init(MODEM_USARTx, &USART_Modem);	

    NVIC_InitTypeDef NVIC_Modem = {
        .NVIC_IRQChannel = MODEM_USART_IRQ,
        .NVIC_IRQChannelPreemptionPriority = 2,
        .NVIC_IRQChannelSubPriority = 0,
        .NVIC_IRQChannelCmd = ENABLE
    };
    NVIC_Init(&NVIC_Modem);
	USART_ITConfig(MODEM_USARTx,USART_IT_RXNE,ENABLE);
    USART_ITConfig(MODEM_USARTx,USART_IT_IDLE,ENABLE);
	USART_Cmd(MODEM_USARTx, ENABLE);
    /*************modem usart init**********/
}

/* usart send byte */
void Usart_SendByte(USART_TypeDef* pUSARTx, uint8_t ch)
{
	USART_SendData(pUSARTx,ch);
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);	
}

/* usart send data */
uint32_t Usart_SendArray(USART_TypeDef* pUSARTx, uint8_t* array, uint32_t num)
{
    uint32_t i;
	for(i=0; i<num; i++) {
	    Usart_SendByte(pUSARTx,array[i]);	
    }
	while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TC) == RESET){};
    return i;
}

/* usart send string */
void Usart_SendString(USART_TypeDef* pUSARTx, char* str)
{
	unsigned int k=0;
    do {
        Usart_SendByte(pUSARTx, *(str + k));
        k++;
    } while(*(str + k)!='\0');
    while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET);
}

/* usart send half world */
void Usart_SendHalfWord(USART_TypeDef* pUSARTx, uint16_t ch)
{
	uint8_t temp_h, temp_l;
	
	temp_h = (ch&0XFF00)>>8;
	temp_l = ch&0XFF;
	
	USART_SendData(pUSARTx,temp_h);	
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);
	
	USART_SendData(pUSARTx,temp_l);	
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);	
}

/* use printf */
int fputc(int ch, FILE* f)
{
	USART_SendData(MASTER_USARTx, (uint8_t) ch);
		
	while (USART_GetFlagStatus(MASTER_USARTx, USART_FLAG_TXE) == RESET);		
	
	return (ch);
}

int fgetc(FILE* f)
{
	while (USART_GetFlagStatus(MASTER_USARTx, USART_FLAG_RXNE) == RESET);

	return (int)USART_ReceiveData(MASTER_USARTx);
}

void MASTER_USART_IRQHandler(void)
{
    uint8_t clear = clear;
	if(USART_GetITStatus(MASTER_USARTx,USART_IT_RXNE) != RESET)
	{
        master_usart.data[master_usart.len++] = MASTER_USARTx->DR;
	}if(USART_GetITStatus(MASTER_USARTx,USART_IT_IDLE) != RESET)
	{
        clear = MASTER_USARTx->SR;//读SR寄存器
		clear = MASTER_USARTx->DR;//读DR寄存器(先读SR再读DR,就是为了清除IDLE中断)
		master_usart.flag = 1;//标记接收到了一帧数据
	}
}

usart_t* GetMasterUsartRecvFlag(void)
{
    if (master_usart.flag == 1) {
        return master_usart.self;
    }else return NULL;
    
}

void ClearMasterUsartFlag(void)
{
    memset((uint8_t* )master_usart.data,0,1024+128);
    master_usart.len = 0;
    master_usart.flag = 0;
}

void MODEM_USART_IRQHandler(void)
{
    uint8_t clear = clear;
	if(USART_GetITStatus(MODEM_USARTx,USART_IT_RXNE) != RESET)
	{
        modem_usart.data[modem_usart.len++] = MODEM_USARTx->DR;
	}if(USART_GetITStatus(MODEM_USARTx,USART_IT_IDLE) != RESET)
	{
        clear = MODEM_USARTx->SR;//读SR寄存器
		clear = MODEM_USARTx->DR;//读DR寄存器(先读SR再读DR,就是为了清除IDLE中断)
		modem_usart.flag = 1;//标记接收到了一帧数据
	}
}

usart_t* GetModemUsartRecvFlag(void)
{
    if (modem_usart.flag == 1) {
        return modem_usart.self;
    }else return NULL;
    
}

void ClearModemUsartFlag(void)
{
    memset((uint8_t* )modem_usart.data,0,1024+128);
    modem_usart.len = 0;
    modem_usart.flag = 0;
}

