/*================================================================================
 * Thermograpy camera GUI mockup
 * Copyright (c) 2024 embedded-kiddie
 * Released under the MIT license
 * https://opensource.org/license/mit
 *================================================================================*/
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
 * Step 1: Select GFX Library
 *--------------------------------------------------------------------------------*/
uint16_t lcd_width;
uint16_t lcd_height;

#if 1
/*---------------------------------------------------
 * LovyanGFX Library
 * https://github.com/lovyan03/LovyanGFX
 *---------------------------------------------------*/
// LovyanGFX requires SD library header file before including <LovyanGFX.hpp>
// #include <SD.h>
#include "SdFat.h"
#include "LGFX_XIAO_ESP32S3_ST7789.hpp"

// require `PSRAM: "OPT PSRAM"` in tool menu
LGFX lcd;
LGFX_Sprite lcd_sprite(&lcd);

#define SCREEN_ROTATION 3
#define GFX_EXEC(x) lcd.x
#define GFX_FAST(x) lcd_sprite.x
//#define drawPixel   writePixel /* This makes slow, why? */

void gfx_setup(void) {
  GFX_EXEC(init());
  GFX_EXEC(clear(0));
  GFX_EXEC(setTextColor(WHITE, BLACK));
  GFX_EXEC(setRotation(SCREEN_ROTATION));
  GFX_FAST(setPsram(true));
  lcd_width  = GFX_EXEC(width());
  lcd_height = GFX_EXEC(height());
}

#else
/*---------------------------------------------------
 * TFT_eSPI Library
 * https://github.com/Bodmer/TFT_eSPI
 *---------------------------------------------------*/
// Neither standard SD nor SdFat works when writing! (reading is OK)
// #include <SD.h>
#include "SdFat.h"
#include "TFT_eSPI.h"

// require `CONFIG_SPIRAM_SUPPORT` in User_Setup.h
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite tft_sprite(&tft);

#define SCREEN_ROTATION 3
#define GFX_EXEC(x) tft.x
#define GFX_FAST(x) tft_sprite.x
#define setClipRect setViewport
#define clearClipRect resetViewport

void gfx_setup(void) {
  GFX_EXEC(init());
  GFX_EXEC(fillScreen(0));
  GFX_EXEC(setTextColor(WHITE, BLACK));
  GFX_EXEC(setRotation(SCREEN_ROTATION));
  lcd_width  = GFX_EXEC(width());
  lcd_height = GFX_EXEC(height());
}
#endif

/*--------------------------------------------------------------------------------
 * Step 2: Select whether to multitask or not
 *--------------------------------------------------------------------------------*/
#define ENA_MULTITASKING  true

/*--------------------------------------------------------------------------------
 * Step 3: Select whether to interpolate or not
 *--------------------------------------------------------------------------------*/
#define ENA_INTERPOLATION true

/*--------------------------------------------------------------------------------
 * Step 4: Select whether to enable transaction or not
 *--------------------------------------------------------------------------------*/
#if defined (LOVYANGFX_HPP_) || defined (_TFT_eSPIH_)
#define ENA_TRANSACTION   true
#else
#define ENA_TRANSACTION   false // 'true' stops display
#endif

/*--------------------------------------------------------------------------------
 * Step 5: Configure the output image resolution
 *--------------------------------------------------------------------------------*/
// INTERPOLATE_SCALE x BOX_SIZE <= 8
#if ENA_INTERPOLATION
#define INTERPOLATE_SCALE 8
#define BOX_SIZE          1
#else
#define INTERPOLATE_SCALE 1
#define BOX_SIZE          8
#endif

/*--------------------------------------------------------------------------------
 * Definitions of graphics helpers
 *--------------------------------------------------------------------------------*/
// Font size for setTextSize(2)
#define FONT_WIDTH    12 // [px] (Device coordinate system)
#define FONT_HEIGHT   16 // [px] (Device coordinate system)
#define LINE_HEIGHT   18 // [px] (FONT_HEIGHT + margin)

/*--------------------------------------------------------------------------------
 * Initial values for range of temperature
 *--------------------------------------------------------------------------------*/
#define MINTEMP 20  // Low  temperature range
#define MAXTEMP 35  // High temperature range

