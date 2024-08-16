#ifndef ESP_PWM_H
#define ESP_PWM_H

#include "driver/ledc.h"

typedef struct {
    ledc_mode_t speed_mode;                /*!< LEDC speed speed_mode, high-speed mode or low-speed mode */
    ledc_timer_bit_t duty_resolution;      /*!< LEDC channel duty resolution */
    ledc_timer_t  timer_num;               /*!< The timer source of channel (0 - 3) */
    uint32_t freq_hz;                      /*!< LEDC timer frequency (Hz) */

    uint8_t gpio_num[LEDC_CHANNEL_MAX];    /*!< the LEDC output gpio_num, if you want to use gpio16, gpio_num = 16 */
} esp_pwm_t;



uint8_t esp_pwm_set(esp_pwm_t * _esp_pwm_t,ledc_channel_t channel, uint32_t duty);

void esp_pwm_init(esp_pwm_t * _esp_pwm_t);

#endif
