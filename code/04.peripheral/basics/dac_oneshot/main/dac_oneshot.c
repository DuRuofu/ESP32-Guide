#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/dac_oneshot.h"
//#include "esp_check.h"

static void dac_output_task(void *args)
{
    dac_oneshot_handle_t handle = (dac_oneshot_handle_t)args;
    uint32_t val = 0;
    while (1) {
        /* Set the voltage every 100 ms */
        ESP_ERROR_CHECK(dac_oneshot_output_voltage(handle, val));
        val += 10;
        val %= 250;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void app_main(void)
{
    /* DAC oneshot init */
    dac_oneshot_handle_t chan0_handle;
    dac_oneshot_config_t chan0_cfg = {
        .chan_id = DAC_CHAN_0,
    };
    ESP_ERROR_CHECK(dac_oneshot_new_channel(&chan0_cfg, &chan0_handle));

    dac_oneshot_handle_t chan1_handle;
    dac_oneshot_config_t chan1_cfg = {
        .chan_id = DAC_CHAN_1,
    };
    ESP_ERROR_CHECK(dac_oneshot_new_channel(&chan1_cfg, &chan1_handle));

    /* DAC oneshot outputting threads */
    xTaskCreate(dac_output_task, "dac_chan0_output_task", 4096, chan0_handle, 5, NULL);
    vTaskDelay(pdMS_TO_TICKS(500)); // To differential the output of two channels
    xTaskCreate(dac_output_task, "dac_chan1_output_task", 4096, chan1_handle, 5, NULL);

}
