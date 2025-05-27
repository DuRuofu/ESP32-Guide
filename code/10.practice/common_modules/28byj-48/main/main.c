#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

// 定义 GPIO 引脚
#define PIN1 GPIO_NUM_16
#define PIN2 GPIO_NUM_17
#define PIN3 GPIO_NUM_18
#define PIN4 GPIO_NUM_19

// 电机步数和旋转延迟（越小越快）
#define STEPS_PER_REVOLUTION 4096
int rotation_speed_ms = 10; // 全局速度变量，单位ms

// 初始化 GPIO
void stepper_gpio_init(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PIN1) | (1ULL << PIN2) | (1ULL << PIN3) | (1ULL << PIN4),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 0,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    printf("步进电机 GPIO 初始化完成\n");
}

// 八步半步激励序列（半步驱动），共8步
// 顺序依次是单相激励与双相激励交替，提升平滑度
const int step_sequence[8][4] = {
    {1, 0, 0, 0}, // 1
    {1, 1, 0, 0}, // 1+2
    {0, 1, 0, 0}, // 2
    {0, 1, 1, 0}, // 2+3
    {0, 0, 1, 0}, // 3
    {0, 0, 1, 1}, // 3+4
    {0, 0, 0, 1}, // 4
    {1, 0, 0, 1}  // 4+1
};

int current_step_index = 0; // 全局步进索引

// 输出对应步进序列的电平
void step_motor(int step) {
    int idx = step % 8;
    gpio_set_level(PIN1, step_sequence[idx][0]);
    gpio_set_level(PIN2, step_sequence[idx][1]);
    gpio_set_level(PIN3, step_sequence[idx][2]);
    gpio_set_level(PIN4, step_sequence[idx][3]);
}

// 顺时针旋转指定步数
void rotate_clockwise(int steps) {
    for (int i = 0; i < steps; i++) {
        current_step_index = (current_step_index + 1) % 8;
        step_motor(current_step_index);
        vTaskDelay(pdMS_TO_TICKS(rotation_speed_ms));
    }
}

// 逆时针旋转指定步数
void rotate_counterclockwise(int steps) {
    for (int i = 0; i < steps; i++) {
        current_step_index = (current_step_index - 1 + 8) % 8;
        step_motor(current_step_index);
        vTaskDelay(pdMS_TO_TICKS(rotation_speed_ms));
    }
}

// 旋转指定角度（正数顺时针，负数逆时针）
void rotate_angle(float degrees) {
    // 细分了，步数要乘以8/4=2倍
    int steps = (int)(STEPS_PER_REVOLUTION * 2 * degrees / 360.0f);
    if (steps > 0) {
        rotate_clockwise(steps);
    } else if (steps < 0) {
        rotate_counterclockwise(-steps);
    }
}

// 主任务
void stepper_task(void *arg) {
    while (1) {
        printf("顺时针旋转一整圈...\n");
        rotate_clockwise(STEPS_PER_REVOLUTION * 2);  // 注意步数乘2
        vTaskDelay(pdMS_TO_TICKS(1000));

        printf("逆时针旋转一整圈...\n");
        rotate_counterclockwise(STEPS_PER_REVOLUTION * 2);
        vTaskDelay(pdMS_TO_TICKS(1000));

        printf("顺时针旋转25度...\n");
        rotate_angle(25);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// app_main 入口
void app_main(void) {
    stepper_gpio_init();
    xTaskCreate(stepper_task, "stepper_task", 2048, NULL, 5, NULL);
}
