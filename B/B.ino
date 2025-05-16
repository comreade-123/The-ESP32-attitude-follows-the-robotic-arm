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
Servo M5, M6;
int currentM5 = 90;   
int currentM6 = 130;  
const int M5_PIN = 13;
const int M6_PIN = 12; 

const int CLAW_OPEN = 130;      // 机械爪完全打开角度
const int CLAW_CLOSED = 180;      // 机械爪完全闭合角度
bool lastClawState = false;


// 运动控制参数
unsigned long lastMoveTime = 0;
const int MOVE_INTERVAL = 20; // 20ms移动间隔

void setup() {
  Serial.begin(115200);

  ssd1306_128x64_i2c_init();
  ssd1306_fillScreen(0x00);
  ssd1306_setFixedFont(ssd1306xled_font6x8);
  
  // 初始化舵机
  M5.attach(M5_PIN, 500, 2500);
  M6.attach(M6_PIN, 500, 2500);
  
  // 初始位置
  M5.write(currentM5);
  M6.write(currentM6);

  connectToWiFi();
}

void loop() {
  if (!client.connected()) reconnect();
  if (client.available()) processData();
  smoothMove();
  console.print("M5:");
  console.print(currentM5);
  console.print(" M6:");
  console.println(currentM6);

}

void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  client.connect("192.168.4.1", 8080); // 连接C主板的AP
}

void reconnect() {
  client.stop();
  delay(1000);
  Serial.println("Reconnecting...");
  connectToWiFi();
}

void processData() {
  String data = client.readStringUntil('\n');
  data.trim();
  
  // 数据格式："roll角度,压力传感器原始值"
  int commaIndex = data.indexOf(',');
  if (commaIndex == -1) return;

  // 解析M5滚转角度
  String rollStr = data.substring(0, commaIndex);
  currentM5 = 90+constrain(rollStr.toInt(), 0, 180);

  // 解析数字信号并控制机械爪
    bool clawState = data.substring(commaIndex+1).toInt();
    if(clawState==0){
      currentM6=CLAW_CLOSED;
    }else{
      currentM6=CLAW_OPEN;
    }
  
  // 调试输出
  Serial.printf("M5:%d° M6:%d° YL:%d\n", 
               currentM5,currentM6,clawState);
}

void smoothMove() {
  if (millis() - lastMoveTime >= MOVE_INTERVAL) {
    M5.write(currentM5);
    M6.write(currentM6);
    lastMoveTime = millis();
  }
}
