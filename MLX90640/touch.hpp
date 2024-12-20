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
 * Event definition
 *--------------------------------------------------------------------------------*/
#define PERIOD_DEBOUNCE     25  // [msec]
#define PERIOD_TOUCHED      50  // [msec]
#define PERIOD_TAP2         200 // [msec]
#define PERIOD_CLEAR_EVENT  100 // [msec]

typedef enum {
  EVENT_NONE    = (0x00), // considered as 'HIGH'
  EVENT_RISING  = (0x01), //   touch --> untouch
  EVENT_FALLING = (0x02), // untouch -->   touch
  EVENT_TOUCHED = (0x04), //   touch -->   touch
  EVENT_TAP2    = (0x08), // double tap
  EVENT_WATCH   = (0x10), // execute a callback every watch cycle

  // alias
  EVENT_INIT    = (EVENT_NONE),
  EVENT_UP      = (EVENT_RISING),
  EVENT_DOWN    = (EVENT_FALLING),
  EVENT_DRAG    = (EVENT_FALLING | EVENT_TOUCHED),
  EVENT_TAP     = (EVENT_FALLING | EVENT_RISING),
  EVENT_CLICK   = (EVENT_FALLING | EVENT_RISING),
  EVENT_CHANGE  = (EVENT_FALLING | EVENT_RISING),
  EVENT_SELECT  = (EVENT_FALLING | EVENT_TAP2),
  EVENT_ALL     = (EVENT_FALLING | EVENT_RISING | EVENT_TOUCHED),
} Event_t;

typedef struct Touch {
  Event_t     event;  // Detected event
  uint16_t    x, y;   // The coordinates where the event fired
} Touch_t;

/*--------------------------------------------------------------------------------
 * Touch panel calibration configuration
 *--------------------------------------------------------------------------------*/
typedef struct TouchConfig {
  // Member variables
  uint16_t    cal[8];
  int8_t      offset[2];

  // Comparison operator
  bool operator >= (TouchConfig &RHS) {
    return !bcmp(cal, RHS.cal, sizeof(cal));
  }
  bool operator <= (TouchConfig &RHS) {
    return (offset[0] != RHS.offset[0]) || (offset[1] != RHS.offset[1]);
  }
  bool operator != (TouchConfig &RHS) {
    return bcmp(cal, RHS.cal, sizeof(cal)) || (offset[0] != RHS.offset[0]) || (offset[1] != RHS.offset[1]);
  }
} TouchConfig_t;

TouchConfig_t tch_cnf = {
#if   defined (LOVYANGFX_HPP_)
  .cal = {319, 384, 3866, 355, 277, 3729, 3832, 3785},
#elif defined (_TFT_eSPIH_)
  .cal = {239, 3660, 266, 3550, 2},
#endif
  .offset = {0, 0},//{-10, +5}
};

/*--------------------------------------------------------------------------------
 * Functions prototyping
 *--------------------------------------------------------------------------------*/
bool touch_setup(void);
bool touch_event(Touch_t &touch);
void touch_clear(void);
void touch_calibrate(TouchConfig_t *config);
bool touch_save(TouchConfig_t *config);
bool touch_load(TouchConfig_t *config);

/*--------------------------------------------------------------------------------
 * Simple touch point correction
 *--------------------------------------------------------------------------------*/
extern uint16_t lcd_width;
extern uint16_t lcd_height;

/*--------------------------------------------------------------------------------
 * Setup touch manager
 *--------------------------------------------------------------------------------*/
bool touch_setup(void) {
  // Load calibration parameters from FLASH
  touch_load(&tch_cnf);

#if   defined (_XPT2046_Touchscreen_h_)

  if (ts.begin()) {
    ts.setRotation(SCREEN_ROTATION);
    return true;
  } else {
    return false;
  }

#elif defined (LOVYANGFX_HPP_)

  // https://github.com/lovyan03/LovyanGFX/discussions/539
  GFX_EXEC(setTouchCalibrate(tch_cnf.cal));
  return true;

#elif defined (_TFT_eSPIH_)

  // https://github.com/Bodmer/TFT_eSPI/tree/master/examples/Generic/Touch_calibrate
  GFX_EXEC(setTouch(tch_cnf.cal));
  return true;

#endif // _XPT2046_Touchscreen_h_
}

