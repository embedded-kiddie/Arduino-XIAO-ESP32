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

#if 0
/*---------------------------------------------------
 * Adafruit GFX Library
 * https://github.com/adafruit/Adafruit-GFX-Library
 *---------------------------------------------------*/
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

#define SCREEN_ROTATION 1
#define GFX_EXEC(x) tft.x
#define GFX_FAST(x) tft.x

void gfx_setup(void) {
  GFX_EXEC(init(TFT_WIDTH, TFT_HEIGHT, SPI_MODE));
  GFX_EXEC(invertDisplay(false));
  GFX_EXEC(setTextColor(WHITE, BLACK));
  GFX_EXEC(setRotation(SCREEN_ROTATION));

#if defined (ARDUINO_XIAO_ESP32S3)
  GFX_EXEC(setSPISpeed(SPI_FREQUENCY)); // or SPISettings() ?
#endif

  lcd_width  = GFX_EXEC(width());
  lcd_height = GFX_EXEC(height());
}

#elif 0
/*---------------------------------------------------
 * Arduino GFX Library
 * https://github.com/moononournation/Arduino_GFX
 *---------------------------------------------------*/
#include <Arduino_GFX_Library.h>

Arduino_DataBus *bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCLK, TFT_MOSI, TFT_MISO);
Arduino_GFX *gfx = new Arduino_ST7789(bus, TFT_RST, 0 /* rotation */, true /* IPS */);

#define SCREEN_ROTATION 3
#define GFX_EXEC(x) gfx->x
#define GFX_FAST(x) gfx->x

void gfx_setup(void) {
  // Init Display
#if defined (ARDUINO_XIAO_ESP32S3)
  if (!GFX_EXEC(begin(SPI_FREQUENCY))) /* specify data bus speed */
#else
  if (!GFX_EXEC(begin())
#endif
  {
    DBG_EXEC(printf("gfx->begin() failed!\n"));
  }

  SPI.setDataMode(SPI_MODE);
  GFX_EXEC(invertDisplay(true));
  GFX_EXEC(setTextColor(WHITE, BLACK));
  GFX_EXEC(setRotation(SCREEN_ROTATION));
  lcd_width  = GFX_EXEC(width());
  lcd_height = GFX_EXEC(height());
}

#elif 1
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
//GFX_EXEC(initDMA()); /* Probably enabled by default if available */
  GFX_EXEC(clear(0));
  GFX_EXEC(setTextColor(WHITE, BLACK));
  GFX_EXEC(setRotation(SCREEN_ROTATION));
  lcd_width  = GFX_EXEC(width());
  lcd_height = GFX_EXEC(height());
  GFX_FAST(setPsram(true));
}

#else
/*---------------------------------------------------
 * TFT_eSPI Library
 * https://github.com/Bodmer/TFT_eSPI
 *---------------------------------------------------*/
// TFT_eSPI can not work with SdFat
#include <SD.h>
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
 * Initial values for range of tempareture
 *--------------------------------------------------------------------------------*/
