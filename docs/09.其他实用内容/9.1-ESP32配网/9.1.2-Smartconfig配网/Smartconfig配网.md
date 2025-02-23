# Smartconfig配网

> [!TIP] 🚀 Smartconfig配网 | 快速连接WiFi  
> - 💡 **碎碎念**😎：本节介绍如何使用 Smartconfig 协议完成 ESP32 的 WiFi 配网程序，并讲解官方提供的示例程序。  
> - 📺 **视频教程**：暂无  
> - 💾 **示例程序**：[ESP32-Guide/code/09.extra/wifi_config/smart_config](https://github.com/DuRuofu/ESP32-Guide/tree/main/code/09.extra/wifi_config/smart_config)
> - 📚 **相关文档**：[ API 参考 » 连网 API » SmartConfig](https://docs.espressif.com/projects/esp-idf/zh_CN/v5.3.2/esp32/api-reference/network/esp_smartconfig.html?highlight=smartconfig)

## 一、介绍
SmartConfig 是由 TI 开发的配网技术，用于将新的 Wi-Fi 设备连接到 Wi-Fi 网络。它使用移动应用程序将无线网凭据从智能手机或平板电脑端广播给未配网的 Wi-Fi 设备。

这项技术的优势在于，设备无需直接获知 AP 的 SSID 或密码，而是通过智能手机获取。这对于没有用户界面的无头设备和系统而言十分重要。

目前， ESP32 支持三种类型的 SmartConfig 配网： Airkiss、ESPTouch 和 ESPTouch v2。ESPTouch v2 自 SmartConfig v3.0 （SmartConfig 的版本可以从 esp_smartconfig_get_version() 获取）起开始支持，ESPTouch v2 和 vESPTouch 采用完全不同的配网算法，因此配网速度更快。此外，ESPTouch v2 还增加了 AES 加密功能和自定义数据字段。

从 SmartConfig v3.0.2 开始，ESPTouch v2 的 AES 加密支持随机 IV。在应用程序端，当随机 IV 的选项关闭的时候，默认的 IV 为 0，与旧版本保持一致，当随机 IV 的选项打开的时候，IV 为随机值。需要注意的是，当启用 AES 加密且 IV 为随机值时，配网时间会延长，因为需要将 IV 传输到配网设备。在配网设备端，设备会根据配网包中的 flag 来识别 AES 的随机 IV 是否开启。
## 二、尝试编译使用官方示例

官方示例程序链接：[smart_config](https://github.com/espressif/esp-idf/tree/master/examples/wifi/smart_config)，编译下载：

![](attachments/Pasted%20image%2020250222114852.png)

下载后串口打印如下，显示开始等待配网：

![](attachments/Pasted%20image%2020250222114936.png)

使用乐鑫提供的APP进行配网：

![](attachments/Pasted%20image%2020250222115304.png)

点击ESPTouch进行配网，手机需要先连接上对应的Wifi:

![](attachments/Pasted%20image%2020250222115412.png)

输入密码后点击确定，等待几秒时间后，配网完成：

![](attachments/Pasted%20image%2020250222115451.png)

ESP32的串口输出如下：

![](attachments/Pasted%20image%2020250222115148.png)


ESP32 支持三种类型的 SmartConfig 配网： Airkiss、ESPTouch 和 ESPTouch v2  使用哪种类型取决于ESP32程序配置。
当配网完成后，ESP32下一次启动，也会去尝试直接连接上一次的设备，如果我们要切换网络，可以使用`esptool.py erase_flash`命令擦除flash后重新烧写程序，使ESP32回到初始状态。也可以自行编写程序调用`esp_wifi_restore()`函数来重置WIFI配置（比如按一个重置按键，调用`esp_wifi_restore()`来重置）。

## 三、示例代码解读

相比于Bluf示例，SmartConfig配网的示例程序要更简单一些，只有一个`smartconfig_main.c`文件，程序展示了如何使用ESP32的智能配网功能（如EspTouch或AirKiss）将设备连接到Wi-Fi网络。程序通过事件驱动的方式处理Wi-Fi连接和智能配网过程，并在成功连接后停止配网任务。

程序的最开始，定义了一个事件组句柄和两个标志位：

```c
static EventGroupHandle_t s_wifi_event_group;
static const int CONNECTED_BIT = BIT0;
static const int ESPTOUCH_DONE_BIT = BIT1;
```
- `s_wifi_event_group` 是一个事件组句柄，用于同步Wi-Fi连接和智能配网完成的事件。
- `CONNECTED_BIT` 和 `ESPTOUCH_DONE_BIT` 是事件标志位，分别表示Wi-Fi连接成功和智能配网完成。


主函数非常简单，就一个初始化NVS闪存和初始化Wi-Fi。

```c
void app_main(void)
{
    ESP_ERROR_CHECK( nvs_flash_init() );
    initialise_wifi();
}
```

`nvs_flash_init()`就不必说了，我们先看看`initialise_wifi()`:

```c
static void initialise_wifi(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );

    ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL) );
    ESP_ERROR_CHECK( esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL) );
    ESP_ERROR_CHECK( esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL) );

    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}
```

主要做了下面几件事：

- 初始化网络接口和事件循环。
- 创建默认的Wi-Fi Station接口。
- 初始化Wi-Fi并设置事件处理函数。
- 设置Wi-Fi模式为Station模式并启动Wi-Fi。

其中的`event_handler` 是一个事件处理函数，用于处理Wi-Fi和智能配网相关的事件，我们需要关注一下事件处理里面（`event_handler`）干了什么：

```c
static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        xTaskCreate(smartconfig_example_task, "smartconfig_example_task", 4096, NULL, 3, NULL);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE) {
        ESP_LOGI(TAG, "Scan done");
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL) {
        ESP_LOGI(TAG, "Found channel");
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) {
        ESP_LOGI(TAG, "Got SSID and password");

        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
        wifi_config_t wifi_config;
        uint8_t ssid[33] = { 0 };
        uint8_t password[65] = { 0 };
        uint8_t rvd_data[33] = { 0 };

        bzero(&wifi_config, sizeof(wifi_config_t));
        memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
        memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));

#ifdef CONFIG_SET_MAC_ADDRESS_OF_TARGET_AP
        wifi_config.sta.bssid_set = evt->bssid_set;
        if (wifi_config.sta.bssid_set == true) {
            ESP_LOGI(TAG, "Set MAC address of target AP: "MACSTR" ", MAC2STR(evt->bssid));
            memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));
        }
#endif

        memcpy(ssid, evt->ssid, sizeof(evt->ssid));
        memcpy(password, evt->password, sizeof(evt->password));
        ESP_LOGI(TAG, "SSID:%s", ssid);
        ESP_LOGI(TAG, "PASSWORD:%s", password);
        if (evt->type == SC_TYPE_ESPTOUCH_V2) {
            ESP_ERROR_CHECK( esp_smartconfig_get_rvd_data(rvd_data, sizeof(rvd_data)) );
            ESP_LOGI(TAG, "RVD_DATA:");
            for (int i=0; i<33; i++) {
                printf("%02x ", rvd_data[i]);
            }
            printf("\n");
        }

        ESP_ERROR_CHECK( esp_wifi_disconnect() );
        ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
        esp_wifi_connect();
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE) {
        xEventGroupSetBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
    }
}
```

- `WIFI_EVENT_STA_START`：当Wi-Fi Station模式启动时，创建智能配网任务。
- `WIFI_EVENT_STA_DISCONNECTED`：当Wi-Fi断开连接时，重新尝试连接。
- `IP_EVENT_STA_GOT_IP`：当获取到IP地址时，设置`CONNECTED_BIT`标志位。
- `SC_EVENT_GOT_SSID_PSWD`：当获取到SSID和密码时，配置Wi-Fi并尝试连接。
- `SC_EVENT_SEND_ACK_DONE`：当智能配网完成时，设置`ESPTOUCH_DONE_BIT`标志位。

最后看看当Wi-Fi Station模式启动时，创建智能配网任务内容：

```c
static void smartconfig_example_task(void * parm)
{
    EventBits_t uxBits;
    ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH_AIRKISS));
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_smartconfig_start(&cfg) );
    while (1) {
        uxBits = xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT | ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY);
        if(uxBits & CONNECTED_BIT) {
            ESP_LOGI(TAG, "WiFi Connected to ap");
        }
        if(uxBits & ESPTOUCH_DONE_BIT) {
            ESP_LOGI(TAG, "smartconfig over");
            esp_smartconfig_stop();
            vTaskDelete(NULL);
        }
    }
}
```

程序内容：
- 设置智能配网类型为`SC_TYPE_ESPTOUCH_AIRKISS`。（对应EspTouch或AirKiss）
- 启动智能配网。
- 等待`CONNECTED_BIT`和`ESPTOUCH_DONE_BIT`事件。
- 当Wi-Fi连接成功时，打印日志。
- 当智能配网完成时，停止智能配网并删除任务。