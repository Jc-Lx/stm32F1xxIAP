/*
 * @Author: Jc-Lx 1031510595@qq.com
 * @Date: 2023-01-11 09:33:37
 * @LastEditors: Jc-Lx 1031510595@qq.com
 * @LastEditTime: 2023-04-21 16:23:39
 * @FilePath: \fac8266\components\BaseB\bsp_modem.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "hal_modem.h"

/**********************************************************************************/
/*
**用户在此实现移植功能
**在此包含串口驱动需要引用的头文件
*/
#include "stm32f10x.h"
#include "bsp_master_usart.h"
#include "bsp_systick.h"
#include "bsp_flash.h"

/* Usart init*/
static void hal_modem_io_init(void)
{
    //MasterUsart_Config();
}

/* Get system runtime */
static uint32_t hal_modem_get_time(void)
{   
    return SysGetTickCount();
}

/* Write file data to flash */
static modem_err_t hal_modem_file_write(uint32_t offset, uint8_t* dst, size_t len)
{
    /* write data to flash */
    if (!FLASH_Write1kData(offset,dst,len)) {
        return HAL_MODEM_OK;   
    }else return HAL_MODEM_FAIL;

}

/* Send data to UART */
static int hal_modem_send_data(uint8_t* data, uint32_t len)
{
    uint32_t count = Usart_SendArray(MODEM_USARTx, data, len);
    return (int)count;
}

/* Read data from UART */
static modem_err_t hal_modem_getrecvdata(hal_modem_handle_t handle)
{
    usart_t* ucb = GetModemUsartRecvFlag();
    if (ucb != NULL) {
        handle->fcb.pkt_len = ucb->len;
        handle->fcb.pkt_data = ucb->data;
        return HAL_MODEM_OK;
    }else return HAL_MODEM_FAIL;    
}

static void hal_modem_clear_transport(void)
{
    /* clear uart */
    ClearModemUsartFlag();
}

/**********************************************************************************/
static uint16_t hal_modem_crc16(uint8_t *buffer, uint32_t len)
{
    uint16_t crc16 = 0;
    while(len != 0) {
        crc16  = (uint8_t)(crc16 >> 8) | (crc16 << 8);
        crc16 ^= *buffer;
        crc16 ^= (uint8_t)(crc16 & 0xff) >> 4;
        crc16 ^= (crc16 << 8) << 4;
        crc16 ^= ((crc16 & 0xff) << 4) << 1;
        buffer++;
        len--;
    }
    return crc16;
}

static modem_err_t hal_modem_check_crc(hal_modem_fcb_t* fcb)
{
    /* pack crc calculation */
    uint16_t crc = hal_modem_crc16(fcb->pkt_data + 3,fcb->pkt_len - 5);

    /* pack crc check */
    uint8_t crc_h = fcb->pkt_data[fcb->pkt_len - 2];
    uint8_t crc_l = fcb->pkt_data[fcb->pkt_len - 1];/* end is len-1 */
    if (crc_h == (uint8_t)(crc >> 8) && crc_l == (uint8_t)crc) {
        return HAL_MODEM_OK;
    }else return HAL_MODEM_FAIL;
}

static void hal_modem_clear_recv(hal_modem_handle_t handle)
{
    /* clear recv flag */
    hal_modem_clear_transport();
}

static modem_err_t hal_modem_check_recv(hal_modem_handle_t handle)
{
    if (!handle) {
        return HAL_MODEM_FAIL;
    } 

    /* get recv flag */
    if (hal_modem_getrecvdata(handle) != HAL_MODEM_OK) {
        return HAL_MODEM_FAIL;
    }

    /* check hal modem no wait */
    if (handle->wait_ch == NO_CHAR_WAIT) {
        return HAL_MODEM_OK;
    }
    
    /* check hal modem wait ch */
    hal_modem_fcb_t* fcb = &handle->fcb;   
    if (handle->wait_ch != fcb->pkt_data[0]) {
        LOGI("ERR recv_data:%02x data_len:%d waitch:0x%02x\r\n",
                                    fcb->pkt_data[0],fcb->pkt_len,handle->wait_ch);
        hal_modem_clear_recv(handle);
        return HAL_MODEM_FAIL; /* 无效接收 */
    }else return HAL_MODEM_OK;
}

