# Revisiting Graphics Libraries Benchmark

The original benchmark is in [moononournation/Arduino_GFX][12] on 2021 Jun 16

| Benchmark          | Adafruit_GFX | Arduino_GFX |  Lovyan_GFX |  TFT_eSPI |
| ------------------ | ------------:| -----------:| -----------:| ---------:|
| Screen fill        |      115,519 |     100,071 |  **81,683** |    83,406 |
| Text               |      109,677 |      15,463 |  **18,627** |    23.982 |
| Pixels             |    1,790,682 |     904,524 | **390,413** | 1,089,390 |
| Lines              |    1,233,294 |     435,673 | **255,570** |   310,244 |
| Horiz/Vert Lines   |       11,798 |       9,245 |   **7,159** |     8,479 |
| Rectangles-filled  |      240,282 |     208,162 | **169,746** |   173,568 |
| Rectangles         |        8,106 |       6,203 |   **4,738** |     5,450 |
| Triangles-filled   |      106,388 |      80,118 |  **61,673** |    68,413 |
| Triangles          |       67,645 |      24,438 |  **14,900** |    18,332 |
| Circles-filled     |       70,073 |      40,776 |  **28,548** |    38,811 |
| Circles            |      138,015 |      42,721 |  **23,124** |    29,515 |
| Arcs-filled        |          N/A |      31,769 |  **20,193** |       N/A |
| Arcs               |          N/A |      72,927 |  **54,902** |       N/A |
| Rounded rects-fill |      248,323 |     214,102 | **170,552** |   177,262 |
| Rounded rects      |       46,158 |      19,740 |   **9,872** |    15,184 |

## Figures

- [Arduino IDE 2.3.2][1]
- [Seeed Studio XIAO ESP32S3][2]
  - ESP32 board package: [espressif/arduino-esp32][3] ([3.0.4][4])
  - PSRAM: Disable
- Display: ST7789
  - Interface: SPI
  - SPI Frequency: 80MHz
- Tested on: September 12, 2024

## Libraries version

- [adafruit/Adafruit-GFX-Library][10] ([1.11.10][11])
- [moononournation/Arduino_GFX][12] ([1.4.9][13])
- [lovyan03/LovyanGFX][14] ([1.1.16][15])
- [Bodmer/TFT_eSPI][16] ([V2.5.43][17])

## Error by TFT_eSPI with ESP32C3／ESP32S3

### Issue

The following exception occurs in `tft.init()`.

```
TFT_eSPI library test!
Guru Meditation Error: Core  1 panic'ed (StoreProhibited). Exception was unhandled.

Core  1 register dump:
PC      : 0x420041cc  PS      : 0x00060430  A0      : 0x82004963  A1      : 0x3fcebd60  
A2      : 0x3fc94b1c  A3      : 0x00000000  A4      : 0x00000008  A5      : 0x00000009  
A6      : 0x000000ff  A7      : 0x00000001  A8      : 0x00000010  A9      : 0x08000000  
A10     : 0x3fc94c40  A11     : 0x019bfcc0  A12     : 0x00000301  A13     : 0x00000000  
A14     : 0x00000031  A15     : 0x3fc93b9c  SAR     : 0x00000002  EXCCAUSE: 0x0000001d  
EXCVADDR: 0x00000010  LBEG    : 0x40056f08  LEND    : 0x40056f12  LCOUNT  : 0x00000000  

Backtrace: 0x420041c9:0x3fcebd60 0x42004960:0x3fcebd90 0x42002335:0x3fcebdc0 0x4200b24d:0x3fcebde0 0x4037d136:0x3fcebe00

ELF file SHA256: 42fe0e0edde69bd7

Rebooting...
ESP-ROM:esp32s3-20210327
Build:Mar 27 2021
rst:0xc (RTC_SW_CPU_RST),boot:0x8 (SPI_FAST_FLASH_BOOT)
Saved PC:0x40378c3a
SPIWP:0xee
mode:DIO, clock div:1
load:0x3fce3818,len:0x109c
load:0x403c9700,len:0x4
load:0x403c9704,len:0xb50
load:0x403cc700,len:0x2fd0
entry 0x403c98ac
```

### Workaround

Enable the following in `User_Setup.h`.

```c++
// The ESP32 has 2 free SPI ports i.e. VSPI and HSPI, the VSPI is the default.
// If the VSPI port is in use and pins are not accessible (e.g. TTGO T-Beam)
// then uncomment the following line:
#define USE_HSPI_PORT
```

### Related GitHub issues

- [Guru Meditation Error on tft.begin() (ESP32-S3 dev module) #3289][18]
- [library not working with espressif32 esp32-s3 Arduino core > 2.0.14 #3329][19]
- [ESP32 Core 1 panic'ed (StoreProhibited). Exception was unhandled when tft.init() is executed. #3332][20]

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
[18]: https://github.com/Bodmer/TFT_eSPI/issues/3289 "Guru Meditation Error on tft.begin() (ESP32-S3 dev module) · Issue #3289 · Bodmer/TFT_eSPI"
[19]: https://github.com/Bodmer/TFT_eSPI/issues/3329 "library not working with espressif32 esp32-s3 Arduino core &gt; 2.0.14 · Issue #3329 · Bodmer/TFT_eSPI"
[20]: https://github.com/Bodmer/TFT_eSPI/issues/3332 "ESP32 Core  1 panic&#39;ed (StoreProhibited). Exception was unhandled when tft.init() is executed. · Issue #3332 · Bodmer/TFT_eSPI"
