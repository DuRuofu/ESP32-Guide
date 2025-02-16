# 任务通知

任务通知（Task Notifications）是一种轻量级的任务间通信和同步机制，它比队列或事件组更加高效，因为它不需要动态分配内存。每个任务都内置了一个任务通知值，其他任务或中断服务例程（ISR）可以用它来通知该任务事件的发生。

**特点**：

- 每个任务都有一个任务通知值（32 位整数），可以用于存储信息。
- 任务通知值默认初始化为 0。
- 一个任务可以通过 通知函数 来操作另一个任务的通知值。
- 可以将任务通知值用作二值信号量、计数信号量或简单的 32 位变量。
- 当一个任务收到通知时，它可以选择阻塞（等待通知）或处理该通知。

## 1. API说明：

任务通知主要涉及以下几个 API：

| 函数名                          | 功能                                           |
| ------------------------------- | --------------------------------------------- |
| xTaskNotify                     | 向指定任务发送通知，通知值可以被覆盖               |
| xTaskNotifyGive                 | 简化版通知函数，用于发送信号量通知                  |
| xTaskNotifyWait                 | 等待通知值更新，并选择是否清除通知值               |
| ulTaskNotifyTake                | 等待任务通知并自动减少通知值（通常用于计数信号量）     |
| xTaskNotifyStateClear           | 清除任务的通知状态                               |
### xTaskNotify

**功能**： 向指定任务发送通知，并修改该任务的通知值。

**原型**：

```c
BaseType_t xTaskNotify(
    TaskHandle_t xTaskToNotify,
    uint32_t ulValue,
    eNotifyAction eAction
);
```
**参数**：
- xTaskToNotify：接收通知的任务句柄。
- ulValue：要发送的通知值。
- eAction：通知值的操作类型，可以是以下值：
- eSetBits：设置通知值的指定位。
- eIncrement：通知值递增。
- eSetValueWithOverwrite：覆盖通知值。
- eSetValueWithoutOverwrite：如果通知值未处理，则不覆盖当前值。
**返回值**：
- pdPASS：操作成功。
- pdFAIL：操作失败（通常在 eSetValueWithoutOverwrite 时发生）。

**示例**：

```c
xTaskNotify(xTaskHandle, 0x01, eSetBits); // 设置任务通知值的第 0 位
```
### xTaskNotifyGive

**功能**： 向任务发送一个 "信号量风格" 的通知，等效于 xTaskNotify() 的 eIncrement 模式。

**原型**：

```c

void xTaskNotifyGive(TaskHandle_t xTaskToNotify);
```
**参数**：
- xTaskToNotify：接收通知的任务句柄。
示例：
```c
xTaskNotifyGive(xTaskHandle); // 发送通知，通知值递增 1
```
### ulTaskNotifyTake

**功能**： 任务等待通知，并在接收到通知时自动减少通知值（通常用于实现计数信号量）。

**原型**：

```c
uint32_t ulTaskNotifyTake(BaseType_t xClearCountOnExit, TickType_t xTicksToWait);
```
**参数**：
- xClearCountOnExit：
  pdTRUE：退出等待时将通知值清零。
  pdFALSE：退出等待时保留剩余通知值。
- xTicksToWait：等待通知的时间（Tick 数，portMAX_DELAY 表示无限等待）。
**返回值**：返回通知值（如果是计数信号量，表示剩余信号量计数）。

**示例**：

```c
if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) > 0) {
    // 收到通知，处理任务
}
```
###  xTaskNotifyWait
**功能**： 任务等待通知，可以选择获取通知值的内容，并决定是否清除通知值。
**原型**：

```c

BaseType_t xTaskNotifyWait(
    uint32_t ulBitsToClearOnEntry,
    uint32_t ulBitsToClearOnExit,
    uint32_t *pulNotificationValue,
    TickType_t xTicksToWait
);
```
**参数**：

- ulBitsToClearOnEntry：进入等待时清除的通知值位。
- ulBitsToClearOnExit：退出等待时清除的通知值位。
- pulNotificationValue：保存通知值的指针。
- xTicksToWait：等待时间（Tick 数）。
**返回值**：
- pdTRUE：收到通知。
- pdFALSE：超时未收到通知。
**示例**：
```c
uint32_t ulNotificationValue;
if (xTaskNotifyWait(0x00, 0xFFFFFFFF, &ulNotificationValue, portMAX_DELAY) == pdTRUE) {
    // 收到通知，ulNotificationValue 保存通知值
}
```
### xTaskNotifyStateClear
**功能**： 清除任务的通知状态（即标记任务为“未通知”）。

**原型**：
```c
void vTaskNotifyStateClear(TaskHandle_t xTask);
```
**参数**：xTask：需要清除通知状态的任务句柄。
**示例**：

```c
vTaskNotifyStateClear(xTaskHandle); // 清除通知状态
```

## 2. 示例代码：

### 1.直接任务通知

由task2控制task1的运行：
```c
// 事件组
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

    while (1)
    {
        ESP_LOGI(TAG, "task1: 等待task通知");
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        ESP_LOGI(TAG, "task1: 收到task通知");
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
        ESP_LOGI(TAG, "task2: 发送task通知");
        xTaskNotifyGive(xTask1);
    }
}

void app_main(void)
{

    xTaskCreate(task1, "task1", 1024 * 2, NULL, 1, &xTask1);
    xTaskCreate(task2, "task2", 1024 * 2, NULL, 1, &xTask2);
}
```

### 2.任务通知值

 任务通知值按位判断-代替队列邮箱或者事件组

```c
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

```