/*--------------------------------------------------------------------------------
 * The size of thermal image (max INTERPOLATE_SCALE = 8)
 *--------------------------------------------------------------------------------*/
#define MLX90640_COLS 32
#define MLX90640_ROWS 24
#define INTERPOLATED_COLS (MLX90640_COLS * 8)
#define INTERPOLATED_ROWS (MLX90640_ROWS * 8)

/*--------------------------------------------------------------------------------
 * MLX90640 instance
 *--------------------------------------------------------------------------------*/
#include <Adafruit_MLX90640.h>
Adafruit_MLX90640 mlx;

#if ENA_MULTITASKING
float src[2][MLX90640_ROWS  * MLX90640_COLS    ];
#else
float src[1][MLX90640_ROWS  * MLX90640_COLS    ];
#endif
float dst[INTERPOLATED_ROWS * INTERPOLATED_COLS];

/*--------------------------------------------------------------------------------
 * MLX90640 control parameters
 *--------------------------------------------------------------------------------*/
typedef struct MLXConfig {
  // Member Variables
  uint8_t   interpolation;    // 1, 2, 4, 6, 8
  uint8_t   box_size;         // 1, 2, 4, 8
  uint8_t   refresh_rate;     // sampline frequency (mlx90640_refreshrate_t)
  uint8_t   color_scheme;     // 0: rainbow, 1: orange
  uint8_t   marker_mode;      // 1: min/max, 2: picked up by user
  bool      range_auto;       // automatic measurement of temperature min/max
  int16_t   range_min;        // minimum temperature
  int16_t   range_max;        // maximum temperature
  float     sampling_period;  // Sampling Period [sec]

  // Comparison Operators
  bool operator >= (const MLXConfig &RHS) {
    return (
      (interpolation != RHS.interpolation) ||
      (box_size      != RHS.box_size     )
    );
  }
  bool operator != (const MLXConfig &RHS) {
    return (
      (color_scheme  != RHS.color_scheme ) ||
      (marker_mode   != RHS.marker_mode  ) ||
      (range_auto    != RHS.range_auto   ) ||
      (range_min     != RHS.range_min    ) ||
      (range_max     != RHS.range_max    )
    );
  }

  // Setup MLX90640 refresh rate (optimized for LovyanGFX and TFT_eSPI)
  void setup(void) {
    refresh_rate = (ENA_MULTITASKING && ENA_TRANSACTION ? MLX90640_32_HZ /*6*/ : MLX90640_16_HZ /*5*/);
    sampling_period = (refresh_rate == MLX90640_32_HZ ? 1.0f / 16.0f : 1.0f / 8.0f);
  }
} MLXConfig_t;

typedef struct MLXCapture {
  uint8_t       capture_mode; // 0: camera, 1: video
  bool          recording;    // false: stop, true: recording
  char          filename[30]; // "/MLX90640/mlx%04d.raw"
} MLXCapture_t;

static constexpr MLXConfig_t mlx_ini = {
  .interpolation  = INTERPOLATE_SCALE,
  .box_size       = BOX_SIZE,
  .refresh_rate   = 0,
  .color_scheme   = 0,
  .marker_mode    = 0,
  .range_auto     = false,
  .range_min      = MINTEMP,
  .range_max      = MAXTEMP,
};

static MLXConfig_t mlx_cnf = mlx_ini;
static MLXCapture_t mlx_cap = {
  .capture_mode   = 0,
  .recording      = false,
};

/*--------------------------------------------------------------------------------
 * Low pass filter
 * x: input, T: sampling time [sec]
 *--------------------------------------------------------------------------------*/
#define TIME_CONSTANT   3.0f // [sec]
typedef struct {
  float y;
  float filter(float x, const float T) {
    return (y += T / (T + TIME_CONSTANT) * (x - y));
  };
} LowPass_t;

static LowPass_t lmin, lmax, lpic;

static void reset_filter(void) {
  lmin.y = MINTEMP;
  lmax.y = MAXTEMP;
  lpic.y = (MINTEMP + MAXTEMP) / 2;
}

/*--------------------------------------------------------------------------------
 * Set MLX90640 refresh rate
 *--------------------------------------------------------------------------------*/
