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

#define ENDIAN_CHANGE_U16(x) ((((x)&0xFF00) >> 8) + (((x)&0xFF) << 8))

//设置ESP32 5秒扫描一次蓝牙设备
int scanTime = 5; //In seconds
BLEScan* pBLEScan;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    if (advertisedDevice.haveName()) {
      if (String(advertisedDevice.getName().c_str()) == "GR10_30")//扫描是否有名为 GR10_30 的蓝牙设备
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

        //从 GR10_30 获取原始数据
        uint16_t state = ((uint16_t)cManufacturerData[2] << 8) | cManufacturerData[3];
        uint16_t gestures = ((uint16_t)cManufacturerData[4] << 8) | cManufacturerData[5];

        Serial.println();
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
