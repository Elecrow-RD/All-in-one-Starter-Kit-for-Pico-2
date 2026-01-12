#include <lvgl.h>
#include "demos/lv_demos.h"
#include <Wire.h>
#include <LovyanGFX.hpp>
#include "ui.h"
#include <Ticker.h>  //Call the ticker. H Library
#include <Servo.h>
#include <Arduino.h>
#include <SPI.h>
#include <BH1750.h>
#include <Adafruit_NeoPixel.h>
#include "DHT20.h"
#include <HCSR04.h>

#include <TFT_eSPI.h>
#include "gameover.h"
#include "noInternet.h"
#include "imgData.h"
#include "config.h"

// 贪吃蛇结构体定义
struct Snake {
  int x[50];
  int y[50];
  int length;
  int direction;
};

Snake snake;
int foodX, foodY;

// 屏幕分辨率
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// 弹珠结构体
struct Marble {
  int x;
  int y;
  int vx;
  int vy;
};

Marble marble;

// 挡板结构体
struct Paddle {
  int x;
  int y;
  int width;
  int height;
};

Paddle paddle;


unsigned long previousTime = 0;
int interval = 0;  // 设置延迟间隔时间，单位毫秒

void delay_new(int wait) {
  interval = wait;
  while (1) {
    unsigned long currentTime = millis();
    if (currentTime - previousTime >= wait) {
      previousTime = currentTime;
      break;
    }
    lv_timer_handler();
  }
}

//RGB
#define LED_PIN 22
#define LED_EN 23
// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 20
#define BRIGHTNESS 10
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);


//测试标志
uint8_t Count = 0;
char CloseData;
char CloseData1;
int NO_Test_Flag = 0;
int Test_Flag = 0;
int Close_Flag = 0;
int touch_flag = 0;
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel_instance;
  //    lgfx::Panel_ILI9488     _panel_instance;
  // lgfx::Panel_ILI9341     _panel_instance;
  lgfx::Bus_SPI _bus_instance;  // SPIバスのインスタンス
  lgfx::Touch_FT5x06 _touch_instance;

public:
  LGFX(void) {
    {
      auto cfg = _bus_instance.config();

      // SPIバスの設定
      cfg.spi_host = 0;  // 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
      // ※ ESP-IDFバージョンアップに伴い、VSPI_HOST , HSPI_HOSTの記述は非推奨になるため、エラーが出る場合は代わりにSPI2_HOST , SPI3_HOSTを使用してください。
      cfg.spi_mode = 0;           // SPI通信モードを設定 (0 ~ 3)
      cfg.freq_write = 80000000;  // 传输时的SPI时钟（最高80MHz，四舍五入为80MHz除以整数得到的值）
      cfg.freq_read = 16000000;   // 接收时的SPI时钟
      // ※ ESP-IDFバージョンアップに伴い、DMAチャンネルはSPI_DMA_CH_AUTO(自動設定)が推奨になりました。1ch,2chの指定は非推奨になります。
      cfg.pin_sclk = 6;   // SPIのSCLKピン番号を設定
      cfg.pin_mosi = 7;   // SPIのCLKピン番号を設定
      cfg.pin_miso = -1;  // SPIのMISOピン番号を設定 (-1 = disable)
      cfg.pin_dc = 16;    // SPIのD/Cピン番号を設定  (-1 = disable)

      _bus_instance.config(cfg);               // 設定値をバスに反映します。
      _panel_instance.setBus(&_bus_instance);  // バスをパネルにセットします。
    }

    {                                       // 表示パネル制御の設定を行います。
      auto cfg = _panel_instance.config();  // 表示パネル設定用の構造体を取得します。

      cfg.pin_cs = 17;  // CSが接続されているピン番号   (-1 = disable)
      //        cfg.pin_rst = 9;   // RSTが接続されているピン番号  (-1 = disable)
      cfg.pin_rst = -1;   // RSTが接続されているピン番号  (-1 = disable)
      cfg.pin_busy = -1;  // BUSYが接続されているピン番号 (-1 = disable)

      // ※ 以下の設定値はパネル毎に一般的な初期値が設定さ BUSYが接続されているピン番号 (-1 = disable)れていますので、不明な項目はコメントアウトして試してみてください。

      cfg.memory_width = 240;    // ドライバICがサポートしている最大の幅
      cfg.memory_height = 320;   // ドライバICがサポートしている最大の高さ
      cfg.panel_width = 240;     // 実際に表示可能な幅
      cfg.panel_height = 320;    // 実際に表示可能な高さ
      cfg.offset_x = 0;          // パネルのX方向オフセット量
      cfg.offset_y = 0;          // パネルのY方向オフセット量
      cfg.offset_rotation = 1;   //值在旋转方向的偏移0~7（4~7是倒置的）
      cfg.dummy_read_pixel = 8;  // 在读取像素之前读取的虚拟位数
      cfg.dummy_read_bits = 1;   // 读取像素以外的数据之前的虚拟读取位数
      cfg.readable = false;      // 如果可以读取数据，则设置为 true
      cfg.invert = true;         // 如果面板的明暗反转，则设置为 true
      cfg.rgb_order = false;     // 如果面板的红色和蓝色被交换，则设置为 true
      cfg.dlen_16bit = false;    // 对于以 16 位单位发送数据长度的面板，设置为 true
      cfg.bus_shared = false;    // 如果总线与 SD 卡共享，则设置为 true（使用 drawJpgFile 等执行总线控制）

      _panel_instance.config(cfg);
    }

    {  // タッチスクリーン制御の設定を行います。（必要なければ削除）
      auto cfg = _touch_instance.config();

      cfg.x_min = 0;            // タッチスクリーンから得られる最小のX値(生の値)
      cfg.x_max = 239;          // タッチスクリーンから得られる最大のX値(生の値)
      cfg.y_min = 0;            // タッチスクリーンから得られる最小のY値(生の値)
      cfg.y_max = 319;          // タッチスクリーンから得られる最大のY値(生の値)
      cfg.pin_int = 25;         // INTが接続されているピン番号
	  cfg.pin_rst = 24;
      cfg.bus_shared = false;   // 如果您使用与屏幕相同的总线，则设置为 true
      cfg.offset_rotation = 0;  // 显示和触摸方向不匹配时的调整 设置为 0 到 7 的值

      // I2C接続の場合
      cfg.i2c_port = 0;     // 使用するI2Cを選択 (0 or 1)
      cfg.i2c_addr = 0x38;  // I2Cデバイスアドレス番号
      cfg.pin_sda = 4;      // SDAが接続されているピン番号
      cfg.pin_scl = 5;      // SCLが接続されているピン番号
      cfg.freq = 400000;    // I2Cクロックを設定

      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);  // タッチスクリーンをパネルにセットします。
    }

    setPanel(&_panel_instance);
  }
};
LGFX gfx;

