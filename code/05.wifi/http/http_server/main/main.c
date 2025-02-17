#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include <sys/param.h>
#include "nvs_flash.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include <sys/socket.h>
#include "esp_tls_crypto.h"
#include <esp_http_server.h>
#include "esp_tls.h"
#include "esp_check.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "esp_netif.h"
#include "esp_tls_crypto.h"
#include <esp_http_server.h>
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_tls.h"
#include "esp_check.h"

// 要连接的WIFI
#define ESP_WIFI_STA_SSID "duruofu_win10"
#define ESP_WIFI_STA_PASSWD "1234567890"

static const char *TAG = "main";

#define EXAMPLE_HTTP_QUERY_KEY_MAX_LEN (64)

char form_html[] = R"(
<!DOCTYPE html>
<html>
<head>
<title>ESP32 Web Server</title>
</head>
<body>
<h1>ESP32 Web Server</h1>
<form action="http://192.168.137.55/echo" method="post">
	<label for="fname">Username:</label><br>
	<input type="text" id="fname" name="username" value="John"><br>
	<label for="lname">Age:</label><br>
	<input type="text" id="lname" name="age" value="Doe"><br><br>
	<input type="submit" value="Submit">
</form>
</body>
</html>)";

// WIFI回调
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


/*路由处理程序*/

/* An HTTP GET handler */
static esp_err_t hello_get_handler(httpd_req_t *req)
{
	char *buf;
	size_t buf_len;

	/* Get header value string length and allocate memory for length + 1,
	 * extra byte for null termination */
	buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
	if (buf_len > 1)
	{
		buf = malloc(buf_len);
		ESP_RETURN_ON_FALSE(buf, ESP_ERR_NO_MEM, TAG, "buffer alloc failed");
		/* Copy null terminated value string into buffer */
		if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK)
		{
			ESP_LOGI(TAG, "Found header => Host: %s", buf);
		}
		free(buf);
	}

	buf_len = httpd_req_get_hdr_value_len(req, "Test-Header-2") + 1;
	if (buf_len > 1)
	{
		buf = malloc(buf_len);
		ESP_RETURN_ON_FALSE(buf, ESP_ERR_NO_MEM, TAG, "buffer alloc failed");
		if (httpd_req_get_hdr_value_str(req, "Test-Header-2", buf, buf_len) == ESP_OK)
		{
			ESP_LOGI(TAG, "Found header => Test-Header-2: %s", buf);
		}
		free(buf);
	}

	buf_len = httpd_req_get_hdr_value_len(req, "Test-Header-1") + 1;
	if (buf_len > 1)
	{
		buf = malloc(buf_len);
		ESP_RETURN_ON_FALSE(buf, ESP_ERR_NO_MEM, TAG, "buffer alloc failed");
		if (httpd_req_get_hdr_value_str(req, "Test-Header-1", buf, buf_len) == ESP_OK)
		{
			ESP_LOGI(TAG, "Found header => Test-Header-1: %s", buf);
		}
		free(buf);
	}

	/* Read URL query string length and allocate memory for length + 1,
	 * extra byte for null termination */
	buf_len = httpd_req_get_url_query_len(req) + 1;
	if (buf_len > 1)
	{
		buf = malloc(buf_len);
		ESP_RETURN_ON_FALSE(buf, ESP_ERR_NO_MEM, TAG, "buffer alloc failed");
		if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK)
		{
			ESP_LOGI(TAG, "Found URL query => %s", buf);
			char param[EXAMPLE_HTTP_QUERY_KEY_MAX_LEN], dec_param[EXAMPLE_HTTP_QUERY_KEY_MAX_LEN] = {0};
			/* Get value of expected key from query string */
			if (httpd_query_key_value(buf, "query1", param, sizeof(param)) == ESP_OK)
			{
				ESP_LOGI(TAG, "Found URL query parameter => query1=%s", param);
				//example_uri_decode(dec_param, param, strnlen(param, EXAMPLE_HTTP_QUERY_KEY_MAX_LEN));
				ESP_LOGI(TAG, "Decoded query parameter => %s", dec_param);
			}
			if (httpd_query_key_value(buf, "query2", param, sizeof(param)) == ESP_OK)
			{
				ESP_LOGI(TAG, "Found URL query parameter => query2=%s", param);
				//example_uri_decode(dec_param, param, strnlen(param, EXAMPLE_HTTP_QUERY_KEY_MAX_LEN));
				ESP_LOGI(TAG, "Decoded query parameter => %s", dec_param);
			}
			if (httpd_query_key_value(buf, "query3", param, sizeof(param)) == ESP_OK)
			{
				ESP_LOGI(TAG, "Found URL query parameter => query3=%s", param);
				//example_uri_decode(dec_param, param, strnlen(param, EXAMPLE_HTTP_QUERY_KEY_MAX_LEN));
				ESP_LOGI(TAG, "Decoded query parameter => %s", dec_param);
			}
		}
		free(buf);
	}

	/* Set some custom headers */
	httpd_resp_set_hdr(req, "Custom-Header-1", "Custom-Value-1");
	httpd_resp_set_hdr(req, "Custom-Header-2", "Custom-Value-2");

	/* Send response with custom headers and body set as the
	 * string passed in user context*/
	const char *resp_str = (const char *)req->user_ctx;
	httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);

	/* After sending the HTTP response the old HTTP request
	 * headers are lost. Check if HTTP request headers can be read now. */
	if (httpd_req_get_hdr_value_len(req, "Host") == 0)
	{
		ESP_LOGI(TAG, "Request headers lost");
	}
	return ESP_OK;
}

