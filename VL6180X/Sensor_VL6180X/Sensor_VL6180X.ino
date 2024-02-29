#include <Arduino.h>
#include <Wire.h>

#define MODULE_I2C_ADDRESS   ((uint8_t)0x29)   // 传感器的 I2C 地址

//Device model identification number：0xB4
#define VL6180X_IDENTIFICATION_MODEL_ID               0x000
#define VL6180X_SYSTEM_MODE_GPIO0                     0X010
#define VL6180X_SYSTEM_MODE_GPIO1                     0x011
#define VL6180X_SYSTEM_INTERRUPT_CONFIG_GPIO          0x014
#define VL6180X_SYSTEM_INTERRUPT_CLEAR                0x015
#define VL6180X_SYSTEM_FRESH_OUT_OF_RESET             0x016
#define VL6180X_SYSTEM_GROUPED_PARAMETER_HOLD         0x017
#define VL6180X_SYSRANGE_START                        0x018
// High Threshold value for ranging comparison. Range 0-255mm.
#define VL6180X_SYSRANGE_THRESH_HIGH                  0x019
//Low Threshold value for ranging comparison. Range 0-255mm.
#define VL6180X_SYSRANGE_THRESH_LOW                   0x01A
// Time delay between measurements in Ranging continuous mode. Range 0-254 (0 = 10ms). Step size = 10ms.
#define VL6180X_SYSRANGE_INTERMEASUREMENT_PERIOD      0x01B
//Maximum time to run measurement in Ranging modes.Range 1 - 63 ms
#define VL6180X_SYSRANGE_MAX_CONVERGENCE_TIME         0x01C
#define VL6180X_SYSRANGE_EARLY_CONVERGENCE_ESTIMATE   0x022
#define VL6180X_SYSRANGE_MAX_AMBIENT_LEVEL_MULT       0x02C
#define VL6180X_SYSRANGE_RANGE_CHECK_ENABLES          0x02D
#define VL6180X_SYSRANGE_VHV_RECALIBRATE              0x02E
#define VL6180X_SYSRANGE_VHV_REPEAT_RATE              0x031
#define VL6180X_SYSALS_START                          0x038
//High Threshold value for ALS comparison. Range 0-65535 codes.
#define VL6180X_SYSALS_THRESH_HIGH                    0x03A
// Low Threshold value for ALS comparison. Range 0-65535 codes.
#define VL6180X_SYSALS_THRESH_LOW                     0x03C
//Time delay between measurements in ALS continuous mode. Range 0-254 (0 = 10ms). Step size = 10ms.
#define VL6180X_SYSALS_INTERMEASUREMENT_PERIOD        0x03E
#define VL6180X_SYSALS_ANALOGUE_GAIN                  0x03F
// Integration period for ALS mode. 1 code = 1 ms (0 = 1 ms). Recommended setting is 100 ms (0x63).
#define VL6180X_SYSALS_INTEGRATION_PERIOD             0x040
#define VL6180X_RESULT_RANGE_STATUS                   0x04D
#define VL6180X_RESULT_ALS_STATUS                     0x04E
#define VL6180X_RESULT_INTERRUPT_STATUS_GPIO          0x04F
#define VL6180X_RESULT_ALS_VAL                        0x050
#define VL6180X_RESULT_RANGE_VAL                      0x062
#define VL6180X_READOUT_AVERAGING_SAMPLE_PERIOD       0x10A
#define VL6180X_FIRMWARE_RESULT_SCALER                0x120
#define VL6180X_I2C_SLAVE_DEVICE_ADDRESS              0x212
#define VL6180X_INTERLEAVED_MODE_ENABLE               0x2A3
#define VL6180X_ID                                    0xB4
#define VL6180X_ALS_GAIN_20                           0x00
#define VL6180X_ALS_GAIN_10                           0x01
#define VL6180X_ALS_GAIN_5                            0x02
#define VL6180X_ALS_GAIN_2_5                          0x03
#define VL6180X_ALS_GAIN_1_67                         0x04
#define VL6180X_ALS_GAIN_1_25                         0x05
#define VL6180X_ALS_GAIN_1                            0x06
#define VL6180X_ALS_GAIN_40                           0x07
#define VL6180X_NO_ERR                                0x00
#define VL6180X_EARLY_CONV_ERR                        0x06
#define VL6180X_MAX_CONV_ERR                          0x07
#define VL6180X_IGNORE_ERR                            0x08
#define VL6180X_MAX_S_N_ERR                           0x0B
#define VL6180X_RAW_Range_UNDERFLOW_ERR               0x0C
#define VL6180X_RAW_Range_OVERFLOW_ERR                0x0D
#define VL6180X_Range_UNDERFLOW_ERR                   0x0E
#define VL6180X_Range_OVERFLOW_ERR                    0x0F
#define VL6180X_DIS_INTERRUPT        0
#define VL6180X_HIGH_INTERRUPT       1
#define VL6180X_LOW_INTERRUPT        2
#define VL6180X_INT_DISABLE          0
#define VL6180X_LEVEL_LOW            1
#define VL6180X_LEVEL_HIGH           2
#define VL6180X_OUT_OF_WINDOW        3
#define VL6180X_NEW_SAMPLE_READY     4


