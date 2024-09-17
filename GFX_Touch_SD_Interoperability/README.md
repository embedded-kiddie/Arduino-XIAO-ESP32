# Interoperability with GFX and SD Libraries

| GFX library         | Touch library       | SD (standard) | SdFat `SHARED_SPI` | SdFat `DEDICATED_SPI` |
| ------------------- | ------------------- | ------------- | ------------------ | --------------------- |
| Adafruit_GFX        | XPT2046_Touchscreen | OK            | OK [^1]            | NG                    |
| Arduino_GFX_Library | XPT2046_Touchscreen | OK            | OK [^1]            | NG                    |
| LovyanGFX           | <--                 | OK [^2]       | NG                 | NG                    |
| TFT_eSPI            | <--                 | OK [^3]       | NG                 | NG                    |

## Figures

- [Arduino IDE 2.3.2][1]
- [Seeed Studio XIAO ESP32S3][2]
  - ESP32 board package: [espressif/arduino-esp32][3] ([3.0.4][4])
  - PSRAM: Disable
- Display: ST7789
  - Interface: SPI
  - SPI Frequency: 80MHz
- Tested on: September 12, 2024

## Library versions

- [adafruit/Adafruit-GFX-Library][10] ([1.11.10][11])
- [moononournation/Arduino_GFX][12] ([1.4.9][13])
- [lovyan03/LovyanGFX][14] ([1.1.16][15])
- [Bodmer/TFT_eSPI][16] ([V2.5.43][17])
- [PaulStoffregen/XPT2046_Touchscreen][18] ([v1.4][19])
- [greiman/SdFat][20] ([2.2.3][21])

## Related discussions and issues

- [Display freezes when saving images to SD card #616][22]
- [Display freezes when saving images to SD card #617][23]

[^1]: Reading and writing to the SD card is several times slower than SD (standard).

[^2]: In ESP-IDF v5.1, `sdspi_host.h` for ESP32S3 defines `SDSPI_DEFAULT_HOST SPI2_HOST` as `SPI2_HOST`. Therefore, select `SPI2_HOST` as the host for the SPI bus.

[^3]: Specify the 2nd parameter in `SD.begin()` as `tft.getSPIinstance()`.

[1]: https://www.arduino.cc/en/software "Software｜Arduino"

[2]: https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/ "Getting Started with Seeed Studio XIAO ESP32S3 (Sense)｜Seeed Studio Wiki"

[3]: https://github.com/espressif/arduino-esp32 "espressif/arduino-esp32: Arduino core for the ESP32"
[4]: https://github.com/espressif/arduino-esp32/releases/tag/3.0.4 "Release Arduino Release v3.0.4 based on ESP-IDF v5.1.4+ · espressif/arduino-esp32"

[10]: https://github.com/adafruit/Adafruit-GFX-Library "adafruit/Adafruit-GFX-Library: Adafruit GFX graphics core Arduino library, this is the &#39;core&#39; class that all our other graphics libraries derive from"
[11]: https://github.com/adafruit/Adafruit-GFX-Library/releases/tag/1.11.10 "Release 1.11.10 Add ATtiny84 support · adafruit/Adafruit-GFX-Library"

[12]: https://github.com/moononournation/Arduino_GFX "moononournation/Arduino_GFX: Arduino GFX developing for various color displays and various data bus interfaces"
[13]: https://github.com/moononournation/Arduino_GFX/releases/tag/v1.4.9 "Release v1.4.9 · moononournation/Arduino_GFX"

[14]: https://github.com/lovyan03/LovyanGFX "lovyan03/LovyanGFX: SPI LCD graphics library for ESP32 (ESP-IDF/ArduinoESP32) / ESP8266 (ArduinoESP8266) / SAMD51(Seeed ArduinoSAMD51)"
[15]: https://github.com/lovyan03/LovyanGFX/releases/tag/1.1.16 "Release 1.1.16 · lovyan03/LovyanGFX"

[16]: https://github.com/Bodmer/TFT_eSPI "Bodmer/TFT_eSPI: Arduino and PlatformIO IDE compatible TFT library optimised for the Raspberry Pi Pico (RP2040), STM32, ESP8266 and ESP32 that supports different driver chips"
[17]: https://github.com/Bodmer/TFT_eSPI/releases/tag/V2.5.43 "Release Bug fixes · Bodmer/TFT_eSPI"

[18]: https://github.com/PaulStoffregen/XPT2046_Touchscreen "PaulStoffregen/XPT2046_Touchscreen: Touchscreen Arduino Library for XPT2046 Touch Controller Chip"
[19]: https://github.com/PaulStoffregen/XPT2046_Touchscreen/releases/tag/v1.4 "Release Version 1.4 · PaulStoffregen/XPT2046_Touchscreen"

[20]: https://github.com/greiman/SdFat "greiman/SdFat: Arduino FAT16/FAT32 exFAT Library"
[21]: https://github.com/greiman/SdFat/releases/tag/2.2.3 "Release Add Move Constructor and bug fixes. · greiman/SdFat"

[22]: https://github.com/lovyan03/LovyanGFX/discussions/616 "Display freezes when saving images to SD card · lovyan03/LovyanGFX · Discussion #616"
[23]: https://github.com/lovyan03/LovyanGFX/issues/617 "Display freezes when saving images to SD card · Issue #617 · lovyan03/LovyanGFX"
