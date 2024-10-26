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
  STATE_INFORMATION,
} State_t;

static State_t state = STATE_ON;

/*--------------------------------------------------------------------------------
 * Widget
 *--------------------------------------------------------------------------------*/
typedef struct {
  const uint8_t   *data;  // pointer to the image
  const size_t    size;   // size of image data
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
 * Check the widget position with RED marker
 *--------------------------------------------------------------------------------*/
#define POSTION_CHECK true
#if     POSTION_CHECK
#define POS_CHECK(w)  check_position(w)
static void check_position(const Widget_t *widget) {
  GFX_EXEC(drawRect(widget->x - 1, widget->y - 1, widget->w + 2, widget->h + 2, RED));
}
#else
#define POS_CHECK(w)
#endif

/*--------------------------------------------------------------------------------
 * Prototype Declaration for drawing widget
 *--------------------------------------------------------------------------------*/
typedef enum {
  DRAW_WIDGET = 0,
  DRAW_SPRITE,
  DRAW_SLIDER,
  DRAW_PRESSED,
} DrawType_t;

State_t widget_state(State_t s);
static void DrawWidget  (const Widget_t *widget, int8_t offset = 0); // Draw screen
static void DrawSprite  (const Widget_t *widget, int8_t offset = 0); // Draw radio and toggle
static void DrawSlider  (const Widget_t *widget, int8_t offset = 0); // Draw slider
static void DrawPressed (const Widget_t *widget, Touch_t &touch);    // Animation on click
static bool DrawSetup   (const Widget_t *widget, Touch_t &touch, DrawType_t type = DRAW_WIDGET, int8_t offset = 0);

/*--------------------------------------------------------------------------------
 * Widgets
 *--------------------------------------------------------------------------------*/
#include "widgets.hpp"

#ifdef _TFT_eSPIH_
//=========================================v==========================================
//  pngDraw: Callback function to draw pixels to the display
//====================================================================================
// https://github.com/Bodmer/TFT_eSPI/tree/master/examples/PNG%20Images
// Include the PNG decoder library, available via the IDE library manager
#include <PNGdec.h>

static PNG png; // PNG decoder instance

// Position variables must be global (PNGdec does not handle position coordinates)
static uint16_t xpos = 0;
static uint16_t ypos = 0;

// This function will be called during decoding of the png file to render each image
// line to the TFT. PNGdec generates the image line and a 1bpp mask.
static void pngDraw(PNGDRAW *pDraw) {
  uint16_t lineBuffer[TFT_HEIGHT];          // Line buffer for rendering
  uint8_t  maskBuffer[TFT_HEIGHT / 8 + 1];  // Mask buffer

  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);

  if (png.getAlphaMask(pDraw, maskBuffer, 255)) {
    // Note: pushMaskedImage is for pushing to the TFT and will not work pushing into a sprite
    tft.pushMaskedImage(xpos, ypos + pDraw->y, pDraw->iWidth, 1, lineBuffer, maskBuffer);
  }
}

static void DrawPNG(uint8_t *img, size_t size, uint16_t x, uint16_t y) {
  xpos = x;
  ypos = y;

  if (png.openFLASH(img, size, pngDraw) == PNG_SUCCESS) {
    png.decode(NULL, 0);
    // png.close(); // Required for files, not needed for FLASH arrays
  }
}
#endif // _TFT_eSPIH_

/*--------------------------------------------------------------------------------
 * Functions for drawing widget
 *--------------------------------------------------------------------------------*/
#define PNG_HEADER_WIDTH    16 // PNG file signature + offset from chunk data
#define PNG_HEADER_HEIGHT   20 // PNG file signature + offset from chunk data

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

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
 * Draw screen
 *--------------------------------------------------------------------------------*/