/*Change to your screen resolution*/
static const uint16_t screenWidth = 320;
static const uint16_t screenHeight = 240;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * screenHeight / 10];
//显示刷新
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  if (gfx.getStartCount() > 0) {
    gfx.endWrite();
  }
  gfx.pushImageDMA(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1, (lgfx::rgb565_t *)&color_p->full);

  lv_disp_flush_ready(disp);  //告诉lvgl刷新完成
}

uint16_t touchX, touchY;
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  data->state = LV_INDEV_STATE_REL;
  if (gfx.getTouch(&touchX, &touchY)) {
    data->state = LV_INDEV_STATE_PR;
    //设置坐标(隆显的屏幕Y轴相反，普阳的屏幕X轴相反)screenHeight -
    data->point.x = touchX;
    data->point.y = touchY;  //旋转后Y轴相反了
    Serial.print("Data x ");
    Serial.println(data->point.x);
    Serial.print("Data y ");
    Serial.println(data->point.y);
  }
}


//UI
#include "ui.h"
static int first_flag = 0;
extern int zero_clean;
extern int goto_widget_flag;
extern int bar_flag;
extern lv_obj_t *ui_MENU;
extern lv_obj_t *ui_TOUCH;
extern lv_obj_t *ui_JIAOZHUN;
extern lv_obj_t *ui_Label2;
static lv_obj_t *ui_Label;   //TOUCH界面label
static lv_obj_t *ui_Label3;  //TOUCH界面label3
static lv_obj_t *ui_Labe2;   //Menu界面进度条label
static lv_obj_t *bar;        //Menu界面进度条

static int val = 100;

/*RGB相关函数*/
void colorWipe(uint32_t color, int wait) {
  if (Style_1 == 0)
    return;
  for (int i = 0; i < strip.numPixels(); i++) {  // For each pixel in strip...
    strip.setPixelColor(i, color);               //  Set pixel's color (in RAM)
    strip.show();                                //  Update strip to match
    delay_new(wait);                             //  Pause for a moment
    lv_timer_handler();
  }
}
void colorWipe1(uint32_t color, int wait) {

  for (int i = 0; i < strip.numPixels(); i++) {  // For each pixel in strip...
    strip.setPixelColor(i, color);               //  Set pixel's color (in RAM)
    strip.show();                                //  Update strip to match
    delay(wait);                                 //  Pause for a moment
  }
}
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;
  if (Style_4 == 0)
    return;
  for (j = 0; j < 39; j++) {  //256 * 5 5 cycles of all colors on wheel
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
      lv_timer_handler();
    }
    strip.show();
    strip.setBrightness(j % 40);
    lv_timer_handler();
    delay_new(wait);
    if (Style_4 == 0)
      return;
  }
  for (j = 39; j > 0; j--) {  //256 * 5 5 cycles of all colors on wheel
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
      lv_timer_handler();
    }
    strip.show();
    strip.setBrightness(j % 40);

    lv_timer_handler();
    delay_new(wait);

    if (Style_4 == 0)
      return;
  }
}
void whiteOverRainbow(int whiteSpeed, int whiteLength) {
  if (Style_2 == 0)
    return;
  if (whiteLength >= strip.numPixels()) whiteLength = strip.numPixels() - 1;

  int head = whiteLength - 1;
  int tail = 0;
  int loops = 3;
  int loopNum = 0;
  uint32_t lastTime = millis();
  uint32_t firstPixelHue = 0;

  for (;;) {                                       // Repeat forever (or until a 'break' or 'return')
    for (int i = 0; i < strip.numPixels(); i++) {  // For each pixel in strip...
      if (((i >= tail) && (i <= head)) ||          //  If between head & tail...
          ((tail > head) && ((i >= tail) || (i <= head)))) {
        strip.setPixelColor(i, strip.Color(0, 0, 0, 255));  // Set white
      } else {                                              // else set rainbow
        int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
        strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
      }
      if (Style_2 == 0)
        return;
    }

    strip.show();  // Update strip with new contents
    lv_timer_handler();
    // There's no delay here, it just runs full-tilt until the timer and
    // counter combination below runs out.

    firstPixelHue += 40;  // Advance just a little along the color wheel

    if ((millis() - lastTime) > whiteSpeed) {  // Time to update head/tail?
      if (++head >= strip.numPixels()) {       // Advance head, wrap around
        head = 0;
        if (++loopNum >= loops) return;
      }
      if (++tail >= strip.numPixels()) {  // Advance tail, wrap around
        tail = 0;
      }
      lastTime = millis();  // Save time of last movement
    }
  }
}

