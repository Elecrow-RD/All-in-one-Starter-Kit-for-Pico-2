// 引入LVGL图形库，用于创建图形用户界面
#include <lvgl.h>
// 引入LVGL的演示程序库
#include "demos/lv_demos.h"
// 引入Wire库，用于I2C通信
#include <Wire.h>
// 引入LovyanGFX库，用于图形显示和触摸控制
#include <LovyanGFX.hpp>
// 引入自定义的UI头文件
#include "ui.h"
// 引入Ticker库，用于定时任务
#include <Ticker.h>
// 引入Servo库，用于控制舵机
#include <Servo.h>
// 引入Arduino核心库
#include <Arduino.h>
// 引入SPI库，用于SPI通信
#include <SPI.h>
// 引入BH1750库，用于光照强度传感器
#include <BH1750.h>
// 引入Adafruit_NeoPixel库，用于控制RGB LED灯条
#include <Adafruit_NeoPixel.h>
// 引入DHT20库，用于温湿度传感器
#include "DHT20.h"
// 引入HCSR04库，用于超声波测距传感器
#include <HCSR04.h>

// 引入TFT_eSPI库，用于TFT屏幕显示
#include <TFT_eSPI.h>
// 引入游戏结束图片数据头文件
#include "gameover.h"
// 引入无网络图片数据头文件
#include "noInternet.h"
// 引入图片数据头文件
#include "imgData.h"
// 引入配置文件头文件
#include "config.h"

// 根据Flash内存大小进行条件编译，8k及以下Flash（如ATtiny85），排除一些特殊协议
#if FLASHEND <= 0x1FFF
#define EXCLUDE_EXOTIC_PROTOCOLS
// 如果未定义DIGISTUMPCORE，即不是Digispark核心，排除通用协议以节省最多1000字节程序内存
#if !defined(DIGISTUMPCORE)
#define EXCLUDE_UNIVERSAL_PROTOCOLS
#endif
#endif

#include "PinDefinitionsAndMore.h"  // 引入自定义引脚定义等宏的头文件
#include <IRremote.hpp>

// 根据是否定义了APPLICATION_PIN来确定DEBUG_BUTTON_PIN，用于控制是否打印接收数据的时间信息
#if defined(APPLICATION_PIN)
#define DEBUG_BUTTON_PIN APPLICATION_PIN
#else
#define DEBUG_BUTTON_PIN 1
#endif



String num;     //用于保存等式的变量
String num1;    //用于保存等式因子1
String num2;    //用于保存等式因子2
String num3;    //用于保存转换后的等式结果
int flag1 = 0;  //符号‘+’标志
int flag2 = 0;  //符号‘-’标志
int flag3 = 0;  //符号‘*’标志
int flag4 = 0;  //符号‘/’标志

long value = 0;//用于保存等式结果
// 贪吃蛇结构体定义
// 包含贪吃蛇每个节点的x和y坐标数组，贪吃蛇的长度，以及当前移动方向
struct Snake
{
  int x[50];     // 贪吃蛇每个节点的x坐标数组，最多支持50个节点
  int y[50];     // 贪吃蛇每个节点的y坐标数组，最多支持50个节点
  int length;    // 贪吃蛇的当前长度
  int direction; // 贪吃蛇的移动方向
};

// 定义贪吃蛇对象
Snake snake;
// 定义食物的x和y坐标
int foodX, foodY;

// 屏幕分辨率
#define SCREEN_WIDTH 320  // 屏幕的宽度
#define SCREEN_HEIGHT 240 // 屏幕的高度

// 弹珠结构体
// 包含弹珠的x和y坐标，以及x和y方向的速度
struct Marble
{
  int x;  // 弹珠的x坐标
  int y;  // 弹珠的y坐标
  int vx; // 弹珠在x方向的速度
  int vy; // 弹珠在y方向的速度
};

// 定义弹珠对象
Marble marble;

// 挡板结构体
// 包含挡板的x和y坐标，以及挡板的宽度和高度
struct Paddle
{
  int x;      // 挡板的x坐标
  int y;      // 挡板的y坐标
  int width;  // 挡板的宽度
  int height; // 挡板的高度
};

// 定义挡板对象
Paddle paddle;

// 记录上一次执行操作的时间
unsigned long previousTime = 0;
// 设置延迟间隔时间，单位毫秒
int interval = 0;

// 自定义延迟函数
// wait为需要延迟的时间（毫秒）
void delay_new(int wait)
{
  interval = wait; // 将传入的延迟时间赋值给interval变量
  while (1)
  {                                       // 进入无限循环
    unsigned long currentTime = millis(); // 获取当前时间
    // 判断是否达到延迟时间
    if (currentTime - previousTime >= wait)
    {
      previousTime = currentTime; // 更新上一次执行操作的时间
      break;                      // 跳出循环
    }
    lv_timer_handler(); // 处理LVGL的定时器事件
  }
}

// RGB LED灯条相关定义
#define LED_PIN 22 // LED灯条的数据引脚
#define LED_EN 23  // LED灯条的使能引脚
// 定义连接到Arduino的NeoPixel灯珠数量
#define LED_COUNT 20
// 定义LED灯条的亮度
#define BRIGHTNESS 10
// 创建Adafruit_NeoPixel对象，用于控制LED灯条
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);

// 测试标志
uint8_t Count = 0;    // 计数器变量
char CloseData;       // 关闭数据变量
char CloseData1;      // 另一个关闭数据变量
int NO_Test_Flag = 0; // 无测试标志
int Test_Flag = 0;    // 测试标志
int Close_Flag = 0;   // 关闭标志
int touch_flag = 0;   // 触摸标志

// 自定义LGFX类，继承自lgfx::LGFX_Device
class LGFX : public lgfx::LGFX_Device
{
  // 定义ST7789面板实例
  lgfx::Panel_ST7789 _panel_instance;
  // 定义SPI总线实例
  lgfx::Bus_SPI _bus_instance;
  // 定义FT5x06触摸实例
  lgfx::Touch_FT5x06 _touch_instance;

public:
  // 构造函数
  LGFX(void)
  {
    {
      // 获取SPI总线的配置结构体
      auto cfg = _bus_instance.config();

      // SPI总线的配置
      cfg.spi_host = 0;          // 使用的SPI主机，ESP32-S2、C3可选择SPI2_HOST或SPI3_HOST，ESP32可选择VSPI_HOST或HSPI_HOST
      cfg.spi_mode = 0;          // SPI通信模式，范围0 ~ 3
      cfg.freq_write = 80000000; // 写入时的SPI时钟频率
      cfg.freq_read = 16000000;  // 读取时的SPI时钟频率
      cfg.pin_sclk = 6;          // SPI的SCLK引脚号
      cfg.pin_mosi = 7;          // SPI的MOSI引脚号
      cfg.pin_miso = -1;         // SPI的MISO引脚号，-1表示禁用
      cfg.pin_dc = 16;           // SPI的D/C引脚号

      _bus_instance.config(cfg);              // 将配置应用到SPI总线
      _panel_instance.setBus(&_bus_instance); // 将SPI总线设置到面板实例
    }

    {
      // 获取面板的配置结构体
      auto cfg = _panel_instance.config();

      cfg.pin_cs = 17;   // CS引脚号，-1表示禁用
      cfg.pin_rst = -1;  // RST引脚号，-1表示禁用
      cfg.pin_busy = -1; // BUSY引脚号，-1表示禁用

      cfg.memory_width = 240;   // 驱动IC支持的最大宽度
      cfg.memory_height = 320;  // 驱动IC支持的最大高度
      cfg.panel_width = 240;    // 实际可显示的宽度
      cfg.panel_height = 320;   // 实际可显示的高度
      cfg.offset_x = 0;         // 面板的X方向偏移量
      cfg.offset_y = 0;         // 面板的Y方向偏移量
      cfg.offset_rotation = 1;  // 旋转方向的偏移，范围0~7（4~7是倒置的）
      cfg.dummy_read_pixel = 8; // 读取像素前的虚拟读取位数
      cfg.dummy_read_bits = 1;  // 读取像素以外数据前的虚拟读取位数
      cfg.readable = false;     // 是否可以读取数据
      cfg.invert = true;        // 面板的明暗是否反转
      cfg.rgb_order = false;    // 面板的红色和蓝色是否交换
      cfg.dlen_16bit = false;   // 数据长度是否以16位单位发送
      cfg.bus_shared = false;   // 总线是否与SD卡共享

      _panel_instance.config(cfg); // 将配置应用到面板实例
    }

    {
      // 获取触摸屏幕的配置结构体
      auto cfg = _touch_instance.config();

      cfg.x_min = 0;           // 触摸屏幕能获取的最小X值
      cfg.x_max = 239;         // 触摸屏幕能获取的最大X值
      cfg.y_min = 0;           // 触摸屏幕能获取的最小Y值
      cfg.y_max = 319;         // 触摸屏幕能获取的最大Y值
      cfg.pin_int = 47;        // INT引脚号
      cfg.bus_shared = false;  // 总线是否与屏幕共享
      cfg.offset_rotation = 0; // 显示和触摸方向不匹配时的调整，范围0到7

      // I2C连接配置
      cfg.i2c_port = 0;    // 使用的I2C端口
      cfg.i2c_addr = 0x38; // I2C设备地址
      cfg.pin_sda = 4;     // SDA引脚号
      cfg.pin_scl = 5;     // SCL引脚号
      cfg.freq = 400000;   // I2C时钟频率

      _touch_instance.config(cfg);                // 将配置应用到触摸实例
      _panel_instance.setTouch(&_touch_instance); // 将触摸实例设置到面板实例
    }

    setPanel(&_panel_instance); // 将面板实例设置到LGFX设备
  }
};

// 创建LGFX对象
LGFX gfx;

// 根据实际情况修改屏幕分辨率
// 定义屏幕宽度为320像素
static const uint16_t screenWidth = 320;
// 定义屏幕高度为240像素
static const uint16_t screenHeight = 240;

// 创建LVGL显示绘制缓冲区对象
static lv_disp_draw_buf_t draw_buf;
// 定义一个颜色缓冲区，大小为屏幕面积的十分之一
static lv_color_t buf[screenWidth * screenHeight / 10];

// 显示刷新函数，用于将LVGL的绘制数据推送到屏幕上
// disp: LVGL显示驱动对象指针
// area: 需要刷新的屏幕区域
// color_p: 颜色数据指针
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  // 检查gfx的开始计数，如果大于0，结束写入操作
  if (gfx.getStartCount() > 0)
  {
    gfx.endWrite();
  }
  // 使用DMA方式将图像数据推送到屏幕指定区域
  gfx.pushImageDMA(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1, (lgfx::rgb565_t *)&color_p->full);
  // 告诉LVGL刷新操作已完成
  lv_disp_flush_ready(disp);
}

// 定义触摸点的x和y坐标变量
uint16_t touchX, touchY;

// 触摸板读取函数，用于获取触摸板的触摸数据
// indev_driver: LVGL输入设备驱动对象指针
// data: LVGL输入设备数据对象指针
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  // 默认设置触摸状态为释放
  data->state = LV_INDEV_STATE_REL;
  // 检查是否有触摸事件发生
  if (gfx.getTouch(&touchX, &touchY))
  {
    // 如果有触摸事件，设置触摸状态为按下
    data->state = LV_INDEV_STATE_PR;
    // 设置触摸点的x坐标
    data->point.x = touchX;
    // 设置触摸点的y坐标
    data->point.y = touchY;
    // 打印触摸点的x坐标
    Serial.print("Data x ");
    Serial.println(data->point.x);
    // 打印触摸点的y坐标
    Serial.print("Data y ");
    Serial.println(data->point.y);
  }
}

// 引入自定义的UI头文件
#include "ui.h"

// 定义一个标志变量，用于判断是否是第一次执行
static int first_flag = 0;
// 声明外部变量，用于清除零值
extern int zero_clean;
// 声明外部变量，用于跳转界面标志
extern int goto_widget_flag;
// 声明外部变量，用于进度条标志
extern int bar_flag;
// 声明外部变量，指向菜单界面的LVGL对象
extern lv_obj_t *ui_MENU;
// 声明外部变量，指向触摸界面的LVGL对象
extern lv_obj_t *ui_TOUCH;
// 声明外部变量，指向校准界面的LVGL对象
extern lv_obj_t *ui_JIAOZHUN;
// 声明外部变量，指向某个标签的LVGL对象
extern lv_obj_t *ui_Label2;
// 定义指向触摸界面标签的LVGL对象指针
static lv_obj_t *ui_Label;
// 定义指向触摸界面标签3的LVGL对象指针
static lv_obj_t *ui_Label3;
// 定义指向菜单界面进度条标签的LVGL对象指针
static lv_obj_t *ui_Labe2;
// 定义指向菜单界面进度条的LVGL对象指针
static lv_obj_t *bar;
// 定义进度条的初始值为100
static int val = 100;

