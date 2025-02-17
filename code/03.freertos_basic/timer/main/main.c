#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

static const char *TAG = "main";

// 定时器回调
void TimerCallback_1(TimerHandle_t xTimer)
{
    // 取到定时器的名字
    const char *timerName = pcTimerGetName(xTimer);
    // 取得定时器的ID
    void *timerID = pvTimerGetTimerID(xTimer);
    ESP_LOGI(TAG, "定时器名称：%s,定时器id:%d,定时器回调函数执行", timerName, (int)timerID);
}

void app_main(void)
{
    // 创建定时器句柄
    TimerHandle_t xTimer_1;
    TimerHandle_t xTimer_2;
    xTimer_1 = xTimerCreate("timer1", pdMS_TO_TICKS(1000), pdTRUE, (void *)0, TimerCallback_1);
    xTimer_2 = xTimerCreate("timer2", pdMS_TO_TICKS(2000), pdTRUE, (void *)1, TimerCallback_1);

    xTimerStart(xTimer_1, 0); // 启动定时器
    xTimerStart(xTimer_2, 0); // 启动定时器

    vTaskDelay(pdMS_TO_TICKS(5000));
    //停止定时器
    xTimerStop(xTimer_1, 0);
    xTimerStop(xTimer_2, 0);
}


// 其他常用函数
// xTimerChangePeriod()：修改定时器周期
// xTimerReset()：重置定时器
// xTimerDelete()：删除定时器
