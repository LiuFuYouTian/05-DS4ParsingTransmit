#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "ps4_control.h"
#include "esp_log.h"

#define DS4_TAG "DS4Recv"
SemaphoreHandle_t xBinarySemaphore = NULL;

uint8_t ds4_data[PS4_RECV_BUFFER_SIZE - PS4_OUT_DATA_OFFSET];

void ds4_ctrl_recv_cb(uint8_t * data,uint8_t len)
{
    if(len != PS4_RECV_BUFFER_SIZE)
    {
        return;
    }

    memcpy(ds4_data,data + PS4_OUT_DATA_OFFSET,PS4_RECV_BUFFER_SIZE - PS4_OUT_DATA_OFFSET);

    xSemaphoreGive(xBinarySemaphore);
}

ds4_ctrl_t ds4_ctrl = 
{
    .da4_mac = {0xf0,0xb6,0x1e,0x01,0x57,0x65},
    .ds4_ctrl_recv_cb = ds4_ctrl_recv_cb,
};


void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    xBinarySemaphore = xSemaphoreCreateBinary();
    ps4_control_init(&ds4_ctrl);
    while (1)
    {
        xSemaphoreTake(xBinarySemaphore, portMAX_DELAY);

        printf("DS4_DATA:");
        for (size_t i = 0; i < PS4_OUTID_MAX; i++)
        {
            printf("%d,",ps4_control_parsing(ds4_data,i));
        }

        printf("\r\n");
    }
}