#define MINTEMP 20  // Low range of the sensor (this will be blue on the screen)
#define MAXTEMP 35  // high range of the sensor (this will be red on the screen)

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
  uint8_t   interpolation;
  uint8_t   box_size;
  uint8_t   refresh_rate;
  uint8_t   color_scheme;
  bool      minmax_auto;
  bool      range_auto;
  int16_t   range_min;
  int16_t   range_max;
  float     sampling_period;

  // Comparison Operator
  bool operator >= (const MLXConfig &RHS) {
    return (
      (interpolation != RHS.interpolation) ||
      (box_size      != RHS.box_size     )
    );
  }
  bool operator != (const MLXConfig &RHS) {
    return (
      (color_scheme  != RHS.color_scheme ) ||
      (minmax_auto   != RHS.minmax_auto  ) ||
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
  bool          recording;    // false: stop, true: recording video
} MLXCapture_t;

static constexpr MLXConfig_t mlx_ini = {
  .interpolation  = INTERPOLATE_SCALE,
  .box_size       = BOX_SIZE,
  .refresh_rate   = 0,
  .color_scheme   = 0,
  .minmax_auto    = false,
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
  mlx.setRefreshRate((mlx90640_refreshrate_t)mlx_cnf.refresh_rate);
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
  float     t;
} Temperature_t;

static Temperature_t tmin, tmax, _tmin, _tmax, tpic;

static void measure_temperature(float *src) {
  // Measure the temperature at the picked up point
  if (tpic.x != 0 || tpic.y != 0) {
    tpic.t = src[tpic.x + (tpic.y * MLX90640_COLS)];
    lpic.filter(tpic.t, mlx_cnf.sampling_period);
  }

  // Measure temperature ranges
  tmin.t =  999.0f;
  tmax.t = -999.0f;

  for (uint16_t y = 0; y < MLX90640_ROWS; y++) {
    for (uint16_t x = 0; x < MLX90640_COLS; x++, src++) {
      float t = *src;
#ifdef  CHECK_VALUE
      if (isinf(t) || isnan(t) || t < -20.0f || 180.0f < t) {
        continue;
      }
#endif
      if (t < tmin.t) { tmin.x = x; tmin.y = y; tmin.t = t; } else
      if (t > tmax.t) { tmax.x = x; tmax.y = y; tmax.t = t; }
    }
  }

  if (mlx_cnf.range_auto) {
    #define RANGE_STEP  2
    mlx_cnf.range_min = ((int)((float)lmin.filter(tmin.t, mlx_cnf.sampling_period) / (float)RANGE_STEP) + 0.5f) * RANGE_STEP;
    mlx_cnf.range_max = ((int)((float)lmax.filter(tmax.t, mlx_cnf.sampling_period) / (float)RANGE_STEP) + 0.5f) * RANGE_STEP;

    // debug for serial ploter
    // DBG_EXEC(printf("%4.1f, %4.1f, %4.1f, %4.1f\n", tmin.t, lmin.y, tmax.t, lmax.y));
  }
}

/*--------------------------------------------------------------------------------
 * Global variables
 *--------------------------------------------------------------------------------*/
// The colors we will be using
const uint16_t camColors[] = {0x480F,
0x400F,0x400F,0x400F,0x4010,0x3810,0x3810,0x3810,0x3810,0x3010,0x3010,
0x3010,0x2810,0x2810,0x2810,0x2810,0x2010,0x2010,0x2010,0x1810,0x1810,
0x1811,0x1811,0x1011,0x1011,0x1011,0x0811,0x0811,0x0811,0x0011,0x0011,
0x0011,0x0011,0x0011,0x0031,0x0031,0x0051,0x0072,0x0072,0x0092,0x00B2,
0x00B2,0x00D2,0x00F2,0x00F2,0x0112,0x0132,0x0152,0x0152,0x0172,0x0192,
0x0192,0x01B2,0x01D2,0x01F3,0x01F3,0x0213,0x0233,0x0253,0x0253,0x0273,
0x0293,0x02B3,0x02D3,0x02D3,0x02F3,0x0313,0x0333,0x0333,0x0353,0x0373,
0x0394,0x03B4,0x03D4,0x03D4,0x03F4,0x0414,0x0434,0x0454,0x0474,0x0474,
0x0494,0x04B4,0x04D4,0x04F4,0x0514,0x0534,0x0534,0x0554,0x0554,0x0574,
0x0574,0x0573,0x0573,0x0573,0x0572,0x0572,0x0572,0x0571,0x0591,0x0591,
0x0590,0x0590,0x058F,0x058F,0x058F,0x058E,0x05AE,0x05AE,0x05AD,0x05AD,
0x05AD,0x05AC,0x05AC,0x05AB,0x05CB,0x05CB,0x05CA,0x05CA,0x05CA,0x05C9,
0x05C9,0x05C8,0x05E8,0x05E8,0x05E7,0x05E7,0x05E6,0x05E6,0x05E6,0x05E5,
0x05E5,0x0604,0x0604,0x0604,0x0603,0x0603,0x0602,0x0602,0x0601,0x0621,
0x0621,0x0620,0x0620,0x0620,0x0620,0x0E20,0x0E20,0x0E40,0x1640,0x1640,
0x1E40,0x1E40,0x2640,0x2640,0x2E40,0x2E60,0x3660,0x3660,0x3E60,0x3E60,
0x3E60,0x4660,0x4660,0x4E60,0x4E80,0x5680,0x5680,0x5E80,0x5E80,0x6680,
0x6680,0x6E80,0x6EA0,0x76A0,0x76A0,0x7EA0,0x7EA0,0x86A0,0x86A0,0x8EA0,
0x8EC0,0x96C0,0x96C0,0x9EC0,0x9EC0,0xA6C0,0xAEC0,0xAEC0,0xB6E0,0xB6E0,
0xBEE0,0xBEE0,0xC6E0,0xC6E0,0xCEE0,0xCEE0,0xD6E0,0xD700,0xDF00,0xDEE0,
0xDEC0,0xDEA0,0xDE80,0xDE80,0xE660,0xE640,0xE620,0xE600,0xE5E0,0xE5C0,
0xE5A0,0xE580,0xE560,0xE540,0xE520,0xE500,0xE4E0,0xE4C0,0xE4A0,0xE480,
0xE460,0xEC40,0xEC20,0xEC00,0xEBE0,0xEBC0,0xEBA0,0xEB80,0xEB60,0xEB40,
0xEB20,0xEB00,0xEAE0,0xEAC0,0xEAA0,0xEA80,0xEA60,0xEA40,0xF220,0xF200,
0xF1E0,0xF1C0,0xF1A0,0xF180,0xF160,0xF140,0xF100,0xF0E0,0xF0C0,0xF0A0,
0xF080,0xF060,0xF040,0xF020,0xF800,};

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
 * Input process - Get thermal image from MLX90640
 *--------------------------------------------------------------------------------*/
void ProcessInput(uint8_t bank) {
  if (mlx.getFrame(src[bank]) != 0) {
    gfx_printf(TFT_WIDTH / 2 - FONT_WIDTH * 2, TFT_HEIGHT / 2 - FONT_HEIGHT * 4, "Failed");
    DBG_EXEC(printf("Failed\n"));
    delay(1000); // false = no new frame capture
  }

  // Measure temperature for min/max/pickup
  measure_temperature(src[bank]);
}

/*--------------------------------------------------------------------------------
 * Output process - Interpolate thermal image and display on LCD.
 *--------------------------------------------------------------------------------*/
void ProcessOutput(uint8_t bank, uint32_t inputStart, uint32_t inputFinish) {
  // Widget controller
  State_t state = widget_control();
  if (state == STATE_MAIN || state == STATE_THERMOGRAPH) {
    static uint32_t prevFinish, prevUpdate;
    uint32_t outputStart = millis();
    const int dst_rows = mlx_cnf.interpolation * MLX90640_ROWS;
    const int dst_cols = mlx_cnf.interpolation * MLX90640_COLS;
    const int box_size = mlx_cnf.box_size;

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

        int colorIndex = map(t, mlx_cnf.range_min, mlx_cnf.range_max, 0, 255);
        colorIndex = constrain(colorIndex, 0, 255);

#if 0
        // Selfie Camera
        GFX_FAST(fillRect(box_size * w, box_size * h, box_size, box_size, camColors[colorIndex]));
#else
        // Front Camera
        if (box_size == 1) {
          GFX_FAST(drawPixel(dst_cols - 1 - w, h, camColors[colorIndex]));
        } else {
          GFX_FAST(fillRect((dst_cols - 1 - w) * box_size, h * box_size, box_size, box_size, camColors[colorIndex]));
        }
#endif
      }
    }

    if (outputStart - prevUpdate > 1000) {
      prevUpdate = outputStart;
      _tmin = tmin; _tmax = tmax;
    }

    if (mlx_cnf.minmax_auto) {
      GFX_EXEC(setClipRect(0, 0, dst_cols * box_size, dst_rows * box_size));
      DrawColorRange(4);
      GFX_EXEC(clearClipRect());
    }

    if (mlx_cnf.range_auto) {
      DrawColorRange(2);
    }

    if (state == STATE_MAIN) {
      // MLX90640
      GFX_EXEC(setTextColor(WHITE, BLACK)); // Use opaque text output
      gfx_printf(260 + FONT_WIDTH, LINE_HEIGHT * 3.5, "%4d", inputFinish - inputStart);

      // Interpolation
      uint32_t outputFinish = millis();
      gfx_printf(260 + FONT_WIDTH, LINE_HEIGHT * 5.0, "%4d", outputFinish - outputStart);

      // FPS
      float v = 1000.0f / (float)(outputFinish - prevFinish);
      gfx_printf(260 + FONT_WIDTH, LINE_HEIGHT * 2.0, "%4.1f", v);
      prevFinish = outputFinish;

      // Ambient temperature
      v = mlx.getTa(false);
      if (0.0f < v && v < 100.0f) {
        gfx_printf(260 + FONT_WIDTH, LINE_HEIGHT * 6.5, "%4.1f", v);
      }
    }
  }

