#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include <sys/socket.h>
#include "esp_eth.h"
#include "esp_websocket_client.h"
#include <cJSON.h>

// 要连接的WIFI
#define ESP_WIFI_STA_SSID "duruofu_win10"
#define ESP_WIFI_STA_PASSWD "1234567890"

// 目标服务器
#define CONFIG_WEBSOCKET_URI "ws://192.168.137.1:5000"
#define NO_DATA_TIMEOUT_SEC 5

static const char *TAG = "main";

static TimerHandle_t shutdown_signal_timer;
static SemaphoreHandle_t shutdown_sema;

void WIFI_CallBack(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
	static uint8_t connect_count = 0;
	// WIFI 启动成功
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
	{
		ESP_LOGI("WIFI_EVENT", "WIFI_EVENT_STA_START");
		ESP_ERROR_CHECK(esp_wifi_connect());
	}
	// WIFI 连接失败
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
	{
		ESP_LOGI("WIFI_EVENT", "WIFI_EVENT_STA_DISCONNECTED");
		connect_count++;
		if (connect_count < 6)
		{
			vTaskDelay(1000 / portTICK_PERIOD_MS);
			ESP_ERROR_CHECK(esp_wifi_connect());
		}
		else
		{
			ESP_LOGI("WIFI_EVENT", "WIFI_EVENT_STA_DISCONNECTED 10 times");
		}
	}
	// WIFI 连接成功(获取到了IP)
	if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
	{
		ESP_LOGI("WIFI_EVENT", "WIFI_EVENT_STA_GOT_IP");
		ip_event_got_ip_t *info = (ip_event_got_ip_t *)event_data;
		ESP_LOGI("WIFI_EVENT", "got ip:" IPSTR "", IP2STR(&info->ip_info.ip));
	}
}

// wifi初始化
static void wifi_sta_init(void)
{
	ESP_ERROR_CHECK(esp_netif_init());

	// 注册事件(wifi启动成功)
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_STA_START, WIFI_CallBack, NULL, NULL));
	// 注册事件(wifi连接失败)
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, WIFI_CallBack, NULL, NULL));
	// 注册事件(wifi连接失败)
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, WIFI_CallBack, NULL, NULL));

	// 初始化STA设备
	esp_netif_create_default_wifi_sta();

	/*Initialize WiFi */
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	// WIFI_INIT_CONFIG_DEFAULT 是一个默认配置的宏

	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	//----------------配置阶段-------------------
	// 初始化WIFI设备( 为 WiFi 驱动初始化 WiFi 分配资源，如 WiFi 控制结构、RX/TX 缓冲区、WiFi NVS 结构等，这个 WiFi 也启动 WiFi 任务。必须先调用此API，然后才能调用所有其他WiFi API)
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
}

static void log_error_if_nonzero(const char *message, int error_code)
{
	if (error_code != 0)
	{
		ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
	}
}

static void shutdown_signaler(TimerHandle_t xTimer)
{
	ESP_LOGI(TAG, "No data received for %d seconds, signaling shutdown", NO_DATA_TIMEOUT_SEC);
	xSemaphoreGive(shutdown_sema);
}

