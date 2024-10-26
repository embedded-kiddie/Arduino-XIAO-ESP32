#include <Arduino.h>
#include <SPI.h>
#include "spi_assign.h"
#include "colors.h"

#define DEBUG       true
#if     DEBUG
#define DBG_EXEC(x) {x;}
#else
#define DBG_EXEC(x)
#endif

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