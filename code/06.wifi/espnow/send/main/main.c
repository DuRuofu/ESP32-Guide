#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "nvs_flash.h"
#include "esp_random.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_now.h"
#include "esp_crc.h"

// 接收方地址
//static uint8_t s_broadcast_mac[ESP_NOW_ETH_ALEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static uint8_t s_broadcast_mac[ESP_NOW_ETH_ALEN] = {0x24, 0xdc, 0xc3, 0x9c, 0xeb, 0x18};

static const char *TAG = "espnow";

/* WiFi should start before using ESPNOW */
static void wifi_init(void)
{
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_start());

}

static void example_espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status)
{
	if (status != ESP_NOW_SEND_SUCCESS)
	{
		ESP_LOGE(TAG, "Send error");
	}
	else
	{
		ESP_LOGI(TAG, "Send success");
	}
}

// ESPNOW发送任务
static void espnow_task(void *pvParameter)
{
	char send_msg[] = "ESPNOW test!";
	ESP_LOGI(TAG, "Start sending broadcast data");
	while (1)
	{
		// 发送数据
		if (esp_now_send(s_broadcast_mac, (uint8_t *)send_msg, sizeof(send_msg)) != ESP_OK)
		{
			ESP_LOGE(TAG, "Send error");
		}
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

static esp_err_t espnow_init(void)
{

	/* Initialize ESPNOW and register sending and receiving callback function. */
	ESP_ERROR_CHECK(esp_now_init());
	// 注册发送回调函数
	ESP_ERROR_CHECK(esp_now_register_send_cb(example_espnow_send_cb));
	//ESP_ERROR_CHECK(esp_now_register_recv_cb(example_espnow_recv_cb));

	// 将广播对等方信息添加到对等方列表
	esp_now_peer_info_t *peer = malloc(sizeof(esp_now_peer_info_t));
	if (peer == NULL)
	{
		ESP_LOGE(TAG, "Malloc peer information fail");
		esp_now_deinit();
		return ESP_FAIL;
	}
	// 初始化对等方信息
	memset(peer, 0, sizeof(esp_now_peer_info_t));
	peer->channel = 1;
	peer->ifidx = ESP_IF_WIFI_STA;
	peer->encrypt = false;
	memcpy(peer->peer_addr, s_broadcast_mac, ESP_NOW_ETH_ALEN);
	// 添加对等方
	ESP_ERROR_CHECK(esp_now_add_peer(peer));
	free(peer);

	xTaskCreate(espnow_task, "espnow_task", 2048, NULL, 4, NULL);

	return ESP_OK;
}

void app_main(void)
{
	// Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	// Initialize WiFi
	wifi_init();

	// Initialize ESPNOW
	espnow_init();
}
