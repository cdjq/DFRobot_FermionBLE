DFRobot_GR10_30
===========================
## 1. 测试 Sensor_GR10_30.ino 
将 GR10_30 传感器参考如下连线图连接至 Arduino，并且上传当前文件夹中的
Sensor_GR10_30.ino 至 Arduino 
当串口监视器中正常读数，说明传感器正常。传感器可以被连接至 Beacon 测试。 
## 2. 烧录 Beacon 并且连接传感器
请您使用 USB-TTL 转换器将.cfg 文件烧录进 Beacon。
NanoBeacon Config Tool 中可以 Load 本文件夹中的 GR10_30.cfg 文件。 
检查 XO 电容配置是否为 12 
烧录流程请参考 Beacon 的 wiki： 
https://wiki.dfrobot.com.cn/_SKU_TEL0168_Fermion_BLE_%E4%BC%A0%E6%84
%9F%E5%99%A8%E4%BF%A1%E6%A0%87#target_4 
在烧录完成后，参考下图连接 Beacon 和传感器。 
注：我们的.cfg 示例文件默认 SCL->GPIO7, SDA->GPIO3。 
## 3. 上传 ESP32 代码并获取读数 
将同目录下的 Beacon_GR10_30.ino 上传至 ESP32 主板。 
并且将 Beacon 和传感器供电，供电方式可选 CR2032 纽扣电池，或者 VCC 和 GND
输入 3.3V。 
您将会看到串口监视器中打印相关数据。 