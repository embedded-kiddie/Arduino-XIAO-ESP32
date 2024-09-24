#include <SPI.h>
#include <Adafruit_MLX90640.h>
#include "spi_assign.h"

/*=============================================================
 * Step 1: Select GFX Library
 *=============================================================*/
#if 0
/*---------------------------------------------------
 * Adafruit GFX Library
 * https://github.com/adafruit/Adafruit-GFX-Library
 *---------------------------------------------------*/
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include "colors.h"

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

#define GFX_EXEC(x) tft.x

void gfx_setup(void) {
  GFX_EXEC(init(TFT_WIDTH, TFT_HEIGHT, SPI_MODE));
  GFX_EXEC(setRotation(1));
  GFX_EXEC(invertDisplay(false));

#if defined (ARDUINO_XIAO_ESP32S3)
  GFX_EXEC(setSPISpeed(SPI_FREQUENCY));
#endif
}

#elif 0
/*---------------------------------------------------
 * Arduino GFX Library
 * https://github.com/moononournation/Arduino_GFX
 *---------------------------------------------------*/
#include <Arduino_GFX_Library.h>

Arduino_DataBus *bus = new Arduino_ESP32SPI(TFT_DC /* DC */, TFT_CS /* CS */, TFT_SCLK /* SCK */, TFT_MOSI /* MOSI */, TFT_MISO /* MISO */);
Arduino_GFX *gfx = new Arduino_ST7789(bus, TFT_RST, 0 /* rotation */, true /* IPS */);

#define GFX_EXEC(x) gfx->x