void pulseWhite(uint8_t wait) {
  for (int j = 0; j < 256; j++) {  // Ramp up from 0 to 255
    // Fill entire strip with white at gamma-corrected brightness level 'j':
    strip.fill(strip.Color(0, 0, 0, strip.gamma8(j)));
    strip.show();
    delay(wait);
    lv_timer_handler();
    if (Style_3 == 0)
      return;
  }

  for (int j = 255; j >= 0; j--) {  // Ramp down from 255 to 0
    strip.fill(strip.Color(0, 0, 0, strip.gamma8(j)));
    strip.show();
    delay(wait);
    lv_timer_handler();
    if (Style_3 == 0)
      return;
  }
}
void theaterChaseRainbow(int wait) {
  int firstPixelHue = 0;           // First pixel starts at red (hue 0)
  for (int a = 0; a < 30; a++) {   // Repeat 30 times...
    for (int b = 0; b < 3; b++) {  //  'b' counts from 0 to 2...
      strip.clear();               //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for (int c = b; c < strip.numPixels(); c += 3) {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (strip.numPixels() steps):
        int hue = firstPixelHue + c * 65536L / strip.numPixels();
        uint32_t color = strip.gamma32(strip.ColorHSV(hue));  // hue -> RGB
        strip.setPixelColor(c, color);                        // Set pixel 'c' to value 'color'
      }
      strip.show();  // Update strip with new contents
      delay(wait);   // Pause for a moment
      lv_timer_handler();
      if (Style_4 == 0)
        return;
      firstPixelHue += 65536 / 90;  // One cycle of color wheel over 90 frames
    }
  }
}
void theaterChase(uint32_t color, int wait) {
  for (int a = 0; a < 10; a++) {   // Repeat 10 times...
    for (int b = 0; b < 3; b++) {  //  'b' counts from 0 to 2...
      strip.clear();               //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for (int c = b; c < strip.numPixels(); c += 3) {
        strip.setPixelColor(c, color);  // Set pixel 'c' to value 'color'
      }
      strip.show();  // Update strip with new contents
      delay(wait);   // Pause for a moment
      lv_timer_handler();
      if (Style_4 == 0)
        return;
    }
  }
}
void rainbowFade2White(int wait, int rainbowLoops, int whiteLoops) {
  if (Style_3 == 0)
    return;

  int fadeVal = 0, fadeMax = 100;

  // Hue of first pixel runs 'rainbowLoops' complete loops through the color
  // wheel. Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to rainbowLoops*65536, using steps of 256 so we
  // advance around the wheel at a decent clip.
  for (uint32_t firstPixelHue = 0; firstPixelHue < rainbowLoops * 65536;
       firstPixelHue += 256) {

    for (int i = 0; i < strip.numPixels(); i++) {  // For each pixel in strip...

      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      uint32_t pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());

      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the three-argument variant, though the
      // second value (saturation) is a constant 255.
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue, 255,
                                                          255 * fadeVal / fadeMax)));
    }

    strip.show();
    delay_new(wait);
    lv_timer_handler();

    if (firstPixelHue < 65536) {                                 // First loop,
      if (fadeVal < fadeMax) fadeVal++;                          // fade in
    } else if (firstPixelHue >= ((rainbowLoops - 1) * 65536)) {  // Last loop,
      if (fadeVal > 0) fadeVal--;                                // fade out
    } else {
      fadeVal = fadeMax;  // Interim loop, make sure fade is at max
    }
  }



  delay_new(200);  // Pause 1/2 second
}
//game1-设置
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite img = TFT_eSprite(&tft);
TFT_eSprite img2 = TFT_eSprite(&tft);
TFT_eSprite img3 = TFT_eSprite(&tft);
TFT_eSprite img4 = TFT_eSprite(&tft);

TFT_eSprite e = TFT_eSprite(&tft);
TFT_eSprite e2 = TFT_eSprite(&tft);

String inputString = "";      // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

int dinoW = 33;
int dinoH = 35;
float linesX[6];
int linesW[6];
float linesX2[6];
int linesW2[6];
float clouds[2] = { (float)random(0, 80), (float)random(100, 180) };
float bumps[2];
int bumpsF[2];
int eW = 18;
int eH = 38;

float eX[2] = { (float)random(240, 310), (float)random(380, 460) };
int ef[2] = { 0, 1 };

float roll_speed = GAME_SPEED;
float cloudSpeed = 0.4;
int x = 30;
int y = 58;
float dir = -1.4;
int frames = 0;
int f = 0;
bool gameRun = 0;
int score = 0;
int score_v = 0;
int last_score = 0;
unsigned long start_t = 0;
int t = 0;
bool button_jump = 0;
bool button_start = 0;


