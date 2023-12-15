#include <Wire.h>

#define MODULE_I2C_ADDRESS   ((uint8_t)0x63)   // 传感器的 I2C 地址, 此处的 ICP10111 为 0x63

#define ICP10111_ID        0x08

#define ICP10111_ID_R_CMD    0xEFC8
#define ICP10111_OPT_W_CMD   0xC595   // 00 66 9C
#define ICP10111_OPT_R_CMD   0xC7F7

/**
 * @struct  sInvInvpres_t
 * @brief   Store the calculated data
 */
typedef struct {
  float sensorConstants[4]; // OTP values
  float pPaCalib[3];
  float LUTLower;
  float LUTUpper;
  float quadrFactor;
  float offstFactor;
}sInvInvpres_t;

/**
 * @struct sGetTempAndAirPressure_t
 * @brief Save the temperature and air pressure data
 */
typedef struct {
  float temp;
  float airPressure;
  float elevation;
}sGetTempAndAirPressure_t;
/**
 * @struct sInitialData_t
 * @brief Store initial check data
 */
typedef struct {
  float s1;
  float s2;
  float s3;
}sInitialData_t;
/**
 * @struct sUltimatelyData_t
 * @brief Store ultimate check data
 */
typedef struct {
  float A;
  float B;
  float C;
}sUltimatelyData_t;

/**
* @enum eWorkPattern_t
* @brief Work mode select
*/
typedef enum {
  eLowPower_P = 0x401A, /**<Low Power Mode Conversion Time: 1.8ms  Pressure RMS Noise:3.2Pa*/
  eNormal_P = 0x48A3, /**<Normal Mode Conversion Time:6.3ms Pressure RMS Noise:1.6Pa*/
  eLowNoise_P = 0x5059, /**<Low Noise Mode Conversion Time:23.8ms  Pressure RMS Noise:0.8Pa*/
  eUltraLowNoise_P = 0x58E0, /**<Ultra Low Noise Mode Conversion Time:94.5  Pressure RMS Noise:0.4Pa*/
  eLowPower_T = 0x609C,
  eNormal_T = 0x6825,
  eLowNoise_T = 0x70DF,
  eUltraLowNoise_T = 0x7866,
}eWorkPattern_t;

sInvInvpres_t _dataStorage;
sInvInvpres_t* _d = &_dataStorage;

sGetTempAndAirPressure_t _tempAndAirPressure;
sGetTempAndAirPressure_t* _t = &_tempAndAirPressure;

sInitialData_t _inputData;
sInitialData_t* _i = &_inputData;

sUltimatelyData_t _outData;
sUltimatelyData_t* _o = &_outData;

int l = 3;// 读字节长度 in100 需 <= 5

void writeReg(uint16_t reg, void* pBuf, size_t size)
{
  if (pBuf == NULL) {
    Serial.println("pBuf ERROR!! : null pointer");
  }
  uint8_t regBuf[2];
  regBuf[0] = reg >> 8;
  regBuf[1] = reg & 0xff;
  uint8_t* _pBuf = (uint8_t*)pBuf;
  Wire.beginTransmission(MODULE_I2C_ADDRESS);
  Wire.write(&regBuf[0], 1);
  Wire.write(&regBuf[1], 1);

  for (uint16_t i = 0; i < size; i++) {
    Wire.write(_pBuf[i]);
  }
  Wire.endTransmission();
}

uint8_t readReg(uint16_t reg, void* pBuf, size_t size)
{
  if (pBuf == NULL) {
    Serial.println("pBuf ERROR!! : null pointer");
  }
  uint8_t regBuf[2];
  regBuf[0] = reg >> 8;
  regBuf[1] = reg & 0xff;
  uint8_t* _pBuf = (uint8_t*)pBuf;
  Wire.beginTransmission(MODULE_I2C_ADDRESS);
  Wire.write(regBuf[0]);
  Wire.write(regBuf[1]);
  if (Wire.endTransmission() != 0)
    return 1;
  delay(7);   // 6.3 ms
  Wire.requestFrom(MODULE_I2C_ADDRESS, (uint8_t)size);
  for (uint16_t i = 0; i < size; i++) {
    _pBuf[i] = Wire.read();
  }
  return 0;
}