static esp_err_t html_get_handler(httpd_req_t *req)
{
	// 指向 HTML 页面字符串的指针
	const char *html_content = form_html;

	// 设置 Content-Type 头
	httpd_resp_set_type(req, "text/html");

	// 发送 HTML 页面作为 HTTP 响应的正文部分
	httpd_resp_send(req, html_content, strlen(html_content));

	return ESP_OK;
}

/* An HTTP POST handler */
static esp_err_t echo_post_handler(httpd_req_t *req)
{
	/* 定义 HTTP POST 请求数据的目标缓存区
	 * httpd_req_recv() 只接收 char* 数据，但也可以是
	 * 任意二进制数据（需要类型转换）
	 * 对于字符串数据，null 终止符会被省略，
	 * content_len 会给出字符串的长度 */
	char buf[100];
	int ret, remaining = req->content_len;

	while (remaining > 0)
	{
		/* Read the data for the request */
		if ((ret = httpd_req_recv(req, buf,
															MIN(remaining, sizeof(buf)))) <= 0)
		{
			if (ret == HTTPD_SOCK_ERR_TIMEOUT)
			{
				/* Retry receiving if timeout occurred */
				continue;
			}
			/* 如果发生了错误，返回 ESP_FAIL 可以确保
			 * 底层套接字被关闭 */
			return ESP_FAIL;
		}

		/* Send back the same data */
		httpd_resp_send_chunk(req, buf, ret);
		remaining -= ret;

		/* Log data received */
		ESP_LOGI(TAG, "=========== RECEIVED DATA ==========");
		ESP_LOGI(TAG, "%.*s", ret, buf);
		ESP_LOGI(TAG, "====================================");
	}

	// End response
	httpd_resp_send_chunk(req, NULL, 0);
	return ESP_OK;
}

/* 注册路由 */
const httpd_uri_t hello = {
		.uri = "/hello",
		.method = HTTP_GET,
		.handler = hello_get_handler,
		.user_ctx = "Hello World!"};

const httpd_uri_t html = {
		.uri = "/html",
		.method = HTTP_GET,
		.handler = html_get_handler,
		.user_ctx = NULL};

const httpd_uri_t echo = {
		.uri = "/echo",
		.method = HTTP_POST,
		.handler = echo_post_handler,
		.user_ctx = NULL};


/* This handler allows the custom error handling functionality to be
 * tested from client side. For that, when a PUT request 0 is sent to
 * URI /ctrl, the /hello and /echo URIs are unregistered and following
 * custom error handler http_404_error_handler() is registered.
 * Afterwards, when /hello or /echo is requested, this custom error
 * handler is invoked which, after sending an error message to client,
 * either closes the underlying socket (when requested URI is /echo)
 * or keeps it open (when requested URI is /hello). This allows the
 * client to infer if the custom error handler is functioning as expected
 * by observing the socket state.
 */
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
	if (strcmp("/hello", req->uri) == 0)
	{
		httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/hello URI is not available");
		/* Return ESP_OK to keep underlying socket open */
		return ESP_OK;
	}
	else if (strcmp("/echo", req->uri) == 0)
	{
		httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/echo URI is not available");
		/* Return ESP_FAIL to close underlying socket */
		return ESP_FAIL;
	}
	/* For any other URI send 404 and close socket */
	httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
	return ESP_FAIL;
}

/* An HTTP PUT handler. This demonstrates realtime
 * registration and deregistration of URI handlers
 */
esp_err_t ctrl_put_handler(httpd_req_t *req)
{
	char buf;
	int ret;

	if ((ret = httpd_req_recv(req, &buf, 1)) <= 0)
	{
		if (ret == HTTPD_SOCK_ERR_TIMEOUT)
		{
			httpd_resp_send_408(req);
		}
		return ESP_FAIL;
	}

	if (buf == '0')
	{
		/* URI handlers can be unregistered using the uri string */
		ESP_LOGI(TAG, "Unregistering /hello and /echo URIs");
		httpd_unregister_uri(req->handle, "/hello");
		httpd_unregister_uri(req->handle, "/echo");
		/* Register the custom error handler */
		httpd_register_err_handler(req->handle, HTTPD_404_NOT_FOUND, http_404_error_handler);
	}
	else
	{
		ESP_LOGI(TAG, "Registering /hello and /echo URIs");
		httpd_register_uri_handler(req->handle, &hello);
		httpd_register_uri_handler(req->handle, &echo);
		/* Unregister custom error handler */
		httpd_register_err_handler(req->handle, HTTPD_404_NOT_FOUND, NULL);
	}

	/* Respond with empty body */
	httpd_resp_send(req, NULL, 0);
	return ESP_OK;
}


static const httpd_uri_t ctrl = {
		.uri = "/ctrl",
		.method = HTTP_PUT,
		.handler = ctrl_put_handler,
		.user_ctx = NULL};

/* 启动 Web 服务器的函数 */
httpd_handle_t start_webserver(void)
{
	/* 生成默认的配置参数 */
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();

	/* 置空 esp_http_server 的实例句柄 */
	httpd_handle_t server = NULL;

	ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
	/* 启动 httpd server */
	if (httpd_start(&server, &config) == ESP_OK)
	{
		/* 注册 URI 处理程序 */
		ESP_LOGI(TAG, "Registering URI handlers");
		httpd_register_uri_handler(server, &hello);
		httpd_register_uri_handler(server, &html);
		httpd_register_uri_handler(server, &echo);
		httpd_register_uri_handler(server, &ctrl);
	}
	/* 如果服务器启动失败，返回的句柄是 NULL */
	return server;
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

	vTaskDelay(3000 / portTICK_PERIOD_MS);

	/* Start the server for the first time */
	server = start_webserver();

	while (server)
	{
		sleep(5);
	}
}
