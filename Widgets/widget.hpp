/*================================================================================
 * Wedget manager
 *================================================================================*/
#include <Arduino.h>

/*--------------------------------------------------------------------------------
 * State of the screen
 *--------------------------------------------------------------------------------*/
typedef enum {
  STATE_OFF = 0,
  STATE_ON,
  STATE_MAIN,
  STATE_CONFIGURATION,
  STATE_RESOLUTION,
  STATE_THERMOGRAPH,
  STATE_FILE_MANAGER,
  STATE_CAPTURE_MODE,
  STATE_CALIBRATION,
  STATE_ADJUST_OFFSET,
  STATE_INFORMATION,
} State_t;

static State_t state = STATE_ON;

/*--------------------------------------------------------------------------------
 * Widget data definition
 *--------------------------------------------------------------------------------*/
typedef struct {
  const uint8_t   *data;  // pointer to the image
  const size_t    size;   // size of image data (only for TFT_eSPI with bitbank2/PNGdec)
} Image_t;

typedef struct {
  const uint16_t  x, y;   // The top left coordinate of the widget
  const uint16_t  w, h;   // Widget width and height
  const Image_t   *image; // Widget image data
  const Event_t   event;  // The touch event to detect
  void            (*callback)(const void *widget, Touch_t &touch);  // Callback event handler
} Widget_t;

#define N_WIDGETS(w)  (sizeof(w) / sizeof(w[0]))

/*--------------------------------------------------------------------------------
 * Check the position of widget by RED rectangle
 *--------------------------------------------------------------------------------*/
#define POSTION_CHECK false
#if     POSTION_CHECK
#define POS_CHECK(x)  x
#else
#define POS_CHECK(x)
#endif

/*--------------------------------------------------------------------------------
 * Prototype Declaration for drawing widget
 *--------------------------------------------------------------------------------*/
State_t widget_state(State_t s);
static void DrawWidget(const Widget_t *widget, uint8_t offset = 0);
static void DrawScreen(const Widget_t *widget, uint8_t offset = 0);
static void DrawButton(const Widget_t *widget, uint8_t offset = 0);
static void DrawSlider(const Widget_t *widget, int16_t offset = 0);
static void DrawToggle(const Widget_t *widget, bool check = false);
static void DrawCheck (const Widget_t *widget, bool check = false);
static void DrawPress (const Widget_t *widget, Event_t event = EVENT_NONE);
static void DrawRadio (const Widget_t *widget, uint8_t n_widget, uint8_t selected = 0);
static void DrawThumb (const Widget_t *widget, const char *path);

/*--------------------------------------------------------------------------------
 * Widgets
 *--------------------------------------------------------------------------------*/
#include "widgets.hpp"

/*--------------------------------------------------------------------------------
 * Functions for drawing widget
 *--------------------------------------------------------------------------------*/
#ifdef _TFT_eSPIH_
// pngDraw: Callback function to draw pixels to the display
// https://github.com/Bodmer/TFT_eSPI/tree/master/examples/PNG%20Images
// Include the PNG decoder library, available via the IDE library manager
#include <PNGdec.h>

static PNG png; // PNG decoder instance

// Position variables must be global (PNGdec does not handle position coordinates)
static uint16_t xpos = 0;
static uint16_t ypos = 0;

// This next function will be called during decoding of the png file to
// render each image line to the TFT.  If you use a different TFT library
// you will need to adapt this function to suit.
void pngDraw(PNGDRAW *pDraw) {
  uint16_t lineBuffer[MAX_IMAGE_WIDTH];
  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
  GFX_EXEC(pushImage(xpos, ypos + pDraw->y, pDraw->iWidth, 1, lineBuffer));
}

static void DrawPNG(const uint8_t *img, size_t size, uint16_t x, uint16_t y) {
  xpos = x;
  ypos = y;

  if (png.openFLASH((uint8_t*)img, size, pngDraw) == PNG_SUCCESS) {
    GFX_EXEC(startWrite());
    png.decode(NULL, 0);
    GFX_EXEC(endWrite());
    // png.close(); // Required for files, not needed for FLASH arrays
  }
}
#endif // _TFT_eSPIH_

