/*
  Adapted from the Adafruit and Xark's PDQ graphicstest sketch.

  See end of file for original header text and MIT license info.
*/
#include "spi_assign.h"

#if 0

/*=============================================================
 * Adafruit GFX Library
 * https://github.com/adafruit/Adafruit-GFX-Library
 *=============================================================*/
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include "colors.h"

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

#define GFX_EXEC(x) tft.x
#define GFX_TITLE   F("Adafruit GFX PDQ")
#define GFX_FILE    "benchmark_Adafruit_GFX"

void gfx_setup(void) {
  GFX_EXEC(init(TFT_WIDTH, TFT_HEIGHT, SPI_MODE));
  GFX_EXEC(setRotation(2));
  GFX_EXEC(invertDisplay(false));

#if defined (ARDUINO_XIAO_ESP32S3)
  GFX_EXEC(setSPISpeed(SPI_FREQUENCY));
#endif
}

#elif 0

/*=============================================================
 * Arduino GFX Library
 * https://github.com/moononournation/Arduino_GFX
 *=============================================================*/
#include <Arduino_GFX_Library.h>

Arduino_DataBus *bus = new Arduino_ESP32SPI(TFT_DC /* DC */, TFT_CS /* CS */, TFT_SCLK /* SCK */, TFT_MOSI /* MOSI */, TFT_MISO /* MISO */);
Arduino_GFX *gfx = new Arduino_ST7789(bus, TFT_RST, 0 /* rotation */, true /* IPS */);

#define GFX_EXEC(x) gfx->x
#define GFX_TITLE   F("Arduino GFX PDQ")
#define GFX_FILE    "benchmark_Arduino_GFX"

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
  GFX_EXEC(invertDisplay(true));
}

#elif 1

/*=============================================================
 * LovyanGFX Library
 * https://github.com/lovyan03/LovyanGFX
 *=============================================================*/
#include <LovyanGFX.hpp>
#include "LGFX_XIAO_ESP32S3_ST7789.hpp"
#include "colors.h"

LGFX lcd;

#define GFX_EXEC(x) lcd.x
#define GFX_TITLE   F("Lovyan GFX PDQ")
#define GFX_FILE    "benchmark_LovyanGFX"

void gfx_setup(void) {
  GFX_EXEC(init());
  GFX_EXEC(setRotation(0));
  GFX_EXEC(setColorDepth(16));
}

#else

/*=============================================================
 * TFT_eSPI Library
 * https://github.com/Bodmer/TFT_eSPI
 *=============================================================*/
#include "SPI.h"
#include "TFT_eSPI.h"
#include "colors.h"

// Use hardware SPI
TFT_eSPI tft = TFT_eSPI();

#define GFX_EXEC(x) tft.x
#define GFX_TITLE   F("TFT_eSPI GFX PDQ")
#define GFX_FILE    "benchmark_TFT_eSPI"

void gfx_setup(void) {
  tft.init();
}

#endif

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

#ifdef ESP32
#undef F
#define F(s) (s)
#endif

int32_t w, h, n, n1, cx, cy, cx1, cy1, cn, cn1;
uint8_t tsa, tsb, tsc, ds;

void setup()
{
  Serial.begin(115200);
  // Serial.setDebugOutput(true);
  // while(!Serial);
  Serial.println(String(GFX_TITLE) + String(F("graphicstest example!")));

#ifdef GFX_EXTRA_PRE_INIT
  GFX_EXTRA_PRE_INIT();
#endif

  gfx_setup();
  touch_setup();
  sdcard_setup();

#ifdef GFX_BL
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
#endif

  w = GFX_EXEC(width());
  h = GFX_EXEC(height());
  n = min(w, h);
  n1 = n - 1;
  cx = w / 2;
  cy = h / 2;
  cx1 = cx - 1;
  cy1 = cy - 1;
  cn = min(cx1, cy1);
  cn1 = cn - 1;
  tsa = ((w <= 176) || (h <= 160)) ? 1 : (((w <= 240) || (h <= 240)) ? 2 : 3); // text size A
  tsb = ((w <= 272) || (h <= 220)) ? 1 : 2;                                    // text size B
  tsc = ((w <= 220) || (h <= 220)) ? 1 : 2;                                    // text size C
  ds = (w <= 160) ? 9 : ((w <= 280) ? 10 : 12);                                // digit size
}

void loop(void)
{
  void DoBenchmark();
  DoBenchmark();

 /*=============================================================
  * SD Card library
  * ToDo: Save image to the SD card.
  *=============================================================*/
  uint32_t start = millis();
  while (millis() - start < 10 * 1000L) {
    if (touch_check()) {
      sdcard_save(GFX_FILE);
    }
  }
}

