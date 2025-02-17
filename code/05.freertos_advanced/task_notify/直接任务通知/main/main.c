// 事件组
#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "main";

static TaskHandle_t xTask1 = NULL, xTask2 = NULL;

void task1(void *pvParameters)
{
    ESP_LOGI(TAG, "-------------------------------");
    ESP_LOGI(TAG, "task1启动!");

    while (1)
    {
        ESP_LOGI(TAG, "task1: 等待task通知");
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        ESP_LOGI(TAG, "task1: 收到task通知");
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void task2(void *pvParameters)
{
    ESP_LOGI(TAG, "-------------------------------");
    ESP_LOGI(TAG, "task2启动!");

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(5000));
        ESP_LOGI(TAG, "task2: 发送task通知");
        xTaskNotifyGive(xTask1);
    }
}

void app_main(void)
{

    xTaskCreate(task1, "task1", 1024 * 2, NULL, 1, &xTask1);
    xTaskCreate(task2, "task2", 1024 * 2, NULL, 1, &xTask2);
}