/*RGB相关函数*/

// 颜色渐变函数，用于将LED灯条逐个像素设置为指定颜色
// color: 要设置的颜色
// wait: 每个像素设置后的延迟时间
void colorWipe(uint32_t color, int wait)
{
  // 如果Style_1标志为0，直接返回
  if (Style_1 == 0)
    return;
  // 遍历LED灯条的每个像素
  for (int i = 0; i < strip.numPixels(); i++)
  {
    // 设置当前像素的颜色
    strip.setPixelColor(i, color);
    // 更新LED灯条显示
    strip.show();
    // 调用自定义延迟函数进行延迟
    delay_new(wait);
    // 处理LVGL的定时器事件
    lv_timer_handler();
  }
}

// 另一个颜色渐变函数，与colorWipe类似，但使用Arduino的delay函数进行延迟
// color: 要设置的颜色
// wait: 每个像素设置后的延迟时间
void colorWipe1(uint32_t color, int wait)
{
  // 遍历LED灯条的每个像素
  for (int i = 0; i < strip.numPixels(); i++)
  {
    // 设置当前像素的颜色
    strip.setPixelColor(i, color);
    // 更新LED灯条显示
    strip.show();
    // 使用Arduino的delay函数进行延迟
    delay(wait);
  }
}

// 根据输入的0 - 255的值生成一个颜色值，颜色过渡为红 - 绿 - 蓝 - 红
// WheelPos: 颜色值输入，范围0 - 255
uint32_t Wheel(byte WheelPos)
{
  // 反转输入值
  WheelPos = 255 - WheelPos;
  // 根据输入值的范围生成不同的颜色
  if (WheelPos < 85)
  {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170)
  {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

// 彩虹循环函数，使LED灯条呈现彩虹效果，且颜色均匀分布
// wait: 每次更新的延迟时间
void rainbowCycle(uint8_t wait)
{
  uint16_t i, j;
  // 如果Style_4标志为0，直接返回
  if (Style_4 == 0)
    return;
  // 正向循环39次
  for (j = 0; j < 39; j++)
  {
    // 遍历LED灯条的每个像素
    for (i = 0; i < strip.numPixels(); i++)
    {
      // 设置当前像素的颜色
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
      // 处理LVGL的定时器事件
      lv_timer_handler();
    }
    // 更新LED灯条显示
    strip.show();
    // 设置LED灯条的亮度
    strip.setBrightness(j % 40);
    // 处理LVGL的定时器事件
    lv_timer_handler();
    // 调用自定义延迟函数进行延迟
    delay_new(wait);
    // 如果Style_4标志为0，直接返回
    if (Style_4 == 0)
      return;
  }
  // 反向循环39次
  for (j = 39; j > 0; j--)
  {
    // 遍历LED灯条的每个像素
    for (i = 0; i < strip.numPixels(); i++)
    {
      // 设置当前像素的颜色
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
      // 处理LVGL的定时器事件
      lv_timer_handler();
    }
    // 更新LED灯条显示
    strip.show();
    // 设置LED灯条的亮度
    strip.setBrightness(j % 40);
    // 处理LVGL的定时器事件
    lv_timer_handler();
    // 调用自定义延迟函数进行延迟
    delay_new(wait);
    // 如果Style_4标志为0，直接返回
    if (Style_4 == 0)
      return;
  }
}

// 白色覆盖彩虹效果函数，在彩虹效果上添加白色移动条
// whiteSpeed: 白色移动条的移动速度
// whiteLength: 白色移动条的长度
void whiteOverRainbow(int whiteSpeed, int whiteLength)
{
  // 如果Style_2标志为0，直接返回
  if (Style_2 == 0)
    return;
  // 如果白色移动条长度超过LED灯条像素数量，调整为最大长度减1
  if (whiteLength >= strip.numPixels())
    whiteLength = strip.numPixels() - 1;
  // 白色移动条的头部位置
  int head = whiteLength - 1;
  // 白色移动条的尾部位置
  int tail = 0;
  // 循环次数
  int loops = 3;
  // 当前循环次数
  int loopNum = 0;
  // 记录上一次移动的时间
  uint32_t lastTime = millis();
  // 第一个像素的色调
  uint32_t firstPixelHue = 0;

  // 无限循环
  for (;;)
  {
    // 遍历LED灯条的每个像素
    for (int i = 0; i < strip.numPixels(); i++)
    {
      // 判断当前像素是否在白色移动条范围内
      if (((i >= tail) && (i <= head)) || ((tail > head) && ((i >= tail) || (i <= head))))
      {
        // 如果在范围内，设置为白色
        strip.setPixelColor(i, strip.Color(0, 0, 0, 255));
      }
      else
      {
        // 如果不在范围内，设置为彩虹颜色
        int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
        strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
      }
      // 如果Style_2标志为0，直接返回
      if (Style_2 == 0)
        return;
    }
    // 更新LED灯条显示
    strip.show();
    // 处理LVGL的定时器事件
    lv_timer_handler();
    // 增加第一个像素的色调
    firstPixelHue += 40;
    // 判断是否到了更新头部和尾部位置的时间
    if ((millis() - lastTime) > whiteSpeed)
    {
      // 头部位置前进
      if (++head >= strip.numPixels())
      {
        head = 0;
        // 循环次数加1
        if (++loopNum >= loops)
          return;
      }
      // 尾部位置前进
      if (++tail >= strip.numPixels())
      {
        tail = 0;
      }
      // 记录当前时间
      lastTime = millis();
    }
  }
}

// 白色脉冲效果函数，使LED灯条的白色亮度从0到255再从255到0渐变
// wait: 每次亮度变化的延迟时间
void pulseWhite(uint8_t wait)
{
  // 亮度从0到255渐变
  for (int j = 0; j < 256; j++)
  {
    // 填充LED灯条为指定亮度的白色
    strip.fill(strip.Color(0, 0, 0, strip.gamma8(j)));
    // 更新LED灯条显示
    strip.show();
    // 使用Arduino的delay函数进行延迟
    delay(wait);
    // 处理LVGL的定时器事件
    lv_timer_handler();
    // 如果Style_3标志为0，直接返回
    if (Style_3 == 0)
      return;
  }
  // 亮度从255到0渐变
  for (int j = 255; j >= 0; j--)
  {
    // 填充LED灯条为指定亮度的白色
    strip.fill(strip.Color(0, 0, 0, strip.gamma8(j)));
    // 更新LED灯条显示
    strip.show();
    // 使用Arduino的delay函数进行延迟
    delay(wait);
    // 处理LVGL的定时器事件
    lv_timer_handler();
    // 如果Style_3标志为0，直接返回
    if (Style_3 == 0)
      return;
  }
}

// 剧院追逐彩虹效果函数，使LED灯条呈现类似剧院灯光追逐的彩虹效果
// wait: 每次更新的延迟时间
void theaterChaseRainbow(int wait)
{
  // 第一个像素的初始色调为红色
  int firstPixelHue = 0;
  // 循环30次
  for (int a = 0; a < 30; a++)
  {
    // 循环3次
    for (int b = 0; b < 3; b++)
    {
      // 清空LED灯条的颜色
      strip.clear();
      // 每隔3个像素设置一次颜色
      for (int c = b; c < strip.numPixels(); c += 3)
      {
        // 计算当前像素的色调
        int hue = firstPixelHue + c * 65536L / strip.numPixels();
        // 将色调转换为RGB颜色
        uint32_t color = strip.gamma32(strip.ColorHSV(hue));
        // 设置当前像素的颜色
        strip.setPixelColor(c, color);
      }
      // 更新LED灯条显示
      strip.show();
      // 使用Arduino的delay函数进行延迟
      delay(wait);
      // 处理LVGL的定时器事件
      lv_timer_handler();
      // 如果Style_4标志为0，直接返回
      if (Style_4 == 0)
        return;
      // 增加第一个像素的色调
      firstPixelHue += 65536 / 90;
    }
  }
}

// 剧院追逐效果函数，使LED灯条呈现类似剧院灯光追逐的单一颜色效果
// color: 要设置的颜色
// wait: 每次更新的延迟时间
void theaterChase(uint32_t color, int wait)
{
  // 循环10次
  for (int a = 0; a < 10; a++)
  {
    // 循环3次
    for (int b = 0; b < 3; b++)
    {
      // 清空LED灯条的颜色
      strip.clear();
      // 每隔3个像素设置一次颜色
      for (int c = b; c < strip.numPixels(); c += 3)
      {
        // 设置当前像素的颜色
        strip.setPixelColor(c, color);
      }
      // 更新LED灯条显示
      strip.show();
      // 使用Arduino的delay函数进行延迟
      delay(wait);
      // 处理LVGL的定时器事件
      lv_timer_handler();
      // 如果Style_4标志为0，直接返回
      if (Style_4 == 0)
        return;
    }
  }
} // 彩虹渐变为白色的效果函数
// wait: 每次更新的延迟时间
// rainbowLoops: 彩虹循环的次数
void rainbowFade2White(int wait, int rainbowLoops)
{
  // 如果Style_3标志为0，直接返回，不执行效果
  if (Style_3 == 0)
    return;

  // 渐变值，初始为0
  int fadeVal = 0;
  // 渐变的最大值
  int fadeMax = 100;

  // 第一个像素的色调值，从0开始，每次增加256，直到达到rainbowLoops * 65536
  // 这里通过循环让第一个像素的色调完成rainbowLoops次完整的颜色轮循环
  for (uint32_t firstPixelHue = 0; firstPixelHue < rainbowLoops * 65536;
       firstPixelHue += 256)
  {

    // 遍历LED灯条的每个像素
    for (int i = 0; i < strip.numPixels(); i++)
    {
      // 计算当前像素的色调值，通过第一个像素的色调值加上偏移量，使颜色在灯条上形成渐变
      uint32_t pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());

      // 设置当前像素的颜色，使用strip.ColorHSV函数根据色调、饱和度和亮度生成颜色
      // 饱和度固定为255，亮度根据渐变值计算
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue, 255,
                                                          255 * fadeVal / fadeMax)));
    }

    // 更新LED灯条显示
    strip.show();
    // 调用自定义延迟函数进行延迟
    delay_new(wait);
    // 处理LVGL的定时器事件
    lv_timer_handler();

    // 在第一个循环中，逐渐增加渐变值，实现淡入效果
    if (firstPixelHue < 65536)
    {
      if (fadeVal < fadeMax)
        fadeVal++;
    }
    // 在最后一个循环中，逐渐减小渐变值，实现淡出效果
    else if (firstPixelHue >= ((rainbowLoops - 1) * 65536))
    {
      if (fadeVal > 0)
        fadeVal--;
    }
    // 在中间的循环中，确保渐变值为最大值
    else
    {
      fadeVal = fadeMax;
    }
  }

  // 暂停200毫秒
  delay_new(200);
}

// 游戏1相关设置

// 创建TFT_eSPI对象，用于TFT屏幕操作
TFT_eSPI tft = TFT_eSPI();
// 创建多个TFT_eSprite对象，用于绘制精灵图像
TFT_eSprite img = TFT_eSprite(&tft);
TFT_eSprite img2 = TFT_eSprite(&tft);
TFT_eSprite img3 = TFT_eSprite(&tft);
TFT_eSprite img4 = TFT_eSprite(&tft);

TFT_eSprite e = TFT_eSprite(&tft);
TFT_eSprite e2 = TFT_eSprite(&tft);

// 用于存储接收到的输入字符串
String inputString = "";
// 标记字符串是否接收完成
bool stringComplete = false;

