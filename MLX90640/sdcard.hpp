/*
 * pin 1 - not used          |  Micro SD card     |
 * pin 2 - CS (SS)           |                   /
 * pin 3 - DI (MOSI)         |                  |__
 * pin 4 - VDD (3.3V)        |                    |
 * pin 5 - SCK (SCLK)        | 8 7 6 5 4 3 2 1   /
 * pin 6 - VSS (GND)         | ▄ ▄ ▄ ▄ ▄ ▄ ▄ ▄  /
 * pin 7 - DO (MISO)         | ▀ ▀ █ ▀ █ ▀ ▀ ▀ |
 * pin 8 - not used          |_________________|
 *                             ║ ║ ║ ║ ║ ║ ║ ║
 *                     ╔═══════╝ ║ ║ ║ ║ ║ ║ ╚═════════╗
 *                     ║         ║ ║ ║ ║ ║ ╚══════╗    ║
 *                     ║   ╔═════╝ ║ ║ ║ ╚═════╗  ║    ║
 * Connections for     ║   ║   ╔═══╩═║═║═══╗   ║  ║    ║
 * full-sized          ║   ║   ║   ╔═╝ ║   ║   ║  ║    ║
 * SD card             ║   ║   ║   ║   ║   ║   ║  ║    ║
 * Pin name         |  -  DO  VSS SCK VDD VSS DI CS    -  |
 * SD pin number    |  8   7   6   5   4   3   2   1   9 /
 *                  |                                  █/
 *                  |__▍___▊___█___█___█___█___█___█___/
 *
 * Note:  The SPI pins can be manually configured by using `SPI.begin(sck, miso, mosi, cs).`
 *        Alternatively, you can change the CS pin and use the other default settings by using `SD.begin(cs)`.
 *
 * +--------------+---------+-------+----------+----------+----------+----------+----------+
 * | SPI Pin Name | ESP8266 | ESP32 | ESP32‑S2 | ESP32‑S3 | ESP32‑C3 | ESP32‑C6 | ESP32‑H2 |
 * +==============+=========+=======+==========+==========+==========+==========+==========+
 * | CS (SS)      | GPIO15  | GPIO5 | GPIO34   | GPIO10   | GPIO7    | GPIO18   | GPIO0    |
 * +--------------+---------+-------+----------+----------+----------+----------+----------+
 * | DI (MOSI)    | GPIO13  | GPIO23| GPIO35   | GPIO11   | GPIO6    | GPIO19   | GPIO25   |
 * +--------------+---------+-------+----------+----------+----------+----------+----------+
 * | DO (MISO)    | GPIO12  | GPIO19| GPIO37   | GPIO13   | GPIO5    | GPIO20   | GPIO11   |
 * +--------------+---------+-------+----------+----------+----------+----------+----------+
 * | SCK (SCLK)   | GPIO14  | GPIO18| GPIO36   | GPIO12   | GPIO4    | GPIO21   | GPIO10   |
 * +--------------+---------+-------+----------+----------+----------+----------+----------+
 *
 * For more info see file README.md in this library or on URL:
 * https://github.com/espressif/arduino-esp32/tree/master/libraries/SD
 */
#include <Arduino.h>
#include "spi_assign.h"

/*================================================================================
 * The configuration of the features defined in this file
 * Note: Only LovyanGFX can capture the screen with `readPixel()` or `readRect()`
 *================================================================================*/
#define CAPTURE_SCREEN  true
#define USE_SDFAT       false

/*--------------------------------------------------------------------------------
 * SD library
 *--------------------------------------------------------------------------------*/
#if USE_SDFAT

#define DISABLE_FS_H_WARNING
#include "SdFat.h"

#undef  FILE_APPEND
#define FILE_APPEND (O_RDWR | O_CREAT | O_AT_END)
#undef  FILE_WRITE
#define FILE_WRITE  (O_RDWR | O_CREAT | O_TRUNC)

// SHARED_SPI makes SD very slow, while DEDICATED_SPI causes GFX libraries to stop working.
#define SD_CONFIG SdSpiConfig(SD_CS, SHARED_SPI /* DEDICATED_SPI */, SD_SCK_MHZ(24))
typedef FsFile  File;
#define FS_TYPE SdFs
SdFs SD;

#else // ! USE_SDFAT

#include "FS.h"
#include "SD.h"
#include "SPI.h"
#define FS_TYPE  fs::FS
#ifdef _TFT_eSPIH_
#define SD_CONFIG SD_CS, GFX_EXEC(getSPIinstance()) //, SPI_READ_FREQUENCY
#else
#define SD_CONFIG SD_CS //, SPI, SPI_READ_FREQUENCY
#endif

#endif // USE_SDFAT

// Uncomment and set up if you want to use custom pins for the SPI communication
// #define REASSIGN_PINS

/*--------------------------------------------------------------------------------
 * Basic file I/O and directory related functions
 * ex)  listDir(SD, "/", 0);
 *      createDir(SD, "/mydir");
 *--------------------------------------------------------------------------------*/