/*--------------------------------------------------------------------------------
 * Converting byte order according to MCU architecture
 *--------------------------------------------------------------------------------*/
#if defined (__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

#define SWAP(type, a, b)  { type tmp = a; a = b; b = tmp; }

// Alternatives to std::byteswap (C++23)
// big-endian (PNG) --> little-endian (ESP32)
uint32_t swap_endian(uint32_t v) {
  uint8_t r[4];
  *(uint32_t*)r = v;
  SWAP(uint8_t, r[0], r[3]);
  SWAP(uint8_t, r[1], r[2]);
  return *(uint32_t*)r;
}

#else // __ORDER_BIG_ENDIAN__ or __ORDER_PDP_ENDIAN__ or not defined

#define swap_endian(v)  (v)

#endif // __BYTE_ORDER__

/*--------------------------------------------------------------------------------
 * Draw widget
 *--------------------------------------------------------------------------------*/
static void DrawWidget(const Widget_t *widget, uint8_t offset /* = 0 */) {
  const Image_t *image = &widget->image[offset];

  if (image) {
    GFX_EXEC(startWrite());

#if defined (_TFT_eSPIH_)

    DrawPNG(image->data, image->size, widget->x, widget->y);

#elif defined (LOVYANGFX_HPP_)

    GFX_EXEC(drawPng(image->data, image->size, widget->x, widget->y));

#endif

    POS_CHECK(GFX_EXEC(drawRect(widget->x, widget->y, widget->w, widget->h, RED)));
    GFX_EXEC(endWrite());
  }
}

/*--------------------------------------------------------------------------------
 * Draw screen
 *--------------------------------------------------------------------------------*/
static void DrawScreen(const Widget_t *widget, uint8_t offset /* = 0 */) {
  DrawWidget(widget, offset);
}

/*--------------------------------------------------------------------------------
 * Draw a button-like icon
 *--------------------------------------------------------------------------------*/
#define PNG_HEADER_WIDTH    16 // PNG file signature + offset from chunk data
#define PNG_HEADER_HEIGHT   20 // PNG file signature + offset from chunk data

static void DrawButton(const Widget_t *widget, uint8_t offset /* = 0 */) {
#if 1

  // faster
  DrawWidget(widget, offset);

#else

  // slower
  const Image_t *image = &widget->image[offset];

  if (image) {
    uint32_t w, h;
    GFX_EXEC(startWrite());

    w = swap_endian(*(uint32_t*)(image->data + PNG_HEADER_WIDTH));
    h = swap_endian(*(uint32_t*)(image->data + PNG_HEADER_HEIGHT));
    DBG_EXEC(printf("w: %d, h: %d\n", w, h));

    static LGFX_Sprite sprite(&lcd);
    sprite.createSprite(w, h);
    sprite.drawPng(image->data, image->size, 0, 0);
    sprite.pushSprite(widget->x, widget->y);
    sprite.deleteSprite();

    POS_CHECK(GFX_EXEC(drawRect(widget->x, widget->y, widget->w, widget->h, RED)));
    GFX_EXEC(endWrite());

  }

#endif
}

/*--------------------------------------------------------------------------------
 * Draw slider
 *--------------------------------------------------------------------------------*/