// 恐龙图像的宽度
int dinoW = 33;
// 恐龙图像的高度
int dinoH = 35;
// 存储线条的x坐标
float linesX[6];
// 存储线条的宽度
int linesW[6];
// 存储另一组线条的x坐标
float linesX2[6];
// 存储另一组线条的宽度
int linesW2[6];
// 存储云朵的x坐标，初始值为随机生成
float clouds[2] = {(float)random(0, 80), (float)random(100, 180)};
// 存储凸起的位置
float bumps[2];
// 存储凸起的标志
int bumpsF[2];
// 精灵的宽度
int eW = 18;
// 精灵的高度
int eH = 38;
// 精灵的x坐标，初始值为随机生成
float eX[2] = {(float)random(240, 310), (float)random(380, 460)};
// 精灵的标志
int ef[2] = {0, 1};
// 游戏滚动速度，使用预定义的GAME_SPEED
float roll_speed = GAME_SPEED;
// 云朵的移动速度
float cloudSpeed = 0.4;
// 恐龙的x坐标
int x = 30;
// 恐龙的y坐标
int y = 58;
// 恐龙的移动方向
float dir = -1.4;
// 帧数
int frames = 0;
// 另一个帧数计数器
int f = 0;
// 游戏是否运行的标志
bool gameRun = 0;
// 游戏得分
int score = 0;
// 得分变量
int score_v = 0;
// 上一次的得分
int last_score = 0;
// 游戏开始时间
unsigned long start_t = 0;
// 时间变量
int t = 0;
// 跳跃按钮是否按下的标志
bool button_jump = 0;
// 开始按钮是否按下的标志
bool button_start = 0;

// 记录按键文本
unsigned char key_text = 0;
// 记录起始触摸点的x坐标
int startX;
// 记录起始触摸点的y坐标
int startY;
// 标记是否正在触摸屏幕
bool isTouching = false;
// 触摸滑动的阈值
const int threshold = 30;
// 点击判定的距离阈值
const int clickThreshold = 10;
// 界面切换标志
bool Interface_flag = false;
// 停止标志
bool stop_flag = false;

// 游戏2相关设置

// 初始化弹珠与挡板的函数
void init_game()
{
  // 设置弹珠的初始x坐标为屏幕宽度的一半
  marble.x = SCREEN_WIDTH / 2;
  // 设置弹珠的初始y坐标为屏幕高度的一半
  marble.y = SCREEN_HEIGHT / 2;
  // 设置弹珠在x方向的初始速度为2
  marble.vx = 2;
  // 设置弹珠在y方向的初始速度为 -2
  marble.vy = -2;

  // 设置挡板的初始x坐标，使其位于屏幕中央偏左30个像素
  paddle.x = SCREEN_WIDTH / 2 - 30;
  // 设置挡板的初始y坐标，使其位于屏幕底部上方20个像素
  paddle.y = SCREEN_HEIGHT - 20;
  // 设置挡板的宽度为60个像素
  paddle.width = 60;
  // 设置挡板的高度为10个像素
  paddle.height = 10;
}

// 绘制弹珠的函数
void draw_marble()
{
  // 在当前屏幕上创建一个圆形对象
  lv_obj_t *circle = lv_obj_create(lv_scr_act());
  // 设置圆形对象的大小为10x10像素
  lv_obj_set_size(circle, 10, 10);
  // 设置圆形对象的位置为弹珠的当前坐标
  lv_obj_set_pos(circle, marble.x, marble.y);
  // 设置圆形对象的背景颜色为红色
  lv_obj_set_style_bg_color(circle, lv_color_hex(0xFF0000), 0);
}

// 绘制挡板的函数
void draw_paddle()
{
  // 在当前屏幕上创建一个矩形对象
  lv_obj_t *rect = lv_obj_create(lv_scr_act());
  // 设置矩形对象的大小为挡板的宽度和高度
  lv_obj_set_size(rect, paddle.width, paddle.height);
  // 设置矩形对象的位置为挡板的当前坐标
  lv_obj_set_pos(rect, paddle.x, paddle.y);
  // 设置矩形对象的背景颜色为绿色
  lv_obj_set_style_bg_color(rect, lv_color_hex(0x00FF00), 0);
}

// 弹珠移动与碰撞检测的函数
void update_marble()
{
  // 根据弹珠的速度更新其x坐标
  marble.x += marble.vx;
  // 根据弹珠的速度更新其y坐标
  marble.y += marble.vy;

  // 弹珠撞墙检测，如果弹珠撞到屏幕左右边界，反转其x方向的速度
  if (marble.x <= 0 || marble.x >= SCREEN_WIDTH)
  {
    marble.vx = -marble.vx;
  }
  // 如果弹珠撞到屏幕顶部边界，反转其y方向的速度
  if (marble.y <= 0)
  {
    marble.vy = -marble.vy;
  }

  // 弹珠与挡板碰撞检测，如果弹珠与挡板发生碰撞，反转其y方向的速度
  if (marble.y >= paddle.y && marble.y <= paddle.y + paddle.height && marble.x >= paddle.x && marble.x <= paddle.x + paddle.width)
  {
    marble.vy = -marble.vy;
  }

  // 检测弹珠是否飞出屏幕下方，如果飞出，则重新初始化游戏
  if (marble.y > SCREEN_HEIGHT)
  {
    init_game();
  }
}
// game3函数

// 初始化贪吃蛇的函数
// 该函数用于设置贪吃蛇的初始状态，包括长度、方向和初始位置
void snake_initialize()
{
  // 初始化贪吃蛇的长度为3
  snake.length = 3;
  // 初始化贪吃蛇的移动方向为向右（0表示向右）
  snake.direction = 0;
  // 初始化贪吃蛇头部的x坐标为屏幕宽度的一半
  snake.x[0] = SCREEN_WIDTH / 2;
  // 初始化贪吃蛇头部的y坐标为屏幕高度的一半
  snake.y[0] = SCREEN_HEIGHT / 2;
  // 初始化贪吃蛇身体其他部分的位置，依次排列在头部左侧
  for (int i = 1; i < snake.length; i++)
  {
    snake.x[i] = snake.x[0] - i * 20;
    snake.y[i] = snake.y[0];
  }
}

// 初始化食物位置的函数
// 该函数用于随机生成食物的位置，并且保证食物位置在屏幕内且是20的倍数
void food_initialize()
{
  // 随机生成食物的x坐标，范围是屏幕宽度减去20
  foodX = random(SCREEN_WIDTH - 20);
  // 调整食物的x坐标，使其是20的倍数
  foodX = foodX - foodX % 20;
  // 随机生成食物的y坐标，范围是屏幕高度减去20
  foodY = random(SCREEN_HEIGHT - 20);
  // 调整食物的y坐标，使其是20的倍数
  foodY = foodY - foodY % 20;
}

// 绘制贪吃蛇的函数
// 该函数用于在屏幕上绘制贪吃蛇的每个部分
void draw_snake()
{
  // 遍历贪吃蛇的每个部分
  for (int i = 0; i < snake.length; i++)
  {
    // 在当前屏幕上创建一个矩形对象，代表贪吃蛇的一个部分
    lv_obj_t *rect = lv_obj_create(lv_scr_act());
    // 设置矩形对象的大小为20x20像素
    lv_obj_set_size(rect, 20, 20);
    // 设置矩形对象的位置为贪吃蛇当前部分的坐标
    lv_obj_set_pos(rect, snake.x[i], snake.y[i]);
    // 设置矩形对象的背景颜色为绿色
    lv_obj_set_style_bg_color(rect, lv_color_hex(0x00FF00), 0);
  }
}

// 绘制食物的函数
// 该函数用于在屏幕上绘制食物
void draw_food()
{
  // 在当前屏幕上创建一个矩形对象，代表食物
  lv_obj_t *foodRect = lv_obj_create(lv_scr_act());
  // 设置矩形对象的大小为20x20像素
  lv_obj_set_size(foodRect, 20, 20);
  // 设置矩形对象的位置为食物的坐标
  lv_obj_set_pos(foodRect, foodX, foodY);
  // 设置矩形对象的背景颜色为红色
  lv_obj_set_style_bg_color(foodRect, lv_color_hex(0xFF0000), 0);
}

// 蛇移动逻辑的函数
// 该函数用于更新贪吃蛇的位置，根据其当前方向移动
void snake_move()
{
  // 先将贪吃蛇身体各部分的位置依次向前移动
  for (int i = snake.length - 1; i > 0; i--)
  {
    snake.x[i] = snake.x[i - 1];
    snake.y[i] = snake.y[i - 1];
  }
  // 根据贪吃蛇的当前方向更新头部的位置
  switch (snake.direction)
  {
  case 0: // 向右移动
    snake.x[0] += 20;
    break;
  case 1: // 向下移动
    snake.y[0] += 20;
    break;
  case 2: // 向左移动
    snake.x[0] -= 20;
    break;
  case 3: // 向上移动
    snake.y[0] -= 20;
    break;
  }
}

// 碰撞检测的函数
// 该函数用于检测贪吃蛇是否发生碰撞，包括撞墙和撞自己
bool collision_detection()
{
  // 撞墙检测，如果贪吃蛇头部超出屏幕边界，则认为发生碰撞
  if (snake.x[0] < 0 || snake.x[0] >= SCREEN_WIDTH || snake.y[0] < 0 || snake.y[0] >= SCREEN_HEIGHT)
  {
    return true;
  }
  // 撞自己检测，如果贪吃蛇头部与身体其他部分重合，则认为发生碰撞
  for (int i = 1; i < snake.length; i++)
  {
    if (snake.x[0] == snake.x[i] && snake.y[0] == snake.y[i])
    {
      return true;
    }
  }
  return false;
}

// 吃食物逻辑的函数
// 该函数用于检测贪吃蛇是否吃到食物，如果吃到则增加长度并重新生成食物
void eat_food()
{
  // 判断贪吃蛇头部是否与食物位置重合
  if (snake.x[0] == foodX && snake.y[0] == foodY)
  {
    // 贪吃蛇长度加1
    snake.length++;
    // 重新生成食物的位置
    food_initialize();
  }
}

// 结束红外相关操作，开始图形显示初始化操作
void IR_end_gfx_init() {
  IrReceiver.end();           // 结束红外接收
  SPI.setTX(7);               // 设置SPI的TX引脚
  SPI.setSCK(6);              // 设置SPI的SCK引脚
  SPI.begin();                // 开始SPI通信
  gfx.begin();                // 开始图形显示
  gfx.fillScreen(TFT_BLACK);  // 用黑色填充屏幕
}

// 结束图形显示相关操作，开始红外初始化操作
void gfx_end_IR_init() {
#if FLASHEND >= 0x3FFF
  pinMode(DEBUG_BUTTON_PIN, INPUT_PULLUP);
#endif
  IrReceiver.begin(11, ENABLE_LED_FEEDBACK);  // 初始化红外接收器
  IrReceiver.printIRSendUsage(&Serial);       // 打印红外发送用法信息到串口
}

// 显示输入的数字s
void print_num() {
  IR_end_gfx_init();  // 先结束红外，初始化图形显示
  gfx.setCursor(0, 20);
  gfx.print("                ");
  gfx.setCursor(0, 20);
  gfx.print(num);     // 在屏幕显示输入的数字
  gfx_end_IR_init();  // 再结束图形显示，初始化红外
}

// 判断字符串是否包含指定字符的函数
bool containsCharacter(String str, char character) {
  return str.indexOf(character) != -1;  // 如果返回值不是 -1，说明包含该字符
}
// 切片函数
int sliceString(String input, String &part1, String &part2) {

  int plusIndex;
  int equalIndex;
  flag1 = containsCharacter(input, '+');
  flag2 = containsCharacter(input, '-');
  flag3 = containsCharacter(input, '*');
  flag4 = containsCharacter(input, '/');
  if ((flag1 || flag2 || flag3 || flag4) && containsCharacter(input, '=')) {
    if (flag1 == 1) {
      plusIndex = input.indexOf('+');   // 查找‘+’的位置
      equalIndex = input.indexOf('=');  // 查找‘=’的位置
    } else if (flag2 == 1) {
      plusIndex = input.indexOf('-');   // 查找‘+’的位置
      equalIndex = input.indexOf('=');  // 查找‘=’的位置
    } else if (flag3 == 1) {
      plusIndex = input.indexOf('*');   // 查找‘*’的位置
      equalIndex = input.indexOf('=');  // 查找‘=’的位置
    } else if (flag4 == 1) {
      plusIndex = input.indexOf('/');   // 查找‘/’的位置
      equalIndex = input.indexOf('=');  // 查找‘=’的位置
    }

  } else {
    return 0;
  }
  if (plusIndex != -1) {
    part1 = input.substring(0, plusIndex);  // 获取算数符号前的部分
  } else {
    part1 = "";  // 如果没有算数符号，返回空字符串
  }

  if (plusIndex != -1 && equalIndex != -1 && equalIndex > plusIndex) {
    part2 = input.substring(plusIndex + 1, equalIndex);  // 获取算数符号和‘=’之间的部分
  } else {
    part2 = "";  // 如果没有有效的部分，返回空字符串
  }
  return 1;
}

