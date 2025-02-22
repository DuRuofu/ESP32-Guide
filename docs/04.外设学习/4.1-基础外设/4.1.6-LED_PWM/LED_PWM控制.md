
# ESP32外设-LEDPWM入门

> [!TIP] 🚀 **ESP32 外设-LEDPWM入门 | 精准控制 LED 灯光效果**  
> - 💡 **碎碎念**😎：本节将介绍 ESP32 的 LEDPWM 外设，帮助你实现对 LED 灯光的精准控制，打造炫酷灯光效果。  
> - 📺 **视频教程**：🚧 *开发中*  
> - 💾 **示例代码**：🚧 *开发中*
## 一、介绍：

LED 控制器 (LEDC) 主要用于控制 LED，也可产生 PWM 信号用于其他设备的控制。该控制器有 8 路通道，可以产生独立的波形，驱动 RGB LED 等设备。

LEDC 通道共有两组，分别为 8 路高速通道和 8 路低速通道。高速通道模式在硬件中实现，可以自动且无干扰地改变 PWM 占空比。低速通道模式下，PWM 占空比需要由软件中的驱动器改变。每组通道都可以使用不同的时钟源。

LED PWM 控制器可在无需 CPU 干预的情况下自动改变占空比，实现亮度和颜色渐变。

