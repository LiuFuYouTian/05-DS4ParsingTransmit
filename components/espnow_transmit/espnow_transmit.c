#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "nvs_flash.h"
#include "nvs.h"

#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_log.h"
#include "esp_crc.h"

#include "espnow_transmit.h"

static const char *TAG = "ESPNOW_TRAMSMIT";

static uint8_t espnow_broadcast_mac[ESP_NOW_ETH_ALEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
#define IS_BROADCAST_ADDR(addr) (memcmp(addr, espnow_broadcast_mac, ESP_NOW_ETH_ALEN) == 0)

esp_now_peer_info_t espnow_peer;
espnow_transmit_t * espnow_transmit = NULL;
static QueueHandle_t espnow_queue = NULL;

/**
 * @brief espnow读取falsh目标设备mac
 * @note 
 *
 * @param mac_addr：目标设备mac
 * @param 
 * @return 
 */
static esp_err_t espnow_get_nvs_dest_mac(uint8_t *mac_addr)
{
    esp_err_t err;
    char nvs_key[12];

    nvs_handle_t dest_mac_nvs_handle;
    err = nvs_open("storage", NVS_READWRITE, &dest_mac_nvs_handle);

    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        for (size_t i = 0; i < ESP_NOW_ETH_ALEN; i++)
        {
            sprintf(nvs_key,"mac_addr[%d]",i);
            err = nvs_get_u8(dest_mac_nvs_handle,nvs_key, mac_addr + i);
            if(err != ESP_OK) return err;
        }
    }

    return err;
}
/**
 * @brief espnow设置目标设备mac到falsh
 * @note 
 *
 * @param mac_addr：目标设备mac
 * @param 
 * @return 
 */
static esp_err_t espnow_set_nvs_dest_mac(uint8_t *mac_addr)
{
    esp_err_t err;
    char nvs_key[12];

    nvs_handle_t dest_mac_nvs_handle;
    err = nvs_open("storage", NVS_READWRITE, &dest_mac_nvs_handle);
    
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        for (size_t i = 0; i < ESP_NOW_ETH_ALEN; i++)
        {
            sprintf(nvs_key,"mac_addr[%d]",i);
            err = nvs_set_u8(dest_mac_nvs_handle,nvs_key,mac_addr[i]);
            if(err != ESP_OK) return err;
        }
    }

    nvs_commit(dest_mac_nvs_handle);

    return err;
}


/**
 * @brief espnow数据发送驱动回调函数
 * @note 
 *
 * @param 
 * @param 
 * @return 
 */
static void espnow_drv_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    if(espnow_transmit->espnow_send_cb != NULL && espnow_transmit->enpnow_need_pairing == false)
    {
        espnow_transmit->espnow_send_cb(mac_addr,status);
    }
}
/**
 * @brief espnow数据接收驱动回调函数
 * @note 
 *
 * @param 
 * @param 
 * @return 
 */
static void espnow_drv_recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len)
{
    if(espnow_transmit->enpnow_need_pairing == true)
    {
        if(espnow_queue != NULL)
        {
            xQueueSend(espnow_queue,recv_info->src_addr,100);
        }
        //memcpy(espnow_peer.peer_addr, recv_info->src_addr, ESP_NOW_ETH_ALEN);
    }
    else if(espnow_transmit->espnow_recv_cb != NULL)
    {
        espnow_transmit->espnow_recv_cb(recv_info,data,len);
    }
}

/**
 * @brief espnow控制当前是否进行配对设置
 * @note 
 *
 * @param true：开启配对 false:关闭配对
 * @param 
 * @return 
 */
void espnow_set_pairing(uint8_t pairing_state)
{
    if(espnow_transmit != NULL)
    {
        espnow_transmit->enpnow_need_pairing = pairing_state;
    }
}

/**
 * @brief espnow获取当前是否进行配对设置
 * @note 
 *
 * @param 
 * @param 
 * @return true：开启配对 false:关闭配对
 */
uint8_t espnow_get_pairing(void)
{
    if(espnow_transmit != NULL)
    {
        return espnow_transmit->enpnow_need_pairing;
    }
    return false;
}

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
esp_err_t espnow_transmit_send(uint8_t *data, int len)
{
    esp_err_t err = esp_now_send(espnow_peer.peer_addr, data,len);
    switch(err)
    {
        case ESP_ERR_ESPNOW_NOT_INIT:
            ESP_LOGE(TAG,"ESP_ERR_ESPNOW_NOT_INIT");
            break;
        case ESP_ERR_ESPNOW_ARG :
            ESP_LOGE(TAG,"ESP_ERR_ESPNOW_ARG");
            break;
        case ESP_ERR_ESPNOW_INTERNAL :
            ESP_LOGE(TAG,"ESP_ERR_ESPNOW_INTERNAL");
            break;
        case ESP_ERR_ESPNOW_NO_MEM :
            ESP_LOGE(TAG,"ESP_ERR_ESPNOW_NO_MEM");
            break;
        case ESP_ERR_ESPNOW_NOT_FOUND :
            ESP_LOGE(TAG,"ESP_ERR_ESPNOW_NOT_FOUND");
            break;
        case ESP_ERR_ESPNOW_IF :
            ESP_LOGE(TAG,"ESP_ERR_ESPNOW_IF");
            break;
    };
    return err;
}

