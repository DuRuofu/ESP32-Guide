#includ<stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_mac.h"
#include "esp_netif.h"

#define ESP_WIFI_STA_SSID "duruofu_win10"
#define ESP_WIFI_STA_PASSWD "1234567890"

// 扫面列表大小
#define DEFAULT_SCAN_LIST_SIZE 10

static const char *TAG = "wifi_scan";

// 打印加密模式
static void print_auth_mode(int authmode)
{
	switch (authmode)
	{
	case WIFI_AUTH_OPEN:
		ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_OPEN");
		break;
	case WIFI_AUTH_OWE:
		ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_OWE");
		break;
	case WIFI_AUTH_WEP:
		ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WEP");
		break;
	case WIFI_AUTH_WPA_PSK:
		ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA_PSK");
		break;
	case WIFI_AUTH_WPA2_PSK:
		ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA2_PSK");
		break;
	case WIFI_AUTH_WPA_WPA2_PSK:
		ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA_WPA2_PSK");
		break;
	case WIFI_AUTH_WPA2_ENTERPRISE:
		ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA2_ENTERPRISE");
		break;
	case WIFI_AUTH_WPA3_PSK:
		ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA3_PSK");
		break;
	case WIFI_AUTH_WPA2_WPA3_PSK:
		ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA2_WPA3_PSK");
		break;
	default:
		ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_UNKNOWN");
		break;
	}
}

// 打印密码类型
static void print_cipher_type(int pairwise_cipher, int group_cipher)
{
	switch (pairwise_cipher)
	{
	case WIFI_CIPHER_TYPE_NONE:
		ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_NONE");
		break;
	case WIFI_CIPHER_TYPE_WEP40:
		ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_WEP40");
		break;
	case WIFI_CIPHER_TYPE_WEP104:
		ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_WEP104");
		break;
	case WIFI_CIPHER_TYPE_TKIP:
		ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_TKIP");
		break;
	case WIFI_CIPHER_TYPE_CCMP:
		ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_CCMP");
		break;
	case WIFI_CIPHER_TYPE_TKIP_CCMP:
		ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_TKIP_CCMP");
		break;
	default:
		ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_UNKNOWN");
		break;
	}

	switch (group_cipher)
	{
	case WIFI_CIPHER_TYPE_NONE:
		ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_NONE");
		break;
	case WIFI_CIPHER_TYPE_WEP40:
		ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_WEP40");
		break;
	case WIFI_CIPHER_TYPE_WEP104:
		ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_WEP104");
		break;
	case WIFI_CIPHER_TYPE_TKIP:
		ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_TKIP");
		break;
	case WIFI_CIPHER_TYPE_CCMP:
		ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_CCMP");
		break;
	case WIFI_CIPHER_TYPE_TKIP_CCMP:
		ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_TKIP_CCMP");
		break;
	default:
		ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_UNKNOWN");
		break;
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

	//----------------初始化阶段-------------------
	ESP_ERROR_CHECK(esp_netif_init());

	ESP_ERROR_CHECK(esp_event_loop_create_default());

	// 初始化WIFI设备( 为 WiFi 驱动初始化 WiFi 分配资源，如 WiFi 控制结构、RX/TX 缓冲区、WiFi NVS 结构等，这个 WiFi 也启动 WiFi 任务。必须先调用此API，然后才能调用所有其他WiFi API)
	esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
	assert(sta_netif);

	/*Initialize WiFi */
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	// WIFI_INIT_CONFIG_DEFAULT 是一个默认配置的宏

	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	//----------------配置阶段-------------------
	// 设置为STA模式
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

	// STA详细配置
	wifi_config_t sta_config = {
		.sta = {
			.ssid = ESP_WIFI_STA_SSID,
			.password = ESP_WIFI_STA_PASSWD,
			.bssid_set = false,
		},
	};
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));

	//----------------启动阶段-------------------
	ESP_ERROR_CHECK(esp_wifi_start());

	//----------------配置省电模式-------------------
	// 不省电(数据传输会更快)
	ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

	//----------------WIFI Scan-------------------
	uint16_t number = DEFAULT_SCAN_LIST_SIZE;
	wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
	uint16_t ap_count = 0;
	memset(ap_info, 0, sizeof(ap_info));
	// 扫描所有有效的AP 阻塞
	esp_wifi_scan_start(NULL, true);
	// 获取上次扫描中找到的AP列表
	ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
	// 获取上次扫描中找到的AP数获取上次扫描中找到的AP数
	ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
	ESP_LOGI(TAG, "Total APs scanned = %u", ap_count);
	for (int i = 0; (i < DEFAULT_SCAN_LIST_SIZE) && (i < ap_count); i++)
	{
		ESP_LOGI(TAG, "SSID \t\t%s", ap_info[i].ssid); // 名称
		ESP_LOGI(TAG, "RSSI \t\t%d", ap_info[i].rssi); // 信号强度
		print_auth_mode(ap_info[i].authmode);		   // wifi加密模式
		if (ap_info[i].authmode != WIFI_AUTH_WEP)
		{
			print_cipher_type(ap_info[i].pairwise_cipher, ap_info[i].group_cipher);
		}
		ESP_LOGI(TAG, "Channel \t\t%d\n", ap_info[i].primary); // 信道
	}
}
