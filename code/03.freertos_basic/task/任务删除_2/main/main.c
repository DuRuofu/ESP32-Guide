#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "main";

// 任务函数
void myTask(void *pvParameters)
{
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "myTask:1");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "myTask:2");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "myTask:3");

    // 删除任务(如果传递 NULL，则删除当前任务)
    vTaskDelete(NULL);
}

void app_main(void)
{
    // 任务句柄
    TaskHandle_t taskHandle = NULL;
    // 创建一个 FreeRTOS 任务
    xTaskCreate(myTask, "myTask", 2048, NULL, 1, &taskHandle);
}
