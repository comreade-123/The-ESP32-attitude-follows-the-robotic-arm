#include <WiFi.h>
#include <ESP32Servo.h>
#include <ssd1306.h>
#include <ssd1306_console.h>
Ssd1306Console console;

// WiFi配置
const char* ssid = "ArmController";
const char* password = "12345678";
WiFiClient client;

// 舵机配置
Servo M1, M2, M3, M4;
const int SERVO_PINS[4] = {13, 12, 14, 27}; // M1-M4引脚号
int targetAngles[4] = {0, 180, 0, 180};

// 运动控制参数
const int MOVE_INTERVAL = 20;      // 20ms运动间隔
const int MAX_STEP = 3;            // 单次最大角度变化
unsigned long lastMoveTime = 0;

void setup() {
  Serial.begin(115200);

  ssd1306_128x64_i2c_init();
  ssd1306_fillScreen(0x00);
  ssd1306_setFixedFont(ssd1306xled_font6x8);
  
  // 初始化舵机
  M1.attach(SERVO_PINS[0], 500, 2500);
  M2.attach(SERVO_PINS[1], 500, 2500);
  M3.attach(SERVO_PINS[2], 500, 2500);
  M4.attach(SERVO_PINS[3], 500, 2500);
  

  
  // 连接WiFi
  connectToWiFi();
}

void loop() {
  maintainConnection();
  receiveData();
  M1.write(targetAngles[0]);
  M2.write(targetAngles[1]);
  M3.write(targetAngles[2]);
  M4.write(targetAngles[3]);
  console.print(targetAngles[0]);
  console.print(" ");
  console.print(targetAngles[1]);
  console.print(" ");
  console.print(targetAngles[2]);
  console.print(" ");
  console.println(targetAngles[3]);
}

void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting...");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nConnected! Connecting to server...");
  client.connect("192.168.4.1", 8080); // 连接C主板
}

void maintainConnection() {
  if (!client.connected()) {
    Serial.println("Connection lost!");
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
  float yaw, pitch;
  
  sscanf(data.c_str(), "%f,%f", &yaw,&pitch);

  targetAngles[0] = yaw + 90;  // M1偏航
  targetAngles[1] = 180 - pitch; // M2
  targetAngles[2] = pitch; // M3
  targetAngles[3] = 180 - pitch; // M4

  Serial.printf("Target: M1:%d M2:%d M3:%d M4:%d\n",
               targetAngles[0], targetAngles[1],
               targetAngles[2], targetAngles[3]);
}
