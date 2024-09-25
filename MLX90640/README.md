# MLX90640 Thermography Camera by XIAO ESP32S3

## Block diagram

![Block diagram](BlockDiagram.jpg)

## Wiring diagram

[Wiring at cirkitdesigner.com](https://app.cirkitdesigner.com/project/837fd6ec-a7d8-4381-a41f-4b953adefee0 "Cirkit Designer IDE")

![MLX90640 on bread board](MLX90640-XIAO-ESP32.jpg)

## Testbed environment

- [Arduino IDE 2.3.2][1]
- [Seeed Studio XIAO ESP32S3][2]
  - ESP32 board package: [espressif/arduino-esp32][3] ([3.0.4][4])
  - PSRAM: Disable
- LCD Display: ST7789 and XPT2046
  - Interface: SPI
  - SPI frequency: 80MHz
  - SPI bus host: SPI2_HOST ([sdspi_host.h][5])
- [MLX90640][6]
  - I2C Frequency: 1MHz (Fm+)
  - [aliexpress][7]

## Related resources

- [**Far infrared thermal sensor array (32x24 RES)**][8]  
  Melexi.com MLX90640 product page.

- [**GY-MCU90640 user manual**][9]  
  Official or private user manual of MLX90640 camera module.

- [**adafruit/Adafruit_MLX90640**][10]  
  MLX90640 library functions by Adafruit.

- [**sparkfun/SparkFun_MLX90640_Arduino_Example**][11]  
  MLX90640 examples by SparkFun.

- [**XPT2046_Touchscreen**][12]  
  Touchscreens library using the XPT2046 controller chip.

- [**espressif/arduino-esp32/libraries/SD**][13]  
  ESP32 and SD (Secure Digital) and MMC (Multi Media Card) cards library without additional modules.

[1]: https://www.arduino.cc/en/software "Software - Arduino"

[2]: https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/ "Getting Started with Seeed Studio XIAO ESP32S3 (Sense) - Seeed Studio Wiki"

[3]: https://github.com/espressif/arduino-esp32 "espressif/arduino-esp32: Arduino core for the ESP32"
[4]: https://github.com/espressif/arduino-esp32/releases/tag/3.0.4 "Release Arduino Release v3.0.4 based on ESP-IDF v5.1.4+ · espressif/arduino-esp32"

[5]: https://github.com/espressif/esp-idf/blob/master/components/esp_driver_sdspi/include/driver/sdspi_host.h#L23-L29 "esp-idf/components/esp_driver_sdspi/include/driver/sdspi_host.h at master · espressif/esp-idf"

[6]: https://www.melexis.com/en/documents/documentation/datasheets/datasheet-mlx90640 "Datasheet for MLX90640 I Melexis"

[7]: https://www.aliexpress.com/item/1005006674751991.html

[8]: https://www.melexis.com/en/product/MLX90640/Far-Infrared-Thermal-Sensor-Array "Far Infrared Thermal Sensor Array (32x24 RES) I Melexis"

[9]: https://github.com/vvkuryshev/GY-MCU90640-RPI-Python/blob/master/GY_MCU9064%20user%20manual%20v1.pdf "vvkuryshev/GY-MCU90640-RPI-Python: The script to connect the thermal image module GY-MCU90640 to Raspberry Pi."

[10]: https://github.com/adafruit/Adafruit_MLX90640 "adafruit/Adafruit_MLX90640: MLX90640 library functions"

[11]: https://github.com/sparkfun/SparkFun_MLX90640_Arduino_Example "sparkfun/SparkFun_MLX90640_Arduino_Example: Controlling and reading from the MLX90640 IR array thermal imaging sensor"

[12]: https://www.arduino.cc/reference/en/libraries/xpt2046_touchscreen/ "XPT2046_Touchscreen - Arduino Reference"

[13]: https://github.com/espressif/arduino-esp32/tree/master/libraries/SD "arduino-esp32/libraries/SD at master · espressif/arduino-esp32"
