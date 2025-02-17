#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "main";

// 任务函数
void myTask(void *pvParameters)
{
	for (;;)
	{
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		ESP_LOGI(TAG, "myTask");
	}
}

void app_main(void)
{
	// 创建一个 FreeRTOS 任务
	// 参数说明：
	// 1. 任务入口函数：myTask
	// 2. 任务名称："myTask"，用于调试时标识任务
	// 3. 任务堆栈大小：2048 字节（适当分配以避免栈溢出）
	// 4. 任务参数：NULL（未传递参数）
	// 5. 任务优先级：1（优先级较低，空闲任务优先级为 0）
	// 6. 任务句柄：NULL（不需要保存任务句柄）
	xTaskCreate(myTask, "myTask", 2048, NULL, 1, NULL);
}
