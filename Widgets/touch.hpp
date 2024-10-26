/*================================================================================
 * Touch event manager
 *================================================================================*/
#if defined(_ADAFRUIT_GFX_H) || defined(_ARDUINO_GFX_LIBRARIES_H_)

#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include "spi_assign.h"

// Use no interrupts when using the SD library
//XPT2046_Touchscreen ts(TOUCH_CS);  // Param 2 - NULL - No interrupts
//XPT2046_Touchscreen ts(TOUCH_CS, 255);  // Param 2 - 255 - No interrupts
XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);  // Param 2 - Touch IRQ Pin - interrupt enabled polling

#endif // _ADAFRUIT_GFX_H || _ARDUINO_GFX_LIBRARIES_H_

/*--------------------------------------------------------------------------------
 * Definition of events
 *--------------------------------------------------------------------------------*/
#define PERIOD_DEBOUNCE 25  // [msec]
#define PERIOD_DRAG     50  // [msec]

typedef enum {
  EVENT_NONE    = (0x00),
  EVENT_FALLING = (0x01), // untouch -->   touch
  EVENT_RISING  = (0x02), //   touch --> untouch
  EVENT_DRAG    = (0x04), //   touch -->   touch
  // alias
  EVENT_UP      = (EVENT_RISING),
  EVENT_DOWN    = (EVENT_FALLING),
  EVENT_CLICK   = (EVENT_FALLING | EVENT_RISING),
  EVENT_CHANGE  = (EVENT_FALLING | EVENT_RISING),
  EVENT_ALL     = (EVENT_FALLING | EVENT_RISING | EVENT_DRAG),
} Event_t;

typedef struct {
  Event_t     event;  // Detected event
  uint16_t    x, y;   // The coordinates where the event fired
} Touch_t;

/*--------------------------------------------------------------------------------
 * Setup touch manager
 *--------------------------------------------------------------------------------*/
#ifdef  _XPT2046_Touchscreen_h_

bool touch_setup(void) {
  if (ts.begin()) {
    ts.setRotation(3);
    return true;
  } else {
    return false;
  }
}

#else // LovyanGFX || TFT_eSPI

bool touch_setup(void) {

#ifdef LOVYANGFX_HPP_
  // https://github.com/lovyan03/LovyanGFX/discussions/539
  uint16_t cal[8] = {319, 384, 3866, 355, 277, 3729, 3832, 3785};
  GFX_EXEC(setTouchCalibrate(cal));
#endif

  return true;
}

#endif // _XPT2046_Touchscreen_h_

/*--------------------------------------------------------------------------------
 * Event manager
 *--------------------------------------------------------------------------------*/
bool touch_event(Touch_t &touch) {
  uint32_t time = millis();
  static uint32_t prev_time = time;

  time -= prev_time;
  if (time < PERIOD_DEBOUNCE) {
    return false;
  } else {
    time = prev_time;
  }

  static uint16_t x, y;
  static bool prev_stat = false;
  Event_t event = EVENT_NONE;

#ifdef  _XPT2046_Touchscreen_h_

  bool stat = ts.touched();
  if (stat) {
    TS_Point p = ts.getPoint();
    x = p.x;
    y = p.y;
  }

#else // LovyanGFX || TFT_eSPI

  bool stat = GFX_EXEC(getTouch(&x, &y));

#endif // _XPT2046_Touchscreen_h_

  // untouch --> touch
  if (prev_stat == false && stat == true) {
    event = EVENT_FALLING;
  } else

  // touch --> untouch
  if (prev_stat == true && stat == false) {
    event = EVENT_RISING;
  } else

  // touch --> touch
  if (prev_stat == true && stat == true) {
    event = time > PERIOD_DRAG ? EVENT_DRAG : EVENT_NONE;
  } /*else

  // untouch --> untouch
  if (stat == false && prev_stat == false) {
    event = EVENT_NONE;
  }*/

  prev_stat = stat;

  if (event != EVENT_NONE) {
    // DBG_EXEC(printf("event: %d, x: %d, y: %d\n", event, x, y));
 
    if (stat) {
        x = constrain(x, 0, 319);
        y = constrain(y, 0, 239);
    }

    touch.event = event;
    touch.x = x;
    touch.y = y;

    return true;
  }

#if     DEBUG
  // Capture screen
  else if (Serial.available()) {
    Serial.readStringUntil('\n');
    sdcard_save();
  }
#endif

  return false;
}

void touch_clear(void) {
  Touch_t touch;
  while(touch_event(touch));
}