/*=============================================================
 * Benchmark
 * cf. https://github.com/moononournation/Arduino_GFX/
 *=============================================================*/
static inline uint32_t micros_start() __attribute__((always_inline));
static inline uint32_t micros_start()
{
  uint8_t oms = millis();
  while ((uint8_t)millis() == oms)
    ;
  return micros();
}

void DoBenchmark(void) {
  Serial.println(F("Benchmark\tmicro-secs"));

  int32_t usecFillScreen = testFillScreen();
  serialOut(F("Screen fill\t"), usecFillScreen, 100, true);

  int32_t usecText = testText();
  serialOut(F("Text\t"), usecText, 3000, true);

  int32_t usecPixels = testPixels();
  serialOut(F("Pixels\t"), usecPixels, 100, true);

  int32_t usecLines = testLines();
  serialOut(F("Lines\t"), usecLines, 100, true);

  int32_t usecFastLines = testFastLines();
  serialOut(F("Horiz/Vert Lines\t"), usecFastLines, 100, true);

  int32_t usecFilledRects = testFilledRects();
  serialOut(F("Rectangles (filled)\t"), usecFilledRects, 100, false);

  int32_t usecRects = testRects();
  serialOut(F("Rectangles (outline)\t"), usecRects, 100, true);

  int32_t usecFilledTrangles = testFilledTriangles();
  serialOut(F("Triangles (filled)\t"), usecFilledTrangles, 100, false);

  int32_t usecTriangles = testTriangles();
  serialOut(F("Triangles (outline)\t"), usecTriangles, 100, true);

  int32_t usecFilledCircles = testFilledCircles(10);
  serialOut(F("Circles (filled)\t"), usecFilledCircles, 100, false);

  int32_t usecCircles = testCircles(10);
  serialOut(F("Circles (outline)\t"), usecCircles, 100, true);

  int32_t usecFilledArcs = testFillArcs();
  serialOut(F("Arcs (filled)\t"), usecFilledArcs, 100, false);

  int32_t usecArcs = testArcs();
  serialOut(F("Arcs (outline)\t"), usecArcs, 100, true);

  int32_t usecFilledRoundRects = testFilledRoundRects();
  serialOut(F("Rounded rects (filled)\t"), usecFilledRoundRects, 100, false);

  int32_t usecRoundRects = testRoundRects();
  serialOut(F("Rounded rects (outline)\t"), usecRoundRects, 100, true);

#ifdef CANVAS
  uint32_t start = micros_start();
  GFX_EXEC(flush());
  int32_t usecFlush = micros() - start;
  serialOut(F("flush (Canvas only)\t"), usecFlush, 0, false);
#endif

  Serial.println(F("Done!"));

  uint16_t c = 4;
  int8_t d = 1;
  for (int32_t i = 0; i < h; i++)
  {
    GFX_EXEC(drawFastHLine(0, i, w, c));
    c += d;
    if (c <= 4 || c >= 11)
    {
      d = -d;
    }
  }

  GFX_EXEC(setCursor(0, 0));

  GFX_EXEC(setTextSize(tsa));
  GFX_EXEC(setTextColor(MAGENTA));
  GFX_EXEC(println(GFX_TITLE));

  if (h > w)
  {
    GFX_EXEC(setTextSize(tsb));
    GFX_EXEC(setTextColor(GREEN));
    GFX_EXEC(print(F("\nBenchmark ")));
    GFX_EXEC(setTextSize(tsc));
    if (ds == 12)
    {
      GFX_EXEC(print(F("   ")));
    }
    GFX_EXEC(println(F(" micro-secs")));
  }

  printnice(F("Screen fill "), usecFillScreen);
  printnice(F("Text        "), usecText);
  printnice(F("Pixels      "), usecPixels);
  printnice(F("Lines       "), usecLines);
  printnice(F("H/V Lines   "), usecFastLines);
  printnice(F("Rectangles F"), usecFilledRects);
  printnice(F("Rectangles  "), usecRects);
  printnice(F("Triangles F "), usecFilledTrangles);
  printnice(F("Triangles   "), usecTriangles);
  printnice(F("Circles F   "), usecFilledCircles);
  printnice(F("Circles     "), usecCircles);
  printnice(F("Arcs F      "), usecFilledArcs);
  printnice(F("Arcs        "), usecArcs);
  printnice(F("RoundRects F"), usecFilledRoundRects);
  printnice(F("RoundRects  "), usecRoundRects);

  if ((h > w) || (h > 240))
  {
    GFX_EXEC(setTextSize(tsc));
    GFX_EXEC(setTextColor(GREEN));
    GFX_EXEC(print(F("\nBenchmark Complete!")));
  }

#ifdef CANVAS
  GFX_EXEC(flush());
#endif
}

