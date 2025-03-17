# FreeRTOS进阶—信号量

> [!TIP] 🚀 FreeRTOS 信号量 | 控制任务的执行顺序与共享资源访问 
> - 💡 **碎碎念**😎：本节将讲解 FreeRTOS 中的信号量机制，帮助你在多任务环境中有效管理资源的访问顺序，避免竞争条件。  
> - 📺 **视频教程**：🚧 *开发中*  
> - 💾 **示例代码**：[ESP32-Guide/code/05.freertos_advanced/semphr](https://github.com/DuRuofu/ESP32-Guide/tree/main/code/05.freertos_advanced/semphr)


FreeRTOS 提供了信号量和互斥锁，用于任务间的同步和资源共享管理。信号量更偏向于任务同步，而互斥锁用于保护共享资源。
## 1. 二进制信号量

二进制信号量是最基本的信号量，仅有两个状态：可用和不可用（或 1 和 0）。
通常用于任务之间或中断与任务之间的同步，当一个事件发生时，由中断或任务释放信号量，等待信号量的任务就会被唤醒。
二进制信号量适用于简单的事件通知场景，比如通知某个任务处理外部输入或完成某项任务。

### 1.1 API说明：

### 1.2 示例代码：

```c
// 二进制信号量
#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

static const char *TAG = "main";

// 二进制信号量
SemaphoreHandle_t semaphoreHandle;

// 公共变量
int shareVariable = 0;

void task1(void *pvParameters)
{
    for (;;)
    {
        // 获取信号量,信号量变为0
        xSemaphoreTake(semaphoreHandle, portMAX_DELAY);
        for (int i = 0; i < 10; i++)
        {
            shareVariable++;
            ESP_LOGI(TAG, "task1 shareVariable:%d", shareVariable);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        // 释放信号量,信号量变为1
        xSemaphoreGive(semaphoreHandle);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void task2(void *pvParameters)
{
    for (;;)
    {
        // 获取信号量
        xSemaphoreTake(semaphoreHandle, portMAX_DELAY);
        for (int i = 0; i < 10; i++)
        {
            shareVariable++;
            ESP_LOGI(TAG, "task2 shareVariable:%d", shareVariable);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        // 释放信号量,信号量变为1
        xSemaphoreGive(semaphoreHandle);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void)
{
    semaphoreHandle = xSemaphoreCreateBinary();
    xSemaphoreGive(semaphoreHandle);

    // 创建任务
    xTaskCreate(task1, "task1", 1024 * 2, NULL, 10, NULL);
    xTaskCreate(task2, "task2", 1024 * 2, NULL, 10, NULL);
}
```

## 2. 计数信号量

### 2.1 示例代码：

```c
// 计数型信号量(占座)
#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

static const char *TAG = "main";

// 信号量
SemaphoreHandle_t semaphoreHandle;


// 占座任务
void task1(void *pvParameters)
{
    // 定义空位
    int seat = 0;
    for (;;)
    {
        // 获取信号量
        seat = uxSemaphoreGetCount(semaphoreHandle);
        // 输出空位
        ESP_LOGI(TAG, "当前空位:%d", seat);

        // 获取信号量(占座)
        if (xSemaphoreTake(semaphoreHandle, portMAX_DELAY) == pdPASS)
        {
            ESP_LOGI(TAG, "占座成功");
        }
        else
        {
            ESP_LOGI(TAG, "占座失败");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// 离开座位任务
void task2(void *pvParameters)
{
    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(6000));
        // 释放信号量
        xSemaphoreGive(semaphoreHandle);
        ESP_LOGI(TAG, "释放座位");
    }
}

void app_main(void)
{
    semaphoreHandle = xSemaphoreCreateCounting(5, 5);

    // 创建占座任务
    xTaskCreate(task1, "task1", 1024 * 2, NULL, 10, NULL);
    // 创建离开座位任务
    xTaskCreate(task2, "task2", 1024 * 2, NULL, 10, NULL);
}

```
