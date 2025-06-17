#include <lvgl.h> // Include the LVGL library for graphical user interface
// #include "demos/lv_demos.h" // Include LVGL demo functions
#include <Wire.h> // Include the Wire library for I2C communication
#include "ui.h" // Include the UI definitions
#include <Arduino.h> // Include the Arduino core library
#include <SPI.h> // Include the SPI library for SPI communication
#include "DHT20.h" // Include the DHT20 library for temperature and humidity sensor
#include <LovyanGFX.hpp> // Include the LovyanGFX library for display control

/* Screen resolution and rotation settings */
#define TFT_HOR_RES   320
#define TFT_VER_RES   240
#define TFT_ROTATION  LV_DISPLAY_ROTATION_0

/* LVGL drawing buffer size */
#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

/* Customize LGFX device class and configure ST7789 display screen */
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel;
  lgfx::Bus_SPI _bus;
  lgfx::Touch_FT5x06 _touch;

public:
  LGFX() {
    // SPI bus configuration
    auto bus_cfg = _bus.config();
    bus_cfg.spi_host = 0;
    bus_cfg.freq_write = 80000000;
    bus_cfg.pin_sclk = 6;
    bus_cfg.pin_mosi = 7;
    bus_cfg.pin_dc = 16;
    _bus.config(bus_cfg);
    _panel.setBus(&_bus);

    // Screen panel configuration
    auto panel_cfg = _panel.config();
    panel_cfg.pin_cs = 17;
    panel_cfg.memory_width = 240;
    panel_cfg.memory_height = 320;
    panel_cfg.panel_width = 240;
    panel_cfg.panel_height = 320;
    panel_cfg.offset_rotation = 1;
    panel_cfg.invert = true;

    _panel.config(panel_cfg);

    //Touch configuration
    auto touch_cfg = _touch.config();
    touch_cfg.x_min = 0;
    touch_cfg.x_max = 239;
    touch_cfg.y_min = 0;
    touch_cfg.y_max = 319;
    touch_cfg.pin_int = 25;
    touch_cfg.pin_rst = 24;
    touch_cfg.i2c_port = 0;
    touch_cfg.i2c_addr = 0x38;
    touch_cfg.pin_sda = 4;
    touch_cfg.pin_scl = 5;
    touch_cfg.freq = 400000;
    _touch.config(touch_cfg);
    _panel.setTouch(&_touch);

    setPanel(&_panel);
  }
};

LGFX gfx; // Create an instance of the custom display class
DHT20 DHT(&Wire1); // Create an instance of the DHT20 sensor using Wire1
unsigned long lastUpdate = 0; // Variable to store the last update time
const unsigned long interval = 1000; // Interval to update the data (1 second)

#if LV_USE_LOG != 0
void my_print(lv_log_level_t level, const char *buf)
{
    LV_UNUSED(level);
    Serial.println(buf);
    Serial.flush();
}
#endif

/* LVGL display refresh callback function */
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;
    
    gfx.setAddrWindow(area->x1, area->y1, w, h);
    gfx.pushPixels((lgfx::rgb565_t *)px_map, w * h);

    lv_display_flush_ready(disp);
}

