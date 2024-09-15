#if defined(_ADAFRUIT_GFX_H) || defined(_ARDUINO_GFX_LIBRARIES_H_)

// https://github.com/PaulStoffregen/XPT2046_Touchscreen/tree/master/examples/TouchTestIRQ
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include "spi_assign.h"

// Use no interrupts when using the SD library
XPT2046_Touchscreen ts(TOUCH_CS);  // Param 2 - NULL - No interrupts
//XPT2046_Touchscreen ts(TOUCH_CS, 255);  // Param 2 - 255 - No interrupts
//XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);  // Param 2 - Touch IRQ Pin - interrupt enabled polling

bool touch_setup() {
  if (ts.begin()) {
    ts.setRotation(3);
    return true;
  } else {
    return false;
  }
}

bool touch_loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    Serial.print("Pressure = ");
    Serial.print(p.z);
    Serial.print(", x = ");
    Serial.print(p.x);
    Serial.print(", y = ");
    Serial.print(p.y);
    delay(30);
    Serial.println();
    return true;
  } else {
    return false;
  }
}

#else // LovyanGFX || TFT_eSPI

bool touch_setup() {
  return true;
}

bool touch_loop() {
  uint16_t x, y;
  if (GFX_EXEC(getTouch(&x, &y))) {
    Serial.println("x = " + String(x) + ", y = " + String(y));
    return true;
  } else {
    return false;
  }
}

#endif  // _ADAFRUIT_GFX_H || _ARDUINO_GFX_LIBRARIES_H_