DFRobot_GR10_30
===========================

* [中文版](./README_CN.md)

The GR10-30 gesture sensor is an integrated gesture recognition sensor based on image analysis. It is capable of recognizing 10 hand gestures: move up, down, left, right, forward & backward, rotate clockwise & counterclockwise (continuously), hover, and wave. 

![product image](./resources/images/SEN0543_forward.jpg) <img src="./resources/images/SEN0543_back.jpg" alt="product image" style="zoom:80%;" />

## Product Link (https://www.dfrobot.com/product-2666.html)
    SKU: SEN0543

## Table of Contents

  * [Summary](#summary)
  * [Installation](#installation)
  * [Methods](#methods)
  * [Compatibility](#compatibility)
  * [History](#history)
  * [Credits](#credits)

## Summary

* Maximum Recognition distance of 30cm<br/>
* Capable of recognizing 12 gestures<br/>
* Configurable recognition threshold & other parameters<br/>
* Support UART & I2C communication


## Installation

Download the library file (https://github.com/DFRobot/DFRobot_GR10_30) and its dependencies (https://github.com/DFRobot/DFRobot_RTU) before use, paste them into the \Arduino\libraries directory, then open the sample folder and run the demo in the folder.

## Methods

```C++
  /**
   * @fn begin
   * @brief Init SEN0543 device
   * @return Return value init status
   * @retval 0  Succeed
   * @retval -1 Failed
   */
  int8_t begin(void);

  /**
   * @fn enGestures
   * @brief Set what gestures the module can recognize to trigger interrupt
   * @param gestures
   *  GESTURE_UP
   *  GESTURE_DOWN
   *  GESTURE_LEFT
   *  GESTURE_RIGHT
   *  GESTURE_FORWARD
   *  GESTURE_BACKWARD
   *  GESTURE_CLOCKWISE
   *  GESTURE_COUNTERCLOCKWISE
   *  GESTURE_WAVE              It is not suggested to enable rotation gesture (CW/CCW) and wave gesture at the same time.
   *  GESTURE_HOVER             Disable other gestures when hover gesture enables.
   *  GESTURE_UNKNOWN
   *  GESTURE_CLOCKWISE_C           // Rotate clockwise continuously
   *  GESTURE_COUNTERCLOCKWISE_C    // Rotate counterclockwise continuously
   * @return NONE
   */
  void enGestures(uint16_t gestures);

  /**
   * @fn setUdlrWin
   * @brief Set the detection window 
   * @param udSize Distance from top to bottom      distance range 1-30
   * @param lrSize Distance from left to right      distance range 1-30
   * @return NONE
   */
  void setUdlrWin(uint8_t udSize, uint8_t lrSize);

  /**
   * @fn setLeftRange
   * @brief Set how far your hand should move to the left so the sensor can recognize it
   * @param range
   * @n     Distance range 5-25, must be less than distance from left to right of the detection window
   * @return NONE
   */
  void setLeftRange(uint8_t range);

  /**
   * @fn setRightRange
   * @brief Set how far your hand should move to the right so the sensor can recognize it
   * @param range
   * @n     Distance range 5-25, must be less than distance from left to right of the detection window
   * @return NONE
   */
  void setRightRange(uint8_t range);

  /**
   * @fn setUpRange
   * @brief Set how far your hand should move up so the sensor can recognize it
   * @param range
   * @n     Distance range 5-25, must be less than distance from top to bottom of the detection window
   * @return NONE
   */
  void setUpRange(uint8_t range);

  /**
   * @fn setDownRange
   * @brief Set how far your hand should move down so the sensor can recognize it
   * @param range
   * @n     Distance range 5-25, must be less than distance from top to bottom of the detection window
   * @return NONE
   */
  void setDownRange(uint8_t range);

  /**
   * @fn setForwardRange
   * @brief Set how far your hand should move forward so the sensor can recognize it
   * @param range
   * @n     Distance range 1-15
   * @return NONE
   */
  void setForwardRange(uint8_t range);

  /**
   * @fn setBackwardRange
   * @brief Set how far your hand should move backward so the sensor can recognize it 
   * @param range
   * @n     Distance range 1-15
   * @return NONE
   */
  void setBackwardRange(uint8_t range);

  /**
   * @fn setWaveNumber
   * @brief Set how many times you need to wave hands so the sensor can recognize it
   * @param number
   * @n     Number range 1-15
   * @return NONE
   */
  void setWaveNumber(uint8_t number);

  /**
   * @fn setHovrWin
   * @brief Set hover detection window
   * @param udSize Distance from top to bottom      distance range 1-30
   * @param lrSize Distance from left to right      distance range 1-30
   * @return NONE
   */
  void setHovrWin(uint8_t udSize, uint8_t lrSize);

  /**
   * @fn setHovrTimer
   * @brief Set how long your hand should hover to trigger the gesture
   * @param timer
   * @n     timer 1-200  10ms-2s  default is 60 600ms
   * @return NONE
   */
  void setHovrTimer(uint16_t timer);

  /**
   * @fn setCwsAngle
   * @brief Set how many degrees your hand should rotate clockwise to trigger the gesture
   * @param count Default 16,  range 1-31
   * @n     count Rotation angle = 22.5 * count
   * @n     For example: count = 16, 22.5*count = 360, rotate 360° to trigger the gesture
   * @return NONE
   */
  void setCwsAngle(uint8_t count);

  /**
   * @fn setCcwAngle
   * @brief Set how many degrees your hand should rotate counterclockwise to trigger the gesture
   * @param count Default is 16 range 1-31
   * @n     count Rotation angle is 22.5 * count
   * @n     For example: count = 16 22.5*count = 360  rotate 360° to trigger the gesture
   * @return NONE
   */
  void setCcwAngle(uint8_t count);

  /**
   * @fn setCwsAngleCount
   * @brief Set how many degrees your hand should rotate clockwise continuously to trigger the gesture
   * @param count Default 4, range 1-31
   * @n     count Continuous rotation angle = 22.5 * count
   * @n     For example: count = 4, 22.5*count = 90
   * @n     Trigger the clockwise/counterclockwise rotation gesture first, 
   * @n     if keep rotating, then the continuous rotation gesture will be triggered once every 90° 
   * @return NONE
   */
  void setCwsAngleCount(uint8_t count);

  /**
   * @fn setCcwAngleCount
   * @brief Set how many degrees your hand should rotate counterclockwise continuously to trigger the gesture
   * @param count Default 4,  range 1-31
   * @n     count Continuous rotation angle = 22.5 * count
   * @n     For example: count = 4 22.5*count = 90
   * @n     Trigger the clockwise/counterclockwise rotation gesture first, 
   * @n     if keep rotating, then the continuous rotation gesture will be triggered once every 90° 
   * @return NONE
   */
  void setCcwAngleCount(uint8_t count);

  /**
   * @fn getExist
   * @brief Get whether the object is in the sensor detection range
   * @return If the object is in the sensor detection range
   * @retval 1  Yes
   * @retval 0  No
   */
  uint16_t getExist(void);

  /**
   * @fn getDataReady
   * @brief Get if a gesture is detected
   * @return If a gesture is detected
   * @retval 1  Detected
   * @retval 0  Not detected
   */
  uint16_t getDataReady(void);

  /**
   * @fn getGesturesState
   * @brief Get gesture type
   * @return Gesture type
   * @retval GESTURE_UP
   * @retval GESTURE_DOWN
   * @retval GESTURE_DOWN
   * @retval GESTURE_LEFT
   * @retval GESTURE_RIGHT
   * @retval GESTURE_FORWARD
   * @retval GESTURE_BACKWARD
   * @retval GESTURE_CLOCKWISE
   * @retval GESTURE_COUNTERCLOCKWISE
   * @retval GESTURE_WAVE
   * @retval GESTURE_HOVER
   * @retval GESTURE_UNKNOWN
   * @retval GESTURE_CLOCKWISE_C
   * @retval GESTURE_COUNTERCLOCKWISE_C
   */
  uint16_t getGesturesState(void);
```

## Compatibility

MCU                | SoftwareSerial | HardwareSerial |      IIC      |
------------------ | :----------: | :----------: | :----------: | 
Arduino Uno        |      √       |      X       |      √       |
Mega2560           |      √       |      √       |      √       |
Leonardo           |      √       |      √       |      √       |
ESP32              |      X       |      √       |      √       |
ESP8266            |      √       |      X       |      √       |
micro:bit          |      X       |      X       |      √       |
FireBeetle M0      |      X       |      √       |      √       |
Raspberry Pi       |      X       |      √       |      √       |

## History

- 2022-07-26 - Version 1.0.0 released.

## Credits

Written by zhixinliu(zhixinliu@dfrobot.com), 2021. (Welcome to our [website](https://www.dfrobot.com/))
