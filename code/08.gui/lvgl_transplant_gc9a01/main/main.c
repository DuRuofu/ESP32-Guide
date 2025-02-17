#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lvgl.h"
#include "sdkconfig.h"
#include "esp_lcd_gc9a01.h"   // 屏幕驱动
#include "lv_demos.h"
// Using SPI2 
#define LCD_HOST SPI2_HOST

// 屏幕尺寸
#define EXAMPLE_LCD_H_RES 240
#define EXAMPLE_LCD_V_RES 240

// 引脚接线
// | Name | Description            				| GPIO   |
// | BLK  | 背光，悬空使能接地关闭，默认上拉至3.3V | 32     |
// | CS   | 片选，低电平使能               	      | 5      |
// | DC   | 数据/命令选择，低电平命令，高电平数据   | 27     |
// | RES  | 复位，低电平使能               	  	  | 33     |
// | SDA  | SPI数据输入端口              		  | 15     |
// | SCL  | SPI时钟信号输入端口            		  | 14     |
#define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL 1
#define EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL !EXAMPLE_LCD_BK_LIGHT_ON_LEVEL
#define EXAMPLE_PIN_NUM_BK_LIGHT      32
#define EXAMPLE_PIN_NUM_LCD_CS 5
#define EXAMPLE_PIN_NUM_LCD_DC 27
#define EXAMPLE_PIN_NUM_LCD_RST 33
#define EXAMPLE_PIN_NUM_DATA0 15
#define EXAMPLE_PIN_NUM_SCLK 14

// 日志标签
static const char *TAG = "main";

static SemaphoreHandle_t    lvgl_mux    = NULL;

// LVGL定时器周期
#define EXAMPLE_LVGL_TICK_PERIOD_MS     2               
// LVGL任务参数
#define EXAMPLE_LVGL_TASK_MAX_DELAY_MS  500
#define EXAMPLE_LVGL_TASK_MIN_DELAY_MS  1
#define EXAMPLE_LVGL_TASK_STACK_SIZE    (5 * 1024)
#define EXAMPLE_LVGL_TASK_PRIORITY      2


// 回调函数:颜色传输完成 (通知LVGL刷新)
static bool notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
	lv_disp_drv_t *disp_driver = (lv_disp_drv_t *)user_ctx;
	lv_disp_flush_ready(disp_driver);
	return false;
}

// 回调函数:刷新屏幕
static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) drv->user_data;
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    /* Copy a buffer's content to a specific area of the display */
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
}

// 回调函数:增加LVGL的tick
static void example_increase_lvgl_tick(void *arg)
{
    /* Tell LVGL how many milliseconds has elapsed */
    lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);

}



static bool example_lvgl_lock(int timeout_ms)
{
    assert(lvgl_mux && "bsp_display_start must be called first");

    const TickType_t timeout_ticks = (timeout_ms == -1) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xSemaphoreTake(lvgl_mux, timeout_ticks) == pdTRUE;
}

static void example_lvgl_unlock(void)
{
    assert(lvgl_mux && "bsp_display_start must be called first");
    xSemaphoreGive(lvgl_mux);
}



// 测试UI
void test_ui(void) {
	// ESP_LOGI(TAG, "Create a test_ui");
    // // 创建一个屏幕对象
    // lv_obj_t *scr = lv_disp_get_scr_act(NULL);

    // // 创建一个矩形对象
    // lv_obj_t *rect = lv_obj_create(scr);
    // lv_obj_set_size(rect, LV_HOR_RES, LV_VER_RES);  // 设置矩形大小为屏幕大小
    // lv_obj_set_style_local_bg_color(rect, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLUE); // 设置矩形对象背景颜色为蓝色
	// lv_obj_set_style_outline_color(rect, LV_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_SIZE); // 设置矩形对象背景颜色为白色






}

// LVGL任务
static void example_lvgl_port_task(void *arg)
{
    ESP_LOGI(TAG, "Starting LVGL task");
    ESP_LOGI(TAG, "Display LVGL UI");

    //test_ui();
	lv_demo_widgets();
	//lv_demo_music();

    uint32_t task_delay_ms = EXAMPLE_LVGL_TASK_MAX_DELAY_MS;
    while (1) {
		ESP_LOGI(TAG, "LVGL task");
        /* Lock the mutex due to the LVGL APIs are not thread-safe */
        if (example_lvgl_lock(-1)) {
            task_delay_ms = lv_timer_handler();
            /* Release the mutex */
            example_lvgl_unlock();
        }
        if (task_delay_ms > EXAMPLE_LVGL_TASK_MAX_DELAY_MS) {
            task_delay_ms = EXAMPLE_LVGL_TASK_MAX_DELAY_MS;
        } else if (task_delay_ms < EXAMPLE_LVGL_TASK_MIN_DELAY_MS) {
            task_delay_ms = EXAMPLE_LVGL_TASK_MIN_DELAY_MS;
        }
        vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
    }
}



