#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "main";

// 任务函数
void myTask(void *pvParameters)
{
    for (;;)
    {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "myTask");
    }
}

void app_main(void)
{
    // 任务句柄
    TaskHandle_t taskHandle = NULL;
    // 创建一个 FreeRTOS 任务
    xTaskCreate(myTask, "myTask", 2048, NULL, 1, &taskHandle);

    vTaskDelay(2000 / portTICK_PERIOD_MS);

    // 删除任务
    if (taskHandle != NULL)
    {
        vTaskDelete(taskHandle);
    }
}
