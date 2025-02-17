// 任务通知值按位判断-代替队列邮箱或者事件组
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

    uint32_t ulNotifiedValue;

    while (1)
    {
        ESP_LOGI(TAG, "task1: 等待task通知");
        xTaskNotifyWait(0x00, ULONG_MAX, &ulNotifiedValue, portMAX_DELAY);
        // 通过不同的bit位来判断通知的来源
        if ((ulNotifiedValue & 0x01) != 0)
        {
            ESP_LOGI(TAG, "task1: 收到task通知-bit0");
        }
        if ((ulNotifiedValue & 0x02) != 0)
        {
            ESP_LOGI(TAG, "task1: 收到task通知-bit1");
        }
        if ((ulNotifiedValue & 0x04) != 0)
        {
            ESP_LOGI(TAG, "task1: 收到task通知-bit2");
        }
    }
}

void task2(void *pvParameters)
{
    ESP_LOGI(TAG, "-------------------------------");
    ESP_LOGI(TAG, "task2启动!");

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(5000));
        ESP_LOGI(TAG, "task2: 发送task通知-bit0");
        xTaskNotify(xTask1, 0x01, eSetValueWithOverwrite);

        vTaskDelay(pdMS_TO_TICKS(5000));
        ESP_LOGI(TAG, "task2: 发送task通知-bit1");
        xTaskNotify(xTask1, 0x02, eSetValueWithOverwrite);

        vTaskDelay(pdMS_TO_TICKS(5000));
        ESP_LOGI(TAG, "task2: 发送task通知-bit2");
        xTaskNotify(xTask1, 0x04, eSetValueWithOverwrite);

    }
}

void app_main(void)
{

    xTaskCreate(task1, "task1", 1024 * 4, NULL, 1, &xTask1);
    xTaskCreate(task2, "task2", 1024 * 4, NULL, 1, &xTask2);
}
