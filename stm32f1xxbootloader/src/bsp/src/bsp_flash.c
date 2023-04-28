#include "bsp_flash.h"

/* BIN VERSION SET */
const char Firmware_Ver[] __attribute__((section(".ARM.__at_0x08002F60"))) = "Stm32F1xxOtaBootV1.0.0";/* MAX 32 Bytes */
const char Compiler_Date[] __attribute__((section(".ARM.__at_0x08002F80"))) = __DATE__;/* MAX 16 Bytes */
const char Compiler_Time[] __attribute__((section(".ARM.__at_0x08002F90"))) = __TIME__;/* MAX 16 Bytes */
const uint32_t OTAFLAG __attribute__((section(".ARM.__at_0x08002FA0"))) = (uint32_t) 0xAAAAAAAA;/* MAX 16 Bytes */

/* write 1k data to flash */
uint32_t FLASH_Write1kData(uint32_t addr, uint8_t* dst, size_t len)
{
    assert_param (len == 1024 && !(addr % 0x400)); /* yomodem-1k pack size */
    
    uint16_t halfword = 0;
    len = len/2;

    /* check erase addr */
    uint32_t pagecount = (addr - ApplicationAddress) / PAGE_SIZE;
    uint32_t pageaddr = pagecount * PAGE_SIZE + ApplicationAddress;
    if (pageaddr < ApplicationAddress || 
                    (addr + 1024) >= (ApplicationAddress + AppImageSize)) {
        return 1;/* ilegal addr */
    }
    
    /* unlock */
    FLASH_Unlock();

    /* erase flag */
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | 
                                    FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    
    /* erase page */
    FLASH_ErasePage(pageaddr);
    
    /* write page */
    while (len -- ){
        halfword = *(dst ++);
        halfword |= *(dst ++) << 8;
        FLASH_ProgramHalfWord(addr,halfword);
        addr += 2;
    }
    
    /* lock */
    FLASH_LockBank1();

    return 0;
}

/* flash read word */
static uint32_t FLASH_ReadWord(uint32_t addr)
{
    return *(volatile uint32_t*)addr;
}

/* get ota flag */
uint32_t FLASH_GetOtaFlag(void)
{
    return FLASH_ReadWord(OtaFlag);
}

pFunction Jump_To_Application;
uint32_t JumpAddress;
/* jump to application */
void jump_to_application(void)
{
    /* Test if user code is programmed starting from address "ApplicationAddress" */
    if (((*(__IO uint32_t*)ApplicationAddress) & 0x2FFF0000 ) == 0x20000000) { 
        /* Jump to user application */
        JumpAddress = *(__IO uint32_t*) (ApplicationAddress + 4);
        Jump_To_Application = (pFunction) JumpAddress;
        /* Initialize user application's Stack Pointer */
        __set_MSP(*(__IO uint32_t*) ApplicationAddress);
        Jump_To_Application();
    }
    
}
