// 互斥量(互斥锁Mutex):互斥锁和二进制信号量极为相似，但 有一些细微差异：互斥锁具有优先级继承机制， 但二进制信号量没有。
#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

static const char *TAG = "main";

SemaphoreHandle_t mutexHandle;

void task1(void *pvParameters)
{
    ESP_LOGI(TAG, "task1启动!");
    while (1)
    {
        if (xSemaphoreTake(mutexHandle, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGI(TAG, "task1获取到互斥量!");
            for (int i = 0; i < 10; i++)
            {
                ESP_LOGI(TAG, "task1执行中...%d", i);
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
            xSemaphoreGive(mutexHandle);
            ESP_LOGI(TAG, "task1释放互斥量!");
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        else
        {
            ESP_LOGI(TAG, "task1获取互斥量失败!");
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}

void task2(void *pvParameters)
{
    ESP_LOGI(TAG, "task2启动!");
    vTaskDelay(pdMS_TO_TICKS(1000));
    while (1)
    {
    }
}

void task3(void *pvParameters)
{
    ESP_LOGI(TAG, "task3启动!");
    vTaskDelay(pdMS_TO_TICKS(1000));
    while (1)
    {
        if (xSemaphoreTake(mutexHandle, 1000) == pdPASS)
        {
            ESP_LOGI(TAG, "task3获取到互斥量!");
            for (int i = 0; i < 10; i++)
            {
                ESP_LOGI(TAG, "task3执行中...%d",i);
                vTaskDelay(pdMS_TO_TICKS(1000));
            }

            xSemaphoreGive(mutexHandle);
            ESP_LOGI(TAG, "task3释放互斥量!");
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
        else
        {
            ESP_LOGI(TAG, "task3未获取到互斥量!");
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}

void app_main(void)
{
    mutexHandle = xSemaphoreCreateMutex();
    // mutexHandle = xSemaphoreCreateBinary();
    xTaskCreate(task1, "task1", 1024 * 2, NULL, 1, NULL);
    xTaskCreate(task2, "task2", 1024 * 2, NULL, 2, NULL);
    xTaskCreate(task3, "task3", 1024 * 2, NULL, 3, NULL);
}
