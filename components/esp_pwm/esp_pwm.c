#include <stdio.h>
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_pwm.h"

void esp_pwm_init(esp_pwm_t * _esp_pwm_t)
{
    ledc_timer_config_t ledc_timer;
    ledc_channel_config_t ledc_channel;

    ledc_timer.speed_mode       = _esp_pwm_t->speed_mode;
    ledc_timer.timer_num        = _esp_pwm_t->timer_num;
    ledc_timer.duty_resolution  = _esp_pwm_t->duty_resolution;
    ledc_timer.freq_hz          = _esp_pwm_t->freq_hz;
    ledc_timer.clk_cfg          = LEDC_AUTO_CLK;

    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    for (size_t i = 0; i < LEDC_CHANNEL_MAX; i++)
    {
        if(_esp_pwm_t->gpio_num[i] < GPIO_NUM_MAX)
        {
            ledc_channel.speed_mode = _esp_pwm_t->speed_mode;
            ledc_channel.channel    = i;
            ledc_channel.timer_sel  = _esp_pwm_t->timer_num;
            ledc_channel.intr_type  = LEDC_INTR_DISABLE;
            ledc_channel.gpio_num   = _esp_pwm_t->gpio_num[i];
            ledc_channel.duty       = 0;
            ledc_channel.hpoint     = 0;
            ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
        }
    }
}

uint8_t esp_pwm_set(esp_pwm_t * _esp_pwm_t,ledc_channel_t channel, uint32_t duty)
{
    if(_esp_pwm_t->gpio_num[channel] >= GPIO_NUM_MAX) return false;
    // Set duty
    ESP_ERROR_CHECK(ledc_set_duty(_esp_pwm_t->speed_mode, channel, duty));
    // Update duty to apply the new value
    ESP_ERROR_CHECK(ledc_update_duty(_esp_pwm_t->speed_mode, channel));

    return true;
}