





注意：与原生 FreeRTOS 不同，在 ESP-IDF 中使用 FreeRTOS 的用户 永远不应调用 `vTaskStartScheduler()` 和 `vTaskEndScheduler()`。相反，ESP-IDF 会自动启动 FreeRTOS。用户必须定义一个 `void app_main(void)` 函数作为用户应用程序的入口点，并在 ESP-IDF 启动时被自动调用。

通常，用户会从 `app_main` 中启动应用程序的其他任务。`app_main` 函数可以在任何时候返回（应用终止前）。`app_main` 函数由 main 任务调用。


# ESP-IDF中的FreeRTOS的基本使用

## FreeRTOS创建任务

### 1. xTaskCreate : 动态创建一个任务

当需要在运行时动态分配内存来创建任务时使用，也就是一般的正常情况。

原型：

```c
static inline BaseType_t xTaskCreate(TaskFunction_t pxTaskCode, 
									 const char *const pcName, 
									 const configSTACK_DEPTH_TYPE usStackDepth, 
									 void *const pvParameters, 
									 UBaseType_t uxPriority, 
									 TaskHandle_t *const pxCreatedTask)
```

创建一个新任务并将其添加到准备运行的任务列表中。在 FreeRTOS 实现中，任务使用两块内存。第一个块用于保存任务的数据结构。任务使用第二个块作为其堆栈。如果使用 xTaskCreate () 创建任务，那么这两个内存块将在 xTaskCreate () 函数中自动分配。

参数：

- `pxTaskCode`：指向任务入口函数的指针，任务必须设计为永不返回（即以无限循环形式实现）。
- `pcName`：任务的描述性名称，主要用于调试方便，最大长度由 `configMAX_TASK_NAME_LEN` 定义（默认 16）。
- `usStackDepth`：任务栈大小（以字节为单位）。注意，这与原生 FreeRTOS 的默认单位不同。
- `pvParameters`：传递给任务函数的参数指针（类型为void指针）。
- `uxPriority`：任务的优先级（数字越大优先级越高,最低为1）。支持 MPU 的系统中，通过设置 `portPRIVILEGE_BIT` 位可创建特权任务（例如 `(2 | portPRIVILEGE_BIT)` 表示优先级为 2 的特权任务）。
- `pxCreatedTask`：用于存储任务句柄（可选），通过句柄可以引用创建的任务。


## FreeRTOS删除任务


## FreeRTOS创建任务传参


## FreeRTOS任务优先级

文档：https://www.freertos.org/zh-cn-cmn-s/Documentation/02-Kernel/02-Kernel-features/01-Tasks-and-co-routines/03-Task-priorities

每个任务均被分配了从 0 到 ( configMAX_PRIORITIES - 1 ) 的优先级，其中 configMAX_PRIORITIES 定义为 FreeRTOSConfig.h。

在ESP-IDF中configMAX_PRIORITIES的值为25，所以任务优先级为0-24.

使用代码
```c
xTaskCreate(Task_1, "Task_1", 2048, NULL, 25, &taskHandle);
```

产生报错如下：

![](attachments/Pasted%20image%2020250106200143.png)

获取任务优先级的函数：

```c
UBaseType_t uxTaskPriorityGet( const TaskHandle_t xTask );
```

修改优先级别的函数：
```c
void vTaskPrioritySet( TaskHandle_t xTask,
                       UBaseType_t uxNewPriority );
```

参数：
- xTask
  正在设置优先级的任务的句柄。空句柄会设置调用任务的优先级。

- uxNewPriority
   将要设置任务的优先级。应断言优先级低于 configMAX_PRIORITIES。 如果 configASSERT未定义，则优先级默认上限为 (configMAX_PRIORITIES - 1)。

## FreeRTOS任务挂起

FreeRTOS任务状态：https://www.freertos.org/zh-cn-cmn-s/Documentation/02-Kernel/02-Kernel-features/01-Tasks-and-co-routines/02-Task-states

![](attachments/Pasted%20image%2020250106202323.png)

- **运行**
  当任务实际执行时，它被称为处于运行状态。任务当前正在使用处理器。 如果运行 RTOS 的处理器只有一个内核， 那么在任何给定时间内都只能有一个任务处于运行状态。
- **准备就绪**
  准备就绪任务指那些能够执行（它们不处于阻塞或挂起状态）， 但目前没有执行的任务， 因为同等或更高优先级的不同任务已经处于运行状态。
- **阻塞**
  如果任务当前正在等待时间或外部事件，则该任务被认为处于阻塞状态。 例如，如果一个任务调用vTaskDelay()，它将被阻塞（被置于阻塞状态）， 直到延迟结束——一个时间事件。 任务也可以通过阻塞来等待队列、信号量、事件组、通知或信号量 事件。处于阻塞状态的任务通常有一个"超时"期， 超时后任务将被超时，并被解除阻塞， 即使该任务所等待的事件没有发生。“阻塞”状态下的任务不使用任何处理时间，不能 被选择进入运行状态。
- **挂起**
  与“阻塞”状态下的任务一样， “挂起”状态下的任务不能 被选择进入运行状态，但处于挂起状态的任务 没有超时。相反，任务只有在分别通过 vTaskSuspend() 和 xTaskResume() API 调用明确命令时 才会进入或退出挂起状态。

相关函数：


## 任务看门狗定时器 (TWDT)

https://docs.espressif.com/projects/esp-idf/zh_CN/stable/esp32/api-reference/system/wdts.html
https://www.bilibili.com/video/BV1jF411B7gv/?spm_id_from=333.1007.top_right_bar_window_history.content.click&vd_source=ef5a0ab0106372751602034cdd9ab98e