设置 LEDC 通道在 [高速模式或低速模式](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/peripherals/ledc.html#ledc-api-high-low-speed-mode) 下运行，需要进行如下配置：
1. [定时器配置](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/peripherals/ledc.html#ledc-api-configure-timer) 指定 PWM 信号的频率和占空比分辨率。
2. [通道配置](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/peripherals/ledc.html#ledc-api-configure-channel) 绑定定时器和输出 PWM 信号的 GPIO。
3. [改变 PWM 信号](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/peripherals/ledc.html#ledc-api-change-pwm-signal) 输出 PWM 信号来驱动 LED。可通过软件控制或使用硬件渐变功能来改变 LED 的亮度。

>首次 LEDC 配置时，建议先配置定时器（调用函数 [`ledc_timer_config()`](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/peripherals/ledc.html#_CPPv417ledc_timer_configPK19ledc_timer_config_t "ledc_timer_config")），再配置通道（调用函数 [`ledc_channel_config()`](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/peripherals/ledc.html#_CPPv419ledc_channel_configPK21ledc_channel_config_t "ledc_channel_config")）。这样可以确保 IO 脚上的 PWM 信号自有输出开始其频率就是正确的。


## 二、使用：

#### 1、定时器配置：

要设置定时器，可调用函数 [`ledc_timer_config()`](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/peripherals/ledc.html#_CPPv417ledc_timer_configPK19ledc_timer_config_t "ledc_timer_config")，并将包括如下配置参数的数据结构 [`ledc_timer_config_t`](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/peripherals/ledc.html#_CPPv419ledc_timer_config_t "ledc_timer_config_t") 传递给该函数：

- 速度模式 [`ledc_mode_t`](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/peripherals/ledc.html#_CPPv411ledc_mode_t "ledc_mode_t")
- 定时器索引 [`ledc_timer_t`](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/peripherals/ledc.html#_CPPv412ledc_timer_t "ledc_timer_t")
- PWM 信号频率（Hz）
- PWM 占空比分辨率
- 时钟源 [`ledc_clk_cfg_t`](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/peripherals/ledc.html#_CPPv414ledc_clk_cfg_t "ledc_clk_cfg_t")

频率和占空比分辨率相互关联。PWM 频率越高，占空比分辨率越低，反之亦然。
 PWM 占空比设置的分辨率范围较广,PWM 频率为 5 kHz 时，占空比分辨率最大可为 13 位。这意味着占空比可为 0 至 100% 之间的任意值，分辨率为 ~0.012%（2 ** 13 = 8192 LED 亮度的离散电平）。

时钟源同样可以限制 PWM 频率。选择的时钟源频率越高，可以配置的 PWM 频率上限就越高。

ESP32 LEDC 时钟源特性:

|时钟名称|时钟频率|速度模式|时钟功能|
|---|---|---|---|
|APB_CLK|80 MHz|高速 / 低速|/|
|REF_TICK|1 MHz|高速 / 低速|支持动态调频 (DFS) 功能|
|RC_FAST_CLK|~ 8 MHz|低速|支持动态调频（DFS）功能，支持 Light-sleep 模式|

> 如果 ESP32 的定时器选用了 `RC_FAST_CLK` 作为其时钟源，驱动会通过内部校准来得知这个时钟源的实际频率。这样确保了输出 PWM 信号频率的精准性。


配置示例：
``` c
ledc_timer_config_t ledc_timer = {
	.speed_mode       = LEDC_MODE,
	.duty_resolution  = LEDC_DUTY_RES,
	.timer_num        = LEDC_TIMER,
	.freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 4kHz
	.clk_cfg          = LEDC_AUTO_CLK
};
ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
```
#### 2、通道配置：

定时器设置好后，请配置所需的通道（[`ledc_channel_t`](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/peripherals/ledc.html#_CPPv414ledc_channel_t "ledc_channel_t") 之一）。配置通道需调用函数 [`ledc_channel_config()`](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/peripherals/ledc.html#_CPPv419ledc_channel_configPK21ledc_channel_config_t "ledc_channel_config")。

通道的配置与定时器设置类似，需向通道配置函数传递包括通道配置参数的结构体 [`ledc_channel_config_t`](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/peripherals/ledc.html#_CPPv421ledc_channel_config_t "ledc_channel_config_t") 。

[`ledc_channel_config_t`](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/peripherals/ledc.html#_CPPv421ledc_channel_config_t "ledc_channel_config_t") 的参数如下：

1. **`int gpio_num`**：指定LEDC输出的GPIO编号。例如，如果你想使用GPIO16，则设置 `gpio_num = 16`。
2. **`ledc_mode_t speed_mode`**：指定LEDC的速度模式，可以是高速模式（仅在ESP32上存在）或低速模式。
3. **`ledc_channel_t channel`**：指定LEDC的通道号，范围从0到`LEDC_CHANNEL_MAX-1`。
4. **`ledc_intr_type_t intr_type`**：配置中断类型，可以启用淡入淡出中断或禁用淡入淡出中断。
5. **`ledc_timer_t timer_sel`**：选择通道的定时器源，范围从0到`LEDC_TIMER_MAX-1`。
6. **`uint32_t duty`**：设置LEDC通道的占空比，其值的范围是[0, (2^duty_resolution)]。
7. **`int hpoint`**：设置LEDC通道的hpoint值，其范围是[0, (2^duty_resolution)-1]。
8. **`unsigned int output_invert`**：启用（1）或禁用（0）GPIO输出反转。
9. **`struct ledc_channel_config_t::[anonymous] flags`**：LEDC标志，用于进一步配置LEDC通道的特定行为。

此时，通道会按照 [`ledc_channel_config_t`](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/peripherals/ledc.html#_CPPv421ledc_channel_config_t "ledc_channel_config_t") 的配置开始运作，并在选定的 GPIO 上生成由定时器设置指定的频率和占空比的 PWM 信号。在通道运作过程中，可以随时通过调用函数 [`ledc_stop()`](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/peripherals/ledc.html#_CPPv49ledc_stop11ledc_mode_t14ledc_channel_t8uint32_t "ledc_stop") 将其暂停。

配置示例：

```c
    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
```

#### 3、改变 PWM 信号

通道开始运行、生成具有恒定占空比和频率的 PWM 信号之后，有几种方式可以改变该信号。驱动 LED 时，主要通过改变占空比来变化光线亮度。

#### 使用软件改变 PWM 占空比

调用函数 [`ledc_set_duty()`](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/peripherals/ledc.html#_CPPv413ledc_set_duty11ledc_mode_t14ledc_channel_t8uint32_t "ledc_set_duty") 可以设置新的占空比。之后，调用函数 [`ledc_update_duty()`](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/peripherals/ledc.html#_CPPv416ledc_update_duty11ledc_mode_t14ledc_channel_t "ledc_update_duty") 使新配置生效。（也可以使用[`ledc_channel_config()`](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/peripherals/ledc.html#_CPPv419ledc_channel_configPK21ledc_channel_config_t "ledc_channel_config")）

传递给函数的占空比数值范围取决于选定的 `duty_resolution`，应为 `0` 至 `(2 ** duty_resolution)`。例如，如选定的占空比分辨率为 10，则占空比的数值范围为 0 至 1024。此时分辨率为 ~ 0.1%。

>在 ESP32 上，当通道绑定的定时器配置了其最大 PWM 占空比分辨率（ `MAX_DUTY_RES` ），通道的占空比不能被设置到 `(2 ** MAX_DUTY_RES)` 。否则，硬件内部占空比计数器会溢出，并导致占空比计算错误。

#### 使用硬件改变 PWM 占空比

LED PWM 控制器硬件可逐渐改变占空比的数值。要使用此功能，需用函数 [`ledc_fade_func_install()`](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/peripherals/ledc.html#_CPPv422ledc_fade_func_installi "ledc_fade_func_install") 使能渐变，之后用下列可用渐变函数之一配置：

1. **`ledc_set_fade_with_time()`**：此函数用于设置指定通道的占空比在给定时间内从当前值渐变到目标值。你需要指定LEDC通道、目标占空比以及变化所需的时间（以毫秒为单位）。
2. **`ledc_set_fade_with_step()`**：此函数允许用户指定一个渐变步骤，包括每一步改变的占空比量、总共的步数以及每一步的时间间隔。这使得用户可以更精细地控制渐变过程。
3. **`ledc_set_fade()`**：这是一个更通用的渐变设置函数，它允许直接设置目标占空比、最大占空比、步长和步长间的时间间隔，但在不同版本的ESP-IDF中可能不总是可用。

最后调用 [`ledc_fade_start()`](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/peripherals/ledc.html#_CPPv415ledc_fade_start11ledc_mode_t14ledc_channel_t16ledc_fade_mode_t "ledc_fade_start") 开启渐变。

如不需要渐变和渐变中断，可用函数 [`ledc_fade_func_uninstall()`](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/peripherals/ledc.html#_CPPv424ledc_fade_func_uninstallv "ledc_fade_func_uninstall") 关闭。
#### 改变 PWM 频率

LED PWM 控制器 API 有多种方式即时改变 PWM 频率：

- 通过调用函数 [`ledc_set_freq()`](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/peripherals/ledc.html#_CPPv413ledc_set_freq11ledc_mode_t12ledc_timer_t8uint32_t "ledc_set_freq") 设置频率。可用函数 [`ledc_get_freq()`](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/peripherals/ledc.html#_CPPv413ledc_get_freq11ledc_mode_t12ledc_timer_t "ledc_get_freq") 查看当前频率。
- 通过调用函数 [`ledc_bind_channel_timer()`](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/peripherals/ledc.html#_CPPv423ledc_bind_channel_timer11ledc_mode_t14ledc_channel_t12ledc_timer_t "ledc_bind_channel_timer") 将其他定时器绑定到该通道来改变频率和占空比分辨率。
- 通过调用函数 [`ledc_channel_config()`](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/peripherals/ledc.html#_CPPv419ledc_channel_configPK21ledc_channel_config_t "ledc_channel_config") 改变通道的定时器。
## 三、案例：

``` c
#include <stdio.h>
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          (2) // Define the output GPIO
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY               (4096) // Set duty to 50%. (2 ** 13) * 50% = 4096
#define LEDC_FREQUENCY          (4000) // Frequency in Hertz. Set frequency at 4 kHz
#define FADE_TIME               (2000) // Fade time in milliseconds

static void example_ledc_init(void)
{
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .duty_resolution  = LEDC_DUTY_RES,
        .timer_num        = LEDC_TIMER,
        .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 4 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO,
        .duty           = 0, // Start with duty 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    // Install LEDC fade function
    ESP_ERROR_CHECK(ledc_fade_func_install(0));
}

void app_main(void)
{
    // Set the LEDC peripheral configuration
    example_ledc_init();

    while (1)
    {
        // Fade to 100% duty cycle within FADE_TIME milliseconds
        ESP_ERROR_CHECK(ledc_set_fade_with_time(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY, FADE_TIME));
        ESP_ERROR_CHECK(ledc_fade_start(LEDC_MODE, LEDC_CHANNEL, LEDC_FADE_WAIT_DONE));

        // Wait for the fade to complete
        vTaskDelay(FADE_TIME / portTICK_PERIOD_MS);

        // Fade back to 0% duty cycle within FADE_TIME milliseconds
        ESP_ERROR_CHECK(ledc_set_fade_with_time(LEDC_MODE, LEDC_CHANNEL, 0, FADE_TIME));
        ESP_ERROR_CHECK(ledc_fade_start(LEDC_MODE, LEDC_CHANNEL, LEDC_FADE_WAIT_DONE));

        // Wait for the fade to complete
        vTaskDelay(FADE_TIME / portTICK_PERIOD_MS);
    }
}


```

效果：

![1.gif](https://image-1308319148.cos.ap-chengdu.myqcloud.com/main/1.gif)


# 参考链接

1. https://blog.csdn.net/qq_36347513/article/details/116651516
2. https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/peripherals/ledc.html#ledc-api-supported-range-frequency-duty-resolution