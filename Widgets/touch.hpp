/*================================================================================
 * Touch event manager
 *================================================================================*/
#if defined (_ADAFRUIT_GFX_H) || defined (_ARDUINO_GFX_LIBRARIES_H_)

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
#define PERIOD_DEBOUNCE     30  // [msec]
#define PERIOD_TOUCHED      60  // [msec]
#define PERIOD_CLEAR_EVENT  100 // [msec]

typedef enum {
  EVENT_NONE    = (0x00),
  EVENT_FALLING = (0x01), // untouch -->   touch
  EVENT_RISING  = (0x02), //   touch --> untouch
  EVENT_TOUCHED = (0x04), //   touch -->   touch
  // alias
  EVENT_UP      = (EVENT_RISING),
  EVENT_DOWN    = (EVENT_FALLING),
  EVENT_DRAG    = (EVENT_FALLING | EVENT_TOUCHED),
  EVENT_CLICK   = (EVENT_FALLING | EVENT_RISING),
  EVENT_CHANGE  = (EVENT_FALLING | EVENT_RISING),
  EVENT_ALL     = (EVENT_FALLING | EVENT_RISING | EVENT_TOUCHED),
} Event_t;

typedef struct Touch {
  Event_t     event;  // Detected event
  uint16_t    x, y;   // The coordinates where the event fired
} Touch_t;

typedef struct TouchConfig {
  // Member Variables
  uint16_t    cal[8];
  int8_t      offset[2];

  // Comparison Operator
  bool operator != (TouchConfig &RHS) {
    return !bcmp(cal, RHS.cal, sizeof(cal)) || (offset[0] != RHS.offset[0]) || (offset[1] != RHS.offset[1]);
  }
} TouchConfig_t;

TouchConfig_t tch_cnf = {
#if defined (LOVYANGFX_HPP_)
  .cal = {319, 384, 3866, 355, 277, 3729, 3832, 3785},
#elif defined (_TFT_eSPIH_)
  .cal = {0, 0, 0, 0, 1, 0,},
#endif
  .offset = {0, 0},//{-10, +5}
};

/*--------------------------------------------------------------------------------
 * Simple touch point correction
 *--------------------------------------------------------------------------------*/
extern uint16_t lcd_width;
extern uint16_t lcd_height;
extern MLXConfig_t mlx_cnf;

/*--------------------------------------------------------------------------------
 * Setup touch manager
 *--------------------------------------------------------------------------------*/
#ifdef  _XPT2046_Touchscreen_h_

bool touch_setup(void) {
  if (ts.begin()) {
    ts.setRotation(SCREEN_ROTATION);
    return true;
  } else {
    return false;
  }
}

#else // LovyanGFX || TFT_eSPI

bool touch_setup(void) {

#if defined (LOVYANGFX_HPP_)

  // https://github.com/lovyan03/LovyanGFX/discussions/539
  GFX_EXEC(setTouchCalibrate(tch_cnf.cal));

#elif defined (_TFT_eSPIH_)

  GFX_EXEC(setTouch(tch_cnf.cal));

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
    event = time > PERIOD_TOUCHED ? EVENT_TOUCHED : EVENT_NONE;
  } /*else

  // untouch --> untouch
  if (stat == false && prev_stat == false) {
    event = EVENT_NONE;
  }*/

  prev_stat = stat;

  if (event != EVENT_NONE) {
    if (stat) {
        x += tch_cnf.offset[0];
        y += tch_cnf.offset[1];
        x = constrain(x, 0, lcd_width  - 1);
        y = constrain(y, 0, lcd_height - 1);
    }

    //DBG_EXEC(printf("event: %d, x: %d, y: %d\n", event, x, y));

    touch.event = event;
    touch.x = x;
    touch.y = y;

    return true;
  }

#if     DEBUG
  // Capture screen
  else if (Serial.available()) {
    Serial.readStringUntil('\n');
    Serial.println("saving screenshot...");
    sdcard_save();
    Serial.println("done.");
  }
#endif

  return false;
}

void touch_clear(void) {
  Touch_t touch;
  delay(PERIOD_CLEAR_EVENT);
  while(touch_event(touch));
}