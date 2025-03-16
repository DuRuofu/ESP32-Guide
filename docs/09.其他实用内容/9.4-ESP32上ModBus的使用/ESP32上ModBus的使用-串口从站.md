# ESP32上ModBus的使用-基于串口的从站


> [!TIP] 🚀 ESP32 ModBus通信 | 让设备与工业场景相适应! 
> - 💡 **碎碎念**😎：本文档记录如何实现 ESP32 + RS485模块实现ModBus从站<br/>  
> - 📺 **视频教程**：🚧 *开发中*<br/>  
> - 💾 **示例程序**：✅ [点击查看代码] <br/>  
> - 📚 **官网文档**：[API 参考 » 应用层协议 » ESP-Modbus](https://docs.espressif.com/projects/esp-idf/zh_CN/v5.3.2/esp32/api-reference/protocols/modbus.html)<br/>  
> - 💻 **官方示例程序**：[esp-idf/examples/protocols/modbus](https://github.com/espressif/esp-idf/tree/v5.3.2/examples/protocols/modbus)

## 一、ModBus介绍

ModBus 是一种串行通信协议，广泛应用于工业自动化领域。ModBus 协议简单可靠，支持主从通信，可以用于连接不同设备和传感器进行数据交换。ModBus 主要有两种传输模式：ASCII 和 RTU。在工业应用中，RTU 模式更为常见，因为它具有更高的传输效率。

ModBus 协议的基本单位是寄存器，寄存器可以存储不同类型的数据，如输入/输出状态、模拟量、计数值等。每个从设备都有一个唯一的地址，主设备通过地址来识别从设备，并与其进行通信。

ModBus 可以通过多种物理介质进行传输，常见的有：

- **RS485**：一种差分信号传输方式，适用于长距离和噪声环境，常用于工业现场设备之间的通信。
- **RS232**：一种单端信号传输方式，适用于短距离通信，通常用于计算机和设备之间的通信。
- **TCP/IP**：基于以太网的通信方式，适用于网络环境，支持远程设备的连接和通信。
- **RS422**：一种差分信号传输方式，适用于长距离和高干扰环境，通常用于设备之间的通信。

本文档介绍基于RS485的情况： 
## 二、使用官方历程

### 1. 硬件准备

- ESP32 开发板
- RS485 模块
- 接线：ESP32 的 UART 引脚连接到 RS485 模块，RS485 模块连接到 ModBus转USB模块并连接到电脑，如下：

![](attachments/Pasted%20image%2020250305155537.png)
（这里RS485 模块连接至ESP32的串口2：	Rx PIN：	GPIO16	Tx PIN：	GPIO17）
### 2. 烧录官方历程：

下载示例工程[mb_slave](https://github.com/espressif/esp-idf/tree/v5.3.2/examples/protocols/modbus/serial/mb_slave)，使用`idf.py menuconfig`命令配置工程：

![](attachments/Pasted%20image%2020250305160833.png)

这里的`UART RTS`我没有，就暂时写`0`，烧录代码如下：

![](attachments/Pasted%20image%2020250305161818.png)

示例程序中配置了以下寄存器：

``` c
// Set register values into known state
static void setup_reg_data(void)
{
    // Define initial state of parameters
    discrete_reg_params.discrete_input0 = 1;
    discrete_reg_params.discrete_input1 = 0;
    discrete_reg_params.discrete_input2 = 1;
    discrete_reg_params.discrete_input3 = 0;
    discrete_reg_params.discrete_input4 = 1;
    discrete_reg_params.discrete_input5 = 0;
    discrete_reg_params.discrete_input6 = 1;
    discrete_reg_params.discrete_input7 = 0;

    holding_reg_params.holding_data0 = 1.34;
    holding_reg_params.holding_data1 = 2.56;
    holding_reg_params.holding_data2 = 3.78;
    holding_reg_params.holding_data3 = 4.90;

    holding_reg_params.holding_data4 = 5.67;
    holding_reg_params.holding_data5 = 6.78;
    holding_reg_params.holding_data6 = 7.79;
    holding_reg_params.holding_data7 = 8.80;

    coil_reg_params.coils_port0 = 0x55;
    coil_reg_params.coils_port1 = 0xAA;

    input_reg_params.input_data0 = 1.12;
    input_reg_params.input_data1 = 2.34;
    input_reg_params.input_data2 = 3.56;
    input_reg_params.input_data3 = 4.78;

    input_reg_params.input_data4 = 1.12;
    input_reg_params.input_data5 = 2.34;
    input_reg_params.input_data6 = 3.56;
    input_reg_params.input_data7 = 4.78;
}
```

整理为表格如下：

| 寄存器类型                        | 起始地址   | 寄存器名称           | 数据类型    | 说明                      |
| ---------------------------- | ------ | --------------- | ------- | ----------------------- |
| **保持寄存器（Holding Registers）** | 0x0000 | holding_data0   | float   | 读/写，初始值 1.34            |
|                              | 0x0001 | holding_data1   | float   | 读/写，初始值 2.56            |
|                              | 0x0002 | holding_data2   | float   | 读/写，初始值 3.78            |
|                              | 0x0003 | holding_data3   | float   | 读/写，初始值 4.90            |
|                              | 0x0004 | holding_data4   | float   | 读/写，初始值 5.67            |
|                              | 0x0005 | holding_data5   | float   | 读/写，初始值 6.78            |
|                              | 0x0006 | holding_data6   | float   | 读/写，初始值 7.79            |
|                              | 0x0007 | holding_data7   | float   | 读/写，初始值 8.80            |
| **输入寄存器（Input Registers）**   | 0x0000 | input_data0     | float   | 只读，初始值 1.12             |
|                              | 0x0001 | input_data1     | float   | 只读，初始值 2.34             |
|                              | 0x0002 | input_data2     | float   | 只读，初始值 3.56             |
|                              | 0x0003 | input_data3     | float   | 只读，初始值 4.78             |
|                              | 0x0004 | input_data4     | float   | 只读，初始值 1.12             |
|                              | 0x0005 | input_data5     | float   | 只读，初始值 2.34             |
|                              | 0x0006 | input_data6     | float   | 只读，初始值 3.56             |
|                              | 0x0007 | input_data7     | float   | 只读，初始值 4.78             |
| **线圈（Coils）**                | 0x0000 | coils_port0     | uint8_t | 读/写，初始值 0x55 (01010101) |
|                              | 0x0001 | coils_port1     | uint8_t | 读/写，初始值 0xAA (10101010) |
| **离散输入（Discrete Inputs）**    | 0x0000 | discrete_input0 | bool    | 只读，初始值 1                |
|                              | 0x0001 | discrete_input1 | bool    | 只读，初始值 0                |
|                              | 0x0002 | discrete_input2 | bool    | 只读，初始值 1                |
|                              | 0x0003 | discrete_input3 | bool    | 只读，初始值 0                |
|                              | 0x0004 | discrete_input4 | bool    | 只读，初始值 1                |
|                              | 0x0005 | discrete_input5 | bool    | 只读，初始值 0                |
|                              | 0x0006 | discrete_input6 | bool    | 只读，初始值 1                |
|                              | 0x0007 | discrete_input7 | bool    | 只读，初始值 0                |

我们可以使用ModBus调试软件[`Modbus调试助手`](https://soft.3dmgame.com/down/249938.html)查看ESP32的ModBus从站信息：

**读取离散输入：**

![](attachments/Pasted%20image%2020250305174044.png)

可以看到离散输入寄存器读取的值与示例代码里的默认值是一样的。

**读取线圈寄存器1：**

![](attachments/Pasted%20image%2020250306000211.png)

读到值为55，没有问题

**读取输入寄存器1（浮点数）：**

![](attachments/Pasted%20image%2020250305235923.png)

这里读取到的值为：`5C29 3F8F`

我们可以使用转换工具进行转换 https://tooltt.com/floatconverter/， 注意这里有**字节反转（Byte Swap）**

![](attachments/Pasted%20image%2020250305235843.png)

可以看到也是没有问题的。

示例程序中对读保持寄存器的事件进行了处理：

```c
    // The cycle below will be terminated when parameter holdingRegParams.dataChan0
    // incremented each access cycle reaches the CHAN_DATA_MAX_VAL value.
    for(;holding_reg_params.holding_data0 < MB_CHAN_DATA_MAX_VAL;) {
        // Check for read/write events of Modbus master for certain events
        (void)mbc_slave_check_event(MB_READ_WRITE_MASK);
        ESP_ERROR_CHECK_WITHOUT_ABORT(mbc_slave_get_param_info(&reg_info, MB_PAR_INFO_GET_TOUT));
        const char* rw_str = (reg_info.type & MB_READ_MASK) ? "READ" : "WRITE";
        // Filter events and process them accordingly
        if(reg_info.type & (MB_EVENT_HOLDING_REG_WR | MB_EVENT_HOLDING_REG_RD)) {
            // Get parameter information from parameter queue
            ESP_LOGI(TAG, "HOLDING %s (%" PRIu32 " us), ADDR:%u, TYPE:%u, INST_ADDR:0x%" PRIx32 ", SIZE:%u",
                            rw_str,
                            reg_info.time_stamp,
                            (unsigned)reg_info.mb_offset,
                            (unsigned)reg_info.type,
                            (uint32_t)reg_info.address,
                            (unsigned)reg_info.size);
            if (reg_info.address == (uint8_t*)&holding_reg_params.holding_data0)
            {
                portENTER_CRITICAL(&param_lock);
                holding_reg_params.holding_data0 += MB_CHAN_DATA_OFFSET;
                if (holding_reg_params.holding_data0 >= (MB_CHAN_DATA_MAX_VAL - MB_CHAN_DATA_OFFSET)) {
                    coil_reg_params.coils_port1 = 0xFF;
                }
                portEXIT_CRITICAL(&param_lock);
            }
        } else if (reg_info.type & MB_EVENT_INPUT_REG_RD) {
            ESP_LOGI(TAG, "INPUT READ (%" PRIu32 " us), ADDR:%u, TYPE:%u, INST_ADDR:0x%" PRIx32 ", SIZE:%u",
                            reg_info.time_stamp,
                            (unsigned)reg_info.mb_offset,
                            (unsigned)reg_info.type,
                            (uint32_t)reg_info.address,
                            (unsigned)reg_info.size);
        } else if (reg_info.type & MB_EVENT_DISCRETE_RD) {
            ESP_LOGI(TAG, "DISCRETE READ (%" PRIu32 " us): ADDR:%u, TYPE:%u, INST_ADDR:0x%" PRIx32 ", SIZE:%u",
                            reg_info.time_stamp,
                            (unsigned)reg_info.mb_offset,
                            (unsigned)reg_info.type,
                            (uint32_t)reg_info.address,
                            (unsigned)reg_info.size);
        } else if (reg_info.type & (MB_EVENT_COILS_RD | MB_EVENT_COILS_WR)) {
            ESP_LOGI(TAG, "COILS %s (%" PRIu32 " us), ADDR:%u, TYPE:%u, INST_ADDR:0x%" PRIx32 ", SIZE:%u",
                            rw_str,
                            reg_info.time_stamp,
                            (unsigned)reg_info.mb_offset,
                            (unsigned)reg_info.type,
                            (uint32_t)reg_info.address,
                            (unsigned)reg_info.size);
            if (coil_reg_params.coils_port1 == 0xFF) break;
        }
    }
```

这段代码是一个Modbus从机的循环处理部分，主要负责监控并处理Modbus主机的读写请求。循环会一直运行，直到holding_reg_params.holding_data0达到最大值MB_CHAN_DATA_MAX_VAL。在每个循环周期内，它检查Modbus主机是否有读写请求，并根据请求类型（如保持寄存器、输入寄存器、离散输入、线圈等）进行相应处理。特别地，如果是对holding_data0的读写请求，会增加其值，并在达到最大值时触发某些动作（如设置线圈端口1的值为0xFF）。

下面我多次读取`holding_data0`:

![](attachments/Pasted%20image%2020250306000916.png)

`holding_reg_params.holding_data0`达到最大值`MB_CHAN_DATA_MAX_VAL`时会退出程序

![](attachments/Pasted%20image%2020250306000928.png)

以上就是官方历程的使用，和现象。
### 三、官方示例程序分析

官方示例程序主要实现了以下功能：

1. **初始化Modbus从站**：调用`mbc_slave_init`函数初始化Modbus控制器。设置通信参数并启动Modbus协议栈。
2. **设置寄存器区域描述符**： 使用`mbc_slave_set_descriptor`函数初始化Modbus保持寄存器、输入寄存器、线圈和离散输入寄存器的区域描述符。
3. **初始化寄存器数据**：调用`setup_reg_data`函数将寄存器值设置为已知状态。
4. **启动Modbus控制器和协议栈**：调用`mbc_slave_start`函数启动Modbus控制器和协议栈。
5. **处理Modbus主机的读写请求**：通过循环监控并处理Modbus主机的读写请求。根据请求类型（如保持寄存器、输入寄存器、离散输入、线圈等）进行相应处理。

以下是每个步骤的详细分析：

#### 1. 初始化Modbus从站

```c
ESP_ERROR_CHECK(mbc_slave_init(MB_PORT_SERIAL_SLAVE, &mbc_slave_handler)); // Initialization of Modbus controller
```

这行代码调用`mbc_slave_init`函数初始化Modbus控制器。`MB_PORT_SERIAL_SLAVE`表示使用串行端口作为Modbus通信端口，`mbc_slave_handler`是Modbus控制器的句柄。

#### 2. 设置通信参数并启动Modbus协议栈

```c
comm_info.mode = MB_MODE_RTU;
comm_info.slave_addr = MB_SLAVE_ADDR;
comm_info.port = MB_PORT_NUM;
comm_info.baudrate = MB_DEV_SPEED;
comm_info.parity = MB_PARITY_NONE;
ESP_ERROR_CHECK(mbc_slave_setup((void*)&comm_info));
```

这段代码设置了Modbus从站的通信参数，包括通信模式（RTU）、从站地址、串口端口号、波特率和奇偶校验。然后调用`mbc_slave_setup`函数应用这些通信参数。

#### 3. 设置寄存器区域描述符

```c
reg_area.type = MB_PARAM_HOLDING;
reg_area.start_offset = MB_REG_HOLDING_START_AREA0;
reg_area.address = (void*)&holding_reg_params.holding_data0;
reg_area.size = (size_t)(HOLD_OFFSET(holding_data4) - HOLD_OFFSET(test_regs));
ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));
```

这段代码为保持寄存器设置了区域描述符。`reg_area.type`表示寄存器类型为保持寄存器，`reg_area.start_offset`表示寄存器区域的起始偏移，`reg_area.address`是寄存器数据的存储地址，`reg_area.size`是寄存器区域的大小。然后调用`mbc_slave_set_descriptor`函数设置区域描述符。

#### 4. 初始化寄存器数据

```c
setup_reg_data();
```

这行代码调用`setup_reg_data`函数初始化寄存器数据，将寄存器值设置为已知状态。

#### 5. 启动Modbus控制器和协议栈

```c
ESP_ERROR_CHECK(mbc_slave_start());
```

这行代码调用`mbc_slave_start`函数启动Modbus控制器和协议栈。

#### 6. 处理Modbus主机的读写请求

```c
for(;holding_reg_params.holding_data0 < MB_CHAN_DATA_MAX_VAL;) {
    (void)mbc_slave_check_event(MB_READ_WRITE_MASK);
    ESP_ERROR_CHECK_WITHOUT_ABORT(mbc_slave_get_param_info(&reg_info, MB_PAR_INFO_GET_TOUT));
    const char* rw_str = (reg_info.type & MB_READ_MASK) ? "READ" : "WRITE";
    if(reg_info.type & (MB_EVENT_HOLDING_REG_WR | MB_EVENT_HOLDING_REG_RD)) {
        ESP_LOGI(TAG, "HOLDING %s", rw_str);
        if (reg_info.address == (uint8_t*)&holding_reg_params.holding_data0)
        {
            portENTER_CRITICAL(&param_lock);
            holding_reg_params.holding_data0 += MB_CHAN_DATA_OFFSET;
            if (holding_reg_params.holding_data0 >= (MB_CHAN_DATA_MAX_VAL - MB_CHAN_DATA_OFFSET)) {
                coil_reg_params.coils_port1 = 0xFF;
            }
            portEXIT_CRITICAL(&param_lock);
        }
    }
}
```

这段代码通过循环监控并处理Modbus主机的读写请求。它首先调用`mbc_slave_check_event`函数检查是否有读写事件，然后调用`mbc_slave_get_param_info`函数获取寄存器参数信息。根据请求类型（如保持寄存器、输入寄存器、离散输入、线圈等）进行相应处理。特别地，如果是对`holding_data0`的读写请求，会增加其值，并在达到最大值时触发某些动作（如设置线圈端口1的值为0xFF）。

## 三、修改示例程序：

下面我提供一个简洁版示例程序，去不必要的内容：

代码地址： https://github.com/DuRuofu/ESP32-Guide/tree/main/code/09.extra/modbus/serial/mb_slave

```c
#include <stdio.h>
#include <stdint.h>
#include "esp_err.h"
#include "mbcontroller.h"  // Modbus 控制器的定义和 API
#include "modbus_params.h" // Modbus 参数结构体
#include "esp_log.h"       // 日志输出
#include "sdkconfig.h"

#define MB_PORT_NUM (CONFIG_MB_UART_PORT_NUM)   // 用于 Modbus 连接的 UART 端口号
#define MB_SLAVE_ADDR (CONFIG_MB_SLAVE_ADDR)    // 设备在 Modbus 网络中的地址
#define MB_DEV_SPEED (CONFIG_MB_UART_BAUD_RATE) // UART 通信速率

// 注意：某些芯片的特定引脚无法用于 UART 通信。
// 请参考所选开发板和目标设备的文档，并使用 Kconfig 进行正确的引脚配置。

// 以下宏定义了 Modbus 各类寄存器的起始地址
#define HOLD_OFFSET(field) ((uint16_t)(offsetof(holding_reg_params_t, field) >> 1)) // 获取保持寄存器字段的偏移量
#define INPUT_OFFSET(field) ((uint16_t)(offsetof(input_reg_params_t, field) >> 1))  // 获取输入寄存器字段的偏移量

#define MB_REG_DISCRETE_INPUT_START (0x0000)                    // 离散输入寄存器起始地址
#define MB_REG_COILS_START (0x0000)                             // 线圈寄存器起始地址
#define MB_REG_INPUT_START_AREA0 (INPUT_OFFSET(input_data0))    // 输入寄存器区域 0 起始地址
#define MB_REG_INPUT_START_AREA1 (INPUT_OFFSET(input_data4))    // 输入寄存器区域 1 起始地址
#define MB_REG_HOLDING_START_AREA0 (HOLD_OFFSET(holding_data0)) // 保持寄存器区域 0 起始地址
#define MB_REG_HOLDING_START_AREA1 (HOLD_OFFSET(holding_data4)) // 保持寄存器区域 1 起始地址

#define MB_PAR_INFO_GET_TOUT (10)  // 获取参数信息的超时时间（单位：ms）
#define MB_CHAN_DATA_MAX_VAL (6)   // 通道数据的最大值
#define MB_CHAN_DATA_OFFSET (0.2f) // 通道数据的偏移量

// Modbus 读操作事件掩码
#define MB_READ_MASK (MB_EVENT_INPUT_REG_RD | MB_EVENT_HOLDING_REG_RD | MB_EVENT_DISCRETE_RD | MB_EVENT_COILS_RD)
// Modbus 写操作事件掩码
#define MB_WRITE_MASK (MB_EVENT_HOLDING_REG_WR | MB_EVENT_COILS_WR)

// Modbus 读写操作事件掩码
#define MB_READ_WRITE_MASK (MB_READ_MASK | MB_WRITE_MASK)

// 日志 TAG
static const char *TAG = "SLAVE_TEST";

static portMUX_TYPE param_lock = portMUX_INITIALIZER_UNLOCKED;

// 设置寄存器初始值
static void setup_reg_data(void)
{
    // 初始化离散输入寄存器
    discrete_reg_params.discrete_input0 = 1;
    discrete_reg_params.discrete_input1 = 0;
    discrete_reg_params.discrete_input2 = 1;
    discrete_reg_params.discrete_input3 = 0;
    discrete_reg_params.discrete_input4 = 1;
    discrete_reg_params.discrete_input5 = 0;
    discrete_reg_params.discrete_input6 = 1;
    discrete_reg_params.discrete_input7 = 0;

    // 初始化保持寄存器
    holding_reg_params.holding_data0 = 1.34;
    holding_reg_params.holding_data1 = 2.56;
    holding_reg_params.holding_data2 = 3.78;
    holding_reg_params.holding_data3 = 4.90;
    holding_reg_params.holding_data4 = 5.67;
    holding_reg_params.holding_data5 = 6.78;
    holding_reg_params.holding_data6 = 7.79;
    holding_reg_params.holding_data7 = 8.80;

    // 初始化线圈寄存器
    coil_reg_params.coils_port0 = 0x55;
    coil_reg_params.coils_port1 = 0xAA;

    // 初始化输入寄存器
    input_reg_params.input_data0 = 1.12;
    input_reg_params.input_data1 = 2.34;
    input_reg_params.input_data2 = 3.56;
    input_reg_params.input_data3 = 4.78;
    input_reg_params.input_data4 = 1.12;
    input_reg_params.input_data5 = 2.34;
    input_reg_params.input_data6 = 3.56;
    input_reg_params.input_data7 = 4.78;
}

// Modbus 从设备示例应用，基于 FreeModbus 协议栈。
// 设备参数定义在 deviceparams.h 文件中。
void app_main(void)
{
    mb_param_info_t reg_info;               // Modbus 寄存器访问信息
    mb_communication_info_t comm_info;      // Modbus 通信参数
    mb_register_area_descriptor_t reg_area; // Modbus 寄存器区域描述符

    // 设置日志等级
    esp_log_level_set(TAG, ESP_LOG_INFO);
    void *mbc_slave_handler = NULL;

    // 初始化 Modbus 从机控制器
    ESP_ERROR_CHECK(mbc_slave_init(MB_PORT_SERIAL_SLAVE, &mbc_slave_handler));

    // 设置通信参数
#if CONFIG_MB_COMM_MODE_ASCII
    comm_info.mode = MB_MODE_ASCII,
#elif CONFIG_MB_COMM_MODE_RTU
    comm_info.mode = MB_MODE_RTU,
#endif
    comm_info.slave_addr = MB_SLAVE_ADDR;
    comm_info.port = MB_PORT_NUM;
    comm_info.baudrate = MB_DEV_SPEED;
    comm_info.parity = MB_PARITY_NONE;
    ESP_ERROR_CHECK(mbc_slave_setup((void *)&comm_info));

    // 初始化 Modbus 寄存器区域
    reg_area.type = MB_PARAM_HOLDING;
    reg_area.start_offset = MB_REG_HOLDING_START_AREA0;
    reg_area.address = (void *)&holding_reg_params.holding_data0;
    reg_area.size = (size_t)(HOLD_OFFSET(holding_data4) - HOLD_OFFSET(test_regs));
    ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));

    reg_area.type = MB_PARAM_HOLDING;
    reg_area.start_offset = MB_REG_HOLDING_START_AREA1;
    reg_area.address = (void *)&holding_reg_params.holding_data4;
    reg_area.size = sizeof(float) << 2;
    ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));

    // 初始化输入寄存器
    reg_area.type = MB_PARAM_INPUT;
    reg_area.start_offset = MB_REG_INPUT_START_AREA0;
    reg_area.address = (void *)&input_reg_params.input_data0;
    reg_area.size = sizeof(float) << 2;
    ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));

    reg_area.type = MB_PARAM_INPUT;
    reg_area.start_offset = MB_REG_INPUT_START_AREA1;
    reg_area.address = (void *)&input_reg_params.input_data4;
    reg_area.size = sizeof(float) << 2;
    ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));

    // 初始化线圈寄存器
    reg_area.type = MB_PARAM_COIL;
    reg_area.start_offset = MB_REG_COILS_START;
    reg_area.address = (void *)&coil_reg_params;
    reg_area.size = sizeof(coil_reg_params);
    ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));

    // 初始化离散输入寄存器
    reg_area.type = MB_PARAM_DISCRETE;
    reg_area.start_offset = MB_REG_DISCRETE_INPUT_START;
    reg_area.address = (void *)&discrete_reg_params;
    reg_area.size = sizeof(discrete_reg_params);
    ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));

    setup_reg_data(); // 设定初始值

    // 启动 Modbus 从设备控制器
    ESP_ERROR_CHECK(mbc_slave_start());

    // 设置 UART 引脚
    ESP_ERROR_CHECK(uart_set_pin(MB_PORT_NUM, CONFIG_MB_UART_TXD,
                                 CONFIG_MB_UART_RXD, CONFIG_MB_UART_RTS,
                                 UART_PIN_NO_CHANGE));

    // 设置 UART 为半双工模式
    ESP_ERROR_CHECK(uart_set_mode(MB_PORT_NUM, UART_MODE_RS485_HALF_DUPLEX));

    ESP_LOGI(TAG, "Modbus 从设备初始化完成。");
    ESP_LOGI(TAG, "开始 Modbus 测试...");
}

```


其中，各种寄存器的定义和声明放在`modbus_slave_params`组件中，`modbus_params.h`如下:

```c
#ifndef _DEVICE_PARAMS
#define _DEVICE_PARAMS

#include <stdint.h>

// This file defines structure of modbus parameters which reflect correspond modbus address space
// for each modbus register type (coils, discreet inputs, holding registers, input registers)
#pragma pack(push, 1)
typedef struct
{
    uint8_t discrete_input0:1;
    uint8_t discrete_input1:1;
    uint8_t discrete_input2:1;
    uint8_t discrete_input3:1;
    uint8_t discrete_input4:1;
    uint8_t discrete_input5:1;
    uint8_t discrete_input6:1;
    uint8_t discrete_input7:1;
    uint8_t discrete_input_port1;
    uint8_t discrete_input_port2;
} discrete_reg_params_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct
{
    uint8_t coils_port0;
    uint8_t coils_port1;
    uint8_t coils_port2;
} coil_reg_params_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct
{
    float input_data0; // 0
    float input_data1; // 2
    float input_data2; // 4
    float input_data3; // 6
    uint16_t data[150]; // 8 + 150 = 158
    float input_data4; // 158
    float input_data5;
    float input_data6;
    float input_data7;
    uint16_t data_block1[150];
} input_reg_params_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct
{
    float holding_data0;
    float holding_data1;
    float holding_data2;
    float holding_data3;
    uint16_t test_regs[150];
    float holding_data4;
    float holding_data5;
    float holding_data6;
    float holding_data7;
} holding_reg_params_t;
#pragma pack(pop)

extern holding_reg_params_t holding_reg_params;
extern input_reg_params_t input_reg_params;
extern coil_reg_params_t coil_reg_params;
extern discrete_reg_params_t discrete_reg_params;

#endif // !defined(_DEVICE_PARAMS)

```

定义了用于 Modbus 通信的各种寄存器参数结构体，包括离散输入、线圈寄存器、输入寄存器和保持寄存器。每个结构体都定义了相应的参数字段，并声明了外部变量以便在其他文件中访问这些实例。

`modbus_params.c`如下：

```c
#include "modbus_params.h"

// Here are the user defined instances for device parameters packed by 1 byte
// These are keep the values that can be accessed from Modbus master
holding_reg_params_t holding_reg_params = { 0 };

input_reg_params_t input_reg_params = { 0 };

coil_reg_params_t coil_reg_params = { 0 };

discrete_reg_params_t discrete_reg_params = { 0 };

```

定义了四个用于 Modbus 通信的参数存储实例，这些实例分别用于保持寄存器、输入寄存器、线圈寄存器和离散输入寄存器。每个实例都初始化为 0，以便在 Modbus 主机访问时有一个初始值。

综上，这就是对ESP32 ModBus从机的使用分析。
## 参考：

6. https://blog.csdn.net/LINKKEEE/article/details/137208685
7. https://www.bilibili.com/video/BV1Qr4y1M7oE?vd_source=a506554ed4d5f4c4e1ea9e76714fcd74
8. https://tooltt.com/floatconverter/
9. https://www.toolhelper.cn/Digit/FractionConvert