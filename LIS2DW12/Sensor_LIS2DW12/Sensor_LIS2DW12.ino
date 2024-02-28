#include <Wire.h>

#define MODULE_I2C_ADDRESS   ((uint8_t)0x19)   // 传感器的 I2C 地址

#define REG_CTRL_REG1    0x20     ///<Control register 1
#define REG_CTRL_REG2    0x21     ///<Control register 2
#define REG_CTRL_REG3    0x22     ///<Control register 3
#define REG_CTRL_REG4    0x23     ///<Control register 4
#define REG_CTRL_REG5    0x24     ///<Control register 5
#define REG_CTRL_REG6    0x25     ///<Control register 6
#define REG_CTRL_REG7    0x3F     ///<Control register 7
#define REG_STATUS_REG   0x27     ///<Status register
#define REG_OUT_X_L      0x28     ///<The low order of the X-axis acceleration register
#define REG_OUT_X_H      0x29     ///<The high point of the X-axis acceleration register
#define REG_OUT_Y_L      0x2A     ///<The low order of the Y-axis acceleration register
#define REG_OUT_Y_H      0x2B     ///<The high point of the Y-axis acceleration register
#define REG_OUT_Z_L      0x2C     ///<The low order of the Z-axis acceleration register
#define REG_OUT_Z_H      0x2D     ///<The high point of the Z-axis acceleration register

int l = 3;// 读字节长度 in100 需 <= 5
float _range = 0;   // 数据满量程 对应的精度值

void writeReg(uint8_t reg, void* pBuf, size_t size)
{
  if (pBuf == NULL) {
    Serial.println("pBuf ERROR!! : null pointer");
  }
  uint8_t* _pBuf = (uint8_t*)pBuf;
  Wire.beginTransmission(MODULE_I2C_ADDRESS);
  Wire.write(&reg, 1);

  for (uint16_t i = 0; i < size; i++) {
    Wire.write(_pBuf[i]);
  }
  Wire.endTransmission();
}

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

  // 这里为简化配置, 不过多解释寄存器详细配置位, 感兴趣可自行参照芯片手册
  uint8_t value = 0x44;   // REG_CTRL_REG2 复位
  writeReg(REG_CTRL_REG2, &value, 1);
  delay(15);   // 模块复位需要等待一点时间
  value = 0x40;   // Low-Power mode1 50 Hz
  writeReg(REG_CTRL_REG1, &value, 1);
  value = 0x00;   // ODR/2   ±2 g   low-pass filter path selected   disabled low-noise
  _range = 0.061f;   // 数据满量程为 ±2 g 时, 对应的精度值
  writeReg(REG_CTRL_REG6, &value, 1);
  Serial.println("Success to initialize the sensor");
}

void loop()
{
  uint8_t sensorData[2];
  Serial.print("x: ");
  //Read the acceleration in the x direction
  readReg(REG_OUT_X_L, sensorData, 2);
  int16_t x = ((int16_t)sensorData[1]) * 256 + (int16_t)sensorData[0];
  Serial.print(x * _range);
  Serial.print(" mg \ty: ");
  //Read the acceleration in the y direction
  readReg(REG_OUT_Y_L, sensorData, 2);
  int16_t y = ((int16_t)sensorData[1]) * 256 + (int16_t)sensorData[0];
  Serial.print(y * _range);
  Serial.print(" mg \tz: ");
  //Read the acceleration in the z direction
  readReg(REG_OUT_Z_L, sensorData, 2);
  int16_t z = ((int16_t)sensorData[1]) * 256 + (int16_t)sensorData[0];
  Serial.print(z * _range);
  Serial.println(" mg");
  delay(100);
}
