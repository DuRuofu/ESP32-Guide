#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "main";

void Task_1(void *pvParameters)
{
    for (;;)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "Task_1");
    }
    vTaskDelete(NULL);
}

void Task_2(void *pvParameters)
{
    for (;;)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "Task_2");
    }
    vTaskDelete(NULL);
}

void app_main(void)
{
    UBaseType_t taskPriority_1 = 0;
    UBaseType_t taskPriority_2 = 0;
    TaskHandle_t taskHandle_1 = NULL;
    TaskHandle_t taskHandle_2 = NULL;
    
    xTaskCreate(Task_1, "Task_1", 2048, NULL, 12, &taskHandle_1);
    taskPriority_1 = uxTaskPriorityGet(taskHandle_1);
    ESP_LOGI(TAG, "Task_1 优先级：%d", taskPriority_1);

    xTaskCreate(Task_2, "Task_1", 2048, NULL, 12, &taskHandle_2);
    taskPriority_2 = uxTaskPriorityGet(taskHandle_2);
    ESP_LOGI(TAG, "Task_1 优先级：%d", taskPriority_2);
}
