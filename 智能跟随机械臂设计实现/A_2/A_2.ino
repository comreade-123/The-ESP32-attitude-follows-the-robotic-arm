#include <WiFi.h>
#include <ESP32Servo.h>
#include <ssd1306.h>
#include <ssd1306_console.h>
#include <Adafruit_NeoPixel.h>
#define LED_PIN     16
#define LED_COUNT   4
Ssd1306Console console;

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// WiFi配置
const char* ssid = "ArmController";
const char* password = "12345678";
WiFiClient client;

// 舵机配置
Servo M1, M2, M3, M4, M5;
const int SERVO_PINS[3] = {23, 26, 25};
int targetAngles[3] = {0, 180, 180};

int currentM4 = 90;   
int currentM5 = 130;  
const int M4_PIN = 33;
const int M5_PIN = 32; 

const int CLAW_OPEN = 130;      // 机械爪完全打开角度
const int CLAW_CLOSED = 180;      // 机械爪完全闭合角度
bool lastClawState = false;

// 运动控制参数
const int MOVE_INTERVAL = 20;      // 20ms运动间隔
const int MAX_STEP = 3;            // 单次最大角度变化
unsigned long lastMoveTime = 0;

int LED_yellow = 19;
int LED_red = 18;

void setup() {
  Serial.begin(115200);

  ssd1306_128x64_i2c_init();
  ssd1306_fillScreen(0x00);
  ssd1306_setFixedFont(ssd1306xled_font6x8);

  strip.begin();
  strip.setBrightness(20);

  pinMode(LED_yellow,OUTPUT);
  pinMode(LED_red,OUTPUT);
  
  // 初始化舵机
  M1.attach(SERVO_PINS[0], 500, 2500);
  M2.attach(SERVO_PINS[1], 500, 2500);
  M3.attach(SERVO_PINS[2], 500, 2500);
  M4.attach(M4_PIN, 500, 2500);
  M5.attach(M5_PIN, 500, 2500);
  //自检
  zj();
  // 连接WiFi
  connectToWiFi();
  
}
// HSV转RGB函数（0 ≤ h <360, 0 ≤ s ≤1, 0 ≤ v ≤1）
uint32_t hsvToRgb(uint16_t h, float s, float v) {
  float c = v * s;
  float x = c * (1 - abs(fmod(h/60.0, 2) - 1));
  float m = v - c;
  
  float r, g, b;
  if(h < 60) { r = c; g = x; b = 0; }
  else if(h < 120) { r = x; g = c; b = 0; }
  else if(h < 180) { r = 0; g = c; b = x; }
  else if(h < 240) { r = 0; g = x; b = c; }
  else if(h < 300) { r = x; g = 0; b = c; }
  else { r = c; g = 0; b = x; }
  
  return strip.Color((r + m)*255, (g + m)*255, (b + m)*255);
}

void loop() {
  maintainConnection();
  receiveData();
  M1.write(targetAngles[0]);
  M2.write(targetAngles[1]);
  M3.write(targetAngles[2]);
  M4.write(currentM4);
  M5.write(currentM5);
  console.print(targetAngles[0]);
  console.print(" ");
  console.print(targetAngles[1]);
  console.print(" ");
  console.print(targetAngles[2]);
  console.print(" ");
  console.print(currentM4);
  console.print(" ");
  console.println(currentM5);
  static uint16_t hue = 0;  // 0-359度色相值
  
  // 生成彩虹色序列‌:ml-citation{ref="1,2" data="citationList"}
  for(int i=0; i<LED_COUNT; i++) {
    strip.setPixelColor(i, hsvToRgb(hue, 1.0, 1.0));
  }
  strip.show();
  
  hue = (hue + 1) % 360;  // 每次增加1度色相
  delay(5);  // 调整延时时间控制变色速度‌:ml-citation{ref="2" data="citationList"}
}

void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting...");
  console.print("Connecting...");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    console.print(".");
  }
  
  Serial.println("\nConnected! Connecting to server...");
  console.print("\nConnected! Connecting to server...");
  client.connect("192.168.4.1", 8080); // 连接C主板
}

void maintainConnection() {
  if (!client.connected()) {
    Serial.println("Connection lost!");
    console.println("Connection lost!");
    client.stop();
    connectToWiFi();
    delay(1000);
  }
}

void receiveData() {
  if (client.available()) {
    String data = client.readStringUntil('\n');
    parseSensorData(data);
  }
}

void parseSensorData(String &data) {
  // 数据格式：yaw,pitch,additional1,additional2
  float yaw, pitch, roll, clawAngle;
  
  sscanf(data.c_str(), "%f,%f,%f,%f", &yaw,&pitch,&roll,&clawAngle);

  targetAngles[0] = yaw + 90;  // M1偏航
  targetAngles[1] = constrain(180 - pitch, 50, 180); // M2
  targetAngles[2] = constrain(160 - pitch, 50, 180); // M3
  currentM5 = 90-roll; //M4
  if(clawAngle==0){
      currentM5=CLAW_CLOSED;
      digitalWrite(LED_red,HIGH);
      digitalWrite(LED_yellow,LOW);
    }else{
      currentM5=CLAW_OPEN;
      digitalWrite(LED_red,LOW);
      digitalWrite(LED_yellow,HIGH);
    }
  

  Serial.printf("Target: M1:%d M2:%d M3:%d\n",
               targetAngles[0], targetAngles[1],
               targetAngles[2]);
}
void zj(){
  M1.write(0);
  M2.write(180);
  M3.write(180);
  M4.write(90);
  M4.write(130);
  digitalWrite(LED_red,HIGH);
  digitalWrite(LED_yellow,LOW);
  delay(1000);
  M1.write(50);
  M2.write(130);
  M3.write(130);
  M4.write(0);
  M4.write(180);
  digitalWrite(LED_red,HIGH);
  digitalWrite(LED_yellow,LOW);
  delay(1000);
  M2.write(180);
  delay(100);
  M1.write(180);
  M3.write(180);
  M4.write(180);
  M4.write(130);
  delay(1000);
  M1.write(90);
  M4.write(90);
  delay(1000);
  digitalWrite(LED_red,LOW);
  digitalWrite(LED_yellow,HIGH);
}