static void DrawWidget(const Widget_t *widget, int8_t offset /* = 0 */) {
  const Image_t *image = &widget->image[offset];

  if (image) {
    GFX_EXEC(startWrite());

#if defined(_TFT_eSPIH_)

    DrawPNG(image->data, image->size, widget->x, widget->y);

#elif defined(LOVYANGFX_HPP_)

    GFX_EXEC(drawPng(image->data, image->size, widget->x, widget->y));

#endif

    GFX_EXEC(endWrite());
  }
}

/*--------------------------------------------------------------------------------
 * Draw radio and toggle
 *--------------------------------------------------------------------------------*/
static void DrawSprite(const Widget_t *widget, int8_t offset /* = 0 */) {
  const Image_t *image = &widget->image[offset];

  if (image) {
    uint32_t w, h;
    GFX_EXEC(startWrite());

    w = swap_endian(*(uint32_t*)(image->data + PNG_HEADER_WIDTH));
    h = swap_endian(*(uint32_t*)(image->data + PNG_HEADER_HEIGHT));
    DBG_EXEC(printf("w: %d, h: %d\n", w, h));

    LGFX_Sprite sprite(&lcd);
    sprite.createSprite(w, h);
    sprite.drawPng(image->data, image->size, 0, 0);
    sprite.pushSprite(widget->x, widget->y);
    sprite.deleteSprite();

    GFX_EXEC(endWrite());
  }
}

/*--------------------------------------------------------------------------------
 * Draw slider
 *--------------------------------------------------------------------------------*/
#define SLIDER_KNOB_OFFSET  7 // offset from BAR

static void DrawSlider(const Widget_t *widget, int8_t offset /* = 0 */) {
  const Image_t *bar, *knob;

  bar  = &widget->image[0];
  knob = &widget->image[1];

  if (bar && knob) {
    uint32_t w, h;
    GFX_EXEC(startWrite());

    LGFX_Sprite sprite_bar(&lcd);
    LGFX_Sprite sprite_knob(&sprite_bar);

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

    sprite_knob.pushSprite(SLIDER_KNOB_OFFSET, 0);
    sprite_bar.pushSprite(widget->x, widget->y);

    sprite_knob.deleteSprite();
    sprite_bar.deleteSprite();

    GFX_EXEC(endWrite());
  }
}

/*--------------------------------------------------------------------------------
 * Rendering the pressed effects
 *--------------------------------------------------------------------------------*/
#define PRESSED_BUFFER_LEN  64
#define PRESSED_OFFSET      2

static void DrawPressed(const Widget_t *widget, Touch_t &touch) {
  const uint16_t x = widget->x;
  const uint16_t y = widget->y;
  const uint16_t w = widget->w;
  const uint16_t h = widget->h;
  int offset, d = PRESSED_OFFSET * 2;
  lgfx::rgb888_t rgb[PRESSED_BUFFER_LEN];

  if (touch.event == EVENT_FALLING) {
    offset = +PRESSED_OFFSET;
  } else if (touch.event == EVENT_RISING) {
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

    GFX_EXEC(endWrite());
  }
}

/*--------------------------------------------------------------------------------
 * Drawing when switching screens
 *--------------------------------------------------------------------------------*/
static bool DrawSetup(const Widget_t *widget, Touch_t &touch, DrawType_t type, int8_t offset) {
  if (touch.event == EVENT_NONE) {
    switch (type) {
      case DRAW_WIDGET: DrawWidget(widget, offset); break;
      case DRAW_SPRITE: DrawSprite(widget, offset); break;
      case DRAW_SLIDER: DrawSlider(widget, offset); break;
    }    
    return true;
  } else {
    return false;
  }
}

/*--------------------------------------------------------------------------------
 * Draw the legend and icons at STATE_ON
 *--------------------------------------------------------------------------------*/