// 执行计算
long performOperation(long a, long b, char operation) {
  switch (operation) {
    case '+':
      return a + b;
    case '-':
      return a - b;
    case '*':
      return a * b;
    case '/':
      return a / b;
    default:
      return a;  // 如果操作符不合法，返回原值
  }
}
// L19 课程的初始化函数，用于初始化硬件、图形显示和相关引脚设置
void L19_init() {
    // 设置 I2C 总线的数据引脚（SDA）为 4
    Wire.setSDA(4);
    // 设置 I2C 总线的时钟引脚（SCL）为 5
    Wire.setSCL(5);
    // 启动 I2C 总线通信
    Wire.begin();
    // 延时 100 毫秒，让 I2C 设备有时间完成初始化
    delay(100);

    // 初始化图形显示设备
    gfx.init();
    // 使用黑色填充整个屏幕，清空之前的显示内容
    gfx.fillScreen(TFT_BLACK);
    // 设置后续要显示的文本大小为 2
    gfx.setTextSize(2);
    // 设置文本输出的起始光标位置为屏幕左上角 (0, 0)
    gfx.setCursor(0, 0);
    // 在屏幕上打印提示信息 "Please enter:"
    gfx.print("Please enter:");

    // 注释掉的代码，原本用于初始化舵机相关参数，将舵机连接到引脚 13，并设置舵机的最小和最大脉冲宽度
    // myservo.attach(13, 650, 2620);

    // 注释掉的代码，原本用于设置舵机的初始角度为 180 度
    // myservo.write(180);

    // 将引脚 14 设置为输入模式，用于检测退出课程的信号
    pinMode(14, INPUT);
}

// L19 课程的主逻辑函数，实现通过红外遥控器输入数据并进行运算和显示的功能
void L19_Curriculum() {
    // 调用 L19 课程的初始化函数，完成硬件和图形显示的初始化
    L19_init();

    // 如果 Flash 内存大小达到或超过 16k（地址范围 0x3FFF 及以上）
    #if FLASHEND >= 0x3FFF
        // 将 DEBUG_BUTTON_PIN 引脚设置为输入上拉模式
        pinMode(DEBUG_BUTTON_PIN, INPUT_PULLUP);
    #endif

    // 初始化红外接收器，将其连接到引脚 11，并启用 LED 反馈功能
    IrReceiver.begin(11, ENABLE_LED_FEEDBACK);
    // 向串口打印红外发送的使用说明信息
    IrReceiver.printIRSendUsage(&Serial);

    // 进入一个无限循环，持续监听红外信号和退出信号
    while (1) {
        // 检测引脚 14 的输入信号，如果为高电平，表示要退出课程
        if (digitalRead(14)) {
            // 调用 LittlevGL 的定时器处理函数，更新界面定时任务
            lv_timer_handler();
            // 使当前屏幕无效，强制刷新界面显示
            lv_obj_invalidate(lv_scr_act());
            // 设置 ui_Label11 标签的文本为 "OFF"，提示课程已结束
            lv_label_set_text(ui_Label11, "OFF");
            // 将课程标志位设置为 0，表示退出课程模式
            Curriculum_flag = 0;
            // 执行红外相关结束和图形显示初始化操作
            IR_end_gfx_init();
            // 跳出当前的无限循环，结束课程逻辑
            break;
        }

        // 检查红外接收器是否成功解码到红外数据
        if (IrReceiver.decode()) {
            // 在串口输出一个空行，用于分隔不同的解码信息
            Serial.println();

            // 检查解码数据的标志位，判断是否发生溢出
            if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_WAS_OVERFLOW) {
                // 如果发生溢出，在串口打印溢出检测信息
                Serial.println(F("Overflow detected"));
                // 提示用户尝试增加 RAW_BUFFER_LENGTH 的值
                Serial.println(F("Try to increase the \"RAW_BUFFER_LENGTH\" value of " STR(RAW_BUFFER_LENGTH) " in " __FILE__));
            } else {
                // 根据接收到的红外指令的命令码，执行不同的操作
                switch (IrReceiver.decodedIRData.command) {
                    case 0x45:
                        // 当接收到的命令码为 0x45 时，在串口打印按下的按键信息
                        Serial.println("- press -\t[CH-]");
                                                // 将字符 '*' 添加到 num 字符串中，记录用户输入的运算符
                        num = num + '*';
                        // 调用 print_num 函数，将当前记录的输入内容显示在屏幕上
                        print_num();
                        break;
                    case 0x46:
                        // 当接收到的命令码为 0x46 时，在串口打印按下的按键信息
                        Serial.println("- press -\t[CH]");
                        break;
                    case 0x47:
                        // 当接收到的命令码为 0x47 时，在串口打印按下的按键信息
                        Serial.println("- press -\t[CH+]");
                        // 将字符 '/' 添加到 num 字符串中，记录用户输入的运算符
                        num = num + '/';
                        // 调用 print_num 函数，将当前记录的输入内容显示在屏幕上
                        print_num();
                        break;
                    case 0x44:
                        // 当接收到的命令码为 0x44 时，在串口打印按下的按键信息
                        Serial.println("- press -\t[PREV]");
                        break;
                    case 0x40:
                        // 当接收到的命令码为 0x40 时，在串口打印按下的按键信息
                        Serial.println("- press -\t[NEXT]");
                        break;
                    case 0x43:
                        // 当接收到的命令码为 0x43 时，在串口打印按下的按键信息
                        Serial.println("- press -\t[PLAY/PAUSE]");
                        break;
                    case 0x07:
                        // 当接收到的命令码为 0x07 时，在串口打印按下的按键信息
                        Serial.println("- press -\t[VOL-]");
                        // 将字符 '-' 添加到 num 字符串中，记录用户输入的运算符
                        num = num + '-';
                        // 调用 print_num 函数，将当前记录的输入内容显示在屏幕上
                        print_num();
                        break;
                    case 0x15:
                        // 当接收到的命令码为 0x15 时，在串口打印按下的按键信息
                        Serial.println("- press -\t[VOL+]");
                        // 将字符 '+' 添加到 num 字符串中，记录用户输入的运算符
                        num = num + '+';
                        // 调用 print_num 函数，将当前记录的输入内容显示在屏幕上
                        print_num();
                        break;
                    case 0x09:
                        // 当接收到的命令码为 0x09 时，在串口打印按下的按键信息
                        Serial.println("- press -\t[EQ]");
                        // 执行红外相关结束、图形显示初始化操作
                        IR_end_gfx_init();
                        // 将字符 '=' 添加到 num 字符串中，标记输入结束
                        num += '=';
                        // 设置 num3 字符串为 "Equal to:"，用于显示结果提示
                        num3 = "Equal to:";
                        // 调用 sliceString 函数，尝试将 num 字符串分割为两个操作数 num1 和 num2
                        if (sliceString(num, num1, num2)) {
                            // 如果分割成功，在串口打印合法数据信息
                            Serial.println("合法数据");
                            // 检查两个操作数是否超出范围（大于 100000000 或小于 0）
                            if (num1.toInt() > 100000000 || num2.toInt() > 100000000 || num1.toInt() < 0 || num2.toInt() < 0) {
                                // 如果超出范围，在串口打印不合法数据信息
                                Serial.println("不合法数据");
                                // 清空 num 字符串
                                num = "";
                                // 设置 num3 字符串为 "Out of range"，提示超出范围
                                num3 = "Out of range";
                                // 清空屏幕上之前的显示内容
                                gfx.setCursor(0, 0);
                                gfx.print("             ");
                                // 在屏幕上打印超出范围提示信息
                                gfx.setCursor(0, 0);
                                gfx.print(num3);
                                break;
                            }
                            // 根据不同的运算符标志位，调用 performOperation 函数进行运算
                            if (flag1) {
                                value = performOperation(num1.toInt(), num2.toInt(), '+');
                                                                // 若为加法运算，执行加法操作并将结果存储在 value 中
                                // 同时在串口打印 "value" 信息，可能用于调试
                                value = performOperation(num1.toInt(), num2.toInt(), '+');
                                Serial.println("value");
                            } else if (flag2) {
                                // 若为减法运算，执行减法操作并将结果存储在 value 中
                                // 同时在串口打印 "-数据" 信息，可能用于调试
                                value = performOperation(num1.toInt(), num2.toInt(), '-');
                                Serial.println("-数据");
                            } else if (flag3) {
                                // 若为乘法运算，执行乘法操作并将结果存储在 value 中
                                // 同时在串口打印 "-数据" 信息，可能用于调试
                                value = performOperation(num1.toInt(), num2.toInt(), '*');
                                Serial.println("-数据");
                            } else if (flag4) {
                                // 若为除法运算，执行除法操作并将结果存储在 value 中
                                // 同时在串口打印 "-数据" 信息，可能用于调试
                                value = performOperation(num1.toInt(), num2.toInt(), '/');
                                Serial.println("-数据");
                            }

                            // 清空 num 字符串，为下一次输入做准备
                            num = "";
                            // 将运算结果转换为字符串存储在 num3 中
                            num3 = String(value);
                            // 清空屏幕上之前的显示内容
                            gfx.setCursor(0, 0);
                            gfx.print("             ");
                            // 在屏幕上打印 "Results:" 提示信息
                            gfx.setCursor(0, 0);
                            gfx.print("Results:");
                            // 在屏幕上打印运算结果
                            gfx.print(num3);
                        } else {
                            // 如果分割字符串失败，清空 num 字符串
                            num = "";
                            // 设置 num3 字符串为 "Error"，提示输入错误
                            num3 = "Error";
                            // 清空屏幕上之前的显示内容
                            gfx.setCursor(0, 0);
                            gfx.print("             ");
                            // 在屏幕上打印错误提示信息
                            gfx.setCursor(0, 0);
                            gfx.print(num3);
                        }
                        // 在串口打印运算结果信息
                        Serial.println("value:");
                        Serial.println(value);
                        Serial.println();
                        Serial.println(num);
                        // 执行图形显示相关结束、红外初始化操作
                        gfx_end_IR_init();
                        break;
                    case 0x16:
                        // 当接收到的命令码为 0x16 时，在串口打印按下的按键信息
                        Serial.println("- press -\t[0]");
                        // 将字符 '0' 添加到 num 字符串中，记录用户输入的数字
                        num = num + '0';
                        // 调用 print_num 函数，将当前记录的输入内容显示在屏幕上
                        print_num();
                        break;
                    case 0x19:
                        // 当接收到的命令码为 0x19 时，在串口打印按下的按键信息
                        Serial.println("- press -\t[100+]");
                        break;
                    case 0x0D:
                        // 当接收到的命令码为 0x0D 时，在串口打印按下的按键信息
                        Serial.println("- press -\t[200+]");
                        break;
                    case 0x0C:
                        // 当接收到的命令码为 0x0C 时，在串口打印按下的按键信息
                        Serial.println("- press -\t[1]");
                        // 将字符 '1' 添加到 num 字符串中，记录用户输入的数字
                        num = num + '1';
                        // 调用 print_num 函数，将当前记录的输入内容显示在屏幕上
                        print_num();
                        break;
                    case 0x18:
                        // 当接收到的命令码为 0x18 时，在串口打印按下的按键信息
                        Serial.println("- press -\t[2]");
                        // 将字符 '2' 添加到 num 字符串中，记录用户输入的数字
                        num = num + '2';
                        // 调用 print_num 函数，将当前记录的输入内容显示在屏幕上
                        print_num();
                        break;
                    case 0x5E:
                                            case 0x5E:
                        // 当接收到的命令码为 0x5E 时，在串口打印按下的按键信息
                        Serial.println("- press -\t[3]");
                        // 将字符 '3' 添加到 num 字符串中，记录用户输入的数字
                        num = num + '3';
                        // 调用 print_num 函数，将当前记录的输入内容显示在屏幕上
                        print_num();
                        break;
                    case 0x08:
                        // 当接收到的命令码为 0x08 时，在串口打印按下的按键信息
                        Serial.println("- press -\t[4]");
                        // 将字符 '4' 添加到 num 字符串中，记录用户输入的数字
                        num = num + '4';
                        // 调用 print_num 函数，将当前记录的输入内容显示在屏幕上
                        print_num();
                        break;
                    case 0x1C:
                        // 当接收到的命令码为 0x1C 时，在串口打印按下的按键信息
                        Serial.println("- press -\t[5]");
                        // 将字符 '5' 添加到 num 字符串中，记录用户输入的数字
                        num = num + '5';
                        // 调用 print_num 函数，将当前记录的输入内容显示在屏幕上
                        print_num();
                        break;
                    case 0x5A:
                        // 当接收到的命令码为 0x5A 时，在串口打印按下的按键信息
                        Serial.println("- press -\t[6]");
                        // 将字符 '6' 添加到 num 字符串中，记录用户输入的数字
                        num = num + '6';
                        // 调用 print_num 函数，将当前记录的输入内容显示在屏幕上
                        print_num();
                        break;
                    case 0x42:
                        // 当接收到的命令码为 0x42 时，在串口打印按下的按键信息
                        Serial.println("- press -\t[7]");
                        // 将字符 '7' 添加到 num 字符串中，记录用户输入的数字
                        num = num + '7';
                        // 调用 print_num 函数，将当前记录的输入内容显示在屏幕上
                        print_num();
                        break;
                    case 0x52:
                        // 当接收到的命令码为 0x52 时，在串口打印按下的按键信息
                        Serial.println("- press -\t[8]");
                        // 将字符 '8' 添加到 num 字符串中，记录用户输入的数字
                        num = num + '8';
                        // 调用 print_num 函数，将当前记录的输入内容显示在屏幕上
                        print_num();
                        break;
                    case 0x4A:
                        // 当接收到的命令码为 0x4A 时，在串口打印按下的按键信息
                        Serial.println("- press -\t[9]");
                        // 将字符 '9' 添加到 num 字符串中，记录用户输入的数字
                        num = num + '9';
                        // 调用 print_num 函数，将当前记录的输入内容显示在屏幕上
                        print_num();
                        break;
                }

                // 如果接收到的红外协议为未知协议，或者 DEBUG_BUTTON_PIN 引脚为低电平
                if (IrReceiver.decodedIRData.protocol == UNKNOWN || digitalRead(DEBUG_BUTTON_PIN) == LOW) {
                    // 可选择打印更多原始的红外解码信息，此处代码被注释掉
                    // IrReceiver.printIRResultRawFormatted(&Serial, true);
                }
            }

            // 允许红外接收器继续接收下一个红外数据
            IrReceiver.resume();

            // 根据接收到的红外数据的地址和命令码执行其他动作
            if (IrReceiver.decodedIRData.address == 0) {
                if (IrReceiver.decodedIRData.command == 0x10) {
                    // 当地址为 0 且命令码为 0x10 时，可执行相应操作，此处代码未具体实现
                    // do something
                } else if (IrReceiver.decodedIRData.command == 0x11) {
                    // 当地址为 0 且命令码为 0x11 时，可执行相应操作，此处代码未具体实现
                    // do something else
                }
            }
        }

                // 延时 10 毫秒，避免循环执行过快，过度占用 CPU 资源
        delay(10);
    }
}

