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
    TaskHandle_t taskHandle_1 = NULL;
    TaskHandle_t taskHandle_2 = NULL;
    
    xTaskCreate(Task_1, "Task_1", 2048, NULL, 12, &taskHandle_1);
    xTaskCreate(Task_2, "Task_1", 2048, NULL, 12, &taskHandle_2);

    
    // 输出任务列表
    static char cBuffer[512]={0};
    vTaskList(cBuffer);
    ESP_LOGI(TAG, "任务列表：\n%s", cBuffer);
}
