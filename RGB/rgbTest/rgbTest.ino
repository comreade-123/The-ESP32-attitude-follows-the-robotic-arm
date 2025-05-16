#include <Adafruit_NeoPixel.h>
#define LED_PIN     16
#define LED_COUNT   4

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  strip.begin();
  strip.setBrightness(20);  // 统一设置亮度‌:ml-citation{ref="3" data="citationList"}
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
  static uint16_t hue = 0;  // 0-359度色相值
  
  // 生成彩虹色序列‌:ml-citation{ref="1,2" data="citationList"}
  for(int i=0; i<LED_COUNT; i++) {
    strip.setPixelColor(i, hsvToRgb(hue, 1.0, 1.0));
  }
  strip.show();
  
  hue = (hue + 1) % 360;  // 每次增加1度色相
  delay(20);  // 调整延时时间控制变色速度‌:ml-citation{ref="2" data="citationList"}
}