static void DrawSlider(const Widget_t *widget, int16_t offset /* = 0 */) {
  const Image_t *bar  = &widget->image[0];
  const Image_t *knob = &widget->image[1];

  if (bar && knob) {
    GFX_EXEC(startWrite());

#if defined (_TFT_eSPIH_)

    static TFT_eSprite sprite_bar  = TFT_eSprite(&tft);
    static TFT_eSprite sprite_knob = TFT_eSprite(&sprite_bar);

#elif defined (LOVYANGFX_HPP_)

    // slower but no flickering
    static LGFX_Sprite sprite_bar(&lcd);
    static LGFX_Sprite sprite_knob(&sprite_bar);

#endif

    uint32_t w, h;
    w = swap_endian(*(uint32_t*)(bar->data + PNG_HEADER_WIDTH));
    h = swap_endian(*(uint32_t*)(bar->data + PNG_HEADER_HEIGHT));
    DBG_EXEC(printf("w: %d, h: %d\n", w, h));

    sprite_bar.createSprite(w, h);
    sprite_bar.drawPng(bar->data, bar->size, 0, 0);

    w = swap_endian(*(uint32_t*)(knob->data + PNG_HEADER_WIDTH));
    h = swap_endian(*(uint32_t*)(knob->data + PNG_HEADER_HEIGHT));
    DBG_EXEC(printf("w: %d, h: %d\n", w, h));

    sprite_knob.createSprite(w, h);
    sprite_knob.drawPng(knob->data, knob->size, 0, 0);

    sprite_knob.pushSprite(offset, 0);
    sprite_bar.pushSprite(widget->x, widget->y);

    sprite_knob.deleteSprite();
    sprite_bar.deleteSprite();

    POS_CHECK(GFX_EXEC(drawRect(widget->x, widget->y, widget->w, widget->h, RED)));
    GFX_EXEC(endWrite());
  }
}

static void DrawToggle(const Widget_t *widget, bool check /* = false */) {
  DrawButton(widget, check ? 1 : 0);
}

static void DrawCheck(const Widget_t *widget, bool check /* = false */) {
  DrawButton(widget, check ? 1 : 0);
}

static void DrawRadio(const Widget_t *widget, uint8_t n_widget, uint8_t selected /* = 0 */) {
  for (int i = 0; i < n_widget; i++) {
    DrawButton(&widget[i], (int8_t)(i == selected));
  }
}

/*--------------------------------------------------------------------------------
 * Rendering the pressed effects
 *--------------------------------------------------------------------------------*/
#define PRESSED_BUFFER_LEN  64
#define PRESSED_OFFSET      1 // 1 or 2

static void DrawPress(const Widget_t *widget, Event_t event) {
  int16_t offset;
  const uint16_t x = widget->x;
  const uint16_t y = widget->y;
  const uint16_t w = widget->w;
  const uint16_t h = widget->h;
  const uint16_t d = PRESSED_OFFSET * 2;
  lgfx::rgb888_t rgb[PRESSED_BUFFER_LEN];

  if (event == EVENT_FALLING) {
    offset = +PRESSED_OFFSET;
  } else if (event == EVENT_RISING) {
    offset = -PRESSED_OFFSET;
  } else {
    offset = 0;
  }

  if (w + d <= PRESSED_BUFFER_LEN) {
    GFX_EXEC(startWrite());

    if (offset >= 0) {
      for (int i = y + h + offset - 1; i >= y - offset; i--) {
        GFX_EXEC(readRect (x - offset, i,          w + d, 1, rgb));
        GFX_EXEC(pushImage(x + offset, i + offset, w + d, 1, rgb));
      }
    } else {
      for (int i = y - offset; i <= y + h + offset - 1; i++) {
        GFX_EXEC(readRect (x - offset, i,          w + d, 1, rgb));
        GFX_EXEC(pushImage(x + offset, i + offset, w + d, 1, rgb));
      }
    }

    POS_CHECK(GFX_EXEC(drawRect(widget->x, widget->y, widget->w, widget->h, RED)));
    GFX_EXEC(endWrite());
  }

  if (event == EVENT_RISING) {
    touch_clear();
  }
}

/*--------------------------------------------------------------------------------
 * Draw thumbnail
 *--------------------------------------------------------------------------------*/
static void DrawThumb(const Widget_t *widget, const char *path) {
#if defined (LOVYANGFX_HPP_) && (!defined (ESP32) || defined (SdFat_h))
  GFX_EXEC(drawBmpFile(
    SD,
    path,
    widget->x, widget->y, widget->w, widget->h,
    0, 0,
    0.4, 0.4
  ));
#endif
}