#ifdef ESP32
void serialOut(const char *item, int32_t v, uint32_t d, bool clear)
#else
void serialOut(const __FlashStringHelper *item, int32_t v, uint32_t d, bool clear)
#endif
{
#ifdef CANVAS
  GFX_EXEC(flush());
#endif
  Serial.print(item);
  if (v < 0)
  {
    Serial.println(F("N/A"));
  }
  else
  {
    Serial.println(v);
  }
  delay(d);
  if (clear)
  {
    GFX_EXEC(fillScreen(BLACK));
  }
}

#ifdef ESP32
void printnice(const char *item, long int v)
#else
void printnice(const __FlashStringHelper *item, long int v)
#endif
{
  GFX_EXEC(setTextSize(tsb));
  GFX_EXEC(setTextColor(CYAN));
  GFX_EXEC(print(item));

  GFX_EXEC(setTextSize(tsc));
  GFX_EXEC(setTextColor(YELLOW));
  if (v < 0)
  {
    GFX_EXEC(println(F("      N / A")));
  }
  else
  {
    char str[32] = {0};
#ifdef RTL8722DM
    sprintf(str, "%d", (int)v);
#else
    sprintf(str, "%ld", v);
#endif
    for (char *p = (str + strlen(str)) - 3; p > str; p -= 3)
    {
      memmove(p + 1, p, strlen(p) + 1);
      *p = ',';
    }
    while (strlen(str) < ds)
    {
      memmove(str + 1, str, strlen(str) + 1);
      *str = ' ';
    }
    GFX_EXEC(println(str));
  }
}

int32_t testFillScreen()
{
  uint32_t start = micros_start();
  // Shortened this tedious test!
  GFX_EXEC(fillScreen(WHITE));
  GFX_EXEC(fillScreen(RED));
  GFX_EXEC(fillScreen(GREEN));
  GFX_EXEC(fillScreen(BLUE));
  GFX_EXEC(fillScreen(BLACK));

  return micros() - start;
}

int32_t testText()
{
  uint32_t start = micros_start();
  GFX_EXEC(setCursor(0, 0));

  GFX_EXEC(setTextSize(1));
  GFX_EXEC(setTextColor(WHITE, BLACK));
  GFX_EXEC(println(F("Hello World!")));

  GFX_EXEC(setTextSize(2));
  GFX_EXEC(setTextColor(GFX_EXEC(color565(0xff, 0x00, 0x00))));
  GFX_EXEC(print(F("RED ")));
  GFX_EXEC(setTextColor(GFX_EXEC(color565(0x00, 0xff, 0x00))));
  GFX_EXEC(print(F("GREEN ")));
  GFX_EXEC(setTextColor(GFX_EXEC(color565(0x00, 0x00, 0xff))));
  GFX_EXEC(println(F("BLUE")));

  GFX_EXEC(setTextSize(tsa));
  GFX_EXEC(setTextColor(YELLOW));
  GFX_EXEC(println(1234.56));

  GFX_EXEC(setTextColor(WHITE));
  GFX_EXEC(println((w > 128) ? 0xDEADBEEF : 0xDEADBEE, HEX));

  GFX_EXEC(setTextColor(CYAN, WHITE));
  GFX_EXEC(println(F("Groop,")));

  GFX_EXEC(setTextSize(tsc));
  GFX_EXEC(setTextColor(MAGENTA, WHITE));
  GFX_EXEC(println(F("I implore thee,")));

  GFX_EXEC(setTextSize(1));
  GFX_EXEC(setTextColor(NAVY, WHITE));
  GFX_EXEC(println(F("my foonting turlingdromes.")));

  GFX_EXEC(setTextColor(DARKGREEN, WHITE));
  GFX_EXEC(println(F("And hooptiously drangle me")));

  GFX_EXEC(setTextColor(DARKCYAN, WHITE));
  GFX_EXEC(println(F("with crinkly bindlewurdles,")));

  GFX_EXEC(setTextColor(MAROON, WHITE));
  GFX_EXEC(println(F("Or I will rend thee")));

  GFX_EXEC(setTextColor(PURPLE, WHITE));
  GFX_EXEC(println(F("in the gobberwartsb")));

  GFX_EXEC(setTextColor(OLIVE, WHITE));
  GFX_EXEC(println(F("with my blurglecruncheon,")));

  GFX_EXEC(setTextColor(DARKGREY, WHITE));
  GFX_EXEC(println(F("see if I don't!")));

  GFX_EXEC(setTextSize(2));
  GFX_EXEC(setTextColor(RED));
  GFX_EXEC(println(F("Size 2")));

  GFX_EXEC(setTextSize(3));
  GFX_EXEC(setTextColor(ORANGE));
  GFX_EXEC(println(F("Size 3")));

  GFX_EXEC(setTextSize(4));
  GFX_EXEC(setTextColor(YELLOW));
  GFX_EXEC(println(F("Size 4")));

  GFX_EXEC(setTextSize(5));
  GFX_EXEC(setTextColor(GREENYELLOW));
  GFX_EXEC(println(F("Size 5")));

  GFX_EXEC(setTextSize(6));
  GFX_EXEC(setTextColor(GREEN));
  GFX_EXEC(println(F("Size 6")));

  GFX_EXEC(setTextSize(7));
  GFX_EXEC(setTextColor(BLUE));
  GFX_EXEC(println(F("Size 7")));

  GFX_EXEC(setTextSize(8));
  GFX_EXEC(setTextColor(PURPLE));
  GFX_EXEC(println(F("Size 8")));

  GFX_EXEC(setTextSize(9));
  GFX_EXEC(setTextColor(PALERED));
  GFX_EXEC(println(F("Size 9")));

  return micros() - start;
}

