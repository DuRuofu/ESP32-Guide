#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_mac.h"

#define ESP_WIFI_AP_SSID "DuRuofu_ESP32"
#define ESP_WIFI_AP_PASSWD "3.1415926"

void WIFI_CallBack(void *event_handler_arg,esp_event_base_t event_base,int32_t event_id,void *event_data)
{
	// 连接事件
	if(event_base == IP_EVENT && event_id == IP_EVENT_AP_STAIPASSIGNED)
	{
		// 解析数据
		ip_event_ap_staipassigned_t* event_info = (ip_event_ap_staipassigned_t *)event_data;
		ESP_LOGW("WIFI_AP", "设备已连接 MAC:"MACSTR"", MAC2STR(event_info->mac));
	}

	// 断开连接事件
	if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED)
	{
		wifi_event_ap_stadisconnected_t* event_info = (wifi_event_ap_stadisconnected_t*)event_data;
		ESP_LOGW("WIFI_AP", "设备已断开 MAC:"MACSTR"", MAC2STR(event_info->mac));
	}
}

void app_main(void)
{
	//----------------准备阶段-------------------
	// Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	//---------------初始化阶段------------------

	ESP_ERROR_CHECK(esp_netif_init());

	// 初始化默认事件循环
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	// 注册设备连接事件回调
	esp_event_handler_instance_register(IP_EVENT, IP_EVENT_AP_STAIPASSIGNED, WIFI_CallBack, NULL, NULL);
	// 注册设备断开连接设备回调
	esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_AP_STADISCONNECTED, WIFI_CallBack, NULL, NULL);

	// *esp_netif_ap 可以用来修改AP设置
	esp_netif_t *esp_netif_ap = esp_netif_create_default_wifi_ap();

	/*Initialize WiFi */
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	// WIFI_INIT_CONFIG_DEFAULT 是一个默认配置的宏
	// 初始化WIFI设备( 为 WiFi 驱动初始化 WiFi 分配资源，如 WiFi 控制结构、RX/TX 缓冲区、WiFi NVS 结构等，这个 WiFi 也启动 WiFi 任务。必须先调用此API，然后才能调用所有其他WiFi API)
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	//---------------配置阶段--------------------
	// 设置为AP模式
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));

	// AP详细配置
	wifi_config_t wifi_ap_config = {
		.ap = {
			.ssid = ESP_WIFI_AP_SSID,			  // WIFI名称
			.ssid_len = strlen(ESP_WIFI_AP_SSID), // 名称长度
			.channel = 1,						  // WIFI信道
			.password = ESP_WIFI_AP_PASSWD,		  // WiFi密码
			.max_connection = 5,				  // 最大连接数，默认值是 10
			.authmode = WIFI_AUTH_WPA2_PSK,		  // WiFi认证方式
		},
	};
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_config));

	//---------------启动阶段--------------------
	/* Start WiFi */
	ESP_ERROR_CHECK(esp_wifi_start());
}