// 程序初始化函数，在程序启动时仅执行一次，用于对硬件和软件环境进行初始化设置
void setup(void) {
    // 初始化串口通信，设置波特率为 115200，用于与外部设备进行数据传输和调试信息输出
    Serial.begin(115200);

    // 将 LED_EN 引脚设置为输出模式，该引脚可能用于控制某个 LED 灯的使能
    pinMode(LED_EN, OUTPUT);

    // 初始化 NeoPixel 灯带对象，为后续控制灯带做准备
    strip.begin();
    // 立即关闭 NeoPixel 灯带的所有像素，确保初始状态下灯带不发光
    strip.show();
    // 设置 NeoPixel 灯带的亮度，亮度值由 BRIGHTNESS 变量决定
    strip.setBrightness(BRIGHTNESS);

    // 设置 I2C 通信的 SDA（数据）引脚为 4
    Wire.setSDA(4);
    // 设置 I2C 通信的 SCL（时钟）引脚为 5
    Wire.setSCL(5);
    // 初始化 I2C 通信，使其可以正常工作
    Wire.begin();
    // 延时 100 毫秒，给 I2C 设备足够的时间完成初始化
    delay(100);

    // 初始化图形显示设备，为后续的图形绘制和显示操作做准备
    gfx.init();
    // 初始化 LittlevGL 图形库，用于创建和管理图形用户界面
    lv_init();

    // 初始化 LittlevGL 的显示缓冲区，用于存储要显示的图形数据
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * screenHeight / 10);

    // 定义并初始化一个静态的显示驱动程序结构体
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    // 设置显示的水平分辨率，该值由 screenWidth 变量决定
    disp_drv.hor_res = screenWidth;
    // 设置显示的垂直分辨率，该值由 screenHeight 变量决定
    disp_drv.ver_res = screenHeight;
    // 设置显示刷新回调函数，当需要更新显示内容时会调用该函数
    disp_drv.flush_cb = my_disp_flush;
    // 将显示缓冲区与显示驱动程序关联起来
    disp_drv.draw_buf = &draw_buf;
    // 注册显示驱动程序，使 LittlevGL 可以使用该驱动进行显示操作
    lv_disp_drv_register(&disp_drv);

    // 定义并初始化一个静态的输入设备驱动程序结构体
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    // 设置输入设备的类型为指针类型，通常用于触摸屏等设备
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    // 设置输入设备的读取回调函数，用于获取输入设备的状态信息
    indev_drv.read_cb = my_touchpad_read;
    // 注册输入设备驱动程序，使 LittlevGL 可以接收输入设备的输入
    lv_indev_drv_register(&indev_drv);

    // 延时 100 毫秒，给输入设备和显示设备足够的时间完成初始化
    delay(100);

    // 将引脚 0 设置为输出模式
    pinMode(0, OUTPUT);
    // 将引脚 0 置为高电平，可能用于控制某个外部设备的开启
    digitalWrite(0, HIGH);

    // 调用 ui_init 函数，初始化开机时显示的用户界面
    ui_init();
    // 调用 LittlevGL 的定时器处理函数，处理界面的定时更新和动画等操作
    lv_timer_handler();

    // 模拟进度条从 0 到 100 的加载过程
    for (int i = 0; i <= 100; i++) {
        // 设置进度条 ui_Bar1 的值为 i，并开启动画效果
        lv_bar_set_value(ui_Bar1, i, LV_ANIM_ON);
        // 定义一个字符数组，用于存储格式化后的进度百分比字符串
        char text[10];
        // 将进度值 i 格式化为百分比字符串，如 "50%"
        sprintf(text, "%d%%", i);
        // 设置标签 ui_Label1 的文本为格式化后的进度百分比字符串
        lv_label_set_text(ui_Label1, text);
                // 调用 LittlevGL 的定时器处理函数，更新界面显示
        lv_timer_handler();
        // 延时 50 毫秒，控制进度条加载速度，使进度条缓慢增长
        delay(50);
    }
    // 切换到 ui_Screen2 界面，使用淡入动画效果，动画时长为 500 毫秒
    _ui_screen_change(&ui_Screen2, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, &ui_Screen2_screen_init);
    // 再次调用 LittlevGL 的定时器处理函数，确保界面更新完成
    lv_timer_handler();
}

// 主循环函数，在 setup 函数执行完成后会不断循环执行，用于处理各种周期性任务和事件
void loop() {
    // 调用 LittlevGL 的定时器处理函数，处理界面的定时更新和动画等操作，保证界面的流畅显示
    lv_timer_handler();
    // 调用 Scan_button 函数，扫描按键状态，处理按键事件，以便响应用户的按键操作
    Scan_button();
    // 延时 10 毫秒，控制循环的执行频率，避免程序运行过快占用过多资源
    delay(10);

    // 判断课程标志位 Curriculum_flag 是否为 1，如果为 1 则表示进入 L19 课程模式
    if (Curriculum_flag == 1) {
        // 调用 L19_Curriculum 函数，执行 L19 课程的具体逻辑，即通过红外遥控器输入数据进行运算和显示
        L19_Curriculum();
    }

    // 判断游戏 1 标志位 game_1 是否为 1，如果为 1 则进入游戏 1 模式
    if (game_1 == 1) {
        // 使用黑色填充屏幕，清空屏幕上之前的显示内容
        gfx.fillScreen(TFT_BLACK);
        // 将引脚 0 置为低电平，关闭背光
        digitalWrite(0, LOW);
        // 调用 game_1_test 函数，执行游戏 1 的具体逻辑
        game_1_test();
        // 将引脚 0 置为高电平，开启背光
        digitalWrite(0, HIGH);
        // 调用 LittlevGL 的定时器处理函数，更新界面显示
        lv_timer_handler();
        // 使当前屏幕无效，强制刷新显示，避免出现黑屏问题
        lv_obj_invalidate(lv_scr_act());
        // 清除 ui_Switch6 的选中状态，可能用于重置游戏 1 的相关开关状态
        lv_obj_clear_state(ui_Switch6, LV_STATE_CHECKED);
    }

    // 判断游戏 2 标志位 game_2 是否为 1，如果为 1 则进入游戏 2 模式
    if (game_2 == 1) {
        // 使用黑色填充屏幕，清空屏幕上之前的显示内容
        gfx.fillScreen(TFT_BLACK);
        // 将引脚 0 置为低电平，关闭背光
        digitalWrite(0, LOW);
        // 调用 game_2_test 函数，执行游戏 2 的具体逻辑
        game_2_test();
        // 将引脚 0 置为低电平，关闭背光
        digitalWrite(0, LOW);
        // 调用 ui_init 函数，重新初始化用户界面
        ui_init();
        // 切换到 ui_Screen2 界面，使用淡入动画效果，动画时长为 500 毫秒
        _ui_screen_change(&ui_Screen2, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, &ui_Screen2_screen_init);
        // 调用 LittlevGL 的定时器处理函数，更新界面显示
        lv_timer_handler();
        // 设置 ui_TabView2 的活动标签页为第 2 页，并开启动画效果
        lv_tabview_set_act(ui_TabView2, 2, LV_ANIM_ON);
        // 再次调用 LittlevGL 的定时器处理函数，更新界面显示
        lv_timer_handler();
        // 使当前屏幕无效，强制刷新显示，避免出现黑屏问题
        lv_obj_invalidate(lv_scr_act());
        // 注释掉的代码，原本可能用于清除 ui_Switch7 的选中状态
        // lv_obj_clear_state(ui_Switch7, LV_STATE_CHECKED);
        // 将引脚 0 置为高电平，开启背光
        digitalWrite(0, HIGH);
    }

    // 判断游戏 3 标志位 game_3 是否为 1，如果为 1 则进入游戏 3 模式
    if (game_3 == 1) {
        // 使用黑色填充屏幕，清空屏幕上之前的显示内容
        gfx.fillScreen(TFT_BLACK);
                // 将引脚 0 置为低电平，关闭背光
        digitalWrite(0, LOW);
        // 调用 game_3_test 函数，执行游戏 3 的具体逻辑
        game_3_test();
        // 将引脚 0 置为低电平，关闭背光
        digitalWrite(0, LOW);
        // 调用 ui_init 函数，重新初始化用户界面
        ui_init();
        // 切换到 ui_Screen2 界面，使用淡入动画效果，动画时长为 500 毫秒
        _ui_screen_change(&ui_Screen2, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, &ui_Screen2_screen_init);
        // 调用 LittlevGL 的定时器处理函数，更新界面显示
        lv_timer_handler();
        // 设置 ui_TabView2 的活动标签页为第 2 页，并开启动画效果
        lv_tabview_set_act(ui_TabView2, 2, LV_ANIM_ON);
        // 再次调用 LittlevGL 的定时器处理函数，更新界面显示
        lv_timer_handler();
        // 使当前屏幕无效，强制刷新显示，避免出现黑屏问题
        lv_obj_invalidate(lv_scr_act());

        // 将引脚 0 置为高电平，开启背光
        digitalWrite(0, HIGH);
    }
}
// 状态标志，可能用于控制某些状态的切换
bool State_flag = true;
// 菜单打开标志，用于判断菜单是否打开
bool Open_menu = false;
// 游戏2的标志，可能用于控制游戏2的某些逻辑
bool game2_flag = true;
// 游戏3的标志，用于控制游戏3的运行状态
bool game3_flag = true;

