#include <Arduino.h>
#include <SPI.h>
#include "spi_assign.h"
#include "colors.h"

#define DEBUG       true
#if     DEBUG
#define DBG_EXEC(x) x
#else
#define DBG_EXEC(x)
#endif

/*--------------------------------------------------------------------------------
 * Widget control parameters
 *--------------------------------------------------------------------------------*/
typedef struct {
  uint8_t       interpolation;
  uint8_t       block_size;
  uint8_t       color_scheme;
  uint8_t       capture_mode; // 0: camera, 1: video
  bool          video_recording;
  bool          file_checkbox;
  bool          minmax_auto;
  bool          range_auto;
  int16_t       range_min;
  int16_t       range_max;
  uint16_t      calibration[8];
} MLXConfig_t;

MLXConfig_t cnf = {
  .interpolation  = 8,
  .block_size     = 1,
  .color_scheme   = 0,
  .capture_mode   = 0,
  .file_checkbox  = false,
  .minmax_auto    = false,
  .range_auto     = false,
  .range_min      = 20,
  .range_max      = 35,
  .calibration    = {319, 384, 3866, 355, 277, 3729, 3832, 3785},
};

/*=============================================================
 * Step 1: Select GFX Library
 *=============================================================*/
#if 1
/*---------------------------------------------------
 * LovyanGFX Library
 * https://github.com/lovyan03/LovyanGFX
 *---------------------------------------------------*/
#include <LovyanGFX.hpp>
#include "LGFX_XIAO_ESP32S3_ST7789.hpp"

LGFX lcd;

#define GFX_EXEC(x) lcd.x

void gfx_setup(void) {
  GFX_EXEC(init());
  GFX_EXEC(clear(0));
  GFX_EXEC(setRotation(3));
}

#else
/*---------------------------------------------------
 * TFT_eSPI Library
 * https://github.com/Bodmer/TFT_eSPI
 *---------------------------------------------------*/
#include "TFT_eSPI.h"

TFT_eSPI tft = TFT_eSPI();

#define GFX_EXEC(x) tft.x

void gfx_setup(void) {
  GFX_EXEC(init());
  GFX_EXEC(fillScreen(0));
  GFX_EXEC(setRotation(3));
}
#endif

/*=============================================================
 * printf library
 *=============================================================*/
#include "printf.hpp"

/*=============================================================
 * SD Card library
 *=============================================================*/
#include "sdcard.hpp"

/*=============================================================
 * Touch event manager
 *=============================================================*/
#include "touch.hpp"

/*=============================================================
 * Wedget manager
 *=============================================================*/
#include "widget.hpp"

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() <= 1000);

  gfx_setup();
  touch_setup();
  sdcard_setup();
  widget_setup();

#ifdef  ESP32
  // https://qiita.com/Dreamwalker/items/01cd216d48b4528c5959
  DBG_EXEC(printf("Heap Size : %d\n", ESP.getHeapSize()));
  DBG_EXEC(printf("Heap Free : %d\n", ESP.getFreeHeap()));
  DBG_EXEC(printf("PSRAM Size: %d\n", ESP.getPsramSize()));
  DBG_EXEC(printf("PSRAM Free: %d\n", ESP.getFreePsram()));
#endif
}

void loop() {
  widget_control();

	if (Serial.available() > 0) {
    while (Serial.available() > 0) {
  		Serial.read();
    }
    Serial.println("saving screenshot...");
    sdcard_save();
    Serial.println("done.");
	}

  delay(1);
}