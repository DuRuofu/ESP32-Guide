// 事件组
#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

/* Bits used by the three tasks. */
#define TASK_0_BIT (1 << 0)
#define TASK_1_BIT (1 << 1)
#define TASK_2_BIT (1 << 2)

#define ALL_SYNC_BITS (TASK_0_BIT | TASK_1_BIT | TASK_2_BIT)

static const char *TAG = "main";
EventGroupHandle_t xEventBits;


void task0(void *pvParameters)
{
    ESP_LOGI(TAG, "-------------------------------");
    ESP_LOGI(TAG, "task0启动!");

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(3000));
        ESP_LOGI(TAG, "task0: 任务同步开始");
        // 事件同步
        xEventGroupSync(
            xEventBits,     /* The event group being tested. */
            TASK_0_BIT,     /* The bits within the event group to wait for. */
            ALL_SYNC_BITS,  /* The bits within the event group to wait for. */
            portMAX_DELAY); /* Wait a maximum of 100ms for either bit to be set. */

    ESP_LOGI(TAG, "task0: 任务同步完成");
    vTaskDelay(pdMS_TO_TICKS(3000));
    }
}
void task1(void *pvParameters)
{
    ESP_LOGI(TAG, "-------------------------------");
    ESP_LOGI(TAG, "task1启动!");

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(4000));
        ESP_LOGI(TAG, "task1: 任务同步开始");

        // 事件同步
        xEventGroupSync(
            xEventBits,     /* The event group being tested. */
            TASK_1_BIT,     /* The bits within the event group to wait for. */
            ALL_SYNC_BITS,  /* The bits within the event group to wait for. */
            portMAX_DELAY); /* Wait a maximum of 100ms for either bit to be set. */

        ESP_LOGI(TAG, "task1: 任务同步完成");
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
        ESP_LOGI(TAG, "task2: 任务同步开始");
        // 事件同步
        xEventGroupSync(
            xEventBits,     /* The event group being tested. */
            TASK_2_BIT,     /* The bits within the event group to wait for. */
            ALL_SYNC_BITS,  /* The bits within the event group to wait for. */
            portMAX_DELAY); /* Wait a maximum of 100ms for either bit to be set. */

        ESP_LOGI(TAG, "task2: 任务同步完成");
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}


void app_main(void)
{

    // 创建事件组
    xEventBits = xEventGroupCreate();

    if (xEventBits == NULL)
    {
        ESP_LOGE(TAG, "创建事件组失败");
    }
    else
    {
        xTaskCreate(task0, "task0", 1024 * 2, NULL, 1, NULL);
        xTaskCreate(task1, "task1", 1024 * 2, NULL, 1, NULL);
        xTaskCreate(task2, "task2", 1024 * 2, NULL, 1, NULL);
    }
}