int l = 3;// 读字节长度 in100 需 <= 5

void write8bit(uint16_t regAddr,uint8_t value)
{
  Wire.beginTransmission(MODULE_I2C_ADDRESS);
  Wire.write(regAddr>>8);
  Wire.write(regAddr);
  Wire.write(value);
  Wire.endTransmission();
}

uint16_t read(uint16_t regAddr,uint8_t readNum)
{
  uint16_t value=0;
  uint8_t  a ,b;
  Wire.beginTransmission(MODULE_I2C_ADDRESS);
  Wire.write(regAddr>>8);
  Wire.write(regAddr&0xFF);
  Wire.endTransmission();
  Wire.requestFrom(MODULE_I2C_ADDRESS, readNum);
  if(readNum==1){
    value = Wire.read();
  }else if(readNum == 2){
    b = Wire.read();
    a = Wire.read();
    value = (b<<8)|a;
  }
  return value;
}

void setup()
{
  Serial.begin(115200);
  Wire.begin();

  delay(10);   // 等待模块上电启动完成
  // 这里为简化配置, 不过多解释寄存器详细配置位, 感兴趣可自行参照芯片手册
  // if(read(VL6180X_SYSTEM_FRESH_OUT_OF_RESET,1)){
    write8bit(0x0207, 0x01);
    write8bit(0x0208, 0x01);
    write8bit(0x0096, 0x00);
    write8bit(0x0097, 0xfd);
    write8bit(0x00e3, 0x00);
    write8bit(0x00e4, 0x04);
    write8bit(0x00e5, 0x02);
    write8bit(0x00e6, 0x01);
    write8bit(0x00e7, 0x03);
    write8bit(0x00f5, 0x02);
    write8bit(0x00d9, 0x05);
    write8bit(0x00db, 0xce);
    write8bit(0x00dc, 0x03);
    write8bit(0x00dd, 0xf8);
    write8bit(0x009f, 0x00);
    write8bit(0x00a3, 0x3c);
    write8bit(0x00b7, 0x00);
    write8bit(0x00bb, 0x3c);
    write8bit(0x00b2, 0x09);
    write8bit(0x00ca, 0x09);
    write8bit(0x0198, 0x01);      ////
    write8bit(0x01b0, 0x17);
    write8bit(0x01ad, 0x00);
    write8bit(0x00ff, 0x05);
    write8bit(0x0100, 0x05);
    write8bit(0x0199, 0x05);
    write8bit(0x01a6, 0x1b);
    write8bit(0x01ac, 0x3e);
    write8bit(0x01a7, 0x1f);
    write8bit(0x0030, 0x00);
  // }
  write8bit(VL6180X_READOUT_AVERAGING_SAMPLE_PERIOD, 0x30);   // 默认 0x30
  write8bit(VL6180X_SYSALS_ANALOGUE_GAIN, 0x46);   // 0x06
  write8bit(VL6180X_SYSRANGE_VHV_REPEAT_RATE, 0xFF);   // 0x00
  write8bit(VL6180X_SYSALS_INTEGRATION_PERIOD, 0x63);   // 0x00
  write8bit(VL6180X_SYSRANGE_VHV_RECALIBRATE, 0x01);   // 0x00
  write8bit(VL6180X_SYSRANGE_INTERMEASUREMENT_PERIOD, 0x09);   // 0xFF
  write8bit(VL6180X_SYSALS_INTERMEASUREMENT_PERIOD, 0x31);   // 0xFF
  write8bit(VL6180X_SYSTEM_INTERRUPT_CONFIG_GPIO, 0x00);   // 默认 0x00
  write8bit(VL6180X_SYSRANGE_MAX_CONVERGENCE_TIME, 0x31);   // 默认 0x31
  write8bit(VL6180X_INTERLEAVED_MODE_ENABLE, 0);   // 0x00
  write8bit(VL6180X_SYSTEM_MODE_GPIO1,0x20);   // 默认 0x20
  write8bit(VL6180X_SYSTEM_FRESH_OUT_OF_RESET,0);   // 0x01
  Serial.println("Success to initialize the sensor");
}

void loop()
{
  /*Poll measurement of distance*/
  write8bit(VL6180X_SYSRANGE_START, 0x01);
  uint8_t range = read(VL6180X_RESULT_RANGE_VAL,1);
  /*Get the judgment of the range value*/
  uint8_t status = read(VL6180X_RESULT_RANGE_STATUS,1)>>4;
  String str1 = "Range: "+String(range) + " mm"; 
  delay(60);   // 必不可少的延时
  switch(status){
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
  write8bit(VL6180X_SYSALS_START, 0x01);
  uint16_t value = read(VL6180X_RESULT_ALS_VAL,2);
  float lux = 0.32 * value;
  String str ="ALS: "+String(lux)+" lux";
  Serial.println(str);
  delay(200);   // 这里的延时必须大，不然无法测量光强
}
