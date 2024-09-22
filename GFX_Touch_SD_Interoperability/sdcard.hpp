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

/*--------------------------------------------------------------------------------
 * The configuration of the features defined in this file
 * Note: Only LovyanGFX can capture the screen using `readPixel()` or `readRect()`.
 *--------------------------------------------------------------------------------*/
#define SCREEN_CAPTURE  true
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

void removeDir(FS_TYPE &fs, const char *path) {
  Serial.printf("Removing dir: %s\n", path);
  if (fs.rmdir(path)) {
    Serial.println("Dir removed");
  } else {
    Serial.println("rmdir failed");
  }
}

void readFile(FS_TYPE &fs, const char *path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(FS_TYPE &fs, const char *path, const char *message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(FS_TYPE &fs, const char *path, const char *message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void renameFile(FS_TYPE &fs, const char *path1, const char *path2) {
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename failed");
  }
}

void deleteFile(FS_TYPE &fs, const char *path) {
  Serial.printf("Deleting file: %s\n", path);
  if (fs.remove(path)) {
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}

void testFileIO(FS_TYPE &fs, const char *path) {
  File file = fs.open(path);
  static uint8_t buf[512];
  size_t len = 0;
  uint32_t start = millis();
  uint32_t end = start;
  if (file) {
    len = file.size();
    size_t flen = len;
    start = millis();
    while (len) {
      size_t toRead = len;
      if (toRead > 512) {
        toRead = 512;
      }
      file.read(buf, toRead);
      len -= toRead;
    }
    end = millis() - start;
    Serial.printf("%u bytes read for %lu ms\n", flen, end);
    file.close();
  } else {
    Serial.println("Failed to open file for reading");
  }

  file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }

  size_t i;
  start = millis();
  for (i = 0; i < 2048; i++) {
    file.write(buf, 512);
  }
  end = millis() - start;
  Serial.printf("%u bytes written for %lu ms\n", 2048 * 512, end);
  file.close();
}

bool BasicTest(void) {
#if USE_SDFAT

  Serial.print("SD card type: ");
  switch (SD.card()->type()) {
    case SD_CARD_TYPE_SD1:  Serial.println("SD1");       break;
    case SD_CARD_TYPE_SD2:  Serial.println("SD2");       break;
    case SD_CARD_TYPE_SDHC: Serial.println("SDHC/SDXC"); break;
    default:                Serial.println("unknown");   break;
  }

  Serial.print("FS type: ");
  switch (SD.vol()->fatType()) {
    case FAT_TYPE_EXFAT: Serial.println("exFat"); break;
    case FAT_TYPE_FAT32: Serial.println("FAT32"); break;
    case FAT_TYPE_FAT16: Serial.println("FAT16"); break;
    case FAT_TYPE_FAT12: Serial.println("FAT12"); break;
  }

  uint32_t cardSize = SD.card()->sectorCount() * 0.000512 + 0.5;
  Serial.printf("SD card Size: %dMB\n", cardSize);

#else

  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return false;
  }

  Serial.print("SD card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD card size: %lluMB\n", cardSize);

#endif // USE_SDFAT

  listDir(SD, "/", 0);
  createDir(SD, "/mydir");
  listDir(SD, "/", 0);
  removeDir(SD, "/mydir");
  listDir(SD, "/", 2);
  writeFile(SD, "/hello.txt", "Hello ");
  appendFile(SD, "/hello.txt", "World!\n");
  readFile(SD, "/hello.txt");
  deleteFile(SD, "/foo.txt");
  renameFile(SD, "/hello.txt", "/foo.txt");
  readFile(SD, "/foo.txt");
  testFileIO(SD, "/test.txt");

#if USE_SDFAT
  Serial.printf("Free space: %dMB\n", (SD.vol()->bytesPerCluster() * SD.vol()->freeClusterCount()) / (1024 * 1024));
  SD.ls(LS_R | LS_DATE | LS_SIZE);
#else
  Serial.printf("Number of sectors: %d\n", SD.numSectors());
  Serial.printf("Size of sector: %d\n", SD.sectorSize());
  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
#endif
  return true;
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

#ifdef _ADAFRUIT_GFX_H
/* create snapshot of 3.5" TFT and save to file in bitmap format
 * https://forum.arduino.cc/t/create-snapshot-of-3-5-tft-and-save-to-file-in-bitmap-format/391367/7
*/
uint16_t readPixA(int x, int y) { // get pixel color code in rgb565 format

    GFX_EXEC(startWrite());    // needed for low-level methods. CS active
    GFX_EXEC(setAddrWindow(x, y, 1, 1));
    GFX_EXEC(writeCommand(0x2E)); // memory read command. sets DC (ILI9341: LCD_RAMRD, ST7789: ST7789_RAMRD)

    uint8_t r, g, b;
    r = GFX_EXEC(spiRead()); // discard dummy read
    r = GFX_EXEC(spiRead());
    g = GFX_EXEC(spiRead());
    b = GFX_EXEC(spiRead());
    GFX_EXEC(endWrite());    // needed for low-level methods. CS idle

    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3);
}
#endif // _ADAFRUIT_GFX_H

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
    if (y % 4 == 0) Serial.print(".");
    for (int x = 0; x < w; x++) {

      // if you are attempting to convert this library to use another display library,
      // this is where you may run into issues
      // the libries must have a readPixel function
#ifdef _ADAFRUIT_GFX_H
      rgb = readPixA(x, y);
#else
      rgb = GFX_EXEC(readPixel(x, y));
#endif
      // convert the 16 bit color to full 24
      // that way we have a legit bmp that can be read into the
      // bmp reader below
      color565toRGB(rgb, r, g, b);

      // write the data in BMP reverse order
      file.write(b);
      file.write(g);
      file.write(r);
    }
  }

  file.close();
  Serial.println("Image was saved successfully");
  return true;
}

/*--------------------------------------------------------------------------------
 * External interface entry function
 *--------------------------------------------------------------------------------*/
void sdcard_setup(void) {
#ifdef REASSIGN_PINS
  // Initialize SD card interface
  SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, SD_CS);
#endif

#if   defined(SPI2_HOST) || defined(SPI3_HOST)
  Serial.println("SPI_PORT = " + String(SPI_PORT)); // LovyanGFX SPI2_HOST = 1
#elif defined(SPI_HOST)
  Serial.println("SPI_HOST = " + String(SPI_HOST)); // TFT_eSPI SPI_HOST = 3
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

#if SCREEN_CAPTURE
  listDir(SD, "/", 0);
  SaveBMP24(SD, "/test.bmp");
  listDir(SD, "/", 0);
#else
  BasicTest();
#endif

//SD.end(); --> Activating this line will cause some GFX libraries to stop working.
  Serial.println("done.");
  return true;
}