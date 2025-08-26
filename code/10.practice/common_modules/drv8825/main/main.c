#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"

// 定义 DRV8825 控制引脚
#define DIR_PIN  GPIO_NUM_16  // 方向控制引脚
#define EN_PIN   GPIO_NUM_17  // 使能控制引脚（低电平使能）
#define STEP_PIN GPIO_NUM_18  // 步进脉冲引脚

// 电机步数和旋转延迟（越小越快）
// DRV8825默认为全步模式，200步/圈（1.8度/步）
// 细分由外部硬件设置（MS1、MS2、MS3引脚）
#define STEPS_PER_REVOLUTION 200
int step_delay_us = 10000; // 步进脉冲延迟，单位微秒

/**
 * @brief 初始化DRV8825控制引脚
 * 配置DIR、EN、STEP三个GPIO引脚为输出模式
 */
void drv8825_gpio_init(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << DIR_PIN) | (1ULL << EN_PIN) | (1ULL << STEP_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 0,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    
    // 初始化引脚状态
    gpio_set_level(DIR_PIN, 0);   // 默认方向
    gpio_set_level(EN_PIN, 0);    // 使能电机（低电平使能）
    gpio_set_level(STEP_PIN, 0);  // 步进引脚初始为低电平
    
    printf("DRV8825 GPIO 初始化完成\n");
}

/**
 * @brief 使能或禁用DRV8825电机驱动
 * @param enable true为使能，false为禁用
 */
void drv8825_enable(bool enable) {
    gpio_set_level(EN_PIN, enable ? 0 : 1);  // 低电平使能，高电平禁用
}

/**
 * @brief 设置电机旋转方向
 * @param clockwise true为顺时针，false为逆时针
 */
void drv8825_set_direction(bool clockwise) {
    gpio_set_level(DIR_PIN, clockwise ? 1 : 0);
}

/**
 * @brief 发送一个步进脉冲
 * 产生一个高-低电平脉冲来驱动电机前进一步
 */
void drv8825_step_once(void) {
    gpio_set_level(STEP_PIN, 1);  // 拉高
    esp_rom_delay_us(10);         // 保持高电平10微秒，确保脉冲稳定
    gpio_set_level(STEP_PIN, 0);  // 拉低
    esp_rom_delay_us(step_delay_us); // 步进间隔延迟
}

/**
 * @brief 顺时针旋转指定步数
 * @param steps 要旋转的步数
 */
void rotate_clockwise(int steps) {
    drv8825_set_direction(true);  // 设置顺时针方向
    drv8825_enable(true);         // 使能电机
    
    for (int i = 0; i < steps; i++) {
        drv8825_step_once();
    }
}

/**
 * @brief 逆时针旋转指定步数
 * @param steps 要旋转的步数
 */
void rotate_counterclockwise(int steps) {
    drv8825_set_direction(false); // 设置逆时针方向
    drv8825_enable(true);         // 使能电机
    
    for (int i = 0; i < steps; i++) {
        drv8825_step_once();
    }
}

/**
 * @brief 旋转指定角度
 * @param degrees 旋转角度（正数顺时针，负数逆时针）
 */
void rotate_angle(float degrees) {
    // 根据细分设置计算实际步数
    // 全步模式：200步/圈，如果使用细分需要相应调整
    int steps = (int)(STEPS_PER_REVOLUTION * degrees / 360.0f);
    
    if (steps > 0) {
        rotate_clockwise(steps);
    } else if (steps < 0) {
        rotate_counterclockwise(-steps);
    }
}

/**
 * @brief 设置步进速度
 * @param delay_us 步进间隔延迟（微秒），值越小速度越快
 */
void drv8825_set_speed(int delay_us) {
    step_delay_us = delay_us;
}

/**
 * @brief DRV8825步进电机控制演示
 * 演示各种旋转控制功能
 */
void drv8825_demo(void) {
    printf("DRV8825 - 顺时针旋转一整圈...\n");
    rotate_clockwise(STEPS_PER_REVOLUTION);
    drv8825_enable(false);  // 旋转完成后禁用电机以节省功耗
    vTaskDelay(pdMS_TO_TICKS(2000));

    printf("DRV8825 - 逆时针旋转一整圈...\n");
    rotate_counterclockwise(STEPS_PER_REVOLUTION);
    drv8825_enable(false);
    vTaskDelay(pdMS_TO_TICKS(2000));

    printf("DRV8825 - 顺时针旋转90度...\n");
    rotate_angle(90);
    drv8825_enable(false);
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    printf("DRV8825 - 逆时针旋转90度...\n");
    rotate_angle(-90);
    drv8825_enable(false);
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // 演示速度控制
    printf("DRV8825 - 高速旋转180度...\n");
    drv8825_set_speed(600);  // 设置高速
    rotate_angle(180);
    drv8825_enable(false);
    drv8825_set_speed(1000); // 恢复默认速度
    vTaskDelay(pdMS_TO_TICKS(2000));
}

/**
 * @brief 应用程序入口
 * 初始化DRV8825并执行控制演示
 */
void app_main(void) {
    printf("DRV8825步进电机驱动器示例程序启动\n");
    printf("引脚配置: DIR=%d, EN=%d, STEP=%d\n", DIR_PIN, EN_PIN, STEP_PIN);
    printf("细分设置由外部硬件控制(MS1, MS2, MS3引脚)\n");
    
    drv8825_gpio_init();
    
    // 循环执行演示
    while (1) {
        drv8825_demo();
    }
}