int32_t testPixels()
{
  uint32_t start = micros_start();

  for (int16_t y = 0; y < h; y++)
  {
    for (int16_t x = 0; x < w; x++)
    {
      GFX_EXEC(drawPixel(x, y, GFX_EXEC(color565(x << 3, y << 3, x * y))));
    }
#ifdef ESP8266
    yield(); // avoid long run triggered ESP8266 WDT restart
#endif
  }

  return micros() - start;
}

int32_t testLines()
{
  uint32_t start;
  int32_t x1, y1, x2, y2;

  start = micros_start();

  x1 = y1 = 0;
  y2 = h - 1;
  for (x2 = 0; x2 < w; x2 += 6)
  {
    GFX_EXEC(drawLine(x1, y1, x2, y2, BLUE));
  }
#ifdef ESP8266
  yield(); // avoid long run triggered ESP8266 WDT restart
#endif

  x2 = w - 1;
  for (y2 = 0; y2 < h; y2 += 6)
  {
    GFX_EXEC(drawLine(x1, y1, x2, y2, BLUE));
  }
#ifdef ESP8266
  yield(); // avoid long run triggered ESP8266 WDT restart
#endif

  x1 = w - 1;
  y1 = 0;
  y2 = h - 1;
  for (x2 = 0; x2 < w; x2 += 6)
  {
    GFX_EXEC(drawLine(x1, y1, x2, y2, BLUE));
  }
#ifdef ESP8266
  yield(); // avoid long run triggered ESP8266 WDT restart
#endif

  x2 = 0;
  for (y2 = 0; y2 < h; y2 += 6)
  {
    GFX_EXEC(drawLine(x1, y1, x2, y2, BLUE));
  }
#ifdef ESP8266
  yield(); // avoid long run triggered ESP8266 WDT restart
#endif

  x1 = 0;
  y1 = h - 1;
  y2 = 0;
  for (x2 = 0; x2 < w; x2 += 6)
  {
    GFX_EXEC(drawLine(x1, y1, x2, y2, BLUE));
  }
#ifdef ESP8266
  yield(); // avoid long run triggered ESP8266 WDT restart
#endif

  x2 = w - 1;
  for (y2 = 0; y2 < h; y2 += 6)
  {
    GFX_EXEC(drawLine(x1, y1, x2, y2, BLUE));
  }
#ifdef ESP8266
  yield(); // avoid long run triggered ESP8266 WDT restart
#endif

  x1 = w - 1;
  y1 = h - 1;
  y2 = 0;
  for (x2 = 0; x2 < w; x2 += 6)
  {
    GFX_EXEC(drawLine(x1, y1, x2, y2, BLUE));
  }
#ifdef ESP8266
  yield(); // avoid long run triggered ESP8266 WDT restart
#endif

  x2 = 0;
  for (y2 = 0; y2 < h; y2 += 6)
  {
    GFX_EXEC(drawLine(x1, y1, x2, y2, BLUE));
  }
#ifdef ESP8266
  yield(); // avoid long run triggered ESP8266 WDT restart
#endif

  return micros() - start;
}

