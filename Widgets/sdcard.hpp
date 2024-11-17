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
 * Uncomment and set up if you want to use custom pins for the SPI communication
 *--------------------------------------------------------------------------------*/
// #define REASSIGN_PINS

// temporary buffer size for string manipulation
#ifndef BUF_SIZE
#define BUF_SIZE  64
#endif

/*================================================================================
 * The configuration of the features defined in this file
 * Note: Currently only LovyanGFX can capture the screen successfully.
 *================================================================================*/
#define CAPTURE_SCREEN  true
#define USE_SDFAT       true  // TFT_eSPI can not work with SdFat

// LovyanGFX requires SD library header file before including <LovyanGFX.hpp>
#if     defined (SdFat_h)
#undef  USE_SDFAT
#define USE_SDFAT       true
#elif   defined (_SD_H_)
#undef  USE_SDFAT
#define USE_SDFAT       false
#endif

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
#define FS_TYPE  fs::SDFS
#ifdef _TFT_eSPIH_
#define SD_CONFIG SD_CS, GFX_EXEC(getSPIinstance()), SPI_READ_FREQUENCY
#else
#define SD_CONFIG SD_CS, SPI, SPI_READ_FREQUENCY
#endif

#endif // USE_SDFAT

/*--------------------------------------------------------------------------------
 * File name and size for GetFileList()
 *--------------------------------------------------------------------------------*/
#include <string>
#include <vector>
#include <exception>

typedef struct {
  std::string path;
  size_t      size;
  bool        isDirectory;
  bool        isSelected;
} FileInfo_t;

/*--------------------------------------------------------------------------------
 * Functions prototyping
 *--------------------------------------------------------------------------------*/
void sdcard_setup (void);
bool sdcard_open  (void);
bool sdcard_save  (void);
void sdcard_size  (uint32_t *total, uint32_t *free);

bool DeleteDir    (FS_TYPE &fs, const char *path);
void DeleteFile   (FS_TYPE &fs, const char *path);
void GetFileList  (FS_TYPE &fs, const char *dirname, uint8_t levels, std::vector<FileInfo_t> &files);

/*--------------------------------------------------------------------------------
 * Sequence number management file
 *--------------------------------------------------------------------------------*/
#define MLX90640_DIR  String("/MLX90640")
#define MLX90640_NUM  String("/@number.txt")

static int GetFileNo(FS_TYPE &fs) {

  if (!fs.exists(MLX90640_DIR)) {
    DBG_EXEC(printf("Creating Dir: %s\n", MLX90640_DIR));
    if (fs.mkdir(MLX90640_DIR)) {
      DBG_EXEC(printf("done.\n"));
    } else {
      DBG_EXEC(printf("failed.\n"));
      return 0;
    }
  }

  int number = 0;
  String path = MLX90640_DIR + MLX90640_NUM;

  File file = fs.open(path, FILE_READ);
  if (file.available()) {
    String n = file.readString();
    number = n.toInt();
  }

  file.close();

  file = fs.open(path, FILE_WRITE);
  if (file.print(++number)) {
    DBG_EXEC(printf("done: %d\n", number));
  } else {
    DBG_EXEC(printf("fail: %d\n", number));
  }

  file.close();
  return number;
}

/*--------------------------------------------------------------------------------
 * A function to get a list of files in a specified directory.
 *--------------------------------------------------------------------------------*/
static void getFileList(FS_TYPE &fs, const char *dirname, uint8_t levels, std::vector<FileInfo_t> &files) {
  File root = fs.open(dirname);
  if (!root) {
    DBG_EXEC(printf("Failed to open directory.\n"));
    return;
  }
  if (!root.isDirectory()) {
    DBG_EXEC(printf("Not a directory.\n"));
    return;
  }

  File file = root.openNextFile();
  while (file) {
    bool isDir = file.isDirectory();

#if USE_SDFAT
    char name[BUF_SIZE]; // BUF_SIZE is defined in printf.hpp
    file.getName(name, sizeof(name));
    if (file.isHidden())
#else
    const char *p = strrchr(file.path(), '/');
    if (p[p ? 1 : 0] == '.')
#endif
    {
      ; // skip dot file
    }
    
    else {
      // Add full path to vector
      // file.path(), file.name(), file.size()
      // https://stackoverflow.com/questions/27609839/about-c-vectorpush-back-exceptions-ellipsis-catch-useful
      // https://cpprefjp.github.io/reference/exception/exception.html
      try {
#if USE_SDFAT
        char path[BUF_SIZE];
        sprintf(path, "/%s/%s", dirname, name);
        files.push_back({path, (size_t)file.fileSize(), isDir, false});
#else
        files.push_back({file.path(), file.size(), isDir, false});
#endif
      } catch (const std::exception &e) {
        DBG_EXEC(printf("Exception: %s\n", e.what()));
        return;
      }

      if (isDir && levels) {
#if USE_SDFAT
        getFileList(fs, name, levels - 1, files);
#else
        getFileList(fs, file.path(), levels - 1, files);
#endif
      }
    }

    file = root.openNextFile();
  }
}

/*--------------------------------------------------------------------------------
 * A function to get a list of files in a specified directory except top directory
 *--------------------------------------------------------------------------------*/
