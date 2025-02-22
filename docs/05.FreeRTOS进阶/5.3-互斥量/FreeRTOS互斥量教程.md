# FreeRTOS进阶—互斥锁

> [!TIP] 🚀 **FreeRTOS 互斥锁 | 保证任务间共享资源的独占访问**  
> - 💡 **碎碎念**😎：本节将讲解 FreeRTOS 中的互斥锁机制，帮助你确保在多任务环境中对共享资源的独占访问，避免数据冲突。  
> - 📺 **视频教程**：🚧 *开发中*  
> - 💾 **示例代码**：🚧 *开发中*

## 1. 互斥量

互斥量(互斥锁Mutex):互斥锁和二进制信号量极为相似，但 有一些细微差异：互斥锁具有优先级继承机制， 但二进制信号量没有。
### 1.1 示例代码：

```c

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

```

## 2. 递归互斥量

非递归互斥锁只能被一个任务 获取一次，如果同一个任务想再次获取则会失败， 因为当任务第一次释放互斥锁时，互斥锁就一直处于释放状态。与非递归互斥锁相反，递归互斥锁可以被同一个任务获取很多次， 获取多少次就需要释放多少次， 此时才会返回递归互斥锁。
### 2.1  示例代码：

```c

#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

static const char *TAG = "main";

SemaphoreHandle_t mutexHandle;

void task1(void *pvParameters)
{
    ESP_LOGI(TAG, "-------------------------------");
    ESP_LOGI(TAG, "task1启动!");
    while (1)
    {
        xSemaphoreTakeRecursive(mutexHandle, portMAX_DELAY);
        ESP_LOGI(TAG, "task1获取到互斥量-使用资源A");
        for (int i = 0; i < 10; i++)
        {
            ESP_LOGI(TAG, "task1执行中...%d -使用资源A", i);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        xSemaphoreTakeRecursive(mutexHandle, portMAX_DELAY);
        ESP_LOGI(TAG, "task1获取到互斥量-使用资源B");
        for (int i = 0; i < 10; i++)
        {
            ESP_LOGI(TAG, "task1执行中...%d -使用资源B", i);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        xSemaphoreGiveRecursive(mutexHandle);
        ESP_LOGI(TAG, "task1释放互斥量-使用资源B");
        vTaskDelay(pdMS_TO_TICKS(3000));
        xSemaphoreGiveRecursive(mutexHandle);
        ESP_LOGI(TAG, "task1释放互斥量-使用资源A");
    }
    vTaskDelete(NULL);
}

void task2(void *pvParameters)
{
    ESP_LOGI(TAG, "task2启动!");
    vTaskDelay(pdMS_TO_TICKS(1000));
    while (1)
    {
        // 获取递归互斥锁
        if (xSemaphoreTakeRecursive(mutexHandle, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGI(TAG, "task2获取到互斥量");
            for (int i = 0; i < 10; i++)
            {
                ESP_LOGI(TAG, "task2执行中...%d", i);
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
            xSemaphoreGiveRecursive(mutexHandle);
            ESP_LOGI(TAG, "task2释放互斥量");
        }
        else
        {
            ESP_LOGI(TAG, "task2获取互斥量失败");
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}

void app_main(void)
{
    // 创建递归互斥量
    mutexHandle = xSemaphoreCreateRecursiveMutex();

    xTaskCreate(task1, "task1", 1024 * 2, NULL, 1, NULL);
    xTaskCreate(task2, "task2", 1024 * 2, NULL, 2, NULL);
}

```