#if ENA_TRANSACTION
    GFX_FAST(pushSprite(0, 0));
    GFX_FAST(deleteSprite());
    GFX_EXEC(endWrite());
#endif

  // Prevent the watchdog from firing
  yield();
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
    DBG_EXEC(printf("\nPSRAM does not work.\n"));
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
    DBG_EXEC(printf("Serial number: %X%X%X\n", mlx.serialNumber[0], mlx.serialNumber[1], mlx.serialNumber[2]));
  }

  // I2C bus clock for MLX90640
  // Note: ESP32S3 supports up to 800 KHz
  Wire.setClock(1000000); // 400 KHz (Sm) or 1 MHz (Fm+)

  // Set MLX90640 operating mode
  mlx.setMode(MLX90640_CHESS);
  mlx.setResolution(MLX90640_ADC_18BIT);  // 16BIT, 17BIT, 18BIT or 19BIT
  mlx_refresh();

  // Start tasks
#if ENA_MULTITASKING
  void task_setup(void (*task1)(uint8_t), void (*task2)(uint8_t, uint32_t, uint32_t));
  task_setup(ProcessInput, ProcessOutput);
#endif
}

void loop() {
#if ENA_MULTITASKING
#if defined (ESP32) && false
  DBG_EXEC(printf("Total heap: %d\n", ESP.getHeapSize()));
  DBG_EXEC(printf("Free  heap: %d\n", ESP.getFreeHeap()));
  DBG_EXEC(printf("Total PSRAM: %d\n",ESP.getPsramSize()));
  DBG_EXEC(printf("Free  PSRAM: %d\n",ESP.getFreePsram()));
#endif
  delay(1000);
#else
  uint32_t inputStart = millis();
  ProcessInput(0); // always use bank 0
  ProcessOutput(0, inputStart, millis());
#endif
}