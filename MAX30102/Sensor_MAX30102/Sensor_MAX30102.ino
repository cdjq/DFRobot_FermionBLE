#include <Arduino.h>
#include <Wire.h>

#define MODULE_I2C_ADDRESS   ((uint8_t)0x57)   // 传感器的 I2C 地址

int l = 3;// 读字节长度 in100 需 <= 5

uint8_t readReg(uint8_t reg, void* pBuf, size_t size)
{
  if (pBuf == NULL) {
    Serial.println("pBuf ERROR!! : null pointer");
  }
  uint8_t* _pBuf = (uint8_t*)pBuf;
  Wire.beginTransmission(MODULE_I2C_ADDRESS);
  Wire.write(&reg, 1);
  if (Wire.endTransmission() != 0)
    return 0;
  // delay(1);   // 1 ms 数据采样延时
  Wire.requestFrom(MODULE_I2C_ADDRESS, (uint8_t)size);
  for (uint16_t i = 0; i < size; i++) {
    _pBuf[i] = Wire.read();
  }
  return size;
}

void setup()
{
  Serial.begin(115200);
  Wire.begin();

  delay(60);   // 等待模块上电启动完成
  // 这里为简化配置, 不过多解释寄存器详细配置位, 感兴趣可自行参照芯片手册
  // 开启测量辅助灯
  Wire.beginTransmission(MODULE_I2C_ADDRESS);
  Wire.write(0x20);
  Wire.write(0x00);
  Wire.write(0x01);
  Wire.endTransmission();
  Serial.println("Success to initialize the sensor");
}

void loop()
{
  uint8_t val = 0, rbuf[4] = { 0 }, tempBuf[2] = {0};
  int SPO2 = -1, heartbeat = -1;
  float temperature = 0.0;
  readReg(0x0C, &val, 1);
  if (val != 0) {
    SPO2 = val;
  }
  Serial.print("SPO2 is : ");
  Serial.print(SPO2);
  Serial.println("%");
  readReg(0x0E, rbuf, 4);
  heartbeat = ((uint32_t)rbuf[0] << 24) | ((uint32_t)rbuf[1] << 16) | ((uint32_t)rbuf[2] << 8) | ((uint32_t)rbuf[3]);
  if (heartbeat == 0) {
    heartbeat = -1;
  }
  Serial.print("heart rate is : ");
  Serial.print(heartbeat);
  Serial.println("Times/min");
  readReg(0x14, tempBuf, 2);
  temperature = tempBuf[0] + tempBuf[1] / 100.0;
  Serial.print("Temperature value of the board is : ");
  Serial.print(temperature);
  Serial.println(" ℃");
  //The sensor updates the data every 4 seconds
  delay(4000);
}
