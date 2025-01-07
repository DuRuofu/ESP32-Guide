
| 文档名称 | 版本 | 作者 | 时间 | 备注 |
| ---- | ---- | ---- | ---- | ---- |
| ESP-IDF项目配置 | v1.0.0 | DuRuofu | 2024-02-18 | 首次建立 |

# ESP-IDF项目配置(组件配置)


## 一、Menuconfig的使用

打开示例工程：esp-idf/examples/get-started/blink

这个历程是用来实现LED闪烁的程序，这个工程使用了一个叫`led_strip`的组件，所以代码稍显复杂，但这并不重要，我们的重点不再这里。

编译代码，烧录，发现并没有现象。（哈哈，这是由于我们并没有配置LED的引脚）

下面使用`idf.py  menuconfig `  命令打开 `Menuconfig` (配置菜单)：

![](attachments/20240218212849.png)

运行命令后，我们得到了一个位于终端的可视化配置菜单。

选择 `Example Configuration` 选项：

![](attachments/20240218212956.png)

会看到两个选项，修改Blink GPIO为自己开发板上的引脚，我这里是2号。

![](attachments/20240218213211.png)

重新编译工程，烧录代码。

![](attachments/20240218213648.png)

成功点亮LED(😊)，相信通过这一番操作，我们对menuconfig有了基本的认识，说人话就是这玩意可以方便的帮助我们修改程序里的参数，在不修改代码的情况下。
## 二、为什么要用组件配置（Menuconfig）

1. 组件配置可以将相关配置常量移出代码，增强配置的可复用度
2. 组件配置变量也可以被CMake访问，可以通过手写逻辑实现根据配置执行不同的构建方案。（如某个源文件要不要添加到构建系统）
3. 组件配置生成的宏定义，配合C语言的预处理语句，可以针对不同的配置改变代码解结构。
4. 组件配置生成的宏定义在C语言中是全局的宏定义，可以在任意处访问配置项。只需要#include "sdkconfig.h"即可。

## 三、组件配置相关文件

在 ESP-IDF 构建系统中，有这么几种文件和组件配置相关。它们分别是：

1. Kconfig文件
2. Kconfig.projbuild文件
3. sdkconfig文件
4. sdkconfig.default文件
5. sdkconfig.old文件
6. sdkconfig.ci文件
7. sdkconfig.old文件
   
>`sdkconfig.default`文件还可以针对某种设备单独添加，例如：  
   对于`ESP32`和`ESP32-S3`，则分别对应`sdkconfig.default.esp32`和`sdkconfig.default.esp32s3`。

在详细讲解组件配置之前，先分别概括这几个文件的作用分别是什么：

| 文件名 | 位置 | 作用 | 概述 |
| ---- | ---- | ---- | ---- |
| Kconfig | 组件目录 | 用于生成menuconfig中的菜单项 | 菜单模板 |
| Kconfig.projbuild | 组件目录 | 用于生成menuconfig中的顶层菜单 | 菜单模板 |
| sdkconfig | 项目根目录 | 用于保存 Kconfig 设置的键值对形式，手动更改无效 | 保存设置 |
| sdkconfig.default | 项目根目录 | 自定义 sdkconfig 的默认值，不会被构建系统更改，格式与 sdkconfig 相同 | 缺省值清单 |
| sdkconfig.old | 项目根目录 | 当保存旧的 menuconfig 设置时，自动备份原来的 sdkconfig 文件 | 备份 |
其中`Kconfig` 和 `Kconfig.projbuild` 文件用于定义菜单，其余文件用于保存我们配置的值。

查看main组件文件夹，我们可以发现这个工程里的 `Kconfig.projbuild` 文件：

![](attachments/20240218215404.png)


文件内容如下：

```

menu "Example Configuration"

    orsource "$IDF_PATH/examples/common_components/env_caps/$IDF_TARGET/Kconfig.env_caps"

    choice BLINK_LED
        prompt "Blink LED type"
        default BLINK_LED_GPIO if IDF_TARGET_ESP32 || !SOC_RMT_SUPPORTED
        default BLINK_LED_RMT
        help
            Defines the default peripheral for blink example

        config BLINK_LED_GPIO
            bool "GPIO"
        config BLINK_LED_RMT
            bool "RMT - Addressable LED"
    endchoice

    config BLINK_GPIO
        int "Blink GPIO number"
        range ENV_GPIO_RANGE_MIN ENV_GPIO_OUT_RANGE_MAX
        default 5 if IDF_TARGET_ESP32
        default 18 if IDF_TARGET_ESP32S2
        default 48 if IDF_TARGET_ESP32S3
        default 8
        help
            GPIO number (IOxx) to blink on and off or the RMT signal for the addressable LED.
            Some GPIOs are used for other purposes (flash connections, etc.) and cannot be used to blink.

    config BLINK_PERIOD
        int "Blink period in ms"
        range 10 3600000
        default 1000
        help
            Define the blinking period in milliseconds.

endmenu

```

不难发现，下面这部分就和我们刚才见到的配置项有关：

```
    config BLINK_GPIO
        int "Blink GPIO number"
        range ENV_GPIO_RANGE_MIN ENV_GPIO_OUT_RANGE_MAX
        default 5 if IDF_TARGET_ESP32
        default 18 if IDF_TARGET_ESP32S2
        default 48 if IDF_TARGET_ESP32S3
        default 8
        help
            GPIO number (IOxx) to blink on and off or the RMT signal for the addressable LED.
            Some GPIOs are used for other purposes (flash connections, etc.) and cannot be used to blink.

    config BLINK_PERIOD
        int "Blink period in ms"
        range 10 3600000
        default 1000
        help
            Define the blinking period in milliseconds.

```

即使不了解 Kconfig （ps： Kconfig 上面这些就是依赖Kconfig写的），我们也能猜出这几行代码干了啥： 定义了两个配置项，`BLINK_GPIO` 和 `BLINK_PERIOD` ,  然后添加默认值，添加说明等。

关于 Kconfig 的写法随便搜索一下就能学会，这里不在赘述。

## 四、创建组件配置


下面我们试着自己写一个组件配置，复制`blink`历程到自己的目录，重命名为 `blink_menuconfig`

![](attachments/20240218221414.png)

我们删除原有的 `Kconfig.projbuild` 文件，自己重新写一个，原来的`Kconfig.projbuild`里有三个配置项：分别是：

- `BLINK_LED`: 控制LED模式（选项）
- `BLINK_GPIO`：LED连接的引脚
- `BLINK_PERIOD`：LED闪烁的时间

### 创建框架（menu）

我们先创建个框架：

``` Kconfig
menu "点灯配置"

endmenu

```

这里菜单项可以使用中文，打开配置菜单可以看到：

![](attachments/20240218222722.png)

### ### 创建单选选项（choice）

```
menu "点灯配置"
    choice BLINK_LED
        prompt "LED模式"
        default BLINK_LED_GPIO

        config BLINK_LED_GPIO
            bool "GPIO"
        config BLINK_LED_RMT
            bool "RMT - Addressable LED"
    endchoice
endmenu

```

效果：
![](attachments/20240218223154.png)

![](attachments/20240218223205.png)
### 创建菜单项目（config）

```
    config BLINK_GPIO
        int "LED引脚"
        default 2
        help
            This is an int BLINK_GPIO.
	
    config BLINK_PERIOD
        int "LED周期"
        default 1000
        help
            This is a int BLINK_GPIO.
```

效果：

![](attachments/20240218224055.png)

# 参考链接

1. https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/kconfig.html