void readOtpFromI2c(short* out)
{
  uint8_t dataWrite[3];
  uint8_t dataRead[8];

  dataWrite[0] = 0x00;
  dataWrite[1] = 0x66;
  dataWrite[2] = 0x9C;
  writeReg(ICP10111_OPT_W_CMD, dataWrite, 3);
  for (uint8_t i = 0; i < 4; i++) {
    readReg(ICP10111_OPT_R_CMD, dataRead, 2);   // 不读crc
    out[i] = dataRead[0] << 8 | dataRead[1];
    Serial.println(out[i], HEX);
  }
}

void initBase(sInvInvpres_t* s, short* otp)
{
  for (uint8_t i = 0; i < 4; i++) {
    s->sensorConstants[i] = otp[i];
  }
  s->pPaCalib[0] = 45000.0;
  s->pPaCalib[1] = 80000.0;
  s->pPaCalib[2] = 105000.0;
  s->LUTLower = 3.5 * ((uint32_t)1 << 20);
  s->LUTUpper = 11.5 * ((uint32_t)1 << 20);
  s->quadrFactor = 1 / 16777216.0;
  s->offstFactor = 2048.0;
}

void calculateConversionConstants(float* pPa, sInitialData_t* i)
{
  _o->C = (i->s1 * i->s2 * (pPa[0] - pPa[1]) + \
    i->s2 * i->s3 * (pPa[1] - pPa[2]) + \
    i->s3 * i->s1 * (pPa[2] - pPa[0])) / \
    (i->s3 * (pPa[0] - pPa[1]) + i->s1 * (pPa[1] - pPa[2]) + \
      i->s2 * (pPa[2] - pPa[0]));
  _o->A = (pPa[0] * i->s1 - pPa[1] * i->s2 - (pPa[1] - pPa[0]) * _o->C) / (i->s1 - i->s2);
  _o->B = (pPa[0] - _o->A) * (i->s1 + _o->C);
}

void getTempAndAirPressure(void)
{
  uint8_t buf[9];
  uint32_t airPressure;
  uint16_t temp;
  float t;
  // 温度
  memset(buf, 0, sizeof(buf));
  readReg(eNormal_T, buf, 2);   // 读取温度原始数据 不读crc
  temp = (uint16_t)buf[0] << 8 | buf[1];
  t = (float)(temp - 32768);
  _i->s1 = _d->LUTLower + (float)(_d->sensorConstants[0] * t * t) * _d->quadrFactor;
  _i->s2 = _d->offstFactor * _d->sensorConstants[3] + (float)(_d->sensorConstants[1] * t * t) * _d->quadrFactor;
  _i->s3 = _d->LUTUpper + (float)(_d->sensorConstants[2] * t * t) * _d->quadrFactor;
  calculateConversionConstants(_d->pPaCalib, _i);

  // 压力
  memset(buf, 0, sizeof(buf));
  readReg(eNormal_P, buf, 4);   // 读取压力原始数据 不读LLSB和第2个crc
  airPressure = (uint32_t)buf[0] << 16 | (uint32_t)buf[1] << 8 | (uint32_t)buf[3];
  _t->airPressure = _o->A + (_o->B / (_o->C + airPressure));
  _t->temp = -45.f + 175.f / 65536.f * temp;

  // 海拔
  _t->elevation = 44330 * (1.0 - pow((_t->airPressure / 100.0) / 1015.0, 0.1903));
}

int8_t begin(void)
{
  uint8_t buf[3];
  short otp[4];
  if (readReg(ICP10111_ID_R_CMD, buf, 3) != 0)   // 读ID   这一步只是在这里测试, 在TEL0168中会省略
    return -1;
  uint16_t data = buf[0] << 8 | buf[1];
  if ((data & 0x3F) == 0x08) {   // 之前是 & 0x08
    readOtpFromI2c(otp);
    initBase(_d, otp);
    return 0;
  }
  return -1;
}

void setup()
{
  Serial.begin(115200);
  Wire.begin();

  while (begin() != 0) {
    Serial.println("Failed to initialize the sensor");
  }
  Serial.println("Success to initialize the sensor");
}

void loop()
{
  short otp[4];
  readOtpFromI2c(otp);   // 为确保能收到校准值，所以重复读取，这里没有意义
  getTempAndAirPressure();
  Serial.print("Read air pressure:");
  Serial.print(_t->airPressure);
  Serial.println("Pa");
  Serial.print("Read temperature:");
  Serial.print(_t->temp);
  Serial.println("°C");
  Serial.print("Read altitude:");
  Serial.print(_t->elevation);
  Serial.println("m");
  delay(1000);
}
