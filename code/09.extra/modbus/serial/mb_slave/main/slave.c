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

    for (;;)
    {
        // 检查 Modbus 主站的读/写事件
        (void)mbc_slave_check_event(MB_READ_WRITE_MASK);
        ESP_ERROR_CHECK_WITHOUT_ABORT(mbc_slave_get_param_info(&reg_info, MB_PAR_INFO_GET_TOUT));
        const char *rw_str = (reg_info.type & MB_READ_MASK) ? "读取" : "写入";
        // 过滤事件并相应处理
        if (reg_info.type & (MB_EVENT_HOLDING_REG_WR | MB_EVENT_HOLDING_REG_RD))
        {
            // 从参数队列获取参数信息
            ESP_LOGI(TAG, "保持寄存器 %s (%" PRIu32 " us), 地址:%u, 类型:%u, 实例地址:0x%" PRIx32 ", 大小:%u",
                     rw_str,
                     reg_info.time_stamp,
                     (unsigned)reg_info.mb_offset,
                     (unsigned)reg_info.type,
                     (uint32_t)reg_info.address,
                     (unsigned)reg_info.size);
        }
        else if (reg_info.type & MB_EVENT_INPUT_REG_RD)
        {
            ESP_LOGI(TAG, "输入寄存器读取 (%" PRIu32 " us), 地址:%u, 类型:%u, 实例地址:0x%" PRIx32 ", 大小:%u",
                     reg_info.time_stamp,
                     (unsigned)reg_info.mb_offset,
                     (unsigned)reg_info.type,
                     (uint32_t)reg_info.address,
                     (unsigned)reg_info.size);
        }
        else if (reg_info.type & MB_EVENT_DISCRETE_RD)
        {
            ESP_LOGI(TAG, "离散输入读取 (%" PRIu32 " us): 地址:%u, 类型:%u, 实例地址:0x%" PRIx32 ", 大小:%u",
                     reg_info.time_stamp,
                     (unsigned)reg_info.mb_offset,
                     (unsigned)reg_info.type,
                     (uint32_t)reg_info.address,
                     (unsigned)reg_info.size);
        }
        else if (reg_info.type & (MB_EVENT_COILS_RD | MB_EVENT_COILS_WR))
        {
            ESP_LOGI(TAG, "线圈 %s (%" PRIu32 " us), 地址:%u, 类型:%u, 实例地址:0x%" PRIx32 ", 大小:%u",
                     rw_str,
                     reg_info.time_stamp,
                     (unsigned)reg_info.mb_offset,
                     (unsigned)reg_info.type,
                     (uint32_t)reg_info.address,
                     (unsigned)reg_info.size);
            if (coil_reg_params.coils_port1 == 0xFF)
                break; // 如果设置了退出标志，可以在这里退出循环
        }

        // 加入适当的延时，避免过高的CPU占用
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    // 销毁 Modbus 控制器
    ESP_LOGI(TAG, "Modbus 控制器已销毁。");
    vTaskDelay(100);
    ESP_ERROR_CHECK(mbc_slave_destroy());
}
