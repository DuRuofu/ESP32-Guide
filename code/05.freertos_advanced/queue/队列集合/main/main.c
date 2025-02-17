// 队列集合:
// https : // www.bilibili.com/video/BV1zq4y1m7UK?spm_id_from=333.788.videopod.sections&vd_source=ef5a0ab0106372751602034cdd9ab98e

#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

static const char *TAG = "main";

void Task_1(void *pvParameters)
{
    // 取得队列句柄
    QueueHandle_t xQueue = (QueueHandle_t)pvParameters;
    int i = 111;

    for (;;)
    {
        // 发送数据到队列
        if (xQueueSend(xQueue, &i, 0) != pdPASS)
        {
            ESP_LOGI(TAG, "任务1数据发送失败");
        }
        else
        {
            ESP_LOGI(TAG, "任务1数据发送成功");
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void Task_2(void *pvParameters)
{
    // 取得队列句柄
    QueueHandle_t xQueue = (QueueHandle_t)pvParameters;
    int i = 222;

    for (;;)
    {
        // 发送数据到队列
        if (xQueueSend(xQueue, &i, 0) != pdPASS)
        {
            ESP_LOGI(TAG, "任务2数据发送失败");
        }
        else
        {
            ESP_LOGI(TAG, "任务2数据发送成功");
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void Task_3(void *pvParameters)
{
    // 取得队列集合句柄
    QueueSetHandle_t xQueueSet = (QueueSetHandle_t)pvParameters;
    // 取得有数据队列句柄
    QueueSetMemberHandle_t QueueData;
    for (;;)
    {
        QueueData = xQueueSelectFromSet(xQueueSet, portMAX_DELAY);
        if (QueueData != NULL)
        {
            int i;
            if (xQueueReceive(QueueData, &i, portMAX_DELAY) != pdPASS)
            {
                ESP_LOGI(TAG, "任务3数据接收失败");
            }
            else
            {
                ESP_LOGI(TAG, "任务3数据接收成功，数据为：%d", i);
            }
        }
    }
    vTaskDelete(NULL);
}

void app_main(void)
{
    QueueHandle_t xQueue_1;
    QueueHandle_t xQueue_2;
    // 创建队列
    xQueue_1 = xQueueCreate(10, sizeof(int));
    xQueue_2 = xQueueCreate(10, sizeof(int));

    // 创建队列集合
    QueueSetHandle_t xQueueSet;
    xQueueSet = xQueueCreateSet(20);

    // 将队列添加到队列集合
    xQueueAddToSet(xQueue_1, xQueueSet);
    xQueueAddToSet(xQueue_2, xQueueSet);

    if ((xQueue_1 != NULL )&& (xQueue_2 != NULL) && (xQueueSet != NULL))
    {
        ESP_LOGI(TAG, "队列创建成功");
        // 发送数据任务
        xTaskCreate(Task_1, "Task_1", 1024 * 4, (void *)xQueue_1, 1, NULL);
        xTaskCreate(Task_2, "Task_1", 1024 * 4, (void *)xQueue_2, 1, NULL);
        // 接收数据任务
        xTaskCreate(Task_3, "Task_1", 1024 * 4, (void *)xQueueSet, 2, NULL);
    }
    else
    {
        ESP_LOGI(TAG, "队列创建失败");
    }
}
