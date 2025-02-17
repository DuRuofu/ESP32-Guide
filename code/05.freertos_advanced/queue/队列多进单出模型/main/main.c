// 队列多进单出: 任务1和任务2发送数据到队列，任务3接收数据  任务3的优先级高于任务1和任务2,已达到数据监听的目的
// 参考:https://www.bilibili.com/video/BV1R44y177VS/?spm_id_from=333.788.top_right_bar_window_history.content.click&vd_source=ef5a0ab0106372751602034cdd9ab98e

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
    // 取得队列句柄
    QueueHandle_t xQueue = (QueueHandle_t)pvParameters;
    for (;;)
    {
        int receivedData;
        // 使用portMAX_DELAY阻塞等待数据
        if (xQueueReceive(xQueue, &receivedData, portMAX_DELAY) != pdPASS)
        {
            ESP_LOGI(TAG, "任务3数据接收失败");
        }
        else
        {
            ESP_LOGI(TAG, "任务3数据接收成功，数据为：%d", receivedData);
        }
    }
    vTaskDelete(NULL);
}

void app_main(void)
{

    QueueHandle_t xQueue;

    // 创建队列
    xQueue = xQueueCreate(10, sizeof(int));

    if (xQueue != NULL)
    {
        ESP_LOGI(TAG, "队列创建成功");
        // 发送数据任务
        xTaskCreate(Task_1, "Task_1", 1024 * 4, (void *)xQueue, 1, NULL);
        xTaskCreate(Task_2, "Task_1", 1024 * 4, (void *)xQueue, 1, NULL);
        // 接收数据任务
        xTaskCreate(Task_3, "Task_1", 1024 * 4, (void *)xQueue, 2, NULL);
    }
    else
    {
        ESP_LOGI(TAG, "队列创建失败");
    }
}
