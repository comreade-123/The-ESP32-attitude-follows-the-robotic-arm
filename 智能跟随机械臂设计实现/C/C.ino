#include <WiFi.h>
#include <Wire.h>
#include <MPU6050_tockn.h>
#include <Adafruit_Sensor.h>

// WiFi配置
const char* ssid = "ArmController";
const char* password = "12345678";
WiFiServer server(8080);
WiFiClient clientA, clientB;

// 传感器配置
MPU6050 zt(Wire);
const int FSR_PIN = 34;
float yaw, pitch, roll;
int pressure = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();  // 启动I2C通信
  
  // 初始化MPU6050
  zt.begin();
  zt.calcGyroOffsets(true);
  
  // 启动WiFi AP
  WiFi.softAP(ssid, password);
  server.begin();
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  pinMode(2,INPUT);
}

void loop() {
  zt.update();
  handleConnections();
  readSensors();
  sendData();
  Serial.print("yaw: "); 
  Serial.print(yaw);
  Serial.print(" pitch: "); 
  Serial.print(pitch);
  Serial.print(" roll: "); 
  Serial.print(roll);
  Serial.print(" 压力传感器：");
  Serial.println(digitalRead(2));

  delay(20);
}

void handleConnections() {
  if (!clientA.connected()) clientA = server.available();
  if (!clientB.connected()) clientB = server.available();
}

void readSensors() {
  
  yaw = zt.getAngleZ();///////////////////////////////////
  pitch = (zt.getAngleX());//////////////////////////////
  roll = zt.getAngleY();
  
}

void sendData() {
  if (clientA.connected()) {
    int clawAngle = digitalRead(2);
    String dataA = String(yaw) + "," + 
                 String(pitch) + "," + 
                 String(roll) + "," + 
                 String(clawAngle);
    clientA.println(dataA);
  }
}