static modem_err_t hal_modem_check_timeout(hal_modem_handle_t handle)
{
    if (!handle) {
        return HAL_MODEM_FAIL;
    }

    /* 超时未处理或NO_TIME_WAIT不检查新的超时 */
    if (handle->_timeout || handle->waitime == NO_TIME_WAIT) {
        return HAL_MODEM_FAIL;
    }
    
    if(handle->waitime < hal_modem_get_time()) {
        handle->_timeout = 1;/* flag timeout */
        return HAL_MODEM_OK;
    }else return HAL_MODEM_FAIL;   
}

static void hal_modem_clear_timeout(hal_modem_handle_t handle)
{
    handle->_timeout = 0;
}

static void hal_modem_set_wait(hal_modem_handle_t handle , uint8_t ch , uint32_t s)
{
    handle->wait_ch = ch;

    if (s == NO_TIME_WAIT) { /* NO_TIME_WAIT 为不设置等待延时 */
        handle->waitime = NO_TIME_WAIT;
    }else {
        handle->waitime = hal_modem_get_time() + s*1000;
    }

    /* clear wait flag */  
    hal_modem_clear_timeout(handle);
}

static void hal_modem_stop(hal_modem_handle_t* p)
{
    free(*p);
    *p = NULL;
}

static modem_err_t hal_modem_dispatch_event(hal_modem_handle_t handle, 
                                                hal_modem_event_id_t evt_id)
{
    if (!handle || !handle->modem_event_handle_cb) {
        return HAL_MODEM_FAIL;
    }

    if (handle->modem_event_handle_cb) {
        handle->evt_id = evt_id;
        return handle->modem_event_handle_cb(handle);
    }
    
    return HAL_MODEM_OK;
}

static void hal_modem_set_state(hal_modem_handle_t handle , hal_modem_state_t state)
{
    handle->state = state;
}

static void hal_modem_send_ch(hal_modem_handle_t handle, uint8_t sendch,
                                                    uint8_t ch, uint32_t time )
{
    /* set ch */
    hal_modem_send_data(&sendch,1);

    /* set wait */
    hal_modem_set_wait(handle,ch,time);
}

static modem_err_t hal_modem_pkt_unpack(hal_modem_handle_t handle)
{
    hal_modem_fcb_t* fcb = &handle->fcb;

    /* pack crc check */
    modem_err_t err = hal_modem_check_crc(fcb);
    if (err != HAL_MODEM_OK) {
        return err;
    }

    /* save to flash */
    err = hal_modem_file_write(fcb->file_offset,fcb->pkt_data+3,fcb->pkt_len-5);
    if (err != HAL_MODEM_OK) {
        return err;
    }else {
        LOGI("HAL_MODEM WRITE DOWN AT: 0x%08x \r\n",fcb->file_offset);
        fcb->file_offset = fcb->file_offset + 1024;
        return HAL_MODEM_OK;
    }
}

