/*
 * Touch event manager
 */

typedef enum {
  none      = 0x0000,
  inside    = 0x0001,
  outside   = 0x1002,
    info    = 0x1003,
    capture = 0x1004,
    config  = 0x1005,
    colors  = 0x1006,
    range   = 0x1007,
    apply   = 0x1008,
} TouchEvent_t;

typedef struct {
  TouchEvent_t  event;
  int16_t       x, y;
} EventPoint_t;

typedef struct {
  TouchEvent_t  event;
  int16_t       x[2];
  int16_t       y[2];
  void          (*callback)(EventPoint_t &ep);
} EventRegion_t;

void onInside (EventPoint_t &ep);
void onInfo   (EventPoint_t &ep);
void onCapture(EventPoint_t &ep);
void onConfig (EventPoint_t &ep);
void onColors (EventPoint_t &ep);
void onRange  (EventPoint_t &ep);
void onApply  (EventPoint_t &ep);

static const EventRegion_t region[] {
  {inside,    0, 255,   0, 192, onInside },
  {info,    256, 319,   0, 118, onInfo   },
  {capture, 260, 310, 136, 164, onCapture},
  {config,  260, 310, 180, 228, onConfig },
  {colors,    0, 255, 195, 210, onColors },
  {range,     0, 255, 212, 239, onRange  },
  {apply,   256, 319, 195, 239, onApply  },
};

#define N_EVENT_REGION  (sizeof(region) / sizeof(region[0]))

void onInside (EventPoint_t &ep) {
  DBG_EXEC(printf("onInside\n"));
}

void onInfo   (EventPoint_t &ep) {
  DBG_EXEC(printf("onInfo\n"));
}

void onCapture(EventPoint_t &ep) {
  DBG_EXEC(printf("onCapture\n"));
  extern bool sdcard_save(void);
  sdcard_save();
}

void onConfig (EventPoint_t &ep) {
  DBG_EXEC(printf("onConfig\n"));
}

void onColors (EventPoint_t &ep) {
  DBG_EXEC(printf("onColors\n"));
}

void onRange  (EventPoint_t &ep) {
  DBG_EXEC(printf("onRange\n"));
}

void onApply  (EventPoint_t &ep) {
  DBG_EXEC(printf("onApply\n"));
}

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

#else // LovyanGFX || TFT_eSPI

bool touch_setup() {
#ifdef LGFX_USE_V1
  // LovyanGFX
  uint16_t cal[8] = {319, 384, 3866, 355, 277, 3729, 3832, 3785};
  GFX_EXEC(setTouchCalibrate(cal));
#endif

  return true;
}

#endif  // _ADAFRUIT_GFX_H || _ARDUINO_GFX_LIBRARIES_H_

bool touch_event() {
  int16_t x = -1, y = -1;

#if defined(_ADAFRUIT_GFX_H) || defined(_ARDUINO_GFX_LIBRARIES_H_)

  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    x = constrain(p.x, 0, 319);
    y = constrain(p.y, 0, 239);
    DBG_EXEC(printf("Pressure = %d, x = %d, y = %d\n", p.z, p.x, p.y));
  }

#else // LovyanGFX || TFT_eSPI

  if (GFX_EXEC(getTouch(&x, &y))) {
    x = constrain(x, 0, 319);
    y = constrain(y, 0, 239);
    DBG_EXEC(printf("x = %d, y = %d\n", x, y));
  }

#endif  // _ADAFRUIT_GFX_H || _ARDUINO_GFX_LIBRARIES_H_

  if (x != -1 && y != -1) {
    for (int i = 0; i < N_EVENT_REGION; i++) {
      if (region[i].x[0] <= x && x <= region[i].x[1] &&
          region[i].y[0] <= y && y <= region[i].y[1]) {
        EventPoint_t ep = {region[i].event, x, y};
        region[i].callback(ep);
        delay(100);
        return true;
      }
    }
  }
  return false;
}