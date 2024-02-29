#include <Arduino.h>
#include <Wire.h>

#define MODULE_I2C_ADDRESS   ((uint8_t)0x73)   // 传感器的 I2C 地址

#define GESTURE_UP                      (1<<0)
#define GESTURE_DOWN                    (1<<1)
#define GESTURE_LEFT                    (1<<2)
#define GESTURE_RIGHT                   (1<<3)
#define GESTURE_FORWARD                 (1<<4)
#define GESTURE_BACKWARD                (1<<5)
#define GESTURE_CLOCKWISE               (1<<6)
#define GESTURE_COUNTERCLOCKWISE        (1<<7)
#define GESTURE_WAVE                    (1<<8)
#define GESTURE_HOVER                   (1<<9)
#define GESTURE_UNKNOWN                 (1<<10)
#define GESTURE_CLOCKWISE_C             (1<<14)   ///< Rotate clockwise continuously
#define GESTURE_COUNTERCLOCKWISE_C      (1<<15)   ///< Rotate counterclockwise continuously

int l = 3;// 读字节长度 in100 需 <= 5
float _range = 0;   // 数据满量程 对应的精度值

void setup()
{
  Serial.begin(115200);
  Wire.begin();

  delay(390);   // 等待模块上电启动完成
  // 这里为简化配置, 不过多解释寄存器详细配置位, 感兴趣可自行参照芯片手册
  Wire.beginTransmission(MODULE_I2C_ADDRESS);
  // reg
  Wire.write(0x09);
  // GESTURE_UP | GESTURE_DOWN | GESTURE_LEFT | GESTURE_RIGHT | GESTURE_FORWARD | GESTURE_BACKWARD | GESTURE_CLOCKWISE
  //  | GESTURE_COUNTERCLOCKWISE | GESTURE_CLOCKWISE_C | GESTURE_COUNTERCLOCKWISE_C
  Wire.write(0xC0);
  Wire.write(0xFF);
  Wire.endTransmission();
  delay(15);   // 等待初始化使能
  Serial.println("Success to initialize the sensor");
}

void loop()
{
  uint8_t buf[4] = { 0 };
  Wire.beginTransmission(MODULE_I2C_ADDRESS);
  Wire.write(0x06);
  Wire.endTransmission();
  // delay(1);   // 1 ms 数据采样延时
  Wire.requestFrom(MODULE_I2C_ADDRESS, (uint8_t)4);
  for (uint16_t i = 0; i < 4; i++) {
    buf[i] = Wire.read();
  }
  uint16_t state = ((uint16_t)buf[0] << 8) | buf[1];
  uint16_t gestures = ((uint16_t)buf[2] << 8) | buf[3];
  if (state) {
    if (gestures & GESTURE_UP) {
      Serial.println("Up");
    }
    if (gestures & GESTURE_DOWN) {
      Serial.println("Down");
    }
    if (gestures & GESTURE_LEFT) {
      Serial.println("Left");
    }
    if (gestures & GESTURE_RIGHT) {
      Serial.println("Right");
    }
    if (gestures & GESTURE_FORWARD) {
      Serial.println("Forward");
    }
    if (gestures & GESTURE_BACKWARD) {
      Serial.println("Backward");
    }
    if (gestures & GESTURE_CLOCKWISE) {
      Serial.println("Clockwise");
    }
    if (gestures & GESTURE_COUNTERCLOCKWISE) {
      Serial.println("Contrarotate");
    }
    if (gestures & GESTURE_WAVE) {
      Serial.println("Wave");
    }
    if (gestures & GESTURE_HOVER) {
      Serial.println("Hover");
    }
    if (gestures & GESTURE_CLOCKWISE_C) {
      Serial.println("Continuous clockwise");
    }
    if (gestures & GESTURE_COUNTERCLOCKWISE_C) {
      Serial.println("Continuous counterclockwise");
    }
  }
  delay(100);
}
