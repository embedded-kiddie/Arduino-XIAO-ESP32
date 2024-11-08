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
typedef struct MLXConfig {
  // Member Variables
  uint8_t       interpolation;
  uint8_t       block_size;
  uint8_t       color_scheme;
  uint8_t       padding;
  bool          minmax_auto;
  bool          range_auto;
  int16_t       range_min;
  int16_t       range_max;

  // Comparison Operator
  bool operator != (const MLXConfig &RHS) {
    return (
      (interpolation != RHS.interpolation) ||
      (block_size    != RHS.block_size   ) ||
      (color_scheme  != RHS.color_scheme ) ||
      (minmax_auto   != RHS.minmax_auto  ) ||
      (range_auto    != RHS.range_auto   ) ||
      (range_min     != RHS.range_min    ) ||
      (range_max     != RHS.range_max    )
    );
  }
} MLXConfig_t;

typedef struct MLXCapture {
  uint8_t       capture_mode; // 0: camera, 1: video
  bool          recording;
} MLXCapture_t;

MLXConfig_t mlx_cnf = {
  .interpolation  = 8,
  .block_size     = 1,
  .color_scheme   = 0,
  .padding        = 0,
  .minmax_auto    = false,
  .range_auto     = false,
  .range_min      = 20,
  .range_max      = 35,
};

MLXCapture_t mlx_cap = {
  .capture_mode   = 0,
  .recording      = false,
};

/*=============================================================
 * Step 1: Select GFX Library
 *=============================================================*/
uint16_t lcd_width;
uint16_t lcd_height;

#if 1
/*---------------------------------------------------
 * LovyanGFX Library
 * https://github.com/lovyan03/LovyanGFX
 *---------------------------------------------------*/
// LovyanGFX need <SdFat.h> before including <LovyanGFX.hpp>
#include "SdFat.h"
#include <LovyanGFX.hpp>
#include "LGFX_XIAO_ESP32S3_ST7789.hpp"

LGFX lcd;

#define GFX_EXEC(x)     lcd.x
#define SCREEN_ROTATION 3

void gfx_setup(void) {
  GFX_EXEC(init());
  GFX_EXEC(clear(0));
  GFX_EXEC(setRotation(3));
  lcd_width  = GFX_EXEC(width());
  lcd_height = GFX_EXEC(height());
}

#else
/*---------------------------------------------------
 * TFT_eSPI Library
 * https://github.com/Bodmer/TFT_eSPI
 *---------------------------------------------------*/
#include "TFT_eSPI.h"

TFT_eSPI tft = TFT_eSPI();

#define GFX_EXEC(x)     tft.x
#define SCREEN_ROTATION 3

void gfx_setup(void) {
  GFX_EXEC(init());
  GFX_EXEC(fillScreen(0));
  GFX_EXEC(setRotation(3));
  lcd_width  = GFX_EXEC(width());
  lcd_height = GFX_EXEC(height());
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
  DBG_EXEC(Serial.begin(115200));
  DBG_EXEC(while (!Serial && millis() <= 1000));

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
  /*
    https://en.cppreference.com/w/cpp/compiler_support
    https://forum.arduino.cc/t/which-version-of-c-is-currently-supported/1285868/13
    https://docs.espressif.com/projects/esp-idf/en/v5.3.1/esp32/api-guides/cplusplus.html
    https://gcc.gnu.org/onlinedocs/cpp/Standard-Predefined-Macros.html#:~:text=__cplusplus,gnu%2B%2B26.
    199711 ➜ C++98 or C++03 standards.
    201103 ➜ C++11 standard.
    201402 ➜ C++14 standard.
    201703 ➜ C++17 standard.
    202002 ➜ C++20 standard.
    202302 ➜ C++23 standard.
  */
  DBG_EXEC(printf("__cplusplus: %d\n", __cplusplus)); // 202100 (may be C++20 for Intel C++ ?)
}

void loop() {
  widget_control();
  delay(1);
}