/*--------------------------------------------------------------------------------
 * Draw all widgets at the start of each state
 *--------------------------------------------------------------------------------*/
void widget_setup(State_t screen = STATE_OFF) {
  int n = 0;
  const Widget_t *widget = NULL;

  switch (screen) {
    case STATE_MAIN:
      widget = widget_main;
      n = N_WIDGETS(widget_main);
      break;

    case STATE_CONFIGURATION:
      cnf.video_recording = false;
      widget = widget_configuration;
      n = N_WIDGETS(widget_configuration);
      break;

    case STATE_RESOLUTION:
      widget = widget_resolution;
      n = N_WIDGETS(widget_resolution);
      break;

    case STATE_THERMOGRAPH:
      widget = widget_thermograph;
      n = N_WIDGETS(widget_thermograph);
      break;

    case STATE_FILE_MANAGER:
      widget = widget_file_manager;
      n = N_WIDGETS(widget_file_manager);
      break;

    case STATE_CAPTURE_MODE:
      widget = widget_capture_mode;
      n = N_WIDGETS(widget_capture_mode);
      break;

    case STATE_CALIBRATION:
      widget = widget_calibration;
      n = N_WIDGETS(widget_calibration);
      break;

    case STATE_ADJUST_OFFSET:
      widget = widget_adjust_offset;
      n = N_WIDGETS(widget_adjust_offset);
      break;

    case STATE_INFORMATION:
      widget = widget_information;
      n = N_WIDGETS(widget_information);
      break;

    default:
      break;
  }

  if (n && widget) {
    Touch_t touch = { EVENT_NONE, 0, 0 };
    for (int i = 0; i < n; i++, widget++) {
      if (widget->callback) {
        widget->callback(widget, touch);
      }
      POS_CHECK(GFX_EXEC(drawRect(widget->x, widget->y, widget->w, widget->h, RED)));
    }
  }

  // Additional
  switch (screen) {
    case STATE_MAIN:
#if 0
      // Draw color bar
      const int n = sizeof(camColors) / sizeof(camColors[0]);
      const int w = dsp.box_size * dsp.interpolate_scale * MLX90640_COLS;
      int       y = dsp.box_size * dsp.interpolate_scale * MLX90640_ROWS + 3;
      for (int i = 0; i < n; i++) {
        int x = map(i, 0, n, 0, w);
        GFX_EXEC(fillRect(x, y, 1, FONT_HEIGHT, camColors[i]));
      }

      // Draw thermal range
      y += FONT_HEIGHT + 4;
      GFX_EXEC(setTextSize(2));
      GFX_EXEC(setTextColor(WHITE));
      gfx_printf(0,                      y, "%d", MINTEMP);
      gfx_printf(w / 2 - FONT_WIDTH * 2, y, "%3.1f", (float)(MINTEMP + MAXTEMP) / 2.0f);
      gfx_printf(w     - FONT_WIDTH * 2, y, "%d", MAXTEMP);

      // Draw resolution
      GFX_EXEC(setTextSize(2));
      gfx_printf(260 + FONT_WIDTH, LINE_HEIGHT * 0.5, "%2d:%d", dsp.interpolate_scale, dsp.box_size);
#endif
      break;
    case STATE_THERMOGRAPH:
      break;
  }
}

/*--------------------------------------------------------------------------------
 * Change state
 *--------------------------------------------------------------------------------*/
State_t widget_state(State_t s = STATE_OFF) {
  if (s != state) {
    state = s;
    touch_clear();
    widget_setup(state);
  }

  return state;
}

/*--------------------------------------------------------------------------------
 * Manage widget events
 *--------------------------------------------------------------------------------*/
