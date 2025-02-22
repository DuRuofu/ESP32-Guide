#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"

#define EXAMPLE_ADC1_CHAN0 ADC_CHANNEL_6  // 根据您的硬件配置选择合适的通道
#define EXAMPLE_ADC1_CHAN1 ADC_CHANNEL_7  // 根据您的硬件配置选择合适的通道

static const char* TAG = "ADC_ONESHOT_EXAMPLE";

void app_main(void)
{
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_11,  // 设置适当的衰减以匹配您的输入电压范围
    };

    // 配置两个通道
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, EXAMPLE_ADC1_CHAN0, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, EXAMPLE_ADC1_CHAN1, &config));

    int adc_raw[2];

    // 对每个通道进行单次转换并读取结果
    for (int i = 0; i < 10; i++) {
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, EXAMPLE_ADC1_CHAN0, &adc_raw[0]));
        ESP_LOGI(TAG, "ADC1 Channel[%d] Raw Data: %d", EXAMPLE_ADC1_CHAN0, adc_raw[0]);

        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, EXAMPLE_ADC1_CHAN1, &adc_raw[1]));
        ESP_LOGI(TAG, "ADC1 Channel[%d] Raw Data: %d", EXAMPLE_ADC1_CHAN1, adc_raw[1]);

        vTaskDelay(pdMS_TO_TICKS(1000));  // 等待1秒再次读取
    }

    // 回收资源
    ESP_ERROR_CHECK(adc_oneshot_del_unit(adc1_handle));
}
