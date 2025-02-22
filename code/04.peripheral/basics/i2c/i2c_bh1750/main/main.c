
#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "sdkconfig.h"

static const char *TAG = "main";


// I2C Master 配置
#define I2C_MASTER_SCL_IO  19              /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO  18              /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM  I2C_NUM_0          /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ 100000          /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0        /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0        /*!< I2C master doesn't need buffer */

// BH1750 Sensor 配置
#define BH1750_SENSOR_ADDR 0x23  //0010_0011    // 传感器地址
#define BH1750_CMD_START   0x23  //0010_0011	// 传感器模式:单次采集模式
#define WRITE_BIT I2C_MASTER_WRITE              // I2C 读取位 :1
#define READ_BIT I2C_MASTER_READ                // I2C 写入位 :0
#define ACK_CHECK_EN 0x1                        // 检测从机应答
#define ACK_CHECK_DIS 0x0                       // 不检测从机应答
#define ACK_VAL 0x0                             // 响应值
#define NACK_VAL 0x1                            // 无响应值



/**
 * @brief test code to operate on BH1750 sensor
 *
 * 1. set operation mode(e.g One time L-resolution mode)
 * _________________________________________________________________
 * | start | slave_addr + wr_bit + ack | write 1 byte + ack  | stop |
 * --------|---------------------------|---------------------|------|
 * 2. wait more than 24 ms
 * 3. read data
 * ______________________________________________________________________________________
 * | start | slave_addr + rd_bit + ack | read 1 byte + ack  | read 1 byte + nack | stop |
 * --------|---------------------------|--------------------|--------------------|------|
 */
static esp_err_t i2c_master_sensor_test(i2c_port_t i2c_num, uint8_t *data_h, uint8_t *data_l)
{
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();     // 创建I2C命令
    i2c_master_start(cmd);						      // 起始信号
    i2c_master_write_byte(cmd, BH1750_SENSOR_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN); //从机地址及读写位
    i2c_master_write_byte(cmd, BH1750_CMD_START, ACK_CHECK_EN);					   //数据位(数组)	
    i2c_master_stop(cmd);														   //终止信号
    ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_PERIOD_MS);		   //i2c_num 发送这个数据帧，timeout设置为1000毫秒
    i2c_cmd_link_delete(cmd);					     //删除i2c_cmd_handle_t对象，释放资源
    if (ret != ESP_OK) {
        return ret;
    }
    vTaskDelay(30 / portTICK_PERIOD_MS);
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, BH1750_SENSOR_ADDR << 1 | READ_BIT, ACK_CHECK_EN);   //从机地址及读写位
    i2c_master_read_byte(cmd, data_h, ACK_VAL);										//读取数据(高位)
    i2c_master_read_byte(cmd, data_l, NACK_VAL);									//读取数据(低位)
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

/**
 * @brief I2C master initialization
 */
static esp_err_t i2c_master_init(void)
{
	// 配置I2C
    int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
        // .clk_flags = 0,          /*!< Optional, you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here. */
    };
    esp_err_t err = i2c_param_config(i2c_master_port, &conf);
    if (err != ESP_OK) {
        return err;
    }
	// 安装I2C驱动
    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}


static void i2c_task(void *arg)
{
    int ret;				// Return value
    int task_idx = (int)arg;
    uint8_t sensor_data_h, sensor_data_l;
    int cnt = 0;

    while (1) {
		// 读取数据
        ESP_LOGI(TAG, "TASK[%d] test cnt: %d", task_idx, cnt++);
        ret = i2c_master_sensor_test(I2C_MASTER_NUM, &sensor_data_h, &sensor_data_l);
        if (ret == ESP_ERR_TIMEOUT) {
            ESP_LOGE(TAG, "I2C Timeout");
        } else if (ret == ESP_OK) {
            printf("*******************\n");
            printf("TASK[%d]  MASTER READ SENSOR( BH1750 )\n", task_idx);
            printf("*******************\n");
            printf("data_h: %02x\n", sensor_data_h);
            printf("data_l: %02x\n", sensor_data_l);
            printf("sensor val: %.02f [Lux]\n", (sensor_data_h << 8 | sensor_data_l) / 1.2);
        } else {
            ESP_LOGW(TAG, "%s: No ack, sensor not connected...skip...", esp_err_to_name(ret));
        }
		vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

}

void app_main(void)
{
	// 初始化I2C
    ESP_ERROR_CHECK(i2c_master_init());
	// 创建I2C采集任务
    xTaskCreate(i2c_task, "i2c_task", 1024 * 2, (void *)0, 10, NULL);
}
