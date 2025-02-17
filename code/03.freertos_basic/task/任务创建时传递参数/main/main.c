#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "main";

typedef struct
{
    int Int;
    int Array[3];
} MyStruct;

// 任务函数1:接收整型参数
void Task_1(void *pvParameters)
{
    int *pInt = (int *)pvParameters;
    ESP_LOGI(TAG, "取得整型参数为：%d", *pInt);
    vTaskDelete(NULL);
}

// 任务函数2:接收数组参数
void Task_2(void *pvParameters)
{
    int *pArray = (int *)pvParameters;
    ESP_LOGI(TAG, "取得数组参数为：%d %d %d", *pArray, *(pArray + 1), *(pArray + 2));
    vTaskDelete(NULL);
}

// 任务函数3:接收结构体参数
void Task_3(void *pvParameters)
{
    MyStruct *pStruct = (MyStruct *)pvParameters;
    ESP_LOGI(TAG, "取得结构体参数为：%d %d %d %d", pStruct->Int, pStruct->Array[0], pStruct->Array[1], pStruct->Array[2]);
    vTaskDelete(NULL);
}

// 任务函数4:接收字符串参数
void Task_4(void *pvParameters)
{
    char *pChar = (char *)pvParameters;
    ESP_LOGI(TAG, "取得字符串参数为：%s", pChar);
    vTaskDelete(NULL);
}

int Parameters_1 = 1;
int Parameters_2[3] = {1, 2, 3};
MyStruct Parameters_3 = {1, {1, 2, 3}};
static const char *Parameters_4 = "Hello World!";

void app_main(void)
{
    // 传递整形参数
    xTaskCreate(Task_1, "Task_1", 2048, (void *)&Parameters_1, 1, NULL);

    // 传递数组参数
    xTaskCreate(Task_2, "Task_2", 2048, (void *)&Parameters_2, 1, NULL);

    // 传递结构体参数
    xTaskCreate(Task_3, "Task_3", 3048, (void *)&Parameters_3, 1, NULL);

    // 传递字符串参数(注意这里没有取地址符号&)
    xTaskCreate(Task_4, "Task_4", 3048, (void *)Parameters_4, 1, NULL);
}
