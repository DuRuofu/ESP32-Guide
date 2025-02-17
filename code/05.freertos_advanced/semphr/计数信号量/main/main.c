// 计数型信号量(占座)
#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

static const char *TAG = "main";

// 信号量
SemaphoreHandle_t semaphoreHandle;


// 占座任务
void task1(void *pvParameters)
{
    // 定义空位
    int seat = 0;
    for (;;)
    {
        // 获取信号量
        seat = uxSemaphoreGetCount(semaphoreHandle);
        // 输出空位
        ESP_LOGI(TAG, "当前空位:%d", seat);

        // 获取信号量(占座)
        if (xSemaphoreTake(semaphoreHandle, portMAX_DELAY) == pdPASS)
        {
            ESP_LOGI(TAG, "占座成功");
        }
        else
        {
            ESP_LOGI(TAG, "占座失败");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// 离开座位任务
void task2(void *pvParameters)
{
    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(6000));
        // 释放信号量
        xSemaphoreGive(semaphoreHandle);
        ESP_LOGI(TAG, "释放座位");
    }
}

void app_main(void)
{
    semaphoreHandle = xSemaphoreCreateCounting(5, 5);

    // 创建占座任务
    xTaskCreate(task1, "task1", 1024 * 2, NULL, 10, NULL);
    // 创建离开座位任务
    xTaskCreate(task2, "task2", 1024 * 2, NULL, 10, NULL);
}
