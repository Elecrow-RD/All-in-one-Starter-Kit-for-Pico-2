#include <Wire.h>
#include <LovyanGFX.hpp>
#include <SPI.h>

// 定义一个继承自lgfx::LGFX_Device的LGFX类
class LGFX : public lgfx::LGFX_Device {
    lgfx::Panel_ST7789 _panel_instance;  // ST7789面板实例
    lgfx::Bus_SPI _bus_instance;        // SPI总线实例
    lgfx::Touch_FT5x06 _touch_instance; // FT5x06触摸实例
public:
    LGFX(void) {
        // 配置SPI总线
        {
            auto cfg = _bus_instance.config();
            cfg.spi_host = 0;
            cfg.spi_mode = 0;
            cfg.freq_write = 80000000;  // 写操作频率80MHz
            cfg.freq_read = 16000000;   // 读操作频率16MHz
            cfg.pin_sclk = 6;          // 时钟引脚
            cfg.pin_mosi = 7;          // 主输出从输入引脚
            cfg.pin_miso = -1;         // 主输入从输出引脚，这里未使用设为-1
            cfg.pin_dc = 16;           // 数据/命令引脚
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }
        // 配置面板
        {
            auto cfg = _panel_instance.config();
            cfg.pin_cs = 17;           // 片选引脚
            cfg.pin_rst = -1;          // 复位引脚，这里未使用设为-1
            cfg.pin_busy = -1;         // 忙碌引脚，这里未使用设为-1
            cfg.memory_width = 240;    // 内存宽度
            cfg.memory_height = 320;   // 内存高度
            cfg.panel_width = 240;     // 面板宽度
            cfg.panel_height = 320;    // 面板高度
            cfg.offset_x = 0;          // X轴偏移
            cfg.offset_y = 0;          // Y轴偏移
            cfg.offset_rotation = 1;   // 旋转偏移
            cfg.dummy_read_pixel = 8;  // 虚拟读像素数
            cfg.dummy_read_bits = 1;   // 虚拟读位数
            cfg.readable = false;      // 不可读
            cfg.invert = true;         // 反转显示
            cfg.rgb_order = false;     // RGB顺序
            cfg.dlen_16bit = false;    // 16位数据长度
            cfg.bus_shared = false;    // 总线不共享
            _panel_instance.config(cfg);
        }
        // 配置触摸实例
        {
            auto cfg = _touch_instance.config();
            cfg.x_min = 0;
            cfg.x_max = 239;
            cfg.y_min = 0;
            cfg.y_max = 319;
            cfg.pin_int = 47;
            cfg.bus_shared = false;
            cfg.offset_rotation = 0;
            cfg.i2c_port = 0;
            cfg.i2c_addr = 0x38;
            cfg.pin_sda = 4;
            cfg.pin_scl = 5;
            cfg.freq = 400000;
            _touch_instance.config(cfg);
            _panel_instance.setTouch(&_touch_instance);
        }
        setPanel(&_panel_instance);
    }
};

LGFX gfx;

#define ADC_pin 28
#define Red_LED 18
int adcValue;
int mappedValue;

void setup() {
    // 设置ADC引脚为输入模式，用于读取模拟值
    pinMode(ADC_pin, INPUT);
    // 设置红色LED引脚为输出模式，用于控制LED亮度
    pinMode(Red_LED, OUTPUT);

    // 初始化I2C总线，设置SDA和SCL引脚
    Wire.setSDA(4);
    Wire.setSCL(5);
    Wire.begin();
    delay(100);

    // 初始化屏幕
    gfx.init();
    gfx.fillScreen(TFT_BLACK);  // 用黑色填充屏幕
    gfx.setTextColor(TFT_WHITE); // 设置文本颜色为白色
    gfx.setTextSize(2);          // 设置文本大小

    // 设置引脚0为输出模式，并输出高电平，具体作用需结合硬件，这里不太明确
    pinMode(0, OUTPUT);
    digitalWrite(0, HIGH);
}

void loop() {
    // 读取ADC引脚的模拟值
    adcValue = analogRead(ADC_pin);

    if (adcValue < 51) {
        // 如果ADC值小于51，关闭LED
        analogWrite(Red_LED, 0);
        gfx.setCursor(0, 0);
        gfx.print("brightness: ");
        gfx.print("0");
    } else if (adcValue > 1000) {
        // 如果ADC值大于1000，LED全亮
        analogWrite(Red_LED, 255);
        gfx.setCursor(0, 0);
        gfx.fillRect(120, 0, 100, 20, TFT_BLACK);
        gfx.setCursor(0, 0);
        gfx.print("brightness: 10");
    } else {
        // 将ADC值映射到0 - 255范围，用于控制LED亮度
        mappedValue = map(adcValue, 0, 1023, 0, 255);
        analogWrite(Red_LED, mappedValue);
        // 将映射后的亮度值再映射到0 - 10范围，用于显示
        int displayValue = map(mappedValue, 0, 255, 0, 10);
        gfx.setCursor(0, 0);
        gfx.fillRect(120, 0, 100, 20, TFT_BLACK);
        gfx.setCursor(0, 0);
        gfx.print("brightness: ");
        gfx.print(displayValue);
    }
    delay(10); // 短暂延时，减少资源占用，稳定程序运行
}