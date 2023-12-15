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
      if (String(advertisedDevice.getName().c_str()) == "VL6180X")//扫描是否有名为 VL6180X 的蓝牙设备
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

        //从 VL6180X 获取原始数据
        uint8_t range = cManufacturerData[2];
        uint8_t status = cManufacturerData[3] >> 4;
        uint16_t value = (uint16_t)cManufacturerData[5] << 8 | cManufacturerData[4];

        //将原始数据value转化为光强
        float lux = 0.32 * value;

        Serial.println();
        String str = "ALS: " + String(lux) + " lux";
        Serial.println(str);
        String str1 = "Range: " + String(range) + " mm";
        switch (status) {
        case VL6180X_NO_ERR:
          Serial.println(str1);
          break;
        case VL6180X_EARLY_CONV_ERR:
          Serial.println("RANGE ERR: ECE check failed !");
          break;
        case VL6180X_MAX_CONV_ERR:
          Serial.println("RANGE ERR: System did not converge before the specified max!");
          break;
        case VL6180X_IGNORE_ERR:
          Serial.println("RANGE ERR: Ignore threshold check failed !");
          break;
        case VL6180X_MAX_S_N_ERR:
          Serial.println("RANGE ERR: Measurement invalidated!");
          break;
        case VL6180X_RAW_Range_UNDERFLOW_ERR:
          Serial.println("RANGE ERR: RESULT_RANGE_RAW < 0!");
          break;
        case VL6180X_RAW_Range_OVERFLOW_ERR:
          Serial.println("RESULT_RANGE_RAW is out of range !");
          break;
        case VL6180X_Range_UNDERFLOW_ERR:
          Serial.println("RANGE ERR: RESULT__RANGE_VAL < 0 !");
          break;
        case VL6180X_Range_OVERFLOW_ERR:
          Serial.println("RANGE ERR: RESULT__RANGE_VAL is out of range !");
          break;
        default:
          Serial.println("RANGE ERR: Systerm err!");
          break;
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
