/*================================================================================
 * Thermograpy camera
 * Copyright (c) 2024 embedded-kiddie
 * Released under the MIT license
 * https://opensource.org/license/mit
 *================================================================================*/
#include <Arduino.h>
#include <SPI.h>
#include "spi_assign.h"
#include "colors.h"

#define DEBUG       false
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
#include "SdFat.h" // version 2.2.3 is required (drawBmpFile() causes a compilation error in version 2.3.0)
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
  GFX_EXEC(initDMA()); // DMA disable with use SPIRAM
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
  GFX_EXEC(initDMA(true)); // DMA does not work on PSRAM. See CONFIG_SPIRAM_SUPPORT in User_Setup.h
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
#define REFRESH_RATE      (ENA_MULTITASKING && ENA_TRANSACTION ? MLX90640_32_HZ : MLX90640_16_HZ)
#else
#define INTERPOLATE_SCALE 1
#define BOX_SIZE          8
#define REFRESH_RATE      (ENA_MULTITASKING && ENA_TRANSACTION ? MLX90640_32_HZ : MLX90640_16_HZ)
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
      (box_size      != RHS.box_size     ) ||
      (refresh_rate  != RHS.refresh_rate )
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

  // Setup refresh_rate according to INTERPOLATE_SCALE and BOX_SIZE
  void setup(void) {
    sampling_period = 2.0f / pow(2.0f, (float)(refresh_rate - 1));
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
  .refresh_rate   = REFRESH_RATE,
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
 * Low pass filter (x: input, T: sampling time [sec])
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
  mlx.setRefreshRate((mlx90640_refreshrate_t)mlx_cnf.refresh_rate);

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
 * Pixel interpolation
 *--------------------------------------------------------------------------------*/
#include "interpolation.hpp"

/*--------------------------------------------------------------------------------
 * Multitask controller
 *--------------------------------------------------------------------------------*/
#include "multitasking.hpp"

/*--------------------------------------------------------------------------------
 * Wedget manager
 *--------------------------------------------------------------------------------*/
#include "widget.hpp"

/*--------------------------------------------------------------------------------
 * Input process - Get thermal image from MLX90640
 *--------------------------------------------------------------------------------*/
void ProcessInput(uint8_t bank) {
  if (mlx.getFrame(src[bank]) != 0) {
    gfx_printf(TFT_WIDTH / 2 - FONT_WIDTH * 2, TFT_HEIGHT / 2 - FONT_HEIGHT * 4, "Failed");
    DBG_EXEC(printf("Failed\n"));
    delay(1000); // false = no new frame capture
  }

  // Measure temperature of min/max/pickup
  measure_temperature(src[bank]);
}

/*--------------------------------------------------------------------------------
 * Output process - Interpolate thermal image and display on LCD.
 *--------------------------------------------------------------------------------*/
void ProcessOutput(uint8_t bank, uint32_t inputStart, uint32_t inputFinish) {
  // Widget controller
  State_t state = widget_control();
  if (state == STATE_MAIN || state == STATE_THERMOGRAPH) {
    static uint32_t outputFinish, outputPeriod, prevStart;
    uint32_t outputStart = millis();
    const int dst_rows = mlx_cnf.interpolation * MLX90640_ROWS;
    const int dst_cols = mlx_cnf.interpolation * MLX90640_COLS;
    const int box_size = mlx_cnf.box_size;
    const uint16_t *hm = heatmap[mlx_cnf.color_scheme];

#if ENA_INTERPOLATION
    interpolate_image(src[bank], MLX90640_ROWS, MLX90640_COLS, dst, dst_rows, dst_cols);
    float *drw = dst;
#else
    float *drw = src[bank];
#endif

#if ENA_TRANSACTION
    GFX_EXEC(startWrite());
    GFX_FAST(createSprite(dst_cols * box_size, dst_rows * box_size));
#endif

    for (int h = 0; h < dst_rows; h++) {
      for (int w = 0; w < dst_cols; w++) {
        float t = drw[h * dst_cols + w];

        t = min((int)t, (int)mlx_cnf.range_max);
        t = max((int)t, (int)mlx_cnf.range_min); 

        int colorIndex = map(t, mlx_cnf.range_min, mlx_cnf.range_max, 0, N_HEATMAP - 1);

#if 0
        // Selfie Camera
        GFX_FAST(fillRect(box_size * w, box_size * h, box_size, box_size, hm[colorIndex]));
#else
        // Front Camera
        if (box_size == 1) {
          GFX_FAST(drawPixel(dst_cols - 1 - w, h, hm[colorIndex]));
        } else {
          GFX_FAST(fillRect((dst_cols - 1 - w) * box_size, h * box_size, box_size, box_size, hm[colorIndex]));
        }
#endif
      }
    }

    if (mlx_cnf.marker_mode) {
      static uint32_t prevUpdate;
      if (outputStart - prevUpdate > 1000) {
        prevUpdate = outputStart;
        _tmin = tmin; _tmax = tmax;
      }
      DrawTemperatureMarker();
    }

    if (mlx_cnf.range_auto) {
      DrawTemperatureRange(2);
    }

    if (state == STATE_MAIN) {
      // MLX90640
      // GFX_EXEC(setTextColor(WHITE, BLACK)); // Use opaque text output
      gfx_printf(260 + FONT_WIDTH, LINE_HEIGHT * 3.5, "%4d", inputFinish - inputStart);

      // Interpolation
      gfx_printf(260 + FONT_WIDTH, LINE_HEIGHT * 5.0, "%4d", outputPeriod);

      // FPS
      float v = 1000.0f / (float)(outputStart - prevStart);
      gfx_printf(260 + FONT_WIDTH, LINE_HEIGHT * 2.0, "%4.1f", v);

      // Ambient temperature
      v = mlx.getTa(false);
      if (0.0f < v && v < 100.0f) {
        gfx_printf(260 + FONT_WIDTH, LINE_HEIGHT * 6.5, "%4.1f", v);
      }
    }

#if ENA_TRANSACTION
    GFX_FAST(pushSprite(0, 0));
    GFX_FAST(deleteSprite());
    GFX_EXEC(endWrite());
#endif

    // Save video
    if (mlx_cap.recording) {
      sdcard_record((uint8_t*)src[bank], sizeof(src[bank]), mlx_cap.filename);
    }

    // Update processing time
    prevStart = outputStart;
    outputFinish = millis();
    outputPeriod = outputFinish - outputStart;
  }
}

/*--------------------------------------------------------------------------------
 * setup() and loop()
 *--------------------------------------------------------------------------------*/
void setup() {
  DBG_EXEC(Serial.begin(115200));
  DBG_EXEC(delay(1000));

  if (psramInit()) {
    DBG_EXEC(printf("\nThe PSRAM is correctly initialized.\n"));
  } else {
    DBG_EXEC(printf("\nPSRAM does not work. Please check 'PSRAM' in IDE menu 'Tools'. \n"));
  }

  // Initialize LCD display with touch and SD card
  gfx_setup();
  touch_setup();
  sdcard_setup();
  widget_setup();

  // Initialize interpolation
  interpolate_setup(mlx_cnf.interpolation);

  if (! mlx.begin(MLX90640_I2CADDR_DEFAULT, &Wire)) {
    DBG_EXEC(printf("MLX90640 not found!\n"));
  } else {
    DBG_EXEC(printf("MLX90640 found.\n"));
    DBG_EXEC(printf("Serial number: %04X%04X%04X\n", mlx.serialNumber[0], mlx.serialNumber[1], mlx.serialNumber[2]));
  }

  // I2C bus clock for MLX90640
  // Note: ESP32S3 supports up to 800 KHz
  Wire.setClock(1000000); // 400 KHz (Sm) or 1 MHz (Fm+)

  // Set MLX90640 operating mode
  mlx.setMode(MLX90640_CHESS); // or MLX90640_INTERLEAVED
  mlx.setResolution(MLX90640_ADC_18BIT);  // 16BIT, 17BIT, 18BIT (default) or 19BIT
  mlx_refresh();

  // Start tasks
#if ENA_MULTITASKING
  void task_setup(void (*task1)(uint8_t), void (*task2)(uint8_t, uint32_t, uint32_t));
  task_setup(ProcessInput, ProcessOutput);
#endif
}

void loop() {
#if ENA_MULTITASKING
  delay(1000);
#else
  uint32_t inputStart = millis();
  ProcessInput(0); // always use bank 0
  ProcessOutput(0, inputStart, millis());
#endif
}