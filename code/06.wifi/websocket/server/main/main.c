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
#include <esp_http_server.h>
#include "esp_eth.h"

// 要连接的WIFI
#define ESP_WIFI_STA_SSID "duruofu_win10"
#define ESP_WIFI_STA_PASSWD "1234567890"

static const char *TAG = "main";

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

/*
 * Structure holding server handle
 * and internal socket fd in order
 * to use out of request send
 */
struct async_resp_arg
{
	httpd_handle_t hd;
	int fd;
};

/*
 * async send function, which we put into the httpd work queue
 */
static void ws_async_send(void *arg)
{
	static const char *data = "Async data";
	struct async_resp_arg *resp_arg = arg;
	httpd_handle_t hd = resp_arg->hd;
	int fd = resp_arg->fd;
	httpd_ws_frame_t ws_pkt;
	memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
	ws_pkt.payload = (uint8_t *)data;
	ws_pkt.len = strlen(data);
	ws_pkt.type = HTTPD_WS_TYPE_TEXT;

	httpd_ws_send_frame_async(hd, fd, &ws_pkt);
	free(resp_arg);
}

static esp_err_t trigger_async_send(httpd_handle_t handle, httpd_req_t *req)
{
	struct async_resp_arg *resp_arg = malloc(sizeof(struct async_resp_arg));
	if (resp_arg == NULL)
	{
		return ESP_ERR_NO_MEM;
	}
	resp_arg->hd = req->handle;
	resp_arg->fd = httpd_req_to_sockfd(req);
	esp_err_t ret = httpd_queue_work(handle, ws_async_send, resp_arg);
	if (ret != ESP_OK)
	{
		free(resp_arg);
	}
	return ret;
}

/*
 * This handler echos back the received ws data
 * and triggers an async send if certain message received
 */
static esp_err_t echo_handler(httpd_req_t *req)
{
	if (req->method == HTTP_GET)
	{
		ESP_LOGI(TAG, "Handshake done, the new connection was opened");
		return ESP_OK;
	}
	httpd_ws_frame_t ws_pkt;
	uint8_t *buf = NULL;
	memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
	ws_pkt.type = HTTPD_WS_TYPE_TEXT;
	/* Set max_len = 0 to get the frame len */
	// 读取数据
	esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
	if (ret != ESP_OK)
	{
		ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
		return ret;
	}
	ESP_LOGI(TAG, "frame len is %d", ws_pkt.len);
	if (ws_pkt.len)
	{
		/* ws_pkt.len + 1 is for NULL termination as we are expecting a string */
		buf = calloc(1, ws_pkt.len + 1);
		if (buf == NULL)
		{
			ESP_LOGE(TAG, "Failed to calloc memory for buf");
			return ESP_ERR_NO_MEM;
		}
		ws_pkt.payload = buf;
		/* Set max_len = ws_pkt.len to get the frame payload */
		ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
		if (ret != ESP_OK)
		{
			ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
			free(buf);
			return ret;
		}
		ESP_LOGI(TAG, "Got packet with message: %s", ws_pkt.payload);
	}
	ESP_LOGI(TAG, "Packet type: %d", ws_pkt.type);
	// 发送数据
	if (ws_pkt.type == HTTPD_WS_TYPE_TEXT &&
			strcmp((char *)ws_pkt.payload, "Trigger async") == 0)
	{
		free(buf);
		return trigger_async_send(req->handle, req);
	}

	ret = httpd_ws_send_frame(req, &ws_pkt);
	if (ret != ESP_OK)
	{
		ESP_LOGE(TAG, "httpd_ws_send_frame failed with %d", ret);
	}
	free(buf);
	return ret;
}

static const httpd_uri_t ws = {
		.uri = "/ws",
		.method = HTTP_GET,
		.handler = echo_handler,
		.user_ctx = NULL,
		.is_websocket = true};
		
static httpd_handle_t start_webserver(void)
{
	httpd_handle_t server = NULL;
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();

	// Start the httpd server
	ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
	if (httpd_start(&server, &config) == ESP_OK)
	{
		// Registering the ws handler
		ESP_LOGI(TAG, "Registering URI handlers");
		httpd_register_uri_handler(server, &ws);
		return server;
	}

	ESP_LOGI(TAG, "Error starting server!");
	return NULL;
}

static esp_err_t stop_webserver(httpd_handle_t server)
{
	// Stop the httpd server
	return httpd_stop(server);
}

void app_main(void)
{
	static httpd_handle_t server = NULL;

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

	// 创建WebSocket服务器
	server = start_webserver();
}
