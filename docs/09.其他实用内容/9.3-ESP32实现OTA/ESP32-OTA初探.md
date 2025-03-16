# ESP32-OTA初探

> [!TIP] 🚀 ESP32 OTA 固件升级 | 让设备秒变“自我进化”! 
> - 💡 **碎碎念**😎：本文档记录如何实现 ESP32 从指定的 URL 更新固件的 OTA 功能，并讲解官方示例程序。<br/>  
> - 📺 **视频教程**：🚧 *开发中*<br/>  
> - 💾 **示例程序**：✅ [点击查看代码] <br/>  
> - 📚 **官网文档**： [API 参考 » 系统 API » 空中升级 (OTA)](https://docs.espressif.com/projects/esp-idf/zh_CN/stable/esp32/api-reference/system/ota.html)<br/>  
> - 💻 **官方示例程序**： [esp-idf/examples/system/ota](https://github.com/espressif/esp-idf/tree/v5.2.4/examples/system/ota)  

## 一、OTA介绍

OTA（Over-The-Air） 技术是通过无线网络对设备进行远程更新和管理的方式。简单来说，就是“空中下载”技术，让设备不需要物理连接就能接收固件、软件更新，或者配置变更。

OTA的基本流程如下：
1. 检测更新：设备通过HTTP/MQTT等协议向服务器请求检查是否有新固件。
2. 下载固件：确认有新版本后，设备开始下载固件，一般采用分块下载以减少错误率。
3. 校验完整性：下载完成后通过校验（如MD5、SHA256）确保文件未损坏。
4. 写入并重启：将固件写入Flash的特定分区，然后重启设备进入新固件。
5. 回滚机制（可选）：若新固件启动失败，设备自动回滚到旧版本。

ESP32原生支持OTA，使用ESP-IDF提供的API可以方便地实现远程升级。ESP32通常使用双分区（A/B分区）模式进行OTA，这样可以确保升级失败时还能从旧版本启动。
## 二、尝试编译使用官方示例





## 三、示例程序解读







## 参考：

5. https://espressif-docs.readthedocs-hosted.com/projects/espressif-esp-moonlight/zh-cn/latest/firmwareupgrade.html