// WebSocket客户端事件处理
static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
	esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
	switch (event_id)
	{
	// 连接成功
	case WEBSOCKET_EVENT_CONNECTED:
		ESP_LOGI(TAG, "WEBSOCKET_EVENT_CONNECTED");
		break;
	// 连接断开
	case WEBSOCKET_EVENT_DISCONNECTED:
		ESP_LOGI(TAG, "WEBSOCKET_EVENT_DISCONNECTED");
		log_error_if_nonzero("HTTP status code", data->error_handle.esp_ws_handshake_status_code);
		if (data->error_handle.error_type == WEBSOCKET_ERROR_TYPE_TCP_TRANSPORT)
		{
			log_error_if_nonzero("reported from esp-tls", data->error_handle.esp_tls_last_esp_err);
			log_error_if_nonzero("reported from tls stack", data->error_handle.esp_tls_stack_err);
			log_error_if_nonzero("captured as transport's socket errno", data->error_handle.esp_transport_sock_errno);
		}
		break;
	// 收到数据
	case WEBSOCKET_EVENT_DATA:
		ESP_LOGI(TAG, "WEBSOCKET_EVENT_DATA");
		ESP_LOGI(TAG, "Received opcode=%d", data->op_code);
		if (data->op_code == 0x08 && data->data_len == 2)
		{
			ESP_LOGW(TAG, "Received closed message with code=%d", 256 * data->data_ptr[0] + data->data_ptr[1]);
		}
		else
		{
			ESP_LOGW(TAG, "Received=%.*s\n\n", data->data_len, (char *)data->data_ptr);
		}

		// If received data contains json structure it succeed to parse
		cJSON *root = cJSON_Parse(data->data_ptr);
		if (root)
		{
			for (int i = 0; i < cJSON_GetArraySize(root); i++)
			{
				cJSON *elem = cJSON_GetArrayItem(root, i);
				cJSON *id = cJSON_GetObjectItem(elem, "id");
				cJSON *name = cJSON_GetObjectItem(elem, "name");
				ESP_LOGW(TAG, "Json={'id': '%s', 'name': '%s'}", id->valuestring, name->valuestring);
			}
			cJSON_Delete(root);
		}

		ESP_LOGW(TAG, "Total payload length=%d, data_len=%d, current payload offset=%d\r\n", data->payload_len, data->data_len, data->payload_offset);

	  // 定时器复位
		xTimerReset(shutdown_signal_timer, portMAX_DELAY);
		break;
	// 错误
	case WEBSOCKET_EVENT_ERROR:
		ESP_LOGI(TAG, "WEBSOCKET_EVENT_ERROR");
		log_error_if_nonzero("HTTP status code", data->error_handle.esp_ws_handshake_status_code);
		if (data->error_handle.error_type == WEBSOCKET_ERROR_TYPE_TCP_TRANSPORT)
		{
			log_error_if_nonzero("reported from esp-tls", data->error_handle.esp_tls_last_esp_err);
			log_error_if_nonzero("reported from tls stack", data->error_handle.esp_tls_stack_err);
			log_error_if_nonzero("captured as transport's socket errno", data->error_handle.esp_transport_sock_errno);
		}
		break;
	}
}

// WebSocket客户端
static void websocket_app_start(void)
{
	esp_websocket_client_config_t websocket_cfg = {};

  // 创建定时器
	shutdown_signal_timer = xTimerCreate("Websocket shutdown timer", NO_DATA_TIMEOUT_SEC * 1000 / portTICK_PERIOD_MS,
																			 pdFALSE, NULL, shutdown_signaler);
	// 创建信号量
	shutdown_sema = xSemaphoreCreateBinary();

	// 配置目标服务器
	websocket_cfg.uri = CONFIG_WEBSOCKET_URI;

	ESP_LOGI(TAG, "Connecting to %s...", websocket_cfg.uri);

	// 创建WebSocket客户端
	esp_websocket_client_handle_t client = esp_websocket_client_init(&websocket_cfg);
	// 注册事件
	esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);
	// 启动WebSocket客户端
	esp_websocket_client_start(client);
	xTimerStart(shutdown_signal_timer, portMAX_DELAY);
	char data[32];
	int i = 0;
	// 发送5次数据
	while (i < 5)
	{
		if (esp_websocket_client_is_connected(client))
		{
			int len = sprintf(data, "hello %04d", i++);
			ESP_LOGI(TAG, "Sending %s", data);
			// 发送文本数据
			esp_websocket_client_send_text(client, data, len, portMAX_DELAY);
		}
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}

	ESP_LOGI(TAG, "Sending fragmented message");
	vTaskDelay(1000 / portTICK_PERIOD_MS);
	memset(data, 'a', sizeof(data));
	esp_websocket_client_send_text_partial(client, data, sizeof(data), portMAX_DELAY);
	memset(data, 'b', sizeof(data));
	esp_websocket_client_send_cont_msg(client, data, sizeof(data), portMAX_DELAY);
	esp_websocket_client_send_fin(client, portMAX_DELAY);

	// 等待信号量
	xSemaphoreTake(shutdown_sema, portMAX_DELAY);
	// 关闭WebSocket客户端
	esp_websocket_client_close(client, portMAX_DELAY);
	ESP_LOGI(TAG, "Websocket Stopped");
	// 销毁WebSocket客户端
	esp_websocket_client_destroy(client);
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

	// 创建默认事件循环
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	// 配置启动WIFI
	wifi_sta_init();

	// 等待wifi连接成功(暂时这样处理)
	vTaskDelay(5000 / portTICK_PERIOD_MS);

	// 创建WebSocket客户端
	websocket_app_start();
}