unsigned char key_text = 0;
// 记录起始触摸点
int startX;
int startY;
bool isTouching = false;
const int threshold = 30;
const int clickThreshold = 10;  // 点击判定的距离阈值
bool Interface_flag = false;
bool stop_flag = false;

//game2


// 初始化弹珠与挡板
void init_game() {
  marble.x = SCREEN_WIDTH / 2;
  marble.y = SCREEN_HEIGHT / 2;
  marble.vx = 2;
  marble.vy = -2;

  paddle.x = SCREEN_WIDTH / 2 - 30;
  paddle.y = SCREEN_HEIGHT - 20;
  paddle.width = 60;
  paddle.height = 10;
}

// 绘制弹珠
void draw_marble() {
  lv_obj_t *circle = lv_obj_create(lv_scr_act());
  lv_obj_set_size(circle, 10, 10);
  lv_obj_set_pos(circle, marble.x, marble.y);
  lv_obj_set_style_bg_color(circle, lv_color_hex(0xFF0000), 0);
}

// 绘制挡板
void draw_paddle() {
  lv_obj_t *rect = lv_obj_create(lv_scr_act());
  lv_obj_set_size(rect, paddle.width, paddle.height);
  lv_obj_set_pos(rect, paddle.x, paddle.y);
  lv_obj_set_style_bg_color(rect, lv_color_hex(0x00FF00), 0);
}

// 弹珠移动与碰撞检测
void update_marble() {
  marble.x += marble.vx;
  marble.y += marble.vy;

  // 撞墙检测
  if (marble.x <= 0 || marble.x >= SCREEN_WIDTH) {
    marble.vx = -marble.vx;
  }
  if (marble.y <= 0) {
    marble.vy = -marble.vy;
  }

  // 与挡板碰撞检测
  if (marble.y >= paddle.y && marble.y <= paddle.y + paddle.height && marble.x >= paddle.x && marble.x <= paddle.x + paddle.width) {
    marble.vy = -marble.vy;
  }

  // 检测弹珠是否飞出屏幕下方
  if (marble.y > SCREEN_HEIGHT) {
    init_game();
  }
}
//game3函数
// 初始化贪吃蛇
void snake_initialize() {
  snake.length = 3;
  snake.direction = 0;  // 0: 向右, 1: 向下, 2: 向左, 3: 向上
  snake.x[0] = SCREEN_WIDTH / 2;
  snake.y[0] = SCREEN_HEIGHT / 2;
  for (int i = 1; i < snake.length; i++) {
    snake.x[i] = snake.x[0] - i * 20;
    snake.y[i] = snake.y[0];
  }
}

// 初始化食物位置
void food_initialize() {
  foodX = random(SCREEN_WIDTH - 20);
  foodX = foodX - foodX % 20;
  foodY = random(SCREEN_HEIGHT - 20);
  foodY = foodY - foodY % 20;
}

// 绘制贪吃蛇
void draw_snake() {
  for (int i = 0; i < snake.length; i++) {
    lv_obj_t *rect = lv_obj_create(lv_scr_act());
    lv_obj_set_size(rect, 20, 20);
    lv_obj_set_pos(rect, snake.x[i], snake.y[i]);
    lv_obj_set_style_bg_color(rect, lv_color_hex(0x00FF00), 0);
  }
}

// 绘制食物
void draw_food() {
  lv_obj_t *foodRect = lv_obj_create(lv_scr_act());
  lv_obj_set_size(foodRect, 20, 20);
  lv_obj_set_pos(foodRect, foodX, foodY);
  lv_obj_set_style_bg_color(foodRect, lv_color_hex(0xFF0000), 0);
}

// 蛇移动逻辑
void snake_move() {
  for (int i = snake.length - 1; i > 0; i--) {
    snake.x[i] = snake.x[i - 1];
    snake.y[i] = snake.y[i - 1];
  }
  switch (snake.direction) {
    case 0:
      snake.x[0] += 20;
      break;
    case 1:
      snake.y[0] += 20;
      break;
    case 2:
      snake.x[0] -= 20;
      break;
    case 3:
      snake.y[0] -= 20;
      break;
  }
}

// 碰撞检测
bool collision_detection() {
  // 撞墙检测
  if (snake.x[0] < 0 || snake.x[0] >= SCREEN_WIDTH || snake.y[0] < 0 || snake.y[0] >= SCREEN_HEIGHT) {
    return true;
  }
  // 撞自己检测
  for (int i = 1; i < snake.length; i++) {
    if (snake.x[0] == snake.x[i] && snake.y[0] == snake.y[i]) {
      return true;
    }
  }
  return false;
}

// 吃食物逻辑
void eat_food() {
  if (snake.x[0] == foodX && snake.y[0] == foodY) {
    snake.length++;
    food_initialize();
  }
}




