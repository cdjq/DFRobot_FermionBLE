/*
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by Evandro Copercini
   Changed to a beacon scanner to report iBeacon, EddystoneURL and EddystoneTLM beacons by beegee-tokyo
*/

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEEddystoneURL.h>
#include <BLEEddystoneTLM.h>
#include <BLEBeacon.h>

#define ENDIAN_CHANGE_U16(x) ((((x)&0xFF00) >> 8) + (((x)&0xFF) << 8))

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

bool optSetFlag = false;

//设置ESP32 5秒扫描一次蓝牙设备
int scanTime = 5; //In seconds
BLEScan* pBLEScan;

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

void getTempAndAirPressure(uint16_t _temp, uint32_t _airPressure)
{
  float t;
  // 温度
  t = (float)(_temp - 32768);
  _i->s1 = _d->LUTLower + (float)(_d->sensorConstants[0] * t * t) * _d->quadrFactor;
  _i->s2 = _d->offstFactor * _d->sensorConstants[3] + (float)(_d->sensorConstants[1] * t * t) * _d->quadrFactor;
  _i->s3 = _d->LUTUpper + (float)(_d->sensorConstants[2] * t * t) * _d->quadrFactor;
  calculateConversionConstants(_d->pPaCalib, _i);

  // 压力
  _t->airPressure = _o->A + (_o->B / (_o->C + _airPressure));
  _t->temp = -45.f + 175.f / 65536.f * _temp;

  // 海拔
  _t->elevation = 44330 * (1.0 - pow((_t->airPressure / 100.0) / 1015.0, 0.1903));
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    if (advertisedDevice.haveName()) {
      if (String(advertisedDevice.getName().c_str()) == "ICP10111")//扫描是否有名为 ICP10111 的蓝牙设备
      {
        Serial.print("Device name: ");
        Serial.println(advertisedDevice.getName().c_str());
        std::string strManufacturerData = advertisedDevice.getManufacturerData();
        uint8_t cManufacturerData[100];
        strManufacturerData.copy((char*)cManufacturerData, strManufacturerData.length(), 0);
        Serial.printf("strManufacturerData: %d ", strManufacturerData.length());

        for (int i = 0; i < strManufacturerData.length(); i++) {
          Serial.printf("[%X]", cManufacturerData[i]);
        }

        if (!optSetFlag) {   // 初始化校准数据
          short out[4];
          for (uint8_t i = 0; i < 4; i++) {
            out[i] = cManufacturerData[i * 2 + 2] << 8 | cManufacturerData[i * 2 + 3];
          }
          initBase(_d, out);
          optSetFlag = true;
        }

        //从 ICP10111 获取原始数据
        uint16_t temp = (uint16_t)cManufacturerData[10] << 8 | cManufacturerData[11];
        uint32_t airPressure = (uint32_t)cManufacturerData[12] << 16 | (uint32_t)cManufacturerData[13] << 8 | (uint32_t)cManufacturerData[15];

        //将原始数据转化为voc指数
        getTempAndAirPressure(temp, airPressure);

        Serial.println();
        Serial.print("Read air pressure:");
        Serial.print(_t->airPressure);
        Serial.println("Pa");
        Serial.print("Read temperature:");
        Serial.print(_t->temp);
        Serial.println("°C");
        Serial.print("Read altitude:");
        Serial.print(_t->elevation);
        Serial.println("m");
        Serial.println("------------------");
      }
    }
  }
};

void setup()
{
  Serial.begin(115200);
  optSetFlag = false;
  Serial.println("Scanning...");

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99); // less or equal setInterval value
}

void loop()
{
  // put your main code here, to run repeatedly:
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
  delay(2000);
}