/*--------------------------------------------------------------------------------
 * Touch event manager
 *--------------------------------------------------------------------------------*/
bool touch_event(Touch_t &touch) {
  uint32_t time = millis();
  static uint32_t prev_time;
  static uint16_t x, y;
  static bool prev_stat;
  static uint8_t count;
  Event_t event = EVENT_NONE;

#if defined (_XPT2046_Touchscreen_h_)

  bool stat = ts.touched();
  if (stat) {
    TS_Point p = ts.getPoint();
    x = p.x;
    y = p.y;
  }

#else // LovyanGFX || TFT_eSPI

  bool stat = GFX_EXEC(getTouch(&x, &y));

#endif // _XPT2046_Touchscreen_h_

  // when state changes, check again after a certain period of time
  uint32_t dt = time - prev_time;
  if (stat != prev_stat) {
    if (dt < PERIOD_DEBOUNCE) {
      return false;
    } else {
      // update the time when state changes
      prev_time = time;

      // reset double tap counter
      if (dt > PERIOD_TAP2 || count >= 4) {
        count = 0;
      }
    }
  }

  // untouch --> touch
  if (prev_stat == false && stat == true) {
    event = EVENT_FALLING;
    count = (count == 0 ? count + 1 : (dt <= PERIOD_TAP2 ? count + 1 : 0));
  } else

  // touch --> untouch
  if (prev_stat == true && stat == false) {
    event = EVENT_RISING;
    count = dt <= PERIOD_TAP2 ? count + 1 : 0;
  } else

  // touch --> touch
  if (prev_stat == true && stat == true) {
    event = EVENT_TOUCHED;
  }

  // update state
  prev_stat = stat;

  if (event != EVENT_NONE) {
    if (stat) {
        x += tch_cnf.offset[0];
        y += tch_cnf.offset[1];
        x = constrain(x, 0, lcd_width  - 1);
        y = constrain(y, 0, lcd_height - 1);
    }

    // TAP2 = FALLING --> RISING --> FALLING --> RISING
    touch.event = (Event_t)(event | (count >= 4 ? EVENT_TAP2 : EVENT_NONE));
    touch.x = x;
    touch.y = y;

//  DBG_EXEC(printf("event: %d, x: %d, y: %d, dt: %d, count: %d\n", touch.event, x, y, dt, count));
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

/*--------------------------------------------------------------------------------
 * Wait until there are no more touch events
 *--------------------------------------------------------------------------------*/
void touch_clear(void) {
  Touch_t touch;
  delay(PERIOD_CLEAR_EVENT);
  while(touch_event(touch));
}

/*--------------------------------------------------------------------------------
 * Calibrating the touch panel
 *--------------------------------------------------------------------------------*/
void touch_calibrate(TouchConfig_t *config) {
#if   defined (_XPT2046_Touchscreen_h_)

#elif defined (LOVYANGFX_HPP_)

  // https://github.com/lovyan03/LovyanGFX/tree/master/examples/HowToUse/2_user_setting
  GFX_EXEC(clear(0));
  GFX_EXEC(setTextSize(2));
  GFX_EXEC(setTextDatum(textdatum_t::middle_center));
  GFX_EXEC(drawString("touch the arrow marker.", lcd_width >> 1, lcd_height >> 1));
  GFX_EXEC(setTextDatum(textdatum_t::top_left));
  GFX_EXEC(calibrateTouch(config->cal, WHITE, BLACK, std::max(lcd_width, lcd_height) >> 3));

  DBG_EXEC({
    Serial.print(".cal = { ");
    for (uint8_t i = 0; i < 8; i++) {
      Serial.print(config->cal[i]);
      Serial.print(i < 7 ? ", " : " };\n");
    }
  })

#elif defined (_TFT_eSPIH_)

  // https://github.com/Bodmer/TFT_eSPI/tree/master/examples/Generic/Touch_calibrate
  GFX_EXEC(fillScreen(BLACK));
  GFX_EXEC(setCursor(20, 20));
  GFX_EXEC(setTextSize(2));
  GFX_EXEC(setTextColor(WHITE, BLACK));
  GFX_EXEC(println("Touch corners in order"));
  GFX_EXEC(calibrateTouch(config->cal, MAGENTA, BLACK, 15));

  DBG_EXEC({
    Serial.print(".cal = { ");
    for (uint8_t i = 0; i < 5; i++) {
      Serial.print(config->cal[i]);
      Serial.print(i < 4 ? ", " : " };\n");
    }
  })

#endif

  // clear offset
  config->offset[0] = config->offset[1] = 0;
}

/*--------------------------------------------------------------------------------
 * Saving and loading calibration configurations using preferences.h
 * https://docs.espressif.com/projects/arduino-esp32/en/latest/tutorials/preferences.html
 * https://github.com/espressif/arduino-esp32/tree/master/libraries/Preferences
 *--------------------------------------------------------------------------------*/
#include <Preferences.h>

#define RW_MODE   false
#define RO_MODE   true
#define INIT_KEY  "initialized"

#if     defined   (_XPT2046_Touchscreen_h_)
#define PREF_KEY  "XPT2046"
#elif   defined   (LOVYANGFX_HPP_)
#define PREF_KEY  "LovyanGFX"
#elif   defined   (_TFT_eSPIH_)
#define PREF_KEY  "TFT_eSPI"
#endif

bool touch_save(TouchConfig_t *config) {
  Preferences touchPref;

  if (touchPref.begin(PREF_KEY, RW_MODE) == false) {
    DBG_EXEC(printf("Preferences: begin(%s) failed.\n", PREF_KEY));
    return false;
  }

  if (touchPref.putBytes("cal", static_cast<const void*>(config->cal), sizeof(config->cal)) != sizeof(config->cal)) {
    DBG_EXEC(printf("Preferences: putBytes(cal) failed.\n"));
    touchPref.end();
    return false;
  }

  if (touchPref.putBytes("offset", static_cast<const void*>(config->offset), sizeof(config->offset)) != sizeof(config->offset)) {
    DBG_EXEC(printf("Preferences: putBytes(offset) failed.\n"));
    touchPref.end();
    return false;
  }

  if (touchPref.putBool(INIT_KEY, true) != sizeof(true)) {
    DBG_EXEC(printf("Preferences: putBool(%s) failed.\n", INIT_KEY));
    touchPref.end();
    return false;
  }

  touchPref.end();
  return true;
}

bool touch_load(TouchConfig_t *config) {
  Preferences touchPref;
  TouchConfig_t c;

  if (touchPref.begin(PREF_KEY, RO_MODE) == false) {
    DBG_EXEC(printf("Preferences: %s does not exist.\n", PREF_KEY));
    return false;
  }

  // Check if it already exists
  if (touchPref.isKey(INIT_KEY) == false) {
    DBG_EXEC(printf("Preferences: %s does not exist.\n", INIT_KEY));
    touchPref.end();
    return false;
  }

  if (touchPref.getBytes("cal", static_cast<void*>(&c.cal), sizeof(c.cal)) != sizeof(c.cal)) {
    DBG_EXEC(printf("Preferences: getBytes(cal) failed.\n"));
    touchPref.end();
    return false;
  }

  if (touchPref.getBytes("offset", static_cast<void*>(&c.offset), sizeof(c.offset)) != sizeof(c.offset)) {
    DBG_EXEC(printf("Preferences: getBytes(offset) failed.\n"));
    touchPref.end();
    return false;
  }

  *config = c;
  touchPref.end();
  return true;
}