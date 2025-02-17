// 二进制信号量
#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

static const char *TAG = "main";

// 二进制信号量
SemaphoreHandle_t semaphoreHandle;

// 公共变量
int shareVariable = 0;

void task1(void *pvParameters)
{
    for (;;)
    {
        // 获取信号量,信号量变为0
        xSemaphoreTake(semaphoreHandle, portMAX_DELAY);
        for (int i = 0; i < 10; i++)
        {
            shareVariable++;
            ESP_LOGI(TAG, "task1 shareVariable:%d", shareVariable);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        // 释放信号量,信号量变为1
        xSemaphoreGive(semaphoreHandle);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void task2(void *pvParameters)
{
    for (;;)
    {
        // 获取信号量
        xSemaphoreTake(semaphoreHandle, portMAX_DELAY);
        for (int i = 0; i < 10; i++)
        {
            shareVariable++;
            ESP_LOGI(TAG, "task2 shareVariable:%d", shareVariable);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        // 释放信号量,信号量变为1
        xSemaphoreGive(semaphoreHandle);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void)
{
    semaphoreHandle = xSemaphoreCreateBinary();
    xSemaphoreGive(semaphoreHandle);

    // 创建任务
    xTaskCreate(task1, "task1", 1024 * 2, NULL, 10, NULL);
    xTaskCreate(task2, "task2", 1024 * 2, NULL, 10, NULL);
}
