#include <stdio.h>
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"

#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO (2) // 控制舵机的GPIO口
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_DUTY_RES LEDC_TIMER_13_BIT // 13位分辨率 -> 8192
#define LEDC_FREQUENCY (50)             // 舵机需要 50Hz 周期 (20ms)

// 占空比计算：
// 13位分辨率，满周期 = 8192（即 100%）
// 对应脉宽范围 0.5ms ~ 2.5ms -> 占周期的 2.5% ~ 12.5%
#define SERVO_MIN_DUTY_US 500  // 0.5ms
#define SERVO_MAX_DUTY_US 2500 // 2.5ms
#define SERVO_PERIOD_US 20000  // 20ms = 50Hz

static void ledc_init(void)
{
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_MODE,
        .duty_resolution = LEDC_DUTY_RES,
        .timer_num = LEDC_TIMER,
        .freq_hz = LEDC_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK};
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL,
        .timer_sel = LEDC_TIMER,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = LEDC_OUTPUT_IO,
        .duty = 0,
        .hpoint = 0};
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

// 设置舵机角度（0~180）
void set_servo_angle(uint8_t angle)
{
    if (angle > 180)
        angle = 180;

    uint32_t pulse_width_us = SERVO_MIN_DUTY_US +
                              ((SERVO_MAX_DUTY_US - SERVO_MIN_DUTY_US) * angle) / 180;

    uint32_t duty = (pulse_width_us * (1 << LEDC_DUTY_RES)) / SERVO_PERIOD_US;

    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

void app_main(void)
{
    ledc_init();

    // 依次转动 0° → 90° → 180°
    set_servo_angle(0);
    vTaskDelay(pdMS_TO_TICKS(1000));

    set_servo_angle(90);
    vTaskDelay(pdMS_TO_TICKS(1000));

    set_servo_angle(180);
    vTaskDelay(pdMS_TO_TICKS(1000));
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
