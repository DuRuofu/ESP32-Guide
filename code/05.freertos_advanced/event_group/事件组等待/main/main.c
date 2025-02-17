// 事件组
#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

static const char *TAG = "main";

EventGroupHandle_t xCreatedEventGroup;

#define BIT_0 (1 << 0)
#define BIT_4 (1 << 4)

void task1(void *pvParameters)
{
    ESP_LOGI(TAG, "-------------------------------");
    ESP_LOGI(TAG, "task1启动!");

    while (1)
    {
        EventBits_t uxBits;
        uxBits = xEventGroupWaitBits(
            xCreatedEventGroup, /* The event group being tested. */
            BIT_0 | BIT_4,      /* The bits within the event group to wait for. */
            pdTRUE,             /* BIT_0 & BIT_4 should be cleared before returning. */
            pdFALSE,            /* Don't wait for both bits, either bit will do. */
            portMAX_DELAY);     /* Wait a maximum of 100ms for either bit to be set. */

        if ((uxBits & (BIT_0 | BIT_4)) == (BIT_0 | BIT_4))
        {
            ESP_LOGI(TAG, "BIT_0 和 BIT_4 都被设置了");
        }
        else
        {
            ESP_LOGI(TAG, "BIT_0 和 BIT_4 有一个被设置了");
        }
    }
}

void task2(void *pvParameters)
{
    ESP_LOGI(TAG, "task2启动!");
    vTaskDelay(pdMS_TO_TICKS(1000));
    while (1)
    {
        xEventGroupSetBits(xCreatedEventGroup, BIT_0);
        ESP_LOGI(TAG, "BIT_0 被设置");
        vTaskDelay(pdMS_TO_TICKS(3000));
        xEventGroupSetBits(xCreatedEventGroup, BIT_4);
        ESP_LOGI(TAG, "BIT_4 被设置");
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void app_main(void)
{

    // 创建事件组
    xCreatedEventGroup = xEventGroupCreate();

    if (xCreatedEventGroup == NULL)
    {
        ESP_LOGE(TAG, "创建事件组失败");
    }
    else
    {
        xTaskCreate(task1, "task1", 1024 * 2, NULL, 1, NULL);
        xTaskCreate(task2, "task2", 1024 * 2, NULL, 1, NULL);
    }
}