void setup(void) {
  Serial.begin(115200);

  pinMode(LED_EN, OUTPUT);
  // put your setup code here, to run once:
  strip.begin();  // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();   // Turn OFF all pixels ASAP
  strip.setBrightness(BRIGHTNESS);

  Wire.setSDA(4);
  Wire.setSCL(5);
  Wire.begin();
  delay(100);
  gfx.init();
  lv_init();
  // delay(5000); // 5 seconds
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * screenHeight / 10);

  //初始化显示
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  //将以下行更改为您的显示分辨率
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  //初始化输入设备驱动程序
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);
  delay(100);
  pinMode(0, OUTPUT);
  digitalWrite(0, HIGH);

  ui_init();  //开机UI界面
  lv_timer_handler();
  for (int i = 0; i <= 100; i++) {
    lv_bar_set_value(ui_Bar1, i, LV_ANIM_ON);
    char text[10];  // 足够存储格式化后的字符串，如 "100%"
    sprintf(text, "%d%%", i);
    lv_label_set_text(ui_Label1, text);
    lv_timer_handler();
    delay(50);
  }
  _ui_screen_change(&ui_Screen2, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, &ui_Screen2_screen_init);
  //lv_tabview_set_active(tabview, id, LV_ANIM_ON)


  lv_timer_handler();
}

void loop() {

  lv_timer_handler();
  Scan_button();
  delay(10);
  if (game_1 == 1) {
    gfx.fillScreen(TFT_BLACK);
    digitalWrite(0, LOW);
    game_1_test();
    //强制刷新，避免黑屏
    digitalWrite(0, HIGH);
    lv_timer_handler();
    lv_obj_invalidate(lv_scr_act());
    lv_obj_clear_state(ui_Switch6, LV_STATE_CHECKED);
  }
  if (game_2 == 1) {
    gfx.fillScreen(TFT_BLACK);
    digitalWrite(0, LOW);
    game_2_test();
    //强制刷新，避免黑屏
    digitalWrite(0, LOW);
    ui_init();
    _ui_screen_change(&ui_Screen2, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, &ui_Screen2_screen_init);
    lv_timer_handler();
    lv_tabview_set_act(ui_TabView2, 2, LV_ANIM_ON);
    lv_timer_handler();
    lv_obj_invalidate(lv_scr_act());
    // lv_obj_clear_state(ui_Switch7, LV_STATE_CHECKED);
    digitalWrite(0, HIGH);
  }
  if (game_3 == 1) {
    gfx.fillScreen(TFT_BLACK);
    digitalWrite(0, LOW);
    game_3_test();
    //强制刷新，避免黑屏
    digitalWrite(0, LOW);
    ui_init();
    _ui_screen_change(&ui_Screen2, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, &ui_Screen2_screen_init);
    lv_timer_handler();
    lv_tabview_set_act(ui_TabView2, 2, LV_ANIM_ON);
    lv_timer_handler();
    lv_obj_invalidate(lv_scr_act());
    // lv_obj_clear_state(ui_Switch7, LV_STATE_CHECKED);
    digitalWrite(0, HIGH);
  }
}
bool State_flag = true;
bool Open_menu = false;
bool game2_flag = true;
bool game3_flag = true;

void game_3_test() {
  snake_initialize();
  food_initialize();
  unsigned long lastKeyCheckTime = 0;
  unsigned long lastKeyCheckTime1 = 0;
  const unsigned long keyCheckInterval = 50;
  const unsigned long keyCheckInterval1 = 400;
  game3_flag = true;
  digitalWrite(0, HIGH);
  while (1) {
    unsigned long currentTime = millis();
    if (currentTime - lastKeyCheckTime >= keyCheckInterval) {
      int adcValue = analogRead(27);
      if (adcValue >= 740 && adcValue <= 750) {
        snake.direction = 3;
      } else if (adcValue >= 860 && adcValue <= 870) {
        snake.direction = 1;
      } else if (adcValue >= 800 && adcValue <= 810) {
        snake.direction = 2;
      } else if (adcValue >= 905 && adcValue <= 915) {
        snake.direction = 0;
      }
      lastKeyCheckTime = currentTime;
    }
    uint16_t touchX1, touchY1;

    // 手势判断
    if (gfx.getTouch(&touchX1, &touchY1)) {
      if (!isTouching) {
        startX = touchX1;
        startY = touchY1;
        isTouching = true;
      } else {
        int yDiff = touchY1 - startY;
        int xDiff = touchX1 - startX;
        // 计算移动距离
        int moveDistance = sqrt(xDiff * xDiff + yDiff * yDiff);
        if (moveDistance < clickThreshold) {
          Serial.println("Click ");
          isTouching = false;
          if (touchY1 >= 200 && touchX1 >= 220 && touchX1 <= 280 && Open_menu == true) {
            State_flag = false;
            game_2_Menu();
            lv_timer_handler();
            Serial.println("State_flag ");
            delay(100);
            while (1) {

              if (-) {
                delay(10);
                if (touchY1 >= 200 && touchX1 >= 220 && touchX1 <= 280) {
                  State_flag = true;
                  Open_menu = false;
                  break;
                }
              } else if (touchY1 >= 200 && touchX1 < 100 && touchX1 > 60) {
                Serial.println("return ");
                State_flag = true;
                Open_menu = false;
                game3_flag = false;
                break;
              }
            }
          } else if (touchY1 >= 200 && touchX1 < 100 && touchX1 > 60 && Open_menu == true) {
            Serial.println("return ");
            State_flag = true;
            Open_menu = false;
            game3_flag = false;
          }
        } else if (yDiff < -threshold) {
          Serial.println("sliding");
          isTouching = false;
          Open_menu = true;
        }
      }
    } else {
      isTouching = false;
    }
    if (game3_flag == false) {
      game_3 = 0;
      break;
    }
    if (currentTime - lastKeyCheckTime1 >= keyCheckInterval1) {

      lv_obj_clean(lv_scr_act());
      lv_obj_t *img_bg = lv_img_create(lv_scr_act());
      lv_img_set_src(img_bg, &ui_img_crowpi_320x2400_game_02_01_png);
      lv_obj_center(img_bg);

      draw_snake();
      draw_food();

      snake_move();
      eat_food();
      if (collision_detection()) {
        snake_initialize();
        food_initialize();
      }
      if (Open_menu == true) {
        game_2_Menu();
      }

      lv_timer_handler();
      lastKeyCheckTime1 = currentTime;
    }
    if (Open_menu == true) {
      game_2_Menu();
    }

    lv_timer_handler();
  }
}