void listDir(FS_TYPE &fs, const char *dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  char buf[16];
  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
#if USE_SDFAT
      file.getName(buf, sizeof(buf));
      Serial.println(buf);
#else
      Serial.println(file.name());
#endif
      if (levels) {
#if USE_SDFAT
        listDir(fs, buf, levels - 1);
#else
        listDir(fs, file.path(), levels - 1);
#endif
      }
    } else {
      Serial.print("  FILE: ");
#if USE_SDFAT
      file.getName(buf, sizeof(buf));
      Serial.print(buf);
#else
      Serial.print(file.name());
#endif
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void createDir(FS_TYPE &fs, const char *path) {
  Serial.printf("Creating dir: %s\n", path);
  if (fs.mkdir(path)) {
    Serial.println("Dir created");
  } else {
    Serial.println("mkdir failed");
  }
}

/*--------------------------------------------------------------------------------
 * LCD screen capture to save image to SD card
 *--------------------------------------------------------------------------------*/
// Converts 565 format 16 bit color to RGB
inline void color565toRGB(uint16_t color, uint8_t &r, uint8_t &g, uint8_t &b) __attribute__((always_inline));
inline void color565toRGB(uint16_t color, uint8_t &r, uint8_t &g, uint8_t &b) {
  r = (color>>8)&0x00F8;
  g = (color>>3)&0x00FC;
  b = (color<<3)&0x00F8;
}

#if defined(_ADAFRUIT_GFX_H)

/* create snapshot of 3.5" TFT and save to file in bitmap format
 * https://forum.arduino.cc/t/create-snapshot-of-3-5-tft-and-save-to-file-in-bitmap-format/391367/7
*/
uint16_t readPixA(int x, int y) { // get pixel color code in rgb565 format

    digitalWrite(TFT_CS, LOW);

    GFX_EXEC(startWrite());    // needed for low-level methods. CS active
    GFX_EXEC(setAddrWindow(x, y, 1, 1));
    GFX_EXEC(writeCommand(0x2E)); // memory read command. sets DC (ILI9341: LCD_RAMRD, ST7789: ST7789_RAMRD)

    uint8_t r, g, b;
    r = GFX_EXEC(spiRead()); // discard dummy read
    r = GFX_EXEC(spiRead());
    g = GFX_EXEC(spiRead());
    b = GFX_EXEC(spiRead());
    GFX_EXEC(endWrite());   // needed for low-level methods. CS idle

    digitalWrite(TFT_CS, HIGH);

    return RGB565(r, g, b); // defined in colors.h
}

#endif // _ADAFRUIT_GFX_H || _ARDUINO_GFX_LIBRARIES_H_

bool SaveBMP24(FS_TYPE &fs, const char *path) {
  uint16_t rgb;
  uint8_t r, g, b;

  File file = fs.open(path, FILE_WRITE);

  if (!file) {
    Serial.println("SD open failed");
    return false;
  }

  int h = GFX_EXEC(height());
  int w = GFX_EXEC(width());

  unsigned char bmFlHdr[14] = {
    'B', 'M', 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0
  };

  // set color depth to 24 as we're storing 8 bits for r-g-b
  unsigned char bmInHdr[40] = {
    40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0,
  };

  unsigned long fileSize = 3ul * h * w + 54;

  bmFlHdr[ 2] = (unsigned char)(fileSize);
  bmFlHdr[ 3] = (unsigned char)(fileSize >> 8);
  bmFlHdr[ 4] = (unsigned char)(fileSize >> 16);
  bmFlHdr[ 5] = (unsigned char)(fileSize >> 24);

  bmInHdr[ 4] = (unsigned char)(w);
  bmInHdr[ 5] = (unsigned char)(w >> 8);
  bmInHdr[ 6] = (unsigned char)(w >> 16);
  bmInHdr[ 7] = (unsigned char)(w >> 24);
  bmInHdr[ 8] = (unsigned char)(h);
  bmInHdr[ 9] = (unsigned char)(h >> 8);
  bmInHdr[10] = (unsigned char)(h >> 16);
  bmInHdr[11] = (unsigned char)(h >> 24);

  file.write(bmFlHdr, sizeof(bmFlHdr));
  file.write(bmInHdr, sizeof(bmInHdr));

  for (int y = h - 1; y >= 0; y--) {
    if (y % 10 == 0) {
      Serial.print(".");
      delay(1); // reset wdt
    }

    for (int x = 0; x < w; x++) {

#if   defined(_ARDUINO_GFX_LIBRARIES_H_)
      rgb = 0; // does not support reading
#elif defined(_ADAFRUIT_GFX_H)
      rgb = readPixA(x, y);
#else // LOVYANGFX_HPP_ || _TFT_eSPIH_
      rgb = GFX_EXEC(readPixel(x, y));
#endif

      color565toRGB(rgb, r, g, b);

      // write the data in BMP reverse order
      file.write(b);
      file.write(g);
      file.write(r);
    }
  }

  file.close();
  Serial.println("saved successfully");
  return true;
}

/*--------------------------------------------------------------------------------
 * API entries
 *--------------------------------------------------------------------------------*/
void sdcard_setup(void) {
#ifdef REASSIGN_PINS
  // Initialize SD card interface
  SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, SD_CS);
#endif
}

bool sdcard_save(void) {
  sdcard_setup();

  uint8_t retry = 0;
  while (!SD.begin(SD_CONFIG)) {
    if (++retry >= 2) {
      Serial.println("Card mount failed");
      return false;
    }
    delay(1000);
  }

  Serial.println("The card was mounted successfully");

#if CAPTURE_SCREEN
  listDir(SD, "/", 0);

  String path = String("/test.bmp");
  SaveBMP24(SD, path.c_str());

  listDir(SD, "/", 0);
#else
  listDir(SD, "/", 0);
#endif

//SD.end(); --> Activating this line will cause some GFX libraries to stop working.
  Serial.println("done.");
  return true;
}