void app_main(void)
{
	static lv_disp_draw_buf_t disp_buf; // contains internal graphic buffer(s) called draw buffer(s)
	static lv_disp_drv_t disp_drv;		// contains callback functions

	// 配置屏幕背光
	ESP_LOGI(TAG, "Turn off LCD backlight");
	gpio_config_t bk_gpio_config = {
		.mode = GPIO_MODE_OUTPUT,
		.pin_bit_mask = 1ULL << EXAMPLE_PIN_NUM_BK_LIGHT};
	ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));

	// 初始化SPI总线
	ESP_LOGI(TAG, "Initialize SPI bus");
	const spi_bus_config_t buscfg = GC9A01_PANEL_BUS_SPI_CONFIG(EXAMPLE_PIN_NUM_SCLK, EXAMPLE_PIN_NUM_DATA0, EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES);
	ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

	// 创建屏幕句柄
	ESP_LOGI(TAG, "Install panel IO");
	esp_lcd_panel_io_handle_t io_handle = NULL;
	const esp_lcd_panel_io_spi_config_t io_config = GC9A01_PANEL_IO_SPI_CONFIG(EXAMPLE_PIN_NUM_LCD_CS, EXAMPLE_PIN_NUM_LCD_DC, notify_lvgl_flush_ready, &disp_drv);
		
	// 将LCD连接到SPI总线
	ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

	// 创建屏幕驱动句柄
	ESP_LOGI(TAG, "Install GC9A01 panel driver");
	esp_lcd_panel_handle_t panel_handle = NULL;
	// 屏幕配置
	const esp_lcd_panel_dev_config_t panel_config = {
		.reset_gpio_num = EXAMPLE_PIN_NUM_LCD_RST, // Set to -1 if not use
		.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
		.bits_per_pixel = 16, // Implemented by LCD command `3Ah` (16/18)
		//.vendor_config = &vendor_config,
	};
	// 创建屏幕实例
	ESP_ERROR_CHECK(esp_lcd_new_panel_gc9a01(io_handle, &panel_config, &panel_handle));
	// 屏幕复位
	ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
	// 初始化屏幕
	ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
	// 反转颜色
	ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true)); 
	// 镜像
	ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, true, false)); 
	// 打开屏幕
	ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
	// 打开背光
	ESP_ERROR_CHECK(gpio_set_level(EXAMPLE_PIN_NUM_BK_LIGHT, EXAMPLE_LCD_BK_LIGHT_ON_LEVEL));



	// 初始化LVGL
	ESP_LOGI(TAG, "Initialize LVGL");
	lv_init();
	// 申请内存 两个buf
	lv_color_t *buf1 = heap_caps_malloc(EXAMPLE_LCD_H_RES * 50 * sizeof(lv_color_t), MALLOC_CAP_DMA);
	assert(buf1); // 检查内存是否申请成功
	lv_color_t *buf2 = heap_caps_malloc(EXAMPLE_LCD_H_RES * 50 * sizeof(lv_color_t), MALLOC_CAP_DMA);
	assert(buf2); 
	// 初始化LVGL显示缓冲区
	lv_disp_draw_buf_init(&disp_buf, buf1, buf2, EXAMPLE_LCD_H_RES * 50);

	// 初始化LVGL显示驱动
	ESP_LOGI(TAG, "Initialize LVGL display driver");
	lv_disp_drv_init(&disp_drv);
	disp_drv.hor_res = EXAMPLE_LCD_H_RES; 	// 设置屏幕水平分辨率
	disp_drv.ver_res = EXAMPLE_LCD_V_RES; 	// 设置屏幕垂直分辨率
	disp_drv.flush_cb = lvgl_flush_cb;  	// 设置刷新回调函数
	disp_drv.draw_buf = &disp_buf; 			// 设置显示缓冲区
	disp_drv.user_data = panel_handle; 		// 设置用户数据
	lv_disp_t *disp = lv_disp_drv_register(&disp_drv); 	// 注册显示驱动

	// 配置输入设备
	ESP_LOGI(TAG, "Initialize LVGL input device");
	// lv_indev_drv_t indev_drv;


	// 创建定时器
	ESP_LOGI(TAG, "Install LVGL tick timer");
    /* Tick interface for LVGL (using esp_timer to generate 2ms periodic event) */
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback   = &example_increase_lvgl_tick,
        .name       = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000));

    // 创建LVGL任务
	ESP_LOGI(TAG, "Create LVGL task");
	lvgl_mux = xSemaphoreCreateMutex();
    assert(lvgl_mux);
    xTaskCreate(example_lvgl_port_task, "LVGL", EXAMPLE_LVGL_TASK_STACK_SIZE, NULL, EXAMPLE_LVGL_TASK_PRIORITY, NULL);
}
