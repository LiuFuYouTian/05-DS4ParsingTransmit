#ifndef ESPNOW_TRAMSMIT_H
#define ESPNOW_TRAMSMIT_H

#include <stdio.h>
#include "esp_now.h"

typedef enum {
    ESPNOW_CHANNEL_00 = 0x0,
    ESPNOW_CHANNEL_01,
    ESPNOW_CHANNEL_02,    
    ESPNOW_CHANNEL_03,    
    ESPNOW_CHANNEL_04,    
    ESPNOW_CHANNEL_05,
    ESPNOW_CHANNEL_06,
    ESPNOW_CHANNEL_07,    
    ESPNOW_CHANNEL_08,    
    ESPNOW_CHANNEL_09,    
    ESPNOW_CHANNEL_10,
    ESPNOW_CHANNEL_11,
    ESPNOW_CHANNEL_12,    
    ESPNOW_CHANNEL_13,    
    ESPNOW_CHANNEL_14,    
    ESPNOW_CHANNEL_MAX,
} espnow_channel_x;

typedef struct {
    uint8_t enpnow_need_pairing;
    uint8_t enpnow_channel;
    uint8_t dest_mac[ESP_NOW_ETH_ALEN];   //MAC address of destination device.
    wifi_phy_rate_t enpnow_rate;


    esp_now_send_cb_t espnow_send_cb;
    esp_now_recv_cb_t espnow_recv_cb;
} espnow_transmit_t;

/**
 * @brief espnow通讯控制任务
 * @note 
 *
 * @param 
 * @param 
 * @return 
 */
void espnow_teansmit_task(void *pvParameters);

/**
 * @brief espnow数据发送
 * @note 
 *
 * @param 
 * @param 
 * @return 
  *          - ESP_OK : succeed
  *          - ESP_ERR_ESPNOW_NOT_INIT : ESPNOW is not initialized
  *          - ESP_ERR_ESPNOW_ARG : invalid argument
  *          - ESP_ERR_ESPNOW_INTERNAL : internal error
  *          - ESP_ERR_ESPNOW_NO_MEM : out of memory, when this happens, you can delay a while before sending the next data
  *          - ESP_ERR_ESPNOW_NOT_FOUND : peer is not found
  *          - ESP_ERR_ESPNOW_IF : current WiFi interface doesn't match that of peer
 */
esp_err_t espnow_transmit_send(uint8_t *data, int len);

/**
 * @brief espnow控制当前是否进行配对设置
 * @note 
 *
 * @param pairing_state：true开启配对
 * @param 
 * @return 
 */
void espnow_set_pairing(uint8_t pairing_state);

/**
 * @brief espnow获取当前是否进行配对设置
 * @note 
 *
 * @param 
 * @param 
 * @return true：开启配对 false:关闭配对
 */
uint8_t espnow_get_pairing(void);

#endif
