// 队列邮箱: FreeRTOS的邮箱概念跟别的RTOS不一样,它是一个队列，队列长度只有1.
// 写邮箱：新数据覆盖旧数据,读邮箱：读数据时，数据不会被移除；
// 这意味着，第一次调用时会因为无数据而阻塞，一旦曾经写入数据，以后读邮箱时总能成功。
// https : // www.bilibili.com/video/BV1zq4y1m7UK?spm_id_from=333.788.videopod.sections&vd_source=ef5a0ab0106372751602034cdd9ab98e

#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

static const char *TAG = "main";

// 写数据队列
void Task_1(void *pvParameters)
{
    // 取得队列句柄
    QueueHandle_t Mailbox = (QueueHandle_t)pvParameters;
    int i = 0;

    for (;;)
    {
        // 发送数据到队列
        if (xQueueOverwrite(Mailbox, &i) != pdPASS)
        {
            ESP_LOGI(TAG, "任务1数据发送失败");
        }
        else
        {
            ESP_LOGI(TAG, "任务1数据发送成功");
        }
        i++;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}


// 读数据队列
void Task_2(void *pvParameters)
{
    // 取得队列句柄
    QueueHandle_t Mailbox = (QueueHandle_t)pvParameters;

    int i = 0;

    for (;;)
    {
        // 读取数据
        if (xQueuePeek(Mailbox, &i, portMAX_DELAY) == pdPASS)
        {
            ESP_LOGI(TAG, "任务2数据读取成功，数据为：%d", i);
        }
        else
        {
            ESP_LOGI(TAG, "任务2数据读取失败");
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void Task_3(void *pvParameters)
{
    // 取得队列句柄
    QueueHandle_t Mailbox = (QueueHandle_t)pvParameters;
    int i = 0;

    for (;;)
    {
        // 读取数据
        if (xQueuePeek(Mailbox, &i, portMAX_DELAY) == pdPASS)
        {
            ESP_LOGI(TAG, "任务3数据读取成功，数据为：%d", i);
        }
        else
        {
            ESP_LOGI(TAG, "任务3数据读取失败");
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void Task_4(void *pvParameters)
{
    // 取得队列句柄
    QueueHandle_t Mailbox = (QueueHandle_t)pvParameters;
    int i = 0;

    for (;;)
    {
        // 读取数据
        if (xQueuePeek(Mailbox, &i, portMAX_DELAY) == pdPASS)
        {
            ESP_LOGI(TAG, "任务4数据读取成功，数据为：%d", i);
        }
        else
        {
            ESP_LOGI(TAG, "任务4数据读取失败");
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void app_main(void)
{
    QueueHandle_t Mailbox; // 创建邮箱

    // 创建队列(注意：队列长度只有1)
    Mailbox = xQueueCreate(1, sizeof(int));

    if ((Mailbox != NULL) )
    {
        ESP_LOGI(TAG, "队列创建成功");
        // 写数据任务
        xTaskCreate(Task_1, "Task_1", 1024 * 4, (void *)Mailbox, 2, NULL);
        xTaskCreate(Task_2, "Task_2", 1024 * 4, (void *)Mailbox, 1, NULL);
        xTaskCreate(Task_3, "Task_3", 1024 * 4, (void *)Mailbox, 1, NULL);
        xTaskCreate(Task_4, "Task_4", 1024 * 4, (void *)Mailbox, 1, NULL);
    }
    else
    {
        ESP_LOGI(TAG, "队列创建失败");
    }
}