static void hal_modem_ymodem1krecver_handle(hal_modem_handle_t handle)
{
    hal_modem_fcb_t* fcb = &handle->fcb;
    handle->cycle = DEFAULT_RETRY;
    
    LOGI("*******\r\n");
        for (size_t i = 0; i < fcb->pkt_len; i++){
            LOGI("%02x ",fcb->pkt_data[i]);
        }
    LOGI("\r\n*******\n");  

    if (handle->role == YOMODEM_1K_RECEIVER) {
        switch (fcb->pkt_data[0]) {
            case STX:{
                if (handle->state == HAL_MODEM_STATE_ON_FILE) {
                    hal_modem_pkt_unpack(handle);
                    hal_modem_send_ch(handle,ACK,NO_CHAR_WAIT,3);
                }
            }break;
            case SOH:{
                if (handle->state == HAL_MODEM_STATE_ON_FILE) {
                    hal_modem_pkt_unpack(handle);
                    hal_modem_send_ch(handle,ACK,NO_CHAR_WAIT,3);
                }
                if (handle->state == HAL_MODEM_STATE_CONNECTING) {
                    hal_modem_set_state(handle,HAL_MODEM_STATE_CONNECTED);
                    hal_modem_dispatch_event(handle,HAL_MODEM_EVENT_CONNECTED);
                    LOGI("******** HAL MODEM CONNECTED ********\r\n");
                    hal_modem_set_state(handle,HAL_MODEM_STATE_ON_FILE);
                    hal_modem_dispatch_event(handle,HAL_MODEM_EVENT_ON_FILE);
                    LOGI("******** HAL MODEM STARTING RECVING FILE ********\r\n");
                    hal_modem_send_ch(handle,ACK,NO_CHAR_WAIT,3);
                    hal_modem_send_ch(handle,CRC16,NO_CHAR_WAIT,3);
                }
                if (handle->state == HAL_MODEM_STATE_RECEIVER_RECEIVE_EOT) {
                    hal_modem_send_ch(handle,ACK,NO_CHAR_WAIT,3);
                    hal_modem_set_state(handle,HAL_MODEM_STATE_FINISH);
                    hal_modem_dispatch_event(handle,HAL_MODEM_EVENT_FINISHED);
                    LOGI("******** HAL MODEM FINISHED ********\r\n");
                    jump_to_application();
                }
            }break;
            case EOT:{
                if (handle->state == HAL_MODEM_STATE_RECEIVER_RECEIVE_EOT) {
                    hal_modem_send_ch(handle,ACK,NO_CHAR_WAIT,3);
                    hal_modem_send_ch(handle,CRC16,NO_CHAR_WAIT,3);
                }
                if (handle->state == HAL_MODEM_STATE_ON_FILE) {
                    hal_modem_set_state(handle,HAL_MODEM_STATE_RECEIVER_RECEIVE_EOT);
                    LOGI("******** HAL MODEM RECVING FILE COMPLETED ********\r\n");
                    hal_modem_send_ch(handle,NAK,EOT,3);
                }
            }break;
            default:
                break;
        }
    }
}

static modem_err_t hal_ymodem_timeout_handle(hal_modem_handle_t handle)
{
    if (!handle->cycle -- ) {
        hal_modem_set_state(handle,HAL_MODEM_STATE_ERROR);
        hal_modem_dispatch_event(handle,HAL_MODEM_EVENT_ERROR);
        LOGI("******** HAL MODEM ERROR ********\r\n");
        handle->waitime = NO_TIME_WAIT;/* stop immediately */
        return HAL_MODEM_FAIL;
    }

    switch (handle->state) {
        case HAL_MODEM_STATE_CONNECTING:{
            hal_modem_send_ch(handle,CRC16,SOH,1);
        }break;

        default:
            break;
    }
    return HAL_MODEM_OK;
}

hal_modem_handle_t hal_modem_start(hal_modem_config_t* config)
{
    assert_param(config != NULL);

    hal_modem_handle_t handle = calloc(1,sizeof(hal_modem_t));
    if (!handle) {
        LOGI("malloc handle fail\r\n");
        return NULL;
    }

    hal_modem_io_init();

    if(config->role == YOMODEM_1K_RECEIVER) {
        handle->role = config->role;
        handle->fcb.file_offset = config->offset;
        handle->cycle = DEFAULT_RETRY;
        hal_modem_set_state(handle,HAL_MODEM_STATE_CONNECTING);
        hal_modem_clear_transport();
        hal_modem_send_ch(handle,CRC16,SOH,1);
    }

    return handle;
}

modem_err_t hal_modem_machine_run(hal_modem_handle_t* handle_addr)
{    
    hal_modem_handle_t handle = *handle_addr;

    if (handle == NULL) {
        return HAL_MODEM_FAIL;
    }

    if(hal_modem_check_recv(handle) == HAL_MODEM_OK) {
        hal_modem_ymodem1krecver_handle(handle);
        hal_modem_clear_recv(handle);
        return HAL_MODEM_OK;
    }

    if(hal_modem_check_timeout(handle) == HAL_MODEM_OK) {
        hal_ymodem_timeout_handle(handle);
        hal_modem_clear_timeout(handle);
        return HAL_MODEM_OK;
    }

    if (handle->state == HAL_MODEM_STATE_FINISH || handle->state == HAL_MODEM_STATE_ERROR) {
        hal_modem_stop(handle_addr);
    }

    return HAL_MODEM_OK;
}