/**
 * @brief espnow通讯控制任务
 * @note 
 *
 * @param 
 * @param 
 * @return 
 */
void espnow_teansmit_task(void *pvParameters)
{
    esp_err_t err;
    espnow_transmit = (espnow_transmit_t*)pvParameters;
    espnow_queue = xQueueCreate(1,6);

    espnow_transmit->enpnow_need_pairing = false;

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_start());

    ESP_ERROR_CHECK( esp_now_init() );
    ESP_ERROR_CHECK( esp_now_register_send_cb(espnow_drv_send_cb));
    ESP_ERROR_CHECK( esp_now_register_recv_cb(espnow_drv_recv_cb));
    ESP_ERROR_CHECK( esp_now_set_wake_window(65535) );
    //ESP_ERROR_CHECK( esp_now_set_pmk((uint8_t *)CONFIG_ESPNOW_PMK) );不设置使用默认PMK
    esp_wifi_config_espnow_rate(ESP_IF_WIFI_STA,espnow_transmit->enpnow_rate);
    //esp_wifi_config_espnow_rate(ESP_IF_WIFI_STA,WIFI_PHY_RATE_MCS7_SGI);
    /* Add broadcast peer information to peer list. */
    memset(&espnow_peer, 0, sizeof(esp_now_peer_info_t));
    espnow_peer.channel = espnow_transmit->enpnow_channel;
    espnow_peer.ifidx = ESP_IF_WIFI_STA;
    espnow_peer.encrypt = false;
    memcpy(espnow_peer.peer_addr,espnow_broadcast_mac, ESP_NOW_ETH_ALEN);

    ESP_ERROR_CHECK( esp_now_add_peer(&espnow_peer));//固定把广播地址加上
    //espnow_set_nvs_dest_mac(espnow_broadcast_mac);
    /*上电时先读取flsh内的目标设备mac，没有读取到则设置为广播*/
    if(espnow_get_nvs_dest_mac(espnow_peer.peer_addr) != ESP_OK)
    {
        ESP_LOGE(TAG, "espnow dest mac nvs no init");
        espnow_set_nvs_dest_mac(espnow_broadcast_mac);
        espnow_get_nvs_dest_mac(espnow_peer.peer_addr);
    }

    printf("nvs_dec_addr:");
    for (size_t i = 0; i < ESP_NOW_ETH_ALEN; i++)
    {
        printf("0x%02X ",espnow_peer.peer_addr[i]);
    }
    printf("\r\n");

    if (esp_now_is_peer_exist(espnow_peer.peer_addr) == false) 
    {
        ESP_ERROR_CHECK( esp_now_add_peer(&espnow_peer));
    }

    while (1)
    {
        if(xQueueReceive(espnow_queue,espnow_peer.peer_addr, 500 / portTICK_PERIOD_MS) == pdTRUE)
        {
            ESP_LOGD(TAG, "espnow pairing rx");

            if(IS_BROADCAST_ADDR(espnow_peer.peer_addr) == false)//收到非广播mac时，将此mac写入nvs
            {
                if (esp_now_is_peer_exist(espnow_peer.peer_addr) == false) 
                {
                    ESP_LOGI(TAG, "espnow pairing dest mac add");
                    ESP_LOGI(TAG, "espnow pairing success");
                    printf("peer_addr:");
                    for (size_t i = 0; i < ESP_NOW_ETH_ALEN; i++)
                    {
                        printf("0x%02X ",espnow_peer.peer_addr[i]);
                    }
                    printf("\r\n");
                    espnow_set_nvs_dest_mac(espnow_peer.peer_addr);
                    ESP_ERROR_CHECK( esp_now_add_peer(&espnow_peer));
                }
                else
                {
                    ESP_LOGI(TAG, "espnow pairing dest mac exist");
                }
            }
        }
        else if(espnow_transmit->enpnow_need_pairing == true)
        {
            ESP_LOGD(TAG, "espnow pairing broadcast tx");
            err = esp_now_send(espnow_broadcast_mac, espnow_broadcast_mac,ESP_NOW_ETH_ALEN);

            switch(err)
            {
                case ESP_ERR_ESPNOW_NOT_INIT:
                    ESP_LOGE(TAG,"ESP_ERR_ESPNOW_NOT_INIT");
                    break;
                case ESP_ERR_ESPNOW_ARG :
                    ESP_LOGE(TAG,"ESP_ERR_ESPNOW_ARG");
                    break;
                case ESP_ERR_ESPNOW_INTERNAL :
                    ESP_LOGE(TAG,"ESP_ERR_ESPNOW_INTERNAL");
                    break;
                case ESP_ERR_ESPNOW_NO_MEM :
                    ESP_LOGE(TAG,"ESP_ERR_ESPNOW_NO_MEM");
                    break;
                case ESP_ERR_ESPNOW_NOT_FOUND :
                    ESP_LOGE(TAG,"ESP_ERR_ESPNOW_NOT_FOUND");
                    break;
                case ESP_ERR_ESPNOW_IF :
                    ESP_LOGE(TAG,"ESP_ERR_ESPNOW_IF");
                    break;
            };
        }
        //ESP_LOGI(TAG, "espnow_teansmit_task");
    }
}