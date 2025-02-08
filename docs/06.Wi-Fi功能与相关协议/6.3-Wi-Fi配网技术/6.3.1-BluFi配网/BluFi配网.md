
### 说明：

1. 本文档由DuRuofu撰写，由DuRuofu负责解释及执行。
2. 本文档记录如何使用BluFi完成ESP32的WiFi配网程序。

### 修订历史：

| 文档名称              | 版本     | 作者      | 时间         | 备注   |
| ----------------- | ------ | ------- | ---------- | ---- |
| ESP32网络入门-BluFi配网 | v1.0.0 | DuRuofu | 2024-04-03 | 首次建立 |

<div STYLE="page-break-after: always;"></div>

# ESP32网络入门-BluFi配网

## 一、介绍

### 1.1 BluFi是什么

BluFi 是一项基于蓝牙通道的 Wi-Fi 网络配置功能，适用于 ESP32。它通过安全协议将 Wi-Fi 的 SSID、密码等配置信息传输到 ESP32。基于这些信息，ESP32 可进而连接到 AP 或建立 SoftAP。

BluFi 流程的关键部分包括数据的分片、加密以及校验和验证。

用户可按需自定义用于对称加密、非对称加密以及校验的算法。此处，我们采用 DH 算法进行密钥协商，128-AES 算法用于数据加密，CRC16 算法用于校验和验证。


### 1.2 流程

BluFi 配网流程包含配置 SoftAP 和配置 Station 两部分。

![](attachments/Pasted%20image%2020240403181235.png)

1. ESP32 开启 GATT Server 模式，发送带有特定 _advertising data_ 的广播。该广播不属于 BluFi Profile，可以按需对其进行自定义。
2. 使用手机应用程序搜索到该广播后，手机将作为 GATT Client 连接 ESP32。该步骤对具体使用哪款手机应用程序并无特殊要求。
3. 成功建立 GATT 连接后，手机会向 ESP32 发送数据帧进行密钥协商（详见 [BluFi 中定义的帧格式](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-guides/blufi.html#frame-formats) ）。
4. ESP32 收到密钥协商的数据帧后，会按照自定义的协商方法进行解析。
5. 手机与 ESP32 进行密钥协商。协商过程可使用 DH/RSA/ECC 等加密算法。
6. 协商结束后，手机端向 ESP32 发送控制帧，用于设置安全模式。
7. ESP32 收到控制帧后，使用共享密钥以及安全配置对通信数据进行加密和解密。
8. 手机向 ESP32 发送 [BluFi 中定义的帧格式](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-guides/blufi.html#frame-formats) 中定义的数据帧，包括 SSID、密码等 Wi-Fi 配置信息。
9. 手机向 ESP32 发送 Wi-Fi 连接请求的控制帧。ESP32 收到控制帧后，即默认手机已完成必要信息的传输，准备连接 Wi-Fi。
10. 连接到 Wi-Fi 后，ESP32 发送 Wi-Fi 连接状态报告的控制帧到手机。至此，配网结束。

>1. ESP32 收到安全模式配置的控制帧后，会根据定义的安全模式进行相关操作。
>2. 进行对称加密和解密时，加密和解密前后的数据长度必须一致。支持原地加密和解密。





## 二、使用



## 三、示例


# 参考链接