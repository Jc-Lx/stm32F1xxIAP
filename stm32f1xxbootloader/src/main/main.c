#include "stm32f10x.h"
#include "bsp_master_usart.h"
#include "bsp_systick.h"
#include "hal_modem.h"
#include "bsp_flash.h"

/**
 * @brief      main
 *
 * @param[in]  void
 *
 * @return
 */
int main(void)
{
	systick_init();
    MasterUsart_Config();

    hal_modem_config_t config = {
        .role = YOMODEM_1K_RECEIVER,
        .offset = ApplicationAddress
    };

    hal_modem_handle_t handle = hal_modem_start(&config);
    if(!handle){
        LOGI("hal_modem_start Failed\n");
    }
    LOGI("Stm32F1xxOtaBootloader Beginning !\r\n");

	while(1) {
        hal_modem_machine_run(&handle);
    }
		
	return 0;
}

/*************************END OF FILE**********************/