// 游戏3的测试函数，包含游戏3的主要逻辑
void game_3_test()
{
  // 初始化贪吃蛇的状态
  snake_initialize();
  // 初始化食物的位置
  food_initialize();
  // 记录上一次按键检查的时间
  unsigned long lastKeyCheckTime = 0;
  // 记录上一次游戏更新的时间
  unsigned long lastKeyCheckTime1 = 0;
  // 按键检查的时间间隔，单位为毫秒
  const unsigned long keyCheckInterval = 50;
  // 游戏更新的时间间隔，单位为毫秒
  const unsigned long keyCheckInterval1 = 400;
  // 设置游戏3的标志为true，表示游戏开始
  game3_flag = true;
  // 将引脚0设置为高电平
  digitalWrite(0, HIGH);

  // 进入游戏3的主循环
  while (1)
  {
    // 获取当前时间
    unsigned long currentTime = millis();

    // 如果距离上一次按键检查的时间超过了设定的时间间隔
    if (currentTime - lastKeyCheckTime >= keyCheckInterval)
    {
      // 读取引脚27的模拟值
      int adcValue = analogRead(27);
      // 根据模拟值判断用户的输入，改变贪吃蛇的移动方向
      if (adcValue >= 740 && adcValue <= 750)
      {
        snake.direction = 3; // 向上移动
      }
      else if (adcValue >= 860 && adcValue <= 870)
      {
        snake.direction = 1; // 向下移动
      }
      else if (adcValue >= 800 && adcValue <= 810)
      {
        snake.direction = 2; // 向左移动
      }
      else if (adcValue >= 905 && adcValue <= 915)
      {
        snake.direction = 0; // 向右移动
      }
      // 更新上一次按键检查的时间
      lastKeyCheckTime = currentTime;
    }

    // 用于存储触摸点的x和y坐标
    uint16_t touchX1, touchY1;

    // 手势判断
    if (gfx.getTouch(&touchX1, &touchY1))
    {
      // 如果之前没有触摸，记录起始触摸点的坐标
      if (!isTouching)
      {
        startX = touchX1;
        startY = touchY1;
        isTouching = true;
      }
      else
      {
        // 计算触摸点的y方向和x方向的偏移量
        int yDiff = touchY1 - startY;
        int xDiff = touchX1 - startX;
        // 计算移动距离
        int moveDistance = sqrt(xDiff * xDiff + yDiff * yDiff);

        // 如果移动距离小于点击判定的阈值，认为是点击操作
        if (moveDistance < clickThreshold)
        {
          Serial.println("Click ");
          isTouching = false;
          // 如果点击位置在特定区域且菜单是打开状态
          if (touchY1 >= 180 && touchX1 >= 205 && touchX1 <= 285 && Open_menu == true)
          {
            State_flag = false;
            // 显示游戏2的菜单
            game_2_Menu();
            // 处理LVGL的定时器事件
            lv_timer_handler();
            Serial.println("State_flag ");
            // 延迟100毫秒
            delay(100);
            while (1)
            {
              // 再次检测触摸事件
              if (gfx.getTouch(&touchX1, &touchY1))
              {
                // 延迟10毫秒
                delay(10);
                // 如果点击位置在特定区域
                if (touchY1 >= 180 && touchX1 >= 205 && touchX1 <= 285)
                {
                  State_flag = true;
                  Open_menu = false;
                  break;
                }
              }
              // 如果点击位置在另一个特定区域
              else if (touchY1 >= 180 && touchX1 < 115 && touchX1 > 45)
              {
                Serial.println("return ");
                State_flag = true;
                Open_menu = false;
                game3_flag = false;
                break;
              }
            }
          }
          // 如果点击位置在另一个特定区域且菜单是打开状态
          else if (touchY1 >= 180 && touchX1 < 115 && touchX1 > 45 && Open_menu == true)
          {
            Serial.println("return ");
            State_flag = true;
            Open_menu = false;
            game3_flag = false;
          }
        }
        // 如果y方向的偏移量小于阈值，认为是向上滑动操作
        else if (yDiff < -threshold)
        {
          Serial.println("sliding");
          isTouching = false;
          Open_menu = true;
        }
      }
    }
    else
    {
      isTouching = false;
    }

    // 如果游戏3的标志为false，表示游戏结束，退出循环
    if (game3_flag == false)
    {
      game_3 = 0;
      break;
    }

    // 如果距离上一次游戏更新的时间超过了设定的时间间隔
    if (currentTime - lastKeyCheckTime1 >= keyCheckInterval1)
    {
      // 清空当前屏幕上的所有对象
      lv_obj_clean(lv_scr_act());
      // 创建一个图像对象，并设置其源为指定的图片
      lv_obj_t *img_bg = lv_img_create(lv_scr_act());
      lv_img_set_src(img_bg, &ui_img_crowpi_320x2400_game_02_01_png);
      // 将图像对象居中显示
      lv_obj_center(img_bg);

      // 绘制贪吃蛇
      draw_snake();
      // 绘制食物
      draw_food();

      // 移动贪吃蛇
      snake_move();
      // 检查贪吃蛇是否吃到食物
      eat_food();
      // 检测贪吃蛇是否发生碰撞
      if (collision_detection())
      {
        // 如果发生碰撞，重新初始化贪吃蛇和食物的状态
        snake_initialize();
        food_initialize();
      }

      // 如果菜单是打开状态，显示游戏2的菜单
      if (Open_menu == true)
      {
        game_2_Menu();
      }

      // 处理LVGL的定时器事件
      lv_timer_handler();
      // 更新上一次游戏更新的时间
      lastKeyCheckTime1 = currentTime;
    }

    // 如果菜单是打开状态，显示游戏2的菜单
    if (Open_menu == true)
    {
      game_2_Menu();
    }

    // 处理LVGL的定时器事件
    lv_timer_handler();
  }
}