void widget_setup(State_t screen = STATE_MAIN) {
  int n = 0;
  const Widget_t *widget = NULL;

  switch (screen) {
    case STATE_MAIN:
      widget = widget_main;
      n = N_WIDGETS(widget_main);
      break;

    case STATE_CONFIGURATION:
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
      if (widget->image && widget->callback) {
        widget->callback(widget, touch);
      }
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
 * Manage widget events
 *--------------------------------------------------------------------------------*/
void widget_event(const Widget_t *widgets, size_t size, Touch_t &touch) {
  for (int i = 0; i < size; i++) {

    // In case the touch event to be detected
    if ((widgets[i].event & touch.event) && widgets[i].callback) {

      // Find the widget where the event fired
      if (widgets[i].x <= touch.x && touch.x <= widgets[i].x + widgets[i].w &&
          widgets[i].y <= touch.y && touch.y <= widgets[i].y + widgets[i].h) {

        DBG_EXEC(printf("event = %d(%d), x = %d, y = %d\n", touch.event, widgets[i].event, touch.x, touch.y));
        widgets[i].callback((void*)&widgets[i], touch);
      }
    }
  }
}

/*--------------------------------------------------------------------------------
 * State controller
 *--------------------------------------------------------------------------------*/
State_t widget_state(State_t s = STATE_OFF) {
  if (s != state) {
    state = s;
    touch_clear();
    widget_setup(state);
  }

  return state;
}

void widget_control(void) {
  Touch_t touch;

  switch (state) {
    case STATE_ON:
      widget_setup(state = STATE_MAIN);
      break;

    case STATE_MAIN:
      if (touch_event(touch)) {
        // when icon 'configuration' is clicked then state becomes 'STATE_CONFIGURATION'
        widget_event(widget_main, N_WIDGETS(widget_main), touch);
      }
      break;

    case STATE_CONFIGURATION:
      do {
        if (touch_event(touch)) {
          // when icon 'back' is clicked then state becomes 'STATE_MAIN'
          widget_event(widget_configuration, N_WIDGETS(widget_configuration), touch);
        }
        delay(1); // reset wdt
      } while (state == STATE_CONFIGURATION);
      break;

    case STATE_RESOLUTION:
      do {
        if (touch_event(touch)) {
          // when icon 'back' is clicked then state becomes 'STATE_MAIN'
          widget_event(widget_resolution, N_WIDGETS(widget_resolution), touch);
        }
        delay(1); // reset wdt
      } while (state == STATE_RESOLUTION);
      break;

    case STATE_THERMOGRAPH:
      if (touch_event(touch)) {
        // when icon 'back' is clicked then state becomes 'STATE_MAIN'
        widget_event(widget_thermograph, N_WIDGETS(widget_thermograph), touch);
      }
      break;

    case STATE_FILE_MANAGER:
      do {
        if (touch_event(touch)) {
          // when icon 'back' is clicked then state becomes 'STATE_MAIN'
          widget_event(widget_file_manager, N_WIDGETS(widget_file_manager), touch);
        }
        delay(1); // reset wdt
      } while (state == STATE_FILE_MANAGER);
      break;

    case STATE_CAPTURE_MODE:
      do {
        if (touch_event(touch)) {
          // when icon 'back' is clicked then state becomes 'STATE_MAIN'
          widget_event(widget_capture_mode, N_WIDGETS(widget_capture_mode), touch);
        }
        delay(1); // reset wdt
      } while (state == STATE_CAPTURE_MODE);
      break;

    case STATE_CALIBRATION:
      do {
        if (touch_event(touch)) {
          // when icon 'back' is clicked then state becomes 'STATE_MAIN'
          widget_event(widget_calibration, N_WIDGETS(widget_calibration), touch);
        }
        delay(1); // reset wdt
      } while (state == STATE_CALIBRATION);
      break;

    case STATE_INFORMATION:
      do {
        if (touch_event(touch)) {
          // when icon 'back' is clicked then state becomes 'STATE_MAIN'
          widget_event(widget_information, N_WIDGETS(widget_information), touch);
        }
        delay(1); // reset wdt
      } while (state == STATE_INFORMATION);
      break;

    default:
      state = STATE_ON;
      break;
  }
}