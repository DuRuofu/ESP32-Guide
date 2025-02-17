#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define LED_GPIO GPIO_NUM_2
#define BUTTON_GPIO GPIO_NUM_0

void app_main(void)
{
    // 初始化LED GPIO为输出模式
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    // 初始化按钮GPIO为输入模式
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_GPIO, GPIO_PULLUP_ONLY);

    while (1) {
        // 读取按钮状态
        int button_state = gpio_get_level(BUTTON_GPIO);

        // 如果按钮被按下（逻辑低电平），点亮LED
        if(button_state == 0) {
            gpio_set_level(LED_GPIO, 1);
        } else {
            gpio_set_level(LED_GPIO, 0);
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
