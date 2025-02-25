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
#include "lwip/err.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>


// 要连接的WIFI
#define ESP_WIFI_STA_SSID "duruofu_win10"
#define ESP_WIFI_STA_PASSWD "1234567890"

// 服务器端口
#define UDP_SREVER_PORT 8080

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

// udp客户端
static void udp_server_task(void *pvParameters)
{
	// 等待wifi连接成功(暂时这样处理)
	vTaskDelay(5000 / portTICK_PERIOD_MS);
	ESP_LOGI("udp_server_task", "udp_server_task start");

	// 创建socket
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) // 创建失败返回-1
	{
		ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
		return;
	}

	// 设置服务器(IPV4)
	struct sockaddr_in server_config;
	server_config.sin_addr.s_addr = htonl(INADDR_ANY);
	server_config.sin_family = AF_INET;
	server_config.sin_port = htons(UDP_SREVER_PORT); // 宏htons 用于将主机的无符号短整型数据转换成网络字节顺序(小端转大端)
	
	// 绑定端口
	int err = bind(sock, (struct sockaddr *)&server_config, sizeof(server_config));
	if (err < 0)
	{
		ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
	}
	ESP_LOGI(TAG, "Socket bound, port %d", UDP_SREVER_PORT);

	char rx_buffer[1024];
	// 接收数据,并发回
	while(1)
	{
		// 清空缓存
		memset(rx_buffer, 0, sizeof(rx_buffer));
		// 接收数据

		struct sockaddr_in source_addr;
		socklen_t socklen = sizeof(source_addr);
		int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);
		if (len < 0)
		{
			ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
			break;
		}
		else
		{
			// 打印接收到的数据
			ESP_LOGI(TAG, "Received %d bytes: %s", len, rx_buffer);
			// 发送数据
			int err = sendto(sock, rx_buffer, len, 0, (struct sockaddr *)&source_addr, sizeof(source_addr));
			if (err < 0)
			{
				ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
				break;
			}
			ESP_LOGI(TAG, "Send success");
		}
	}
	// 关闭socket
	close(sock);
	vTaskDelete(NULL);
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

	// 创建TCP客户端任务
	xTaskCreate(udp_server_task, "udp_server_task", 4096, NULL, 5, NULL);
}
