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
      if (String(advertisedDevice.getName().c_str()) == "MAX30102")//扫描是否有名为 MAX30102 的蓝牙设备
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

        //从 MAX30102 获取数据
        int SPO2 = -1, heartbeat = -1;
        float temperature = 0.0;
        if (cManufacturerData[2] != 0) {
          SPO2 = cManufacturerData[2];
        }
        heartbeat = ((uint32_t)cManufacturerData[3] << 24) | ((uint32_t)cManufacturerData[4] << 16) | \
          ((uint32_t)cManufacturerData[5] << 8) | ((uint32_t)cManufacturerData[6]);
        if (heartbeat == 0) {
          heartbeat = -1;
        }
        temperature = cManufacturerData[7] + cManufacturerData[8] / 100.0;

        Serial.println();
        Serial.print("SPO2 is : ");
        Serial.print(SPO2);
        Serial.println("%");
        Serial.print("heart rate is : ");
        Serial.print(heartbeat);
        Serial.println("Times/min");
        Serial.print("Temperature value of the board is : ");
        Serial.print(temperature);
        Serial.println(" ℃");
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