void GetFileList(FS_TYPE &fs, const char *dirname, uint8_t levels, std::vector<FileInfo_t> &files) {
  getFileList(fs, dirname, levels, files);

  // erase the first file because it's a directory
  files.erase(files.begin());
}

/*--------------------------------------------------------------------------------
 * Basic functions to delete a directory and a file.
 *--------------------------------------------------------------------------------*/
bool DeleteDir(FS_TYPE &fs, const char *path) {
  // `path` must be empty
  if (fs.rmdir(path)) {
    DBG_EXEC(printf("Delete %s: done.\n", path));
    return true;
  } else {
    DBG_EXEC(printf("Delete %s: failed.\n", path));
    return false;
  }
}

void DeleteFile(FS_TYPE &fs, const char *path) {
  if (fs.remove(path)) {
    DBG_EXEC(printf("Delete %s: done.\n", path));
  } else {
    DBG_EXEC(printf("Delete %s: failed.\n", path));
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

#if defined (_ADAFRUIT_GFX_H)

/* create snapshot of 3.5" TFT and save to file in bitmap format
 * https://forum.arduino.cc/t/create-snapshot-of-3-5-tft-and-save-to-file-in-bitmap-format/391367/7
*/
static uint16_t readPixA(int x, int y) { // get pixel color code in rgb565 format
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

/*--------------------------------------------------------------------------------
 * Save LCD screenshot as a 24bits bitmap file
 *--------------------------------------------------------------------------------*/
static bool SaveBMP24(FS_TYPE &fs, const char *path) {

#if   defined (LOVYANGFX_HPP_)

  lgfx::rgb888_t rgb[TFT_WIDTH > TFT_HEIGHT ? TFT_WIDTH : TFT_HEIGHT];

#elif defined (_TFT_eSPIH_)

  uint16_t rgb[TFT_WIDTH > TFT_HEIGHT ? TFT_WIDTH : TFT_HEIGHT]; // check ReadWrite_Test

#else

  uint16_t rgb;
  uint8_t r, g, b;

#endif

  File file = fs.open(path, FILE_WRITE);

  if (!file) {
    DBG_EXEC(printf("SD open failed.\n"));
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
      DBG_EXEC(printf("."));
      delay(1); // reset wdt
    }

#if defined (LOVYANGFX_HPP_) || defined(_TFT_eSPIH_)

//  GFX_EXEC(beginTransaction());
    GFX_EXEC(readRect(0, y, w, 1, rgb));
//  GFX_EXEC(endTransaction());
    file.write((uint8_t*)rgb, w * sizeof(rgb[0])); // SD: 2966 msec, SdFat: 2777 msec

#else // defined (LOVYANGFX_HPP_) || defined(_TFT_eSPIH_)

    for (int x = 0; x < w; x++) {

#if   defined (_ARDUINO_GFX_LIBRARIES_H_)

      rgb = 0; // does not support reading

#elif defined (_ADAFRUIT_GFX_H)

      rgb = readPixA(x, y);

#endif

      // write the data in BMP reverse order
      color565toRGB(rgb, r, g, b);
      file.write(b);
      file.write(g);
      file.write(r);
    }
  
#endif // defined (LOVYANGFX_HPP_) || defined(_TFT_eSPIH_)
  }

  file.close();
  DBG_EXEC(printf("saved successfully.\n"));
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

bool sdcard_open(void) {
  uint8_t retry = 0;

  while (!SD.begin(SD_CONFIG)) {
    if (++retry >= 2) {
      DBG_EXEC(printf("Card mount failed.\n"));
      return false;
    }
    delay(1000);
  }

  DBG_EXEC(printf("The card was mounted successfully.\n"));
  return true;
}

void sdcard_size(uint32_t *total, uint32_t *free) {
#if USE_SDFAT
  *total = (uint32_t)(0.000512 * (uint32_t)SD.card()->sectorCount() + 0.5);
  *free  = (uint32_t)((SD.vol()->bytesPerCluster() * SD.vol()->freeClusterCount()) / (1024 * 1024));
#else
  *total = (uint32_t)(SD.totalBytes() / (1024 * 1024));
  *free  = (uint32_t)(*total - SD.usedBytes()  / (1024 * 1024));
#endif
}

bool sdcard_save(void) {
  if (!sdcard_open()) {
    return false;
  }

#if CAPTURE_SCREEN
  int no = GetFileNo(SD);
  char path[BUF_SIZE];
  sprintf(path, "%s/mlx%04d.bmp", MLX90640_DIR, no);
  DBG_EXEC(printf("%s\n", path));

  DBG_EXEC(uint32_t start = millis());

  if (!SaveBMP24(SD, path)) {
    return false;
  }

  DBG_EXEC(printf("Elapsed time: %d msec\n", millis() - start)); // SD: 6264 msec, SdFat: 4202 msec
#endif

  DBG_EXEC({
    std::vector<FileInfo_t> files;
    GetFileList(SD, "/", 1, files);
    for (const auto& file : files) {
      printf("%s, %lu\n", file.path.c_str(), file.size);
    }
  });

  // SD.end(); // Activating this line will cause some GFX libraries to stop working.

  DBG_EXEC({
    uint32_t total; uint32_t free;
    sdcard_size(&total, &free);
    printf("Card size: %luMB\nFree size: %luMB\n", total, free);
  });

  return true;
}