void game_2_Menu() {
  ui_Image10 = lv_img_create(lv_scr_act());
  lv_img_set_src(ui_Image10, &ui_img_bg_11_png);
  lv_obj_set_width(ui_Image10, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Image10, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Image10, 0);
  lv_obj_set_y(ui_Image10, 105);
  lv_obj_set_align(ui_Image10, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_Image10, LV_OBJ_FLAG_ADV_HITTEST);   /// Flags
  lv_obj_clear_flag(ui_Image10, LV_OBJ_FLAG_SCROLLABLE);  /// Flags


  ui_btn1 = lv_btn_create(ui_Image10);
  lv_obj_set_width(ui_btn1, 20);
  lv_obj_set_height(ui_btn1, 20);
  lv_obj_set_x(ui_btn1, -70);
  lv_obj_set_y(ui_btn1, 0);
  lv_obj_set_align(ui_btn1, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_btn1, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_btn1, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  // lv_obj_set_style_bg_color(ui_btn1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  // lv_obj_set_style_bg_opa(ui_btn1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_img_src(ui_btn1, &ui_img_icon_01_png, LV_PART_MAIN | LV_STATE_DEFAULT);
  ui_btn2 = lv_btn_create(ui_Image10);
  lv_obj_set_width(ui_btn2, 20);
  lv_obj_set_height(ui_btn2, 20);
  lv_obj_set_x(ui_btn2, 70);
  lv_obj_set_y(ui_btn2, 0);
  lv_obj_set_align(ui_btn2, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_btn2, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_btn2, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  // lv_obj_set_style_bg_color(ui_btn1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  // lv_obj_set_style_bg_opa(ui_btn1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  if (State_flag == true) {

    lv_obj_set_style_bg_img_src(ui_btn2, &ui_img_icon_02_png, LV_PART_MAIN | LV_STATE_DEFAULT);

  } else {

    lv_obj_set_style_bg_img_src(ui_btn2, &ui_img_icon_03_png, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
}
void game_2_test() {

  pinMode(27, INPUT_PULLUP);
  init_game();
  digitalWrite(0, HIGH);
  unsigned long lastKeyCheckTime = 0;
  const unsigned long keyCheckInterval = 50;
  game2_flag = true;
  while (1) {
    int adcValue = analogRead(27);
    if (adcValue >= 740 && adcValue <= 750) {
      // 上按键逻辑，可按需添加
      game_2 = 0;
      break;
    }

    unsigned long currentTime = millis();
    if (currentTime - lastKeyCheckTime >= keyCheckInterval) {
      int adcValue = analogRead(27);
      if (adcValue >= 800 && adcValue <= 810) {
        paddle.x -= 5;
        if (paddle.x < 0) {
          paddle.x = 0;
        }
      } else if (adcValue >= 905 && adcValue <= 915) {
        paddle.x += 5;
        if (paddle.x > SCREEN_WIDTH - paddle.width) {
          paddle.x = SCREEN_WIDTH - paddle.width;
        }
      }

      uint16_t touchX1, touchY1;

      // 手势判断
      if (gfx.getTouch(&touchX1, &touchY1)) {
        if (!isTouching) {
          startX = touchX1;
          startY = touchY1;
          isTouching = true;
        } else {
          int yDiff = touchY1 - startY;
          int xDiff = touchX1 - startX;
          // 计算移动距离
          int moveDistance = sqrt(xDiff * xDiff + yDiff * yDiff);
          if (moveDistance < clickThreshold) {
            Serial.println("Click ");
            isTouching = false;
            if (touchY1 >= 200 && touchX1 >= 220 && touchX1 <= 280 && Open_menu == true) {
              State_flag = false;
              game_2_Menu();
              lv_timer_handler();
              Serial.println("State_flag ");
              delay(100);
              while (1) {

                if (gfx.getTouch(&touchX1, &touchY1)) {
                  delay(10);
                  if (touchY1 >= 200 && touchX1 >= 220 && touchX1 <= 280) {
                    State_flag = true;
                    Open_menu = false;
                    break;
                  }
                } else if (touchY1 >= 200 && touchX1 < 100 && touchX1 > 60) {
                  Serial.println("return ");
                  State_flag = true;
                  Open_menu = false;
                  game2_flag = false;
                  break;
                }
              }
            } else if (touchY1 >= 200 && touchX1 < 100 && touchX1 > 60 && Open_menu == true) {
              Serial.println("return ");
              State_flag = true;
              Open_menu = false;
              game2_flag = false;
            }
          } else if (yDiff < -threshold) {
            Serial.println("sliding");
            isTouching = false;
            Open_menu = true;
          }
        }
      } else {
        isTouching = false;
      }

      lastKeyCheckTime = currentTime;
    }
    if (game2_flag == false) {
      game_2 = 0;
      break;
    }
    lv_obj_clean(lv_scr_act());
    lv_obj_t *img_bg = lv_img_create(lv_scr_act());
    lv_img_set_src(img_bg, &ui_img_crowpi_320x2400_game_02_02_png);
    lv_obj_center(img_bg);
    draw_marble();
    draw_paddle();
    update_marble();
    if (Open_menu == true) {
      game_2_Menu();
    }

    lv_timer_handler();
    delay(20);
  }
}
void game_1_test() {
  tft.begin();

  delay(100);
  tft.invertDisplay(true);
  tft.setSwapBytes(true);
  tft.fillScreen(TFT_WHITE);
  tft.setRotation(1);
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



  img.createSprite(320, 100);  //整体画面
  img2.createSprite(33, 35);   //恐龙画面
  img3.createSprite(320, 70);  //分数画板
  img4.createSprite(320, 30);  //控制面板

  e.createSprite(eW, eH);     //障碍物1
  e2.createSprite(eW, eH);    //障碍物2
  tft.fillScreen(TFT_WHITE);  //整体填充颜色


  for (int i = 0; i < 6; i++) {
    linesX[i] = random(i * 60, (i + 1) * 60);
    linesW[i] = random(1, 14);
    linesX2[i] = random(i * 60, (i + 1) * 60);
    linesW2[i] = random(1, 14);
  }

  for (int n = 0; n < 2; n++) {
    bumps[n] = random(n * 100, (n + 1) * 120);
    bumpsF[n] = random(0, 2);
  }

  pinMode(0, OUTPUT);
  digitalWrite(0, HIGH);


  key_text = 1;

  gameRun = 1;
  while (1) {

    uint16_t touchX1, touchY1;

    // 手势判断
    if (gfx.getTouch(&touchX1, &touchY1)) {
      if (!isTouching) {
        startX = touchX1;
        startY = touchY1;
        isTouching = true;
      } else {
        int yDiff = touchY1 - startY;
        int xDiff = touchX1 - startX;
        // 计算移动距离
        int moveDistance = sqrt(xDiff * xDiff + yDiff * yDiff);
        if (moveDistance < clickThreshold) {
          Serial.println("Click ");
          isTouching = false;
          key_text = 2;
          if (startY >= 210 && startX >= 230 && startX <= 290 && Interface_flag == true) {
            delay(100);
            while (1) {

              Serial.println("Pause ");
              delay(10);
              tft.pushImage(0, 210, 320, 30, gImage_Pause);
              if (gfx.getTouch(&touchX1, &touchY1)) {
                Serial.println("touchX1 ");
                Serial.println(touchX1);
                Serial.println("touchY1 ");
                Serial.println(touchY1);
                if (touchY1 >= 210 && touchX1 >= 230 && touchX1 <= 290) {
                  Interface_flag = false;
                  tft.fillScreen(TFT_WHITE);
                  break;
                } else if (touchY1 >= 210 && touchX1 < 80 && touchX1 > 20) {
                  game_1 = 0;
                  digitalWrite(0, LOW);
                  gameRun = 0;
                  Interface_flag = false;
                  tft.fillScreen(TFT_WHITE);
                  Serial.println("retrue ");
                  break;
                }
              }
            }
          } else if (startY >= 210 && startX < 80 && startX > 20 && Interface_flag == true) {
            game_1 = 0;
            digitalWrite(0, LOW);
            gameRun = 0;
            Interface_flag = false;
            tft.fillScreen(TFT_WHITE);
          }
        } else if (yDiff < -threshold) {
          Serial.println("sliding");
          isTouching = false;
          Interface_flag = true;
        }
      }
    } else {
      isTouching = false;
    }
    if (gameRun == 1) {
      if (key_text == 2) {
        f = 0;
      }

      if (key_text == 2) {
        y = y + dir * roll_speed;
        if (y <= 2) {
          y = 2;
          dir = dir * -1.00;
        } else if (y >= 58) {
          key_text = 0;
          dir = dir * -1.00;
        }
      }

      if (frames < 9 && key_text == 0) {
        f = 1;
      }
      if (frames > 9 && key_text == 0) {
        f = 2;
      }

      drawS(x, y, f);
      frames++;
      if (frames == 16)
        frames = 0;


      checkColision();
    }
    if (game_1 == 0) {

      start_t = millis();
      tft.fillScreen(TFT_WHITE);
      eX[0] = random(240, 310);
      eX[1] = random(380, 460);
      key_text = 0;
      x = 30;
      y = 58;
      dir = -1.4;
      roll_speed = GAME_SPEED;
      score_v = 0;
      score = 0;
      last_score = 0;
      break;
    }
    if (gameRun == 0) {
      while (1) {

        if (gfx.getTouch(&touchX1, &touchY1)) {
          key_text = 1;
          tft.fillScreen(TFT_WHITE);
          break;
        }
      }
    }

    if (key_text == 1) {
      key_text = 0;
      if (gameRun == 0) {
        gameRun = 1;
        start_t = millis();
        tft.fillScreen(TFT_WHITE);
        eX[0] = random(240, 310);
        eX[1] = random(380, 460);
        key_text = 0;
        x = 30;
        y = 58;
        dir = -1.4;
        roll_speed = GAME_SPEED;
        score_v = 0;
        score = 0;
        last_score = 0;
      }
    }
  }
}
//game1函数
void drawS(int x, int y, int frame) {

  img.fillSprite(TFT_WHITE);
  img.drawLine(0, 84, 320, 84, TFT_BLACK);

  for (int i = 0; i < 6; i++) {
    img.drawLine(linesX[i], 100, linesX[i] + linesW[i], 100, TFT_BLACK);
    linesX[i] = linesX[i] - roll_speed;
    if (linesX[i] < -14) {
      linesX[i] = random(245, 320);
      linesW[i] = random(1, 14);
    }
    img.drawLine(linesX2[i], 98, linesX2[i] + linesW2[i], 98, TFT_BLACK);
    linesX2[i] = linesX2[i] - roll_speed;
    if (linesX2[i] < -14) {
      linesX2[i] = random(245, 320);
      linesW2[i] = random(1, 14);
    }
  }

  for (int j = 0; j < 2; j++) {  //云朵随机生成2
    img.drawXBitmap(clouds[j], 20, cloud, 38, 11, TFT_BLACK, TFT_WHITE);
    clouds[j] = clouds[j] - cloudSpeed;
    if (clouds[j] < -40)
      clouds[j] = random(320, 364);
  }

  for (int n = 0; n < 2; n++) {
    img.drawXBitmap(bumps[n], 80, bump[bumpsF[n]], 34, 5, TFT_BLACK, TFT_WHITE);
    bumps[n] = bumps[n] - roll_speed;
    if (bumps[n] < -40) {
      bumps[n] = random(320, 364);
      bumpsF[n] = random(0, 2);
    }
  }

  for (int m = 0; m < 2; m++) {

    eX[m] = eX[m] - roll_speed;
    if (eX[m] < -30)
      eX[m] = random(320, 364);
    ef[m] = random(0, 2);
  }

  e.drawXBitmap(0, 0, enemy[0], eW, eH, TFT_BLACK, TFT_WHITE);
  e2.drawXBitmap(0, 0, enemy[1], eW, eH, TFT_BLACK, TFT_WHITE);
  img2.drawXBitmap(0, 0, dino[frame], 33, 35, TFT_BLACK, TFT_WHITE);

  e.pushToSprite(&img, eX[0], 56, TFT_WHITE);
  e2.pushToSprite(&img, eX[1], 56, TFT_WHITE);
  img2.pushToSprite(&img, x, y, TFT_WHITE);
  img.pushSprite(0, 17);

  score = (millis() - start_t) / 120;

  if (score % 20 == 0 && last_score != score) {
    score_v += 100;
    last_score = score;
  }
  img3.fillSprite(TFT_WHITE);
  img3.drawString("Score:", 40, 20, 2);

  img3.drawString(String(score_v), 100, 20, 2);
  // 恢复文本颜色设置（如果后续绘制其他内容需要原来的颜色）
  // img.setTextColor(TFT_BLACK, TFT_WHITE);
  img3.drawString("Speed:", 190, 20, 2);
  img3.drawString(String(roll_speed), 250, 20, 2);
  // img3.pushToSprite(&img,0, 17, TFT_WHITE);
  img3.pushSprite(0, 140);

  if (Interface_flag == true) {
    tft.pushImage(0, 210, 320, 30, gImage_Starte);
  }

  if (score > t + GAME_SPEEDUP_SCORE) {
    t = score;
    roll_speed = roll_speed + GAME_SPEEDUP_GAP;
  }
}

void checkColision() {

  for (int i = 0; i < 2; i++) {
    if (eX[i] < x + dinoW / 2 && eX[i] > x && y > 25) {
      gameRun = 0;
      tft.fillRect(0, 30, 320, 110, TFT_WHITE);  //碰到障碍物结束画面的填充
      tft.fillScreen(TFT_WHITE);
      tft.drawXBitmap(45, 35, gameover, 223, 100, TFT_BLACK, TFT_WHITE);  //;结束的画面

      delay(500);
    }
  }
}


void Scan_button() {

  if (Style_1 == 1) {
    Serial.println("Style_1 ON");
    RGB_style1();
  } else if (Style_2 == 1) {
    Serial.println("Style_2 ON");
    RGB_style2();
  } else if (Style_3 == 1) {
    Serial.println("Style_3 ON");
    RGB_style3();
  } else if (Style_4 == 1) {
    Serial.println("Style_4 ON");
    RGB_style4();
  } else {

    digitalWrite(LED_EN, LOW);
  }
}
void RGB_style1() {

  digitalWrite(LED_EN, HIGH);

  colorWipe(strip.Color(255, 0, 0), 50);  // Red
  strip.show();
  colorWipe(strip.Color(0, 255, 0), 50);  // Green
  strip.show();
  colorWipe(strip.Color(0, 0, 255), 50);  // Blue
  strip.show();
}
void RGB_style2() {

  digitalWrite(LED_EN, HIGH);
  whiteOverRainbow(75, 5);
}
void RGB_style3() {
  digitalWrite(LED_EN, HIGH);
  rainbowFade2White(3, 3, 1);
}

void RGB_style4() {
  digitalWrite(LED_EN, HIGH);

  rainbowCycle(20);
}