// 该函数用于创建游戏2的菜单界面
void game_2_Menu()
{
  // 创建一个图像对象 ui_Image10 并将其添加到当前屏幕
  ui_Image10 = lv_img_create(lv_scr_act());
  // 设置图像的源为 ui_img_pico_nav_01_png
  lv_img_set_src(ui_Image10, &ui_img_pico_nav_01_png);
  // 设置图像的宽度为内容自适应
  lv_obj_set_width(ui_Image10, LV_SIZE_CONTENT);
  // 设置图像的高度为内容自适应
  lv_obj_set_height(ui_Image10, LV_SIZE_CONTENT);
  // 设置图像的 x 坐标为 0
  lv_obj_set_x(ui_Image10, 0);
  // 设置图像的 y 坐标为 95
  lv_obj_set_y(ui_Image10, 95);
  // 将图像居中对齐
  lv_obj_set_align(ui_Image10, LV_ALIGN_CENTER);
  // 添加高级点击测试标志
  lv_obj_add_flag(ui_Image10, LV_OBJ_FLAG_ADV_HITTEST);
  // 清除可滚动标志
  lv_obj_clear_flag(ui_Image10, LV_OBJ_FLAG_SCROLLABLE);

  // 在 ui_Image10 上创建一个按钮 ui_btn1
  ui_btn1 = lv_btn_create(ui_Image10);
  // 设置按钮的宽度为 30
  lv_obj_set_width(ui_btn1, 30);
  // 设置按钮的高度为 30
  lv_obj_set_height(ui_btn1, 30);
  // 设置按钮相对于父对象的 x 坐标为 -85
  lv_obj_set_x(ui_btn1, -85);
  // 设置按钮相对于父对象的 y 坐标为 0
  lv_obj_set_y(ui_btn1, 0);
  // 将按钮在父对象内居中对齐
  lv_obj_set_align(ui_btn1, LV_ALIGN_CENTER);
  // 添加聚焦时滚动标志
  lv_obj_add_flag(ui_btn1, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
  // 清除可滚动标志
  lv_obj_clear_flag(ui_btn1, LV_OBJ_FLAG_SCROLLABLE);
  // 设置按钮的背景图像源为 ui_img_pico_icon_01_png
  lv_obj_set_style_bg_img_src(ui_btn1, &ui_img_pico_icon_01_png, LV_PART_MAIN | LV_STATE_DEFAULT);

  // 在 ui_Image10 上创建另一个按钮 ui_btn2
  ui_btn2 = lv_btn_create(ui_Image10);
  // 设置按钮的宽度为 30
  lv_obj_set_width(ui_btn2, 30);
  // 设置按钮的高度为 30
  lv_obj_set_height(ui_btn2, 30);
  // 设置按钮相对于父对象的 x 坐标为 65
  lv_obj_set_x(ui_btn2, 65);
  // 设置按钮相对于父对象的 y 坐标为 0
  lv_obj_set_y(ui_btn2, 0);
  // 将按钮在父对象内居中对齐
  lv_obj_set_align(ui_btn2, LV_ALIGN_CENTER);
  // 添加聚焦时滚动标志
  lv_obj_add_flag(ui_btn2, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
  // 清除可滚动标志
  lv_obj_clear_flag(ui_btn2, LV_OBJ_FLAG_SCROLLABLE);

  // 根据 State_flag 的状态设置按钮 ui_btn2 的背景图像源
  if (State_flag == true)
  {
    // 如果 State_flag 为 true，设置背景图像源为 ui_img_pico_icon_02_png
    lv_obj_set_style_bg_img_src(ui_btn2, &ui_img_pico_icon_02_png, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else
  {
    // 如果 State_flag 为 false，设置背景图像源为 ui_img_pico_icon_03_png
    lv_obj_set_style_bg_img_src(ui_btn2, &ui_img_pico_icon_03_png, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
}

// 该函数是游戏2的主测试函数，包含游戏2的主要逻辑
void game_2_test()
{
  // 将引脚 27 设置为上拉输入模式
  pinMode(27, INPUT_PULLUP);
  // 初始化游戏，设置弹珠和挡板的初始位置和速度
  init_game();
  // 将引脚 0 设置为高电平
  digitalWrite(0, HIGH);
  // 记录上一次按键检查的时间
  unsigned long lastKeyCheckTime = 0;
  // 定义按键检查的时间间隔为 50 毫秒
  const unsigned long keyCheckInterval = 50;
  // 设置游戏2的运行标志为 true
  game2_flag = true;

  // 进入游戏2的主循环
  while (1)
  {
    // 获取当前时间
    unsigned long currentTime = millis();

    // 每间隔 keyCheckInterval 毫秒进行一次按键检查和触摸检测
    if (currentTime - lastKeyCheckTime >= keyCheckInterval)
    {
      // 读取引脚 27 的模拟值
      int adcValue = analogRead(27);

      // 根据模拟值判断按键输入，控制挡板的左右移动
      if (adcValue >= 800 && adcValue <= 810)
      {
        // 向左移动挡板，每次移动 5 个像素
        paddle.x -= 5;
        // 确保挡板不会移出屏幕左侧
        if (paddle.x < 0)
        {
          paddle.x = 0;
        }
      }
      else if (adcValue >= 905 && adcValue <= 915)
      {
        // 向右移动挡板，每次移动 5 个像素
        paddle.x += 5;
        // 确保挡板不会移出屏幕右侧
        if (paddle.x > SCREEN_WIDTH - paddle.width)
        {
          paddle.x = SCREEN_WIDTH - paddle.width;
        }
      }

      // 用于存储触摸点的 x 和 y 坐标
      uint16_t touchX1, touchY1;

      // 进行手势判断
      if (gfx.getTouch(&touchX1, &touchY1))
      {
        if (!isTouching)
        {
          // 记录起始触摸点的坐标
          startX = touchX1;
          startY = touchY1;
          // 设置触摸标志为 true
          isTouching = true;
          Serial.println("!isTouching");
        }
        else
        {
          // 计算触摸点在 y 方向和 x 方向的偏移量
          int yDiff = touchY1 - startY;
          int xDiff = touchX1 - startX;
          // 计算触摸点移动的距离
          int moveDistance = sqrt(xDiff * xDiff + yDiff * yDiff);

          // 如果移动距离小于点击判定阈值，认为是点击操作
          if (moveDistance < clickThreshold)
          {
            Serial.println("Click ");
            // 设置触摸标志为 false
            isTouching = false;
            // 如果点击位置在特定区域且菜单处于打开状态
            if (touchY1 >= 180 && touchX1 >= 205 && touchX1 <= 285 && Open_menu == true)
            {
              // 设置状态标志为 false
              State_flag = false;
              // 显示游戏2的菜单
              game_2_Menu();
              // 处理 LVGL 的定时器事件
              lv_timer_handler();
              Serial.println("State_flag ");
              // 延迟 100 毫秒
              delay(100);
              while (1)
              {
                if (gfx.getTouch(&touchX1, &touchY1))
                {
                  // 延迟 10 毫秒
                  delay(10);
                  // 如果再次点击特定区域
                  if (touchY1 >= 180 && touchX1 >= 205 && touchX1 <= 285)
                  {
                    // 设置状态标志为 true
                    State_flag = true;
                    // 设置菜单打开标志为 false
                    Open_menu = false;
                    Serial.println("State_flag ");
                    break;
                  }
                }
                // 如果点击另一个特定区域
                else if (touchY1 >= 180 && touchX1 < 115 && touchX1 > 45)
                {
                  Serial.println("return ");
                  // 设置状态标志为 true
                  State_flag = true;
                  // 设置菜单打开标志为 false
                  Open_menu = false;
                  // 设置游戏2运行标志为 false
                  game2_flag = false;
                  break;
                }
              }
            }
            // 如果点击另一个特定区域且菜单处于打开状态
            else if (touchY1 >= 180 && touchX1 < 115 && touchX1 > 45 && Open_menu == true)
            {
              Serial.println("return ");
              // 设置状态标志为 true
              State_flag = true;
              // 设置菜单打开标志为 false
              Open_menu = false;
              // 设置游戏2运行标志为 false
              game2_flag = false;
            }
          }
          // 如果 y 方向的偏移量小于阈值，认为是向上滑动操作
          else if (yDiff < -threshold)
          {
            Serial.println("sliding");
            // 设置触摸标志为 false
            isTouching = false;
            // 设置菜单打开标志为 true
            Open_menu = true;
          }
        }
      }
      else
      {
        // 如果没有触摸操作，设置触摸标志为 false
        isTouching = false;
      }

      // 更新上一次按键检查的时间
      lastKeyCheckTime = currentTime;
    }

    // 如果游戏2的运行标志为 false，退出游戏循环
    if (game2_flag == false)
    {
      // 将游戏2的启动标志设置为 0
      game_2 = 0;
      break;
    }

    // 清空当前屏幕上的所有 LVGL 对象
    lv_obj_clean(lv_scr_act());
    // 创建一个图像对象并设置其源为 ui_img_crowpi_320x2400_game_02_02_png
    lv_obj_t *img_bg = lv_img_create(lv_scr_act());
    lv_img_set_src(img_bg, &ui_img_crowpi_320x2400_game_02_02_png);
    // 将图像对象居中显示
    lv_obj_center(img_bg);
    // 绘制弹珠
    draw_marble();
    // 绘制挡板
    draw_paddle();
    // 更新弹珠的位置并进行碰撞检测
    update_marble();

    // 如果菜单处于打开状态，显示游戏2的菜单
    if (Open_menu == true)
    {
      game_2_Menu();
    }

    // 处理 LVGL 的定时器事件
    lv_timer_handler();
    // 延迟 20 毫秒
    delay(20);
  }
}
// 游戏1的主测试函数，负责游戏的初始化、循环逻辑以及处理用户输入
void game_1_test()
{
  // 初始化 TFT 屏幕驱动，为后续的屏幕显示操作做准备
  tft.begin();

  // 延迟 100 毫秒，确保屏幕初始化稳定
  delay(100);

  // 反转屏幕的显示颜色，可能是为了适配特定的显示需求
  tft.invertDisplay(true);
  // 设置字节交换，确保图像数据正确显示
  tft.setSwapBytes(true);
  // 将整个屏幕填充为白色，作为游戏的背景
  tft.fillScreen(TFT_WHITE);
  // 设置屏幕的旋转方向为 1，改变屏幕的显示朝向
  tft.setRotation(1);

  // 设置不同精灵对象的文本颜色和颜色深度
  img.setTextColor(TFT_BLACK, TFT_WHITE);
  img.setColorDepth(1);
  img2.setColorDepth(1);
  img3.setColorDepth(16);
  img3.setTextColor(TFT_RED, TFT_WHITE);
  img3.setTextSize(1);
  img4.setColorDepth(1);
  img4.setTextColor(TFT_BLACK, TFT_WHITE);
  img4.setTextSize(2);
  e.setColorDepth(1);
  e2.setColorDepth(1);

  // 创建不同功能的精灵对象
  img.createSprite(320, 100); // 用于绘制整体游戏画面的精灵
  img2.createSprite(33, 35);  // 用于绘制恐龙形象的精灵
  img3.createSprite(320, 50); // 用于绘制分数显示区域的精灵
  img4.createSprite(320, 50); // 用于绘制控制面板的精灵
  e.createSprite(eW, eH);     // 用于绘制第一个障碍物的精灵
  e2.createSprite(eW, eH);    // 用于绘制第二个障碍物的精灵

  // 再次将屏幕填充为白色，确保背景干净
  tft.fillScreen(TFT_WHITE);

  // 随机初始化游戏中线条的位置和宽度
  for (int i = 0; i < 6; i++)
  {
    linesX[i] = random(i * 60, (i + 1) * 60);
    linesW[i] = random(1, 14);
    linesX2[i] = random(i * 60, (i + 1) * 60);
    linesW2[i] = random(1, 14);
  }

  // 随机初始化游戏中凸起的位置和标志
  for (int n = 0; n < 2; n++)
  {
    bumps[n] = random(n * 100, (n + 1) * 120);
    bumpsF[n] = random(0, 2);
  }

  // 将引脚 0 设置为输出模式，并将其电平设置为高
  pinMode(0, OUTPUT);
  digitalWrite(0, HIGH);

  // 初始化按键文本标志为 1
  key_text = 1;
  // 初始化游戏运行标志为 1，表示游戏开始运行
  gameRun = 1;

  // 游戏主循环，不断处理游戏逻辑和用户输入
  while (1)
  {
    // 用于存储触摸点的 x 和 y 坐标
    uint16_t touchX1, touchY1;

    // 检测是否有触摸事件发生
    if (gfx.getTouch(&touchX1, &touchY1))
    {
      if (!isTouching)
      {
        // 如果之前没有触摸，记录当前触摸点的起始坐标
        startX = touchX1;
        startY = touchY1;
        // 标记为正在触摸
        isTouching = true;
      }
      else
      {
        // 计算触摸点在 y 方向和 x 方向的偏移量
        int yDiff = touchY1 - startY;
        int xDiff = touchX1 - startX;
        // 计算触摸点移动的距离
        int moveDistance = sqrt(xDiff * xDiff + yDiff * yDiff);

        // 如果移动距离小于点击判定的阈值，认为是点击操作
        if (moveDistance < clickThreshold)
        {
          Serial.println("Click ");
          // 标记为不再触摸
          isTouching = false;
          // 设置按键文本标志为 2，表示触发了某种点击相关的操作
          key_text = 2;
          // 如果点击位置在特定区域（y 坐标 >= 180 且 x 坐标在 205 到 285 之间）且界面标志为 true
          if (startY >= 180 && startX >= 205 && startX <= 285 && Interface_flag == true)
          {
            // 延迟 100 毫秒
            delay(100);
            while (1)
            {
              // 打印提示信息，表示游戏暂停
              Serial.println("Pause ");
              // 延迟 10 毫秒
              delay(10);
              // 在屏幕指定位置（0, 190）绘制一个 320x50 大小的暂停图像
              tft.pushImage(0, 190, 320, 50, gImage_Pause);
              // 再次检测是否有触摸事件
              if (gfx.getTouch(&touchX1, &touchY1))
              {
                // 打印触摸点的 x 坐标
                Serial.println("touchX1 ");
                Serial.println(touchX1);
                // 打印触摸点的 y 坐标
                Serial.println("touchY1 ");
                Serial.println(touchY1);
                // 如果点击位置在特定区域（y 坐标 >= 180 且 x 坐标在 205 到 285 之间）
                if (touchY1 >= 180 && touchX1 >= 205 && touchX1 <= 285)
                {
                  // 设置界面标志为 false，表示取消暂停状态
                  Interface_flag = false;
                  // 将屏幕填充为白色
                  tft.fillScreen(TFT_WHITE);
                  // 跳出内层循环，继续游戏
                  break;
                }
                // 如果点击位置在另一个特定区域（y 坐标 >= 180 且 x 坐标在 45 到 115 之间）
                else if (touchY1 >= 180 && touchX1 < 115 && touchX1 > 45)
                {
                  // 设置游戏 1 启动标志为 0，表示退出游戏
                  game_1 = 0;
                  // 将引脚 0 电平设置为低
                  digitalWrite(0, LOW);
                  // 设置游戏运行标志为 0，表示游戏停止
                  gameRun = 0;
                  // 设置界面标志为 false
                  Interface_flag = false;
                  // 将屏幕填充为白色
                  tft.fillScreen(TFT_WHITE);
                  // 打印提示信息，表示返回
                  Serial.println("retrue ");
                  // 跳出内层循环
                  break;
                }
              }
            }
          }
          // 如果点击位置在另一个特定区域（y 坐标 >= 180 且 x 坐标在 45 到 115 之间）且界面标志为 true
          else if (startY >= 180 && startX < 115 && startX > 45 && Interface_flag == true)
          {
            // 设置游戏 1 启动标志为 0，表示退出游戏
            game_1 = 0;
            // 将引脚 0 电平设置为低
            digitalWrite(0, LOW);
            // 设置游戏运行标志为 0，表示游戏停止
            gameRun = 0;
            // 设置界面标志为 false
            Interface_flag = false;
            // 将屏幕填充为白色
            tft.fillScreen(TFT_WHITE);
          }
        }
        // 如果 y 方向的偏移量小于阈值，认为是向上滑动操作
        else if (yDiff < -threshold)
        {
          // 打印提示信息，表示检测到滑动操作
          Serial.println("sliding");
          // 标记为不再触摸
          isTouching = false;
          // 设置界面标志为 true，表示打开某个界面
          Interface_flag = true;
        }
      }
    }
    else
    {
      // 如果没有检测到触摸事件，标记为不再触摸
      isTouching = false;
    }

    // 如果游戏处于运行状态
    if (gameRun == 1)
    {
      // 如果按键文本标志为 2
      if (key_text == 2)
      {
        // 将帧数计数器 f 重置为 0
        f = 0;
      }

      // 如果按键文本标志
      // 如果按键文本标志为 2，表示触发了特定操作（如跳跃）
      if (key_text == 2)
      {
        // 根据方向和滚动速度更新恐龙的 y 坐标
        y = y + dir * roll_speed;
        // 如果恐龙的 y 坐标小于等于 2，说明到达了跳跃上限
        if (y <= 2)
        {
          // 将 y 坐标固定为 2
          y = 2;
          // 反转方向，准备下落
          dir = dir * -1.00;
        }
        // 如果恐龙的 y 坐标大于等于 58，说明回到了地面
        else if (y >= 58)
        {
          // 将按键文本标志重置为 0，表示跳跃结束
          key_text = 0;
          // 反转方向，为下次跳跃做准备
          dir = dir * -1.00;
        }
      }

      // 如果帧数小于 9 且按键文本标志为 0（即未处于跳跃状态）
      if (frames < 9 && key_text == 0)
      {
        // 设置帧数计数器 f 为 1，可能对应恐龙的一种动画帧
        f = 1;
      }
      // 如果帧数大于 9 且按键文本标志为 0
      if (frames > 9 && key_text == 0)
      {
        // 设置帧数计数器 f 为 2，可能对应恐龙的另一种动画帧
        f = 2;
      }

      // 调用 drawS 函数，绘制游戏画面，包括恐龙、障碍物等
      drawS(x, y, f);
      // 帧数计数器加 1
      frames++;
      // 如果帧数达到 16，将帧数计数器重置为 0，实现循环计数
      if (frames == 16)
        frames = 0;

      // 调用 checkColision 函数，检查恐龙是否与障碍物发生碰撞
      checkColision();
    }

    // 如果游戏 1 的启动标志为 0，表示要退出游戏
    if (game_1 == 0)
    {
      // 记录当前时间，作为新游戏的开始时间
      start_t = millis();
      // 将屏幕填充为白色，清空画面
      tft.fillScreen(TFT_WHITE);
      // 随机初始化第一个障碍物的 x 坐标
      eX[0] = random(240, 310);
      // 随机初始化第二个障碍物的 x 坐标
      eX[1] = random(380, 460);
      // 将按键文本标志重置为 0
      key_text = 0;
      // 将恐龙的 x 坐标重置为 30
      x = 30;
      // 将恐龙的 y 坐标重置为 58
      y = 58;
      // 将恐龙的移动方向重置为 -1.4
      dir = -1.4;
      // 将滚动速度重置为初始游戏速度
      roll_speed = GAME_SPEED;
      // 将得分变量重置为 0
      score_v = 0;
      // 将得分重置为 0
      score = 0;
      // 将上一次得分重置为 0
      last_score = 0;
      // 跳出游戏主循环，结束游戏
      break;
    }

    // 如果游戏运行标志为 0，表示游戏暂停或结束，等待用户重新开始
    if (gameRun == 0)
    {
      while (1)
      {
        // 检测是否有触摸事件
        if (gfx.getTouch(&touchX1, &touchY1))
        {
          // 设置按键文本标志为 1，表示准备重新开始游戏
          key_text = 1;
          // 将屏幕填充为白色，清空画面
          tft.fillScreen(TFT_WHITE);
          // 跳出内层循环，准备重新开始游戏
          break;
        }
      }
    }

    // 如果按键文本标志为 1，表示用户触发了重新开始游戏的操作
    if (key_text == 1)
    {
      // 将按键文本标志重置为 0
      key_text = 0;
      // 如果游戏处于暂停或结束状态
      if (gameRun == 0)
      {
        // 设置游戏运行标志为 1，表示游戏重新开始
        gameRun = 1;
        // 记录当前时间，作为新游戏的开始时间
        start_t = millis();
        // 将屏幕填充为白色，清空画面
        tft.fillScreen(TFT_WHITE);
        // 随机初始化第一个障碍物的 x 坐标
        eX[0] = random(240, 310);
        // 随机初始化第二个障碍物的 x 坐标
        eX[1] = random(380, 460);
        // 将按键文本标志重置为 0
        key_text = 0;
        // 将恐龙的 x 坐标重置为 30
        x = 30;
        // 将恐龙的 y 坐标重置为 58
        y = 58;
        // 将恐龙的移动方向重置为 -1.4
        dir = -1.4;
        // 将滚动速度重置为初始游戏速度
        roll_speed = GAME_SPEED;
        // 将得分变量重置为 0
        score_v = 0;
        // 将得分重置为 0
        score = 0;
        // 将上一次得分重置为 0
        last_score = 0;
      }
    }
  }
}
// game1函数：绘制游戏画面
// 参数说明：
// x：恐龙的x坐标
// y：恐龙的y坐标
// frame：恐龙的动画帧
void drawS(int x, int y, int frame)
{
  // 将整体画面精灵填充为白色，清空之前的绘制内容
  img.fillSprite(TFT_WHITE);
  // 在整体画面精灵上绘制一条黑色直线，模拟地面
  img.drawLine(0, 84, 320, 84, TFT_BLACK);

  // 循环绘制并更新6条线条的位置
  for (int i = 0; i < 6; i++)
  {
    // 在整体画面精灵上绘制一条黑色线条
    img.drawLine(linesX[i], 100, linesX[i] + linesW[i], 100, TFT_BLACK);
    // 根据滚动速度更新线条的x坐标，使其向左移动
    linesX[i] = linesX[i] - roll_speed;
    // 如果线条移出屏幕左侧
    if (linesX[i] < -14)
    {
      // 随机重新生成线条的x坐标，使其从屏幕右侧重新进入
      linesX[i] = random(245, 320);
      // 随机重新生成线条的宽度
      linesW[i] = random(1, 14);
    }
    // 绘制另一条位置稍高的线条
    img.drawLine(linesX2[i], 98, linesX2[i] + linesW2[i], 98, TFT_BLACK);
    // 更新该线条的x坐标
    linesX2[i] = linesX2[i] - roll_speed;
    // 如果该线条移出屏幕左侧
    if (linesX2[i] < -14)
    {
      // 随机重新生成该线条的x坐标
      linesX2[i] = random(245, 320);
      // 随机重新生成该线条的宽度
      linesW2[i] = random(1, 14);
    }
  }

  // 循环绘制并更新2朵云朵的位置
  for (int j = 0; j < 2; j++)
  {
    // 在整体画面精灵上绘制云朵的位图
    img.drawXBitmap(clouds[j], 20, cloud, 38, 11, TFT_BLACK, TFT_WHITE);
    // 根据云朵移动速度更新云朵的x坐标，使其向左移动
    clouds[j] = clouds[j] - cloudSpeed;
    // 如果云朵移出屏幕左侧
    if (clouds[j] < -40)
    {
      // 随机重新生成云朵的x坐标，使其从屏幕右侧重新进入
      clouds[j] = random(320, 364);
    }
  }

  // 循环绘制并更新2个凸起的位置
  for (int n = 0; n < 2; n++)
  {
    // 在整体画面精灵上绘制凸起的位图
    img.drawXBitmap(bumps[n], 80, bump[bumpsF[n]], 34, 5, TFT_BLACK, TFT_WHITE);
    // 根据滚动速度更新凸起的x坐标，使其向左移动
    bumps[n] = bumps[n] - roll_speed;
    // 如果凸起移出屏幕左侧
    if (bumps[n] < -40)
    {
      // 随机重新生成凸起的x坐标
      bumps[n] = random(320, 364);
      // 随机重新生成凸起的类型标志
      bumpsF[n] = random(0, 2);
    }
  }

  // 循环更新2个障碍物的位置
  for (int m = 0; m < 2; m++)
  {
    // 根据滚动速度更新障碍物的x坐标，使其向左移动
    eX[m] = eX[m] - roll_speed;
    // 如果障碍物移出屏幕左侧
    if (eX[m] < -30)
    {
      // 随机重新生成障碍物的x坐标
      eX[m] = random(320, 364);
      // 随机重新生成障碍物的类型标志
      ef[m] = random(0, 2);
    }
  }

  // 在障碍物精灵 e 上绘制第一个障碍物的位图
  e.drawXBitmap(0, 0, enemy[0], eW, eH, TFT_BLACK, TFT_WHITE);
  // 在障碍物精灵 e2 上绘制第二个障碍物的位图
  e2.drawXBitmap(0, 0, enemy[1], eW, eH, TFT_BLACK, TFT_WHITE);
  // 在恐龙精灵上绘制对应帧的恐龙位图
  img2.drawXBitmap(0, 0, dino[frame], 33, 35, TFT_BLACK, TFT_WHITE);

  // 将第一个障碍物精灵推送到整体画面精灵上指定位置
  e.pushToSprite(&img, eX[0], 56, TFT_WHITE);
  // 将第二个障碍物精灵推送到整体画面精灵上指定位置
  e2.pushToSprite(&img, eX[1], 56, TFT_WHITE);
  // 将恐龙精灵推送到整体画面精灵上指定位置
  img2.pushToSprite(&img, x, y, TFT_WHITE);
  // 将整体画面精灵推送到屏幕指定位置显示
  img.pushSprite(0, 17);

  // 根据游戏开始时间和当前时间计算得分，每 120 毫秒增加 1 分
  score = (millis() - start_t) / 120;

  // 如果得分是 20 的倍数且和上一次得分不同
  if (score % 20 == 0 && last_score != score)
  {
    // 得分变量增加 100
    score_v += 100;
    // 更新上一次得分记录
    last_score = score;
  }

  // 将分数画板精灵填充为白色，清空之前内容
  img3.fillSprite(TFT_WHITE);
  // 在分数画板精灵上绘制 "Score:" 文本
  img3.drawString("Score:", 40, 20, 2);
  // 在分数画板精灵上绘制当前得分
  img3.drawString(String(score_v), 100, 20, 2);

  // 在分数画板精灵上绘制 "Speed:" 文本
  img3.drawString("Speed:", 190, 20, 2);
  // 在分数画板精灵上绘制当前滚动速度
  img3.drawString(String(roll_speed), 250, 20, 2);
  // 将分数画板精灵推送到屏幕指定位置显示
  img3.pushSprite(0, 140);

  // 如果界面标志为 true，显示特定图像
  if (Interface_flag == true)
  {
    // 在屏幕指定位置绘制指定大小的图像
    tft.pushImage(0, 190, 320, 50, gImage_Starte);
  }

  // 如果得分超过 t 加上加速阈值
  if (score > t + GAME_SPEEDUP_SCORE)
  {
    // 更新 t 为当前得分
    t = score;
    // 增加滚动速度
    roll_speed = roll_speed + GAME_SPEEDUP_GAP;
  }
}

// 碰撞检测函数，检查恐龙是否与障碍物发生碰撞
void checkColision()
{
  // 遍历两个障碍物
  for (int i = 0; i < 2; i++)
  {
    // 判断障碍物的 x 坐标是否在恐龙的有效碰撞范围内，且恐龙的 y 坐标大于 25
    if (eX[i] < x + dinoW / 2 && eX[i] > x && y > 25)
    {
      // 设置游戏运行标志为 0，表示游戏停止
      gameRun = 0;
      // 在屏幕指定区域填充白色，清除原有画面
      tft.fillRect(0, 30, 320, 110, TFT_WHITE);
      // 将整个屏幕填充为白色
      tft.fillScreen(TFT_WHITE);
      // 在屏幕指定位置绘制游戏结束的位图
      tft.drawXBitmap(45, 35, gameover, 223, 100, TFT_BLACK, TFT_WHITE);
      // 延迟 500 毫秒，给玩家显示游戏结束画面的时间
      delay(500);
    }
  }
}

// 扫描按钮状态并执行对应 RGB 灯效的函数
void Scan_button()
{
  // 如果 Style_1
  // 如果 Style_1 标志为 1，表示选择了样式 1
  if (Style_1 == 1)
  {
    // 在串口监视器打印提示信息，表示样式 1 开启
    Serial.println("Style_1 ON");
    // 调用 RGB_style1 函数，执行样式 1 的 RGB 灯效
    RGB_style1();
  }
  // 如果 Style_2 标志为 1，表示选择了样式 2
  else if (Style_2 == 1)
  {
    // 在串口监视器打印提示信息，表示样式 2 开启
    Serial.println("Style_2 ON");
    // 调用 RGB_style2 函数，执行样式 2 的 RGB 灯效
    RGB_style2();
  }
  // 如果 Style_3 标志为 1，表示选择了样式 3
  else if (Style_3 == 1)
  {
    // 在串口监视器打印提示信息，表示样式 3 开启
    Serial.println("Style_3 ON");
    // 调用 RGB_style3 函数，执行样式 3 的 RGB 灯效
    RGB_style3();
  }
  // 如果 Style_4 标志为 1，表示选择了样式 4
  else if (Style_4 == 1)
  {
    // 在串口监视器打印提示信息，表示样式 4 开启
    Serial.println("Style_4 ON");
    // 调用 RGB_style4 函数，执行样式 4 的 RGB 灯效
    RGB_style4();
  }
  // 如果以上样式标志都不为 1
  else
  {
    // 将 LED 使能引脚设置为低电平，关闭 LED 灯
    digitalWrite(LED_EN, LOW);
  }
}

// RGB 灯效样式 1 函数
void RGB_style1()
{
  // 将 LED 使能引脚设置为高电平，开启 LED 灯
  digitalWrite(LED_EN, HIGH);

  // 调用 colorWipe 函数，以 50 毫秒的间隔将灯条颜色依次设置为红色
  colorWipe(strip.Color(255, 0, 0), 50);
  // 更新灯条显示，使设置的颜色生效
  strip.show();
  // 调用 colorWipe 函数，以 50 毫秒的间隔将灯条颜色依次设置为绿色
  colorWipe(strip.Color(0, 255, 0), 50);
  // 更新灯条显示，使设置的颜色生效
  strip.show();
  // 调用 colorWipe 函数，以 50 毫秒的间隔将灯条颜色依次设置为蓝色
  colorWipe(strip.Color(0, 0, 255), 50);
  // 更新灯条显示，使设置的颜色生效
  strip.show();
}

// RGB 灯效样式 2 函数
void RGB_style2()
{
  // 将 LED 使能引脚设置为高电平，开启 LED 灯
  digitalWrite(LED_EN, HIGH);
  // 调用 whiteOverRainbow 函数，执行白色覆盖彩虹的灯效，参数 75 可能表示速度，5 可能表示循环次数
  whiteOverRainbow(75, 5);
}

// RGB 灯效样式 3 函数
void RGB_style3()
{
  // 将 LED 使能引脚设置为高电平，开启 LED 灯
  digitalWrite(LED_EN, HIGH);
  // 调用 rainbowFade2White 函数，执行彩虹渐变为白色的灯效，参数 3 可能分别表示延迟时间和彩虹循环次数
  rainbowFade2White(3, 3);
}

// RGB 灯效样式 4 函数
void RGB_style4()
{
  // 将 LED 使能引脚设置为高电平，开启 LED 灯
  digitalWrite(LED_EN, HIGH);
  // 调用 rainbowCycle 函数，执行彩虹循环灯效，参数 20 可能表示延迟时间
  rainbowCycle(20);
}