bool widget_event(const Widget_t *widgets, const size_t n_widgets, Touch_t &touch) {
  for (int i = 0; i < n_widgets; i++) {

    // In case the touch event to be detected
    if ((widgets[i].event & touch.event) && widgets[i].callback) {

      // Find the widget where the event fired
      if (widgets[i].x <= touch.x && touch.x <= widgets[i].x + widgets[i].w &&
          widgets[i].y <= touch.y && touch.y <= widgets[i].y + widgets[i].h) {

        DBG_EXEC(printf("event = %d(%d), x = %d, y = %d\n", touch.event, widgets[i].event, touch.x, touch.y));
        widgets[i].callback((void*)&widgets[i], touch);
        return true;
      }
    }
  }

  return false;
}

/*--------------------------------------------------------------------------------
 * Monitor events for a specific widget
 *--------------------------------------------------------------------------------*/
bool widget_monitor(const Widget_t *widgets, const size_t n_widgets) {
  Touch_t touch;

  if (touch_event(touch)) {
    // when icon 'configuration' is clicked then state becomes 'STATE_CONFIGURATION'
    if (widget_event(widgets, n_widgets, touch) == true) {
      return true;
    }
  }

  return false;
}

/*--------------------------------------------------------------------------------
 * Finite State Machines
 *--------------------------------------------------------------------------------*/
void widget_control(void) {
  switch (state) {
    case STATE_ON:
      widget_setup(state = STATE_MAIN);
      break;

    case STATE_MAIN:
      // when icon 'configuration' is clicked then state becomes 'STATE_CONFIGURATION'
      widget_monitor(widget_main, N_WIDGETS(widget_main));
      break;

    case STATE_CONFIGURATION:
      do {
        // when icon 'back' is clicked then state becomes 'STATE_MAIN'
        widget_monitor(widget_configuration, N_WIDGETS(widget_configuration));
        delay(1); // reset wdt
      } while (state == STATE_CONFIGURATION);
      break;

    case STATE_RESOLUTION:
      do {
        // when icon 'back' is clicked then state becomes 'STATE_MAIN'
        widget_monitor(widget_resolution, N_WIDGETS(widget_resolution));
        delay(1); // reset wdt
      } while (state == STATE_RESOLUTION);
      break;

    case STATE_THERMOGRAPH:
      // when icon 'back' is clicked then state becomes 'STATE_MAIN'
      widget_monitor(widget_thermograph, N_WIDGETS(widget_thermograph));
      break;

    case STATE_FILE_MANAGER:
      do {
        // when icon 'back' is clicked then state becomes 'STATE_MAIN'
        widget_monitor(widget_file_manager, N_WIDGETS(widget_file_manager));
        delay(1); // reset wdt
      } while (state == STATE_FILE_MANAGER);
      break;

    case STATE_CAPTURE_MODE:
      do {
        // when icon 'back' is clicked then state becomes 'STATE_MAIN'
        widget_monitor(widget_capture_mode, N_WIDGETS(widget_capture_mode));
        delay(1); // reset wdt
      } while (state == STATE_CAPTURE_MODE);
      break;

    case STATE_CALIBRATION:
      do {
        // when icon 'back' is clicked then state becomes 'STATE_MAIN'
        widget_monitor(widget_calibration, N_WIDGETS(widget_calibration));
        delay(1); // reset wdt
      } while (state == STATE_CALIBRATION);
      break;

    case STATE_ADJUST_OFFSET:
      do {
        // when icon 'back' is clicked then state becomes 'STATE_CALIBRATION'
        widget_monitor(widget_adjust_offset, N_WIDGETS(widget_adjust_offset));
        delay(1); // reset wdt
      } while (state == STATE_ADJUST_OFFSET);
      break;

    case STATE_INFORMATION:
      do {
        // when icon 'back' is clicked then state becomes 'STATE_MAIN'
        widget_monitor(widget_information, N_WIDGETS(widget_information));
        delay(1); // reset wdt
      } while (state == STATE_INFORMATION);
      break;

    default:
      state = STATE_ON;
      break;
  }
}