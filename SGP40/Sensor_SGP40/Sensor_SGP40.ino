#include <Wire.h>

extern "C" {
#include "sensirion_arch_config.h"
#include "sensirion_voc_algorithm.h"
};

#define MODULE_I2C_ADDRESS   ((uint8_t)0x59)   // 传感器的 I2C 地址, 此处的 SGP40 为 0x59

#define TEST_OK                                           0xD400

#define CMD_HEATER_OFF_H                                  0x36
#define CMD_HEATER_OFF_L                                  0x15
#define CMD_HEATER_OFF_SIZE                               2

#define CMD_MEASURE_TEST_H                                0x28
#define CMD_MEASURE_TEST_L                                0x0E
#define CMD_MEASURE_TEST_SIZE                             2

#define CMD_SOFT_RESET_H                                  0x00
#define CMD_SOFT_RESET_L                                  0x06
#define CMD_SOFT_RESET_SIZE                               2

#define CMD_MEASURE_RAW_H                                 0x26
#define CMD_MEASURE_RAW_L                                 0x0F

#define INDEX_MEASURE_RAW_H                               0
#define INDEX_MEASURE_RAW_L                               1
#define INDEX_RH_H                                        2
#define INDEX_RH_L                                        3
#define INDEX_RH_CHECK_CRC                                4
#define INDEX_TEM_H                                       5
#define INDEX_TEM_L                                       6
#define INDEX_TEM_CHECK_CRC                               7

#define DURATION_READ_RAW_VOC                             30   // 读取原始数据的延时
#define DURATION_WAIT_MEASURE_TEST                        320

int l = 3;// 读字节长度 in100 需 <= 5
float _relativeHumidity = 50.0;   // 用于校准测量值的环境相对湿度
float _temperatureC = 25.0;   // 用于校准测量值的环境温度
uint8_t _rhTemData[8];
VocAlgorithmParams _vocaAgorithmParams;

bool sgp40MeasureTest()
{
  uint8_t data[l] = { 0,0,0 };
  uint16_t value = 0;
  uint8_t testCommand[CMD_MEASURE_TEST_SIZE] = { CMD_MEASURE_TEST_H,CMD_MEASURE_TEST_L };
  Wire.beginTransmission(MODULE_I2C_ADDRESS);
  for (uint8_t i = 0;i < CMD_MEASURE_TEST_SIZE;i++) {
    Wire.write(testCommand[i]);
  }
  Wire.endTransmission();
  delay(DURATION_WAIT_MEASURE_TEST);
  Wire.requestFrom(MODULE_I2C_ADDRESS, (uint8_t)l);
  for (uint8_t i = 0;i < l;i++) {
    data[i] = Wire.read();
  }
  value = (data[0] << 8) | data[1];
  if (value == TEST_OK) {
    return true;
  }
  return false;
}

uint8_t checkCrc(uint8_t data1, uint8_t data2)
{
  uint8_t crc = 0xFF;
  uint8_t data[2];
  data[0] = data1;
  data[1] = data2;
  for (int i = 0; i < 2;i++) {
    crc ^= data[i];
    for (uint8_t bit = 8;bit > 0;--bit) {
      if (crc & 0x80) {
        crc = (crc << 1) ^ 0x31u;
      } else {
        crc = (crc << 1);
      }
    }
  }
  return crc;
}

void dataTransform(void)
{
  uint16_t RH = (uint16_t)((_relativeHumidity * 65535) / 100 + 0.5);
  uint16_t TemC = (uint16_t)((_temperatureC + 45) * (65535 / 175) + 0.5);
  _rhTemData[INDEX_MEASURE_RAW_H] = CMD_MEASURE_RAW_H;
  _rhTemData[INDEX_MEASURE_RAW_L] = CMD_MEASURE_RAW_L;
  _rhTemData[INDEX_RH_H] = RH >> 8;
  _rhTemData[INDEX_RH_L] = RH & 0x00FF;
  _rhTemData[INDEX_RH_CHECK_CRC] = checkCrc(_rhTemData[INDEX_RH_H], _rhTemData[INDEX_RH_L]);
  _rhTemData[INDEX_TEM_H] = TemC >> 8;
  _rhTemData[INDEX_TEM_L] = TemC & 0x00FF;
  _rhTemData[INDEX_TEM_CHECK_CRC] = checkCrc(_rhTemData[INDEX_TEM_H], _rhTemData[INDEX_TEM_L]);
}

uint16_t getVoclndex(void)
{
  uint8_t data[l] = { 0,0,0 };
  int32_t value;
  int32_t vocIndex = 0;
  dataTransform();
  Wire.beginTransmission(MODULE_I2C_ADDRESS);
  for (int i = 0;i < 8;i++) {   // i2ctx:3
    Wire.write(_rhTemData[i]);   // 在50相对湿度25摄氏度时0x26, 0x0F, 0x80, 0x00, 0xA2, 0x66, 0x66, 0x93
  }
  Wire.endTransmission();//i2c null

  delay(DURATION_READ_RAW_VOC); // 程序等待30ms，让SGP40做好准备

  Wire.requestFrom(MODULE_I2C_ADDRESS, (uint8_t)l);
  if (Wire.available() >= l) {
    for (int i = 0; i < l; i++) {//读取I2C传感器输出的数据
      data[i] = Wire.read();
      Serial.print(data[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
  Wire.endTransmission();//i2c null
  value = (data[0] << 8) | data[1];
  VocAlgorithm_process(&_vocaAgorithmParams, value, &vocIndex);
  return vocIndex;
}

void setup()
{
  Serial.begin(115200);
  Wire.begin();

  /*
   * The preheating time of the sensor is 10s.
   * duration:Initialize the wait time. Unit: millisecond. Suggestion: duration > = 10000 ms
   */
  do {
    /* code */
    VocAlgorithm_init(&_vocaAgorithmParams);
    unsigned long timestamp = millis();
    while (millis() - timestamp < 10000) {
      getVoclndex();
    }
  } while (!sgp40MeasureTest());
  Serial.println("----------------------------------------------");
  Serial.println("SGP40 initialized successfully!");
  Serial.println("----------------------------------------------");
}

void loop()
{
  /**
   * @brief  Measure VOC index after humidity compensation
   * @note   VOC index can indicate the quality of the air directly. The larger the value, the worse the air quality.
   * @note       0-100，no need to ventilate, purify
   * @note       100-200，no need to ventilate, purify
   * @note       200-400，ventilate, purify
   * @note       400-500，ventilate, purify intensely
   * @return The VOC index measured, ranged from 0 to 500
   */
  uint16_t index = getVoclndex();//Declare variable that is to be used to store VOC index.

  Serial.print("vocIndex = ");
  Serial.println(index);
  delay(1000);
}
