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

//设置ESP32 5秒扫描一次蓝牙设备
int scanTime = 5; //In seconds
BLEScan* pBLEScan;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    if (advertisedDevice.haveName()) {
      if (String(advertisedDevice.getName().c_str()) == "LIS2DW12")//扫描是否有名为 LIS2DW12 的蓝牙设备
      {
        Serial.print("Device name: ");
        Serial.println(advertisedDevice.getName().c_str());
        std::string strManufacturerData = advertisedDevice.getManufacturerData();
        uint8_t cManufacturerData[100];
        strManufacturerData.copy((char*)cManufacturerData, strManufacturerData.length(), 0);
        Serial.printf("strManufacturerData: %d ", strManufacturerData.length());

        for (int i = 0; i < strManufacturerData.length(); i++) {
          Serial.printf("[%x]", cManufacturerData[i]);
        }

        //从 LIS2DW12 获取原始数据
        int16_t xRaw = (uint16_t)cManufacturerData[3] << 8 | cManufacturerData[2];
        int16_t yRaw = (uint16_t)cManufacturerData[5] << 8 | cManufacturerData[4];
        int16_t zRaw = (uint16_t)cManufacturerData[7] << 8 | cManufacturerData[6];

        //将原始数据转化为voc指数
        float _range = 0.061f;   // 数据满量程为 ±2 g 时, 对应的精度值
        float _x = xRaw * _range;
        float _y = yRaw * _range;
        float _z = zRaw * _range;

        Serial.println();
        Serial.print("x: ");
        //Read the acceleration in the x direction
        Serial.print(_x);
        Serial.print(" mg \ty: ");
        //Read the acceleration in the y direction
        Serial.print(_y);
        Serial.print(" mg \tz: ");
        //Read the acceleration in the z direction
        Serial.print(_z);
        Serial.println(" mg");
        Serial.println("------------------");
      }
    }
  }
};

void setup()
{
  Serial.begin(115200);
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
