// 定义红色LED连接的引脚为18
#define Red_LED 18
// 定义黄色LED连接的引脚为20
#define Yellow_LED 20
// 定义绿色LED连接的引脚为19
#define Green_LED 19

// setup函数，程序启动时运行一次
void setup() {
  // 将红色LED引脚设置为输出模式，用于控制LED点亮或熄灭
  pinMode(Red_LED, OUTPUT);
  // 将黄色LED引脚设置为输出模式
  pinMode(Yellow_LED, OUTPUT);
  // 将绿色LED引脚设置为输出模式
  pinMode(Green_LED, OUTPUT);
}

// loop函数，程序运行后会不断循环执行这里面的代码
void loop() {
  // 点亮红色LED
  digitalWrite(Red_LED, HIGH);
  // 点亮绿色LED
  digitalWrite(Green_LED, HIGH);
  // 点亮黄色LED
  digitalWrite(Yellow_LED, HIGH);
  // 延迟1000毫秒（1秒），让LED保持点亮状态1秒
  delay(1000);

  // 关闭红色LED
  digitalWrite(Red_LED, LOW);
  // 关闭绿色LED
  digitalWrite(Green_LED, LOW);
  // 关闭黄色LED
  digitalWrite(Yellow_LED, LOW);
  // 延迟1000毫秒（1秒），让LED保持熄灭状态1秒
  delay(1000);
}