int32_t testFastLines()
{
  uint32_t start;
  int32_t x, y;

  start = micros_start();

  for (y = 0; y < h; y += 5)
  {
    GFX_EXEC(drawFastHLine(0, y, w, RED));
  }
  for (x = 0; x < w; x += 5)
  {
    GFX_EXEC(drawFastVLine(x, 0, h, BLUE));
  }

  return micros() - start;
}

int32_t testFilledRects()
{
  uint32_t start;
  int32_t i, i2;

  start = micros_start();

  for (i = n; i > 0; i -= 6)
  {
    i2 = i / 2;

    GFX_EXEC(fillRect(cx - i2, cy - i2, i, i, GFX_EXEC(color565(i, i, 0))));
  }

  return micros() - start;
}

int32_t testRects()
{
  uint32_t start;
  int32_t i, i2;

  start = micros_start();
  for (i = 2; i < n; i += 6)
  {
    i2 = i / 2;
    GFX_EXEC(drawRect(cx - i2, cy - i2, i, i, GREEN));
  }

  return micros() - start;
}

int32_t testFilledCircles(uint8_t radius)
{
  uint32_t start;
  int32_t x, y, r2 = radius * 2;

  start = micros_start();

  for (x = radius; x < w; x += r2)
  {
    for (y = radius; y < h; y += r2)
    {
      GFX_EXEC(fillCircle(x, y, radius, MAGENTA));
    }
  }

  return micros() - start;
}

int32_t testCircles(uint8_t radius)
{
  uint32_t start;
  int32_t x, y, r2 = radius * 2;
  int32_t w1 = w + radius;
  int32_t h1 = h + radius;

  // Screen is not cleared for this one -- this is
  // intentional and does not affect the reported time.
  start = micros_start();

  for (x = 0; x < w1; x += r2)
  {
    for (y = 0; y < h1; y += r2)
    {
      GFX_EXEC(drawCircle(x, y, radius, WHITE));
    }
  }

  return micros() - start;
}

int32_t testFillArcs()
{
  uint32_t start = micros_start();

#if !defined(_ADAFRUIT_GFX_H) && !defined(_TFT_eSPIH_)
  int16_t i, r = 360 / cn;
  for (i = 6; i < cn; i += 6)
  {
    GFX_EXEC(fillArc(cx1, cy1, i, i - 3, 0, i * r, RED));
  }
#endif

  return micros() - start;
}

int32_t testArcs()
{
  uint32_t start = micros_start();

#if !defined(_ADAFRUIT_GFX_H) && !defined(_TFT_eSPIH_)
  int16_t i, r = 360 / cn;
  for (i = 6; i < cn; i += 6)
  {
    GFX_EXEC(drawArc(cx1, cy1, i, i - 3, 0, i * r, WHITE));
  }
#endif

  return micros() - start;
}

int32_t testFilledTriangles()
{
  uint32_t start;
  int32_t i;

  start = micros_start();

  for (i = cn1; i > 10; i -= 5)
  {
    GFX_EXEC(fillTriangle(cx1, cy1 - i, cx1 - i, cy1 + i, cx1 + i, cy1 + i,
                      GFX_EXEC(color565(0, i, i))));
  }

  return micros() - start;
}

int32_t testTriangles()
{
  uint32_t start;
  int32_t i;

  start = micros_start();

  for (i = 0; i < cn; i += 5)
  {
    GFX_EXEC(drawTriangle(
        cx1, cy1 - i,     // peak
        cx1 - i, cy1 + i, // bottom left
        cx1 + i, cy1 + i, // bottom right
        GFX_EXEC(color565(0, 0, i))));
  }

  return micros() - start;
}

int32_t testFilledRoundRects()
{
  uint32_t start;
  int32_t i, i2;

  start = micros_start();

  for (i = n1; i > 20; i -= 6)
  {
    i2 = i / 2;
    GFX_EXEC(fillRoundRect(cx - i2, cy - i2, i, i, i / 8, GFX_EXEC(color565(0, i, 0))));
  }

  return micros() - start;
}

int32_t testRoundRects()
{
  uint32_t start;
  int32_t i, i2;

  start = micros_start();

  for (i = 20; i < n1; i += 6)
  {
    i2 = i / 2;
    GFX_EXEC(drawRoundRect(cx - i2, cy - i2, i, i, i / 8, GFX_EXEC(color565(i, 0, 0))));
  }

  return micros() - start;
}

/***************************************************
  Original sketch text:

  This is an example sketch for the Adafruit 2.2" SPI display.
  This library works with the Adafruit 2.2" TFT Breakout w/SD card
  ----> http://www.adafruit.com/products/1480

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/