/* Touch to read callback function */
void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data)
{
    uint16_t touchX, touchY;
    bool touched = gfx.getTouch(&touchX, &touchY);
    
    if (touched) {
        data->state = LV_INDEV_STATE_PRESSED;
        data->point.x = touchX;
        data->point.y = touchY;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

/* Using Arduino's millis() as the clock source for LVGL */
static uint32_t my_tick(void)
{
    return millis();
}

void setup(void) {
  Serial.begin(115200); // Initialize serial communication at 115200 baud rate
  Serial.println("Starting Temperature and Humidity Monitor...");

  Wire.setSDA(4); // Set the SDA pin for Wire
  Wire.setSCL(5); // Set the SCL pin for Wire
  Wire.begin(); // Initialize Wire
  delay(100); // Wait for 100 milliseconds

  Wire1.setSDA(2); // Set the SDA pin for Wire1
  Wire1.setSCL(3); // Set the SCL pin for Wire1
  Wire1.begin(); // Initialize Wire1
  
  Serial.println("Initializing display...");
  gfx.init();
  gfx.invertDisplay(false);
  gfx.setRotation(TFT_ROTATION);

  pinMode(0, OUTPUT); // Set pin 0 as an output for the backlight
  digitalWrite(0, HIGH); // Turn on the backlight

  // Initialize LVGL
  lv_init();
  lv_tick_set_cb(my_tick);

  // Register log printing function
#if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print);
#endif

  // Create display device
  lv_display_t *disp = lv_display_create(TFT_HOR_RES, TFT_VER_RES);
  lv_display_set_flush_cb(disp, my_disp_flush);
  lv_display_set_buffers(disp, draw_buf, NULL, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);

  // Initialize touch input device
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touchpad_read);

  ui_init(); // Initialize the UI
  lv_label_set_text(ui_TempLabel, "Initializing..."); // Set the temperature label text
  lv_label_set_text(ui_HumiLabel, "Initializing..."); // Set the humidity label text
  lv_label_set_text(ui_AlarmLabel, ""); // Clear the alarm label

  Serial.println("Initializing DHT20 sensor...");
  DHT.begin(); // Initialize the DHT20 sensor

  // Initialize LEDs and buzzer
  pinMode(20, OUTPUT); // Yellow LED
  pinMode(18, OUTPUT); // Red LED
  pinMode(19, OUTPUT); // GREEN LED
  pinMode(10, OUTPUT); // Buzzer
  digitalWrite(20, LOW); // Turn off yellow LED
  digitalWrite(18, LOW); // Turn off red LED
  digitalWrite(19, LOW); // Turn off green LED
  noTone(10); // Turn off buzzer
}

void loop() {
  lv_timer_handler(); // Handle LVGL timers

  unsigned long currentMillis = millis(); // Get the current time in milliseconds
  if (currentMillis - lastUpdate >= interval) { // Check if the interval has passed
    int status = DHT.read(); // Read data from the DHT20 sensor
    if (status == DHT20_OK) { // Check if the read was successful
      float humidity = DHT.getHumidity(); // Get the humidity value
      float temperature = DHT.getTemperature(); // Get the temperature value

      char tempStr[16]; // Buffer for temperature string
      char humiStr[16]; // Buffer for humidity string
      snprintf(tempStr, sizeof(tempStr), "%.1f C", temperature); // Format temperature string
      snprintf(humiStr, sizeof(humiStr), "%.1f %%", humidity); // Format humidity string

      lv_label_set_text(ui_TempLabel, tempStr); // Update the temperature label
      lv_label_set_text(ui_HumiLabel, humiStr); // Update the humidity label

      Serial.print("Temp: "); // Print temperature to serial
      Serial.print(temperature);
      Serial.print("  Humi: ");
      Serial.println(humidity);

      // Control LEDs and alarm messages
      if (temperature > 30) { // If temperature is above 30 degrees
        digitalWrite(18, HIGH); // Turn on red LED
        digitalWrite(20, LOW); // Turn off yellow LED
        lv_label_set_text(ui_AlarmLabel, "It's hot"); // Set alarm label text
      } else if (temperature > 25) { // If temperature is above 25 degrees
        digitalWrite(20, HIGH); // Turn on yellow LED
        digitalWrite(18, LOW); // Turn off red LED
        lv_label_set_text(ui_AlarmLabel, "Temperature is high"); // Set alarm label text
      } else {
        digitalWrite(20, LOW); // Turn off yellow LED
        digitalWrite(18, LOW); // Turn off red LED
      }

      if (humidity < 50.0) { // If humidity is below 40%
        tone(10, 1300); // Sound the buzzer
        lv_label_set_text(ui_AlarmLabel, "Air is dry"); // Set alarm label text
      } else {
        noTone(10); // Turn off buzzer
        if (temperature <= 25 && humidity >= 40.0) { // If temperature is below 25 and humidity is above 40%
          lv_label_set_text(ui_AlarmLabel, ""); // Clear the alarm label
        }
      }
    }
    lastUpdate = currentMillis; // Update the last update time
  }

  delay(5); // Short delay to reduce CPU usage
}