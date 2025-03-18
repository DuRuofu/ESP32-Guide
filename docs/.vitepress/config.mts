import { defineConfig } from 'vitepress'

// 使用 defineConfig 定义 VitePress 的配置
export default defineConfig(

  {
    // 网站的标题
    title: "ESP32-Guide",

    // 网站的描述，通常用于 SEO 和元数据
    description: "ESP32-Guide从初见到入门",

    base: '/ESP32-Guide/',

    // 网站头部设置
    head: [['link', { rel: 'icon', href: '/ESP32-Guide/favicon.ico' }]],
    // markdown配置
    markdown: {
      //显示行数
      lineNumbers: true,
      //中文配置
      container: {
        tipLabel: "提示",
        warningLabel: "警告",
        noteLabel: "注意",
        dangerLabel: "危险",
        detailsLabel: "详情",
        infoLabel: "信息",
      }
    },
    // 主题配置部分
    themeConfig: {
      //logo设置
      logo: "/logo.png",
      siteTitle: "ESP32-Guide",
      //网站底部设置
      footer: {
        //底部版权
        copyright: " Copyright © 2023 - ESP32-Guide.All Rights Reserved. ",
      },
      //本地搜索
      search: {
        provider: 'local'
      },

      // 编辑链接设置
      editLink: {
        pattern: 'https://github.com/DuRuofu/ESP32-Guide/tree/main/docs/:path',
        text: '在GitHub中编辑此页'
      },

      // 导航栏配置
      nav: [
        { text: '首页', link: '/' }, // 首页导航项

        {
          text: '🔥ESP32教程',
          link: 'docs/guide/目录.md',
        },
        { text: '关于', link: 'docs/about/About.md'},
      ],

      // // 侧边栏配置
      sidebar: {
        "docs/guide/": [
        {
            text: '目录',
            link: 'docs/guide/目录.md',
          items: [
            {
              text: '〇、常见问题及解决',
              link: 'docs/guide/00.常见问题及解决方案/常见问题及解决方案.md',
            },
            {
              text: '一、认识ESP32',
              collapsed: false,
              items: [
                { text: '1.1-ESP32介绍', link: 'docs/guide/01.认识ESP32/1.1-ESP32介绍/ESP32介绍.md' },
                { text: '1.2.1-环境搭建-Windows', link: 'docs/guide/01.认识ESP32/1.2-ESP32开发环境搭建/1.环境搭建-Windows.md' },
                { text: '1.2.2-环境搭建-Ubuntu', link: 'docs/guide/01.认识ESP32/1.2-ESP32开发环境搭建/2.环境搭建-Ubuntu虚拟机.md' },
                { text: '1.2.3-环境搭建Windows+Ubuntu', link: 'docs/guide/01.认识ESP32/1.2-ESP32开发环境搭建/3.环境搭建-Windows与虚拟机组合使用.md' },
                // { text: '1.2.4-环境搭建-WSL2', link: '/01.认识ESP32/1.2-ESP32开发环境搭建/4.环境搭建-WSL2.md' },
                { text: '1.3-IDF.py的基本使用', link: 'docs/guide/01.认识ESP32/1.3-初试ESP32-idf.py的基本使用/idf.py的基本使用.md' },
              ]
            },
            {
              text: '二、IDF基础与工具使用',
              collapsed: false,
              items: [
                { text: '2.1-ESP32工程结构', link: 'docs/guide/02.ESP-IDF基础/2.1-ESP32工程结构/ESP-IDF的工程结构.md' },
                { text: '2.2-ESP-IDF项目配置', link: 'docs/guide/02.ESP-IDF基础/2.2-ESP-IDF项目配置/ESP-IDF项目配置.md' },
                { text: '2.2-ESP-IDF自定义组件', link: 'docs/guide/02.ESP-IDF基础/2.3-ESP-IDF自定义组件/ESP-IDF自定义组件.md' },
                { text: '2.2-ESP-IDF组件管理器', link: 'docs/guide/02.ESP-IDF基础/2.4-ESP-IDF组件管理器/ESP-IDF组件管理器.md' },
              ]
            },
            {
              text: '三、FreeRTOS基础',
              collapsed: false,
              items: [
                { text: '3.1-FreeRTOS介绍与引入', link: 'docs/guide/03.FreeRTOS基础/3.1-FreeRTOS概述/FreeRTOS概述.md' },
                { text: '3.2-FreeRTOS多任务管理', link: 'docs/guide/03.FreeRTOS基础/3.2-FreeRTOS多任务与任务管理/FreeRTOS多任务与任务管理.md' },
                { text: '3.3-FreeRTOS任务看门狗', link: 'docs/guide/03.FreeRTOS基础/3.3-FreeRTOS任务看门狗/FreeRTOS任务看门狗.md' },
                { text: '3.4-FreeRTOS软件定时器', link: 'docs/guide/03.FreeRTOS基础/3.4-FreeRTOS软件定时器/FreeRTOS软件定时器.md' },
              ]
            },
            {
              text: '四、外设学习',
              collapsed: false,
              items: [
                {
                  text: '4.1-基础外设',
                  collapsed: false,
                  items: [
                    { text: '4.1.1-GPIO', link: 'docs/guide/04.外设学习/4.1-基础外设/4.1.1-GPIO/GPIO入门.md' },
                    { text: '4.1.2-UART', link: 'docs/guide/04.外设学习/4.1-基础外设/4.1.2-UART/UART串口通信.md' },
                    { text: '4.1.3-定时器  ', link: 'docs/guide/04.外设学习/4.1-基础外设/4.1.3-定时器/定时器.md' },
                    { text: '4.1.4-ADC', link: 'docs/guide/04.外设学习/4.1-基础外设/4.1.4-ADC/ADC模数转换.md' },
                    { text: '4.1.5-DAC', link: 'docs/guide/04.外设学习/4.1-基础外设/4.1.5-DAC/DAC数模转换.md' },
                    { text: '4.1.6-LEDPWM', link: 'docs/guide/04.外设学习/4.1-基础外设/4.1.6-LED_PWM/LED_PWM控制.md' },
                    { text: '4.1.7-I2C', link: 'docs/guide/04.外设学习/4.1-基础外设/4.1.7-I2C/I2C通信.md' },
                    { text: '4.1.8-SPI', link: 'docs/guide/04.外设学习/4.1-基础外设/4.1.8-SPI/SPI通信.md' },
                    { text: '4.1.9-I2S', link: 'docs/guide/04.外设学习/4.1-基础外设/4.1.9-I2S/I2S入门.md' },
                  ]
                },
                {
                  text: '4.2-存储外设',
                  collapsed: false,
                  items: [
                    { text: '4.2.1-分区表', link: 'docs/guide/04.外设学习/4.2-存储外设/4.2.1-分区表/分区表.md' },
                    { text: '4.2.2-非易失性存储', link: 'docs/guide/04.外设学习/4.2-存储外设/4.2.2-非易失性存储/非易失性存储.md' },
                    { text: '4.2.3-FAT文件系统', link: 'docs/guide/04.外设学习/4.2-存储外设/4.2.3-FAT文件系统/FAT文件系统.md' },
                    { text: '4.2.4-SPIFFS文件系统', link: 'docs/guide/04.外设学习/4.2-存储外设/4.2.4-SPIFFS文件系统/SPIFFS文件系统.md' },
                    { text: '4.2.5-SD、SDIO、MMC驱动', link: 'docs/guide/04.外设学习/4.2-存储外设/4.2.5-SD卡驱动/SD卡驱动.md' },
                  ]
                },
                {
                  text: '4.3-其他外设',
                  collapsed: false,
                  items: [
                    // { text: '4.3.1-电机控制脉宽调制器', link: 'docs/guide/04.外设学习/4.3-其他外设/4.3.1-MCPWM/MCPWM.md' },
                  ]
                },
              ]
            },
            {
              text: '五、FreeRTOS进阶',
              collapsed: false,
              items: [
                { text: '5.1-队列', link: '/docs/guide/05.FreeRTOS进阶/5.1-队列/FreeRTOS队列使用教程.md' },
                { text: '5.2-信号量', link: '/docs/guide/05.FreeRTOS进阶/5.2-信号量/FreeRTOS信号量教程.md' },
                { text: '5.3-互斥量', link: '/docs/guide/05.FreeRTOS进阶/5.3-互斥量/FreeRTOS互斥量教程.md' },
                { text: '5.4-事件组', link: '/docs/guide/05.FreeRTOS进阶/5.4-事件组/FreeRTOS事件组教程.md' },
                { text: '5.5-任务通知', link: '/docs/guide/05.FreeRTOS进阶/5.5-任务通知/FreeRTOS任务通知教程.md' },
              ]
            },
            {
              text: '六、Wi-Fi功能与相关协议',
              collapsed: false,
              items: [
                {
                  text: '6.1-WIFI基础',
                  collapsed: false,
                  items: [
                    { text: '6.1.1-Wi-Fi模式与介绍', link: 'docs/guide/06.Wi-Fi功能与相关协议/6.1-Wi-Fi功能入门/6.1.1-Wi-Fi模式与介绍/Wi-Fi模式与介绍.md' },
                    { text: '6.1.2-Wi-Fi的STA、AP配置  ', link: 'docs/guide/06.Wi-Fi功能与相关协议/6.1-Wi-Fi功能入门/6.1.2-Wi-Fi的STA与AP配置/Wi-Fi的STA与AP配置.md' },
                  ]
                },
                {
                  text: '6.2-通信协议',
                  collapsed: false,
                  items: [
                    { text: '6.2.1-TCP协议', link: 'docs/guide/06.Wi-Fi功能与相关协议/6.2-网络协议应用/6.2.1-TCP协议/TCP协议应用.md' },
                    { text: '6.2.2-UDP协议', link: 'docs/guide/06.Wi-Fi功能与相关协议/6.2-网络协议应用/6.2.2-UDP协议/UDP协议应用.md' },
                    { text: '6.2.3-HTTP服务端', link: 'docs/guide/06.Wi-Fi功能与相关协议/6.2-网络协议应用/6.2.3-HTTP协议/HTTP协议(服务端)/HTTP协议（服务端）.md' },
                    { text: '6.2.3-HTTP客户端', link: 'docs/guide/06.Wi-Fi功能与相关协议/6.2-网络协议应用/6.2.3-HTTP协议/HTTP协议(客户端)/HTTP协议（客户端）.md' },
                    { text: '6.2.4-WebSocket协议', link: 'docs/guide/06.Wi-Fi功能与相关协议/6.2-网络协议应用/6.2.4-WebSocket协议/WebSocket协议.md' },
                    { text: '6.2.5-MQTT协议', link: 'docs/guide/06.Wi-Fi功能与相关协议/6.2-网络协议应用/6.2.5-MQTT协议/MQTT协议应用.md' },
                    { text: '6.2.6-ESP-NOW协议', link: 'docs/guide/06.Wi-Fi功能与相关协议/6.2-网络协议应用/6.2.6-ESP-NOW协议/ESP-NOW协议应用.md' },
                    { text: '6.2.7-SNTP校时协议', link: 'docs/guide/06.Wi-Fi功能与相关协议/6.2-网络协议应用/6.2.7-SNTP校时/SNTP校时.md' },
                  ]
                },
              ]
            },
            {
              text: '七、蓝牙功能与相关协议',
              collapsed: false,
              items: [
              ]
            },
            {
              text: '八、图形界面开发',
              collapsed: false,
              items: [
              ]
            },
            {
              text: '九、其他实用内容',
              collapsed: false,
              items: [
                {
                  text: '9.1-ESP32配网',
                  collapsed: false,
                  items: [
                    { text: '9.1.1-Blufi配网', link: 'docs/guide/09.其他实用内容/9.1-ESP32配网/9.1.1-BluFi配网/BluFi配网.md' },
                    { text: '9.1.2-Smartconfig配网', link: 'docs/guide/09.其他实用内容/9.1-ESP32配网/9.1.2-Smartconfig配网/Smartconfig配网.md' },
                    // { text: '9.1.3-SoftAP配网', link: 'docs/guide/06.Wi-Fi功能与相关协议/9.1-Wi-Fi配网技术/9.1.3-SoftAP配网/SoftAP配网.md' },
                  ]
                },
                { text: '9.2-WEB服务程序(Vue3)', link: 'docs/guide/09.其他实用内容/9.2-WEB服务程序(Vue3+element-plus)/WEB服务程序.md' },
                { text: '9.3-ESP32-OTA初探.md', link: 'docs/guide/09.其他实用内容/9.3-ESP32实现OTA/ESP32-OTA初探.md' },
                {
                  text: '9.4-Modbus协议',
                  collapsed: false,
                  items: [
                    { text: '9.4.1-Modbus串口从站', link: 'docs/guide/09.其他实用内容/9.4-ESP32上ModBus的使用/ESP32上ModBus的使用-串口从站.md' },
                    // { text: '9.1.2-Smartconfig配网', link: 'docs/guide/09.其他实用内容/9.1-ESP32配网/9.1.2-Smartconfig配网/Smartconfig配网.md' },
                    // { text: '9.1.3-SoftAP配网', link: 'docs/guide/06.Wi-Fi功能与相关协议/9.1-Wi-Fi配网技术/9.1.3-SoftAP配网/SoftAP配网.md' },
                  ]
                },
              ]
            },
          ],
        },
        ],
      },

      // 社交链接配置
      socialLinks: [
        { icon: 'github', link: 'https://github.com/DuRuofu/ESP32-Guide' }, // GitHub 链接
        { icon: 'gitee', link: 'https://gitee.com/duruofu/ESP32-Guide' },
        { icon: 'bilibili', link: 'https://space.bilibili.com/505630520' },

      ],
      //更新时间
      lastUpdated: {
        text: '最后更新时间', 
        formatOptions: {
          dateStyle: 'full',
          timeStyle: 'medium'
        }
      },
      //手机端配置返回顶部按钮显示文字
      returnToTopLabel: "返回顶部",
      //手机端配置侧边栏菜单按钮显示文字
      sidebarMenuLabel: "菜单",
      //右侧内容导航栏
      outline: {
        level: [2, 6],
        label: "导航"
      },
      //翻页
      docFooter: {
        prev: "上一页",
        next: "下一页",
      },

    }
  }
)