void gfx_setup(void) {
  // Init Display
#if defined (ARDUINO_XIAO_ESP32S3)
  if (!GFX_EXEC(begin(SPI_FREQUENCY))) /* specify data bus speed */
#else
  if (!GFX_EXEC(begin())
#endif
  {
    Serial.println("gfx->begin() failed!");
  }

  SPI.setDataMode(SPI_MODE);
  GFX_EXEC(setRotation(3));
  GFX_EXEC(invertDisplay(true));
}

#elif 1
/*---------------------------------------------------
 * LovyanGFX Library
 * https://github.com/lovyan03/LovyanGFX
 *---------------------------------------------------*/
#include <LovyanGFX.hpp>
#include "LGFX_XIAO_ESP32S3_ST7789.hpp"
#include "colors.h"

LGFX lcd;

#define GFX_EXEC(x) lcd.x

void gfx_setup(void) {
  GFX_EXEC(init());
  GFX_EXEC(setRotation(3));
  GFX_EXEC(invertDisplay(false));
}

#else
/*---------------------------------------------------
 * TFT_eSPI Library
 * https://github.com/Bodmer/TFT_eSPI
 *---------------------------------------------------*/
#include "SPI.h"
#include "TFT_eSPI.h"
#include "colors.h"

// Use hardware SPI
TFT_eSPI tft = TFT_eSPI();

#define GFX_EXEC(x) tft.x

void gfx_setup(void) {
  GFX_EXEC(init());
  GFX_EXEC(setRotation(3));
}
#endif

/*=============================================================
 * Step 2: Select whether to multitask or not
 *=============================================================*/
#define ENA_MULTITASKING  true

/*=============================================================
 * Step 3: Select whether to interpolate or not
 *=============================================================*/
#define ENA_INTERPOLATION true

/*=============================================================
 * Step 4: Select whether to enable transaction or not
 *=============================================================*/
#if defined(LOVYANGFX_HPP_) || defined(_TFT_eSPIH_)
#define ENA_TRANSACTION   true
#else
#define ENA_TRANSACTION   false
#endif

/*=============================================================
 * Step 5: Configure the output image resolution
 *=============================================================*/
// INTERPOLATE_SCALE x BOX_SIZE = 8
#if ENA_INTERPOLATION
#define INTERPOLATE_SCALE 4
#define BOX_SIZE          2
#else
#define INTERPOLATE_SCALE 1
#define BOX_SIZE          8
#endif

#define MLX90640_COLS 32
#define MLX90640_ROWS 24

// The size of thermal image
#define INTERPOLATED_COLS (MLX90640_COLS * INTERPOLATE_SCALE)
#define INTERPOLATED_ROWS (MLX90640_ROWS * INTERPOLATE_SCALE)

#define MINTEMP 20  // Low range of the sensor (this will be blue on the screen)
#define MAXTEMP 35  // high range of the sensor (this will be red on the screen)

/*=============================================================
 * Touch library
 * This should be included after GFX_EXEC() definition
 *=============================================================*/
#include "touch.hpp"

/*=============================================================
 * SD Card library
 * This should be included after GFX_EXEC() definition
 *=============================================================*/
#include "sdcard.hpp"

/*=============================================================
 * Interpolation
 * This should be included after GFX_EXEC() definition
 *=============================================================*/
#include "interpolation.hpp"

/*=============================================================
 * Global variables
 *=============================================================*/
Adafruit_MLX90640 mlx;
float src[2][MLX90640_ROWS  * MLX90640_COLS    ];
float dst[INTERPOLATED_ROWS * INTERPOLATED_COLS];

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

/*=============================================================
 * Definitions of graphics helpers
 *=============================================================*/
// Font size for setTextSize(2)
#define FONT_WIDTH    12 // [px] (Device coordinate system)
#define FONT_HEIGHT   16 // [px] (Device coordinate system)
#define LINE_HEIGHT   18 // [px] (FONT_HEIGHT + margin)

#define ClearScreen() GFX_EXEC(fillScreen(BLACK))

void gfx_printf(uint16_t x, uint16_t y, const char* fmt, ...) {
  int len = 0;
  char buf[16];

  va_list arg_ptr;
  va_start(arg_ptr, fmt);
  len = vsnprintf(buf, sizeof(buf), fmt, arg_ptr);
  va_end(arg_ptr);

  GFX_EXEC(fillRect(x, y, len * FONT_WIDTH, FONT_HEIGHT, BLACK));
  GFX_EXEC(setCursor(x, y));
  GFX_EXEC(print(buf));
}

/*=============================================================
 * Input process
 * Get thermal image from MLX90640
 *=============================================================*/
void ProcessInput(uint8_t bank) {
  if (mlx.getFrame(src[bank]) != 0) {
    gfx_printf(TFT_WIDTH / 2 - FONT_WIDTH * 3, TFT_HEIGHT / 2 - FONT_HEIGHT * 5, "Failed");
    Serial.println("Failed");
    delay(1000); // false = no new frame capture
    return;
  }
}

/*=============================================================
 * Output process
 * Interpolate thermal image and display on LCD.
 *=============================================================*/
void ProcessOutput(uint8_t bank, uint32_t inputStart, uint32_t inputFinish) {
  static uint32_t prevFinish;
  uint32_t outputStart = millis();

#if ENA_INTERPOLATION
  interpolate_image(src[bank], MLX90640_ROWS, MLX90640_COLS, dst, INTERPOLATED_ROWS, INTERPOLATED_COLS);
  float *drw = dst;
#else
  float *drw = src[bank];
#endif

#if ENA_TRANSACTION
  GFX_EXEC(startWrite());
#endif

  for (int h = 0; h < INTERPOLATED_ROWS; h++) {
    for (int w = 0; w < INTERPOLATED_COLS; w++) {
      float t = drw[h * INTERPOLATED_COLS + w];

      t = min((int)t, MAXTEMP);
      t = max((int)t, MINTEMP); 

      int colorIndex = map(t, MINTEMP, MAXTEMP, 0, 255);
      colorIndex = constrain(colorIndex, 0, 255);

#if 0
      // Selfie Camera
      GFX_EXEC(fillRect(BOX_SIZE * w, BOX_SIZE * h, BOX_SIZE, BOX_SIZE, camColors[colorIndex]));
#else
      // Front Camera
#if BOX_SIZE == 1
      GFX_EXEC(drawPixel(INTERPOLATED_COLS - 1 - w, h, camColors[colorIndex]));
#else
      GFX_EXEC(fillRect((INTERPOLATED_COLS - 1 - w) * BOX_SIZE, h * BOX_SIZE, BOX_SIZE, BOX_SIZE, camColors[colorIndex]));
#endif
#endif
    }
  }

  // MLX90640
  gfx_printf(260 + FONT_WIDTH, LINE_HEIGHT * 3.5, "%4d", inputFinish - inputStart);

  // Interpolation
  uint32_t outputFinish = millis();
  gfx_printf(260 + FONT_WIDTH, LINE_HEIGHT * 5.0, "%4d", outputFinish - outputStart);

  // FPS
  float v = 1000.0f / (float)(outputFinish - prevFinish) + 0.05f; // 2 frames per display
  gfx_printf(260 + FONT_WIDTH, LINE_HEIGHT * 2.0, "%4.1f", v);
  prevFinish = outputFinish;

  // Ambient temperature
  v = mlx.getTa(false) + 0.05f;
  if (0.0f < v && v < 100.0f) {
    gfx_printf(260 + FONT_WIDTH, LINE_HEIGHT * 0.5, "%4.1f", v);
  }

#if ENA_TRANSACTION
  GFX_EXEC(endWrite());
#endif

  /*=============================================================
  * SD Card library
  * ToDo: Save image to the SD card.
  *=============================================================*/
  if (touch_check()) {
    sdcard_save();
  }
}

/*=============================================================
 * setup() and loop()
 *=============================================================*/
void setup() {
  Serial.begin(115200);

  // Initialize LCD display with touch and SD card
  gfx_setup();
  touch_setup();
  sdcard_setup();

  // Initialize interpolation
  interpolate_setup(INTERPOLATED_ROWS, INTERPOLATED_COLS, INTERPOLATE_SCALE);
 
  // Draw color bar
  ClearScreen();
  const int n = sizeof(camColors) / sizeof(camColors[0]);
  const int w = BOX_SIZE * INTERPOLATED_COLS;
  int       y = BOX_SIZE * INTERPOLATED_ROWS + 3;
  for (int i = 0; i < n; i++) {
    int x = map(i, 0, n, 0, w);
    GFX_EXEC(fillRect(x, y, 1, FONT_HEIGHT - 4, camColors[i]));
  }

  y += FONT_HEIGHT;
  GFX_EXEC(setTextSize(2));
  GFX_EXEC(setTextColor(WHITE));
  gfx_printf(0,                      y, "%d", MINTEMP);
  gfx_printf(w / 2 - FONT_WIDTH * 2, y, "%3.1f", (float)(MINTEMP + MAXTEMP) / 2.0f);
  gfx_printf(w     - FONT_WIDTH * 2, y, "%d", MAXTEMP);

  GFX_EXEC(setTextSize(1));
  gfx_printf(260, LINE_HEIGHT * 0.0, "Sensor['C]");
  gfx_printf(260, LINE_HEIGHT * 1.5, "FPS [Hz]");
  gfx_printf(260, LINE_HEIGHT * 3.0, "Input [ms]");
  gfx_printf(260, LINE_HEIGHT * 4.5, "Output[ms]");
  GFX_EXEC(setTextSize(2));

  if (! mlx.begin(MLX90640_I2CADDR_DEFAULT, &Wire)) {
    Serial.println("MLX90640 not found!");
  } else {
    Serial.println("MLX90640 found");
  }

  Serial.print("Serial number: ");
  Serial.print(mlx.serialNumber[0], HEX);
  Serial.print(mlx.serialNumber[1], HEX);
  Serial.println(mlx.serialNumber[2], HEX);

  // MLX90640
  mlx.setMode(MLX90640_CHESS);
  mlx.setResolution(MLX90640_ADC_18BIT);  // 16BIT, 18BIT or 18BIT
  mlx.setRefreshRate(MLX90640_16_HZ);     // 8_HZ, 16_HZ or 32_HZ

  // I2C Clock for MLX90640
  Wire.setClock(1000000); // 400 KHz (Sm) or 1 MHz (Fm+)

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
  ProcessInput(0);
  ProcessOutput(0, inputStart, millis());
#endif
}