static void mlx_refresh(void) {
  // configure refresh rate
  mlx_cnf.setup();
//mlx.setRefreshRate((mlx90640_refreshrate_t)mlx_cnf.refresh_rate);
//DBG_EXEC(printf("refresh rate: %d\n", mlx_cnf.refresh_rate));

  // initialize lowpass filter
  reset_filter();
}

/*--------------------------------------------------------------------------------
 * Measure temperature and updates range, min/max points
 *--------------------------------------------------------------------------------*/
#if     false
#define CHECK_VALUE true
#include <math.h>
#endif

typedef struct {
  uint16_t  x, y;
  float     v;
} Temperature_t;

static Temperature_t tmin, tmax, _tmin, _tmax, tpic;

static void measure_temperature(float *src) {
  // Measure the temperature at the picked up point
  if (tpic.x != 0 || tpic.y != 0) {
    tpic.v = src[tpic.x + (tpic.y * MLX90640_COLS)];
    tpic.v = lpic.filter(tpic.v, mlx_cnf.sampling_period);
  }

  // Measure min/max temperature
  tmin.v =  999.0f;
  tmax.v = -999.0f;

  for (uint16_t y = 0; y < MLX90640_ROWS; y++) {
    for (uint16_t x = 0; x < MLX90640_COLS; x++, src++) {
      float t = *src;
#ifdef  CHECK_VALUE
      if (isinf(t) || isnan(t) || t < -20.0f || 180.0f < t) {
        continue;
      }
#endif
      if (t < tmin.v) { tmin.x = x; tmin.y = y; tmin.v = t; } else
      if (t > tmax.v) { tmax.x = x; tmax.y = y; tmax.v = t; }
    }
  }

  if (mlx_cnf.range_auto) {
    #define RANGE_STEP  2
    mlx_cnf.range_min = ((int)((float)lmin.filter(tmin.v, mlx_cnf.sampling_period) / (float)RANGE_STEP) + 0.5f) * RANGE_STEP;
    mlx_cnf.range_max = ((int)((float)lmax.filter(tmax.v, mlx_cnf.sampling_period) / (float)RANGE_STEP) + 0.5f) * RANGE_STEP;

    // debug for serial ploter
    // DBG_EXEC(printf("%4.1f, %4.1f, %4.1f, %4.1f\n", tmin.v, lmin.y, tmax.v, lmax.y));
  }
}

/*--------------------------------------------------------------------------------
 * Heat map
 *--------------------------------------------------------------------------------*/
#define N_HEATMAP   256
const uint16_t heatmap[2][N_HEATMAP] = {
#include "heatmap.h"
};

/*--------------------------------------------------------------------------------
 * printf library
 *--------------------------------------------------------------------------------*/
#include "printf.hpp"

/*--------------------------------------------------------------------------------
 * SD Card library
 *--------------------------------------------------------------------------------*/
#include "sdcard.hpp"

/*--------------------------------------------------------------------------------
 * Touch event manager
 *--------------------------------------------------------------------------------*/
#include "touch.hpp"

/*--------------------------------------------------------------------------------
 * Wedget manager
 *--------------------------------------------------------------------------------*/
#include "widget.hpp"

/*--------------------------------------------------------------------------------
 * Pixel interpolation
 *--------------------------------------------------------------------------------*/
#include "interpolation.hpp"

/*--------------------------------------------------------------------------------
 * setup() and loop()
 *--------------------------------------------------------------------------------*/
void setup() {
  DBG_EXEC(Serial.begin(115200));
  DBG_EXEC(delay(1000));

  if (psramInit()) {
    DBG_EXEC(printf("\nThe PSRAM is correctly initialized.\n"));
  } else {
    DBG_EXEC(printf("\nPSRAM does not work.\n"));
  }

  // Initialize LCD display with touch and SD card
  gfx_setup();
  touch_setup();
  sdcard_setup();
  widget_setup();

#if defined (ESP32)
  DBG_EXEC(printf("Total heap: %d\n", ESP.getHeapSize()));
  DBG_EXEC(printf("Free  heap: %d\n", ESP.getFreeHeap()));
  DBG_EXEC(printf("Total PSRAM: %d\n",ESP.getPsramSize()));
  DBG_EXEC(printf("Free  PSRAM: %d\n",ESP.getFreePsram()));
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
  yield();
}