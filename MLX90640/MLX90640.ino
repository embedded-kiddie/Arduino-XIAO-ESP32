#include <SPI.h>
#include <Adafruit_MLX90640.h>
#include "spi_assign.h"
#include "colors.h"

#if 1
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
#define TFT_ORIGIN  1
#define TFT_INVERT  false
#define TFT_INIT()  { tft.init(TFT_WIDTH, TFT_HEIGHT, SPI_MODE); tft.setSPISpeed(SPI_FREQUENCY); touch_setup(); }
#else
#include <LovyanGFX.hpp>
#include "LGFX_XIAO_ESP32S3_ST7789.hpp"
LGFX_XIAO_ESP32S3_ST7789 tft;
lgfx::touch_point_t tp;
#define TFT_ORIGIN  3
#define TFT_INVERT  false
#define TFT_INIT()  { tft.init(); }
#endif

#define ClearScreen() tft.fillScreen(BLACK)

// 2.4 inch ... "RESET" on breakout board can be connected to "RESET" or +3.3V on UNO R4 instead of D9.

// Font size for setTextSize(2)
#define FONT_WIDTH    12 // [px] (Device coordinate system)
#define FONT_HEIGHT   16 // [px] (Device coordinate system)

#define MLX90640_COLS 32
#define MLX90640_ROWS 24

#define MINTEMP 20  // Low range of the sensor (this will be blue on the screen)
#define MAXTEMP 35  // high range of the sensor (this will be red on the screen)

// Definitoins for Interpolation
#define USE_INTERPOLATION true

#if     USE_INTERPOLATION
#define INTERPOLATE_SCALE 2
#define BOX_SIZE          4
#else
#define INTERPOLATE_SCALE 1
#define BOX_SIZE          8
#endif

#define INTERPOLATED_COLS (MLX90640_COLS * INTERPOLATE_SCALE)
#define INTERPOLATED_ROWS (MLX90640_ROWS * INTERPOLATE_SCALE)

// Global variables
Adafruit_MLX90640 mlx;
float src[MLX90640_ROWS     * MLX90640_COLS    ];
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

#include "Interpolation.h"

void TFT_Printf(uint16_t x, uint16_t y, const char* fmt, ...) {
  int len = 0;
  char buf[16];

  va_list arg_ptr;
  va_start(arg_ptr, fmt);
  len = vsnprintf(buf, sizeof(buf), fmt, arg_ptr);
  va_end(arg_ptr);

  tft.fillRect(x, y, len * FONT_WIDTH, FONT_HEIGHT, BLACK);
  tft.setCursor(x, y);
  tft.print(buf);
}

extern bool touch_setup(void);
extern bool touch_loop(void);
extern void sd_setup(void);
extern bool sd_loop(void);

void setup() {
  Serial.begin(115200);

  // Initialize ST7789
  TFT_INIT();
  tft.setRotation(TFT_ORIGIN);
  tft.invertDisplay(TFT_INVERT);
  tft.setTextColor(WHITE);
  ClearScreen();
 
  // Draw color bar
  const int n = sizeof(camColors) / sizeof(camColors[0]);
  const int w = BOX_SIZE * INTERPOLATED_COLS;
  int       y = BOX_SIZE * INTERPOLATED_ROWS + 3;
  for (int i = 0; i < n; i++) {
    int x = map(i, 0, n, 0, w);
    tft.fillRect(x, y, 1, FONT_HEIGHT - 4, camColors[i]);
  }

  y += FONT_HEIGHT;
  tft.setTextSize(2);
  TFT_Printf(0,                      y, "%d", MINTEMP);
  TFT_Printf(w / 2 - FONT_WIDTH * 2, y, "%3.1f", (float)(MINTEMP + MAXTEMP) / 2.0f);
  TFT_Printf(w     - FONT_WIDTH * 2, y, "%d", MAXTEMP);

  tft.setTextSize(1);
  TFT_Printf(260 + FONT_WIDTH * 4, FONT_HEIGHT / 2,         "Hz");
  TFT_Printf(260 + FONT_WIDTH * 4, FONT_HEIGHT / 2 * 3 + 2, "'C");
  tft.setTextSize(2);

  delay(100);

  Serial.println("Adafruit MLX90640 Camera");
  if (! mlx.begin(MLX90640_I2CADDR_DEFAULT, &Wire)) {
    Serial.println("MLX90640 not found!");
  }
  Serial.println("Found Adafruit MLX90640");

  Serial.print("Serial number: ");
  Serial.print(mlx.serialNumber[0], HEX);
  Serial.print(mlx.serialNumber[1], HEX);
  Serial.println(mlx.serialNumber[2], HEX);

  // MLX90640
  mlx.setMode(MLX90640_CHESS);
//mlx.setResolution(MLX90640_ADC_16BIT);
  mlx.setResolution(MLX90640_ADC_18BIT);
//mlx.setResolution(MLX90640_ADC_19BIT);
//mlx.setRefreshRate(MLX90640_8_HZ); // 8 FPS
  mlx.setRefreshRate(MLX90640_16_HZ); // 16 FPS
//mlx.setRefreshRate(MLX90640_32_HZ); // 32 FPS

  // I2C Clock
//Wire.setClock(400000); // 400 KHz (Sm)
  Wire.setClock(1000000); // 1 MHz (Fm+)

  // Interpolation
  setup_interpolate(INTERPOLATED_ROWS, INTERPOLATED_COLS, INTERPOLATE_SCALE);
}

void loop() {
#if defined(_ADAFRUIT_GFX_H)
  if (touch_loop()) {
    sd_loop();
  }
#else
  if (tft.getTouch(&tp)) {
    sd_loop();
  }
#endif

  uint32_t timestamp = millis();
  if (mlx.getFrame(src) != 0) {
    TFT_Printf(TFT_WIDTH / 2 - FONT_WIDTH * 3, TFT_HEIGHT / 2 - FONT_HEIGHT * 5, "Failed");
    Serial.println("Failed");
    delay(1000); // false = no new frame capture
    return;
  }

#if USE_INTERPOLATION
  interpolate_image(src, MLX90640_ROWS, MLX90640_COLS, dst, INTERPOLATED_ROWS, INTERPOLATED_COLS);
#endif

#if 1
  for (int h = 0; h < INTERPOLATED_ROWS; h++) {
    for (int w = 0; w < INTERPOLATED_COLS; w++) {
#if USE_INTERPOLATION
      float t = dst[h * INTERPOLATED_COLS + w];
#else
      float t = src[h * INTERPOLATED_COLS + w];
#endif
      t = min((int)t, MAXTEMP);
      t = max((int)t, MINTEMP); 

      int colorIndex = map(t, MINTEMP, MAXTEMP, 0, 255);
      colorIndex = constrain(colorIndex, 0, 255);

#if 0
      // Selfie
      tft.fillRect(BOX_SIZE * w, BOX_SIZE * h, BOX_SIZE, BOX_SIZE, camColors[colorIndex]);
#else
      // Front
#if BOX_SIZE == 1
      tft.drawPixel(INTERPOLATED_COLS - 1 - w, h, camColors[colorIndex]);
#else
      tft.fillRect(BOX_SIZE * (INTERPOLATED_COLS - 1 - w), BOX_SIZE * h, BOX_SIZE, BOX_SIZE, camColors[colorIndex]);
#endif
#endif
    }
  }
#endif

  // FPS
  float v = 2000.0f / (float)(millis() - timestamp) + 0.05f; // 2 frames per display
  TFT_Printf(260, 0, "%4.1f", v);

  // Ambient temperature
  v = mlx.getTa(false) + 0.05f;
  if (0.0f < v && v < 100.0f) {
    TFT_Printf(260, FONT_HEIGHT + 3, "%4.1f", v);
  }

  delay(33);
}