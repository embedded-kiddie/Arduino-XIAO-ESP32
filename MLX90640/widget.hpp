/*================================================================================
 * Wedget manager
 *================================================================================*/
#include <Arduino.h>
#include "icons.h"

/*--------------------------------------------------------------------------------
 * State of the screen
 *--------------------------------------------------------------------------------*/
typedef enum {
  STATE_ON = 0,
  STATE_RUN,
  STATE_CONFIG,
  STATE_CONFIG_RUN,
} State_t;

static State_t state = STATE_ON;

/*--------------------------------------------------------------------------------
 * Widget
 *--------------------------------------------------------------------------------*/
typedef struct {
  uint16_t      x, y;   // The top left coordinate of the widget
  uint16_t      w, h;   // Widget width and height
  const uint8_t *img;   // PNG image data
  size_t        size;   // Size of PNG image data
  EventType_t   event;  // The touch event to detect
  void          (*callback)(EventPoint_t &ep);  // Callback event handler
} Widget_t;

#define N_WIDGETS(w)  (sizeof(w) / sizeof(w[0]))

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

/*--------------------------------------------------------------------------------
 * Function for drawing icons
 *--------------------------------------------------------------------------------*/
static void DrawPNG(uint8_t *img, size_t size, uint16_t x, uint16_t y) {
  xpos = x;
  ypos = y;

  if (png.openFLASH(img, size, pngDraw) == PNG_SUCCESS) {
    png.decode(NULL, 0);
    // png.close(); // Required for files, not needed for FLASH arrays
  }
}
#endif // _TFT_eSPIH_

static void DrawWidget(const Widget_t *widget) {
  GFX_EXEC(startWrite());

  if (widget && widget->img) {
#if defined(_TFT_eSPIH_)
    DrawPNG((uint8_t*)widget->img, widget->size, widget->x, widget->y);
#elif defined(LOVYANGFX_HPP_)
    GFX_EXEC(drawPng((uint8_t*)widget->img, widget->size, widget->x, widget->y));
#endif
  }

  GFX_EXEC(endWrite());
}

/*--------------------------------------------------------------------------------
 * Widget event callbacks for main screen
 *--------------------------------------------------------------------------------*/
const int get_widget_count(State_t screen);
const Widget_t* get_widget(State_t screen);
const Widget_t* find_widget(State_t screen, const uint8_t* icon);

// Screen - Main
static void onInside(EventPoint_t &ep) {
  DBG_EXEC(printf("onInside\n"));
}

static void onOutside(EventPoint_t &ep) {
  DBG_EXEC(printf("onOutside\n"));
}

static void onThermograph(EventPoint_t &ep) {
  DBG_EXEC(printf("onThermograph\n"));
}

static void onCamera(EventPoint_t &ep) {
  DBG_EXEC(printf("onCamera\n"));
  DrawWidget(find_widget(STATE_RUN, icon_camera2));
  sdcard_save();
  DrawWidget(find_widget(STATE_RUN, icon_camera1));
}

static void onConfig(EventPoint_t &ep) {
  DBG_EXEC(printf("onConfig\n"));
  GFX_EXEC(fillScreen(BLACK));

  const int n = get_widget_count(STATE_CONFIG);
  const Widget_t *widget = get_widget(STATE_CONFIG);
  for (int i = 0; i < n; i++, widget++) {
    DrawWidget(widget);
  }

  touch_clear(ep);
  state = STATE_CONFIG;
}

// Screen - Config
static void onResolution(EventPoint_t &ep) {
  DBG_EXEC(printf("onResolution\n"));
}

static void onFolder(EventPoint_t &ep) {
  DBG_EXEC(printf("onFolder\n"));
}

static void onCapMode(EventPoint_t &ep) {
  DBG_EXEC(printf("onCapMode\n"));
}

static void onCalibration(EventPoint_t &ep) {
  DBG_EXEC(printf("onCalibration\n"));
}

static void onDevInfo(EventPoint_t &ep) {
  DBG_EXEC(printf("onDevice\n"));
}

static void onReturn(EventPoint_t &ep) {
  DBG_EXEC(printf("onReturn\n"));
  touch_clear(ep);
  state = STATE_ON;
}

/*--------------------------------------------------------------------------------
 * Instances of widgets
 *--------------------------------------------------------------------------------*/
// Screen - Main
static const Widget_t widget_main[] {
  {   0,   0, 256,  92, NULL, 0, EVENT_ALL, onInside      },
  { 256,   0,  64, 140, NULL, 0, EVENT_ALL, onOutside     },
  {   0, 195, 256,  45, NULL, 0, EVENT_ALL, onThermograph },
  { 265, 135, ICON1_WIDTH, ICON1_HEIGHT, icon_camera1, sizeof(icon_camera1), EVENT_CLICK, onCamera },
  { 265, 135, ICON1_WIDTH, ICON1_HEIGHT, icon_camera2, sizeof(icon_camera2), EVENT_NONE,  nullptr  },
  { 265, 185, ICON1_WIDTH, ICON1_HEIGHT, icon_config,  sizeof(icon_config ), EVENT_ALL,   onConfig },
};

// Screen - Config
static const Widget_t widget_config[] {
  {  22,  10, ICON2_WIDTH, ICON2_HEIGHT, icon_resolution,  sizeof(icon_resolution ), EVENT_ALL, onResolution  },
  { 124,  10, ICON2_WIDTH, ICON2_HEIGHT, icon_thermograph, sizeof(icon_thermograph), EVENT_ALL, onThermograph },
  { 226,  10, ICON2_WIDTH, ICON2_HEIGHT, icon_folder,      sizeof(icon_folder     ), EVENT_ALL, onFolder      },
  {  22, 100, ICON2_WIDTH, ICON2_HEIGHT, icon_capmode,     sizeof(icon_capmode    ), EVENT_ALL, onCapMode     },
  { 124, 100, ICON2_WIDTH, ICON2_HEIGHT, icon_calibration, sizeof(icon_calibration), EVENT_ALL, onCalibration },
  { 226, 100, ICON2_WIDTH, ICON2_HEIGHT, icon_devinfo,     sizeof(icon_devinfo    ), EVENT_ALL, onDevInfo     },
  { 134, 190, ICON1_WIDTH, ICON1_HEIGHT, icon_apply,       sizeof(icon_apply      ), EVENT_ALL, onReturn      },
};

/*--------------------------------------------------------------------------------
 * Manage widget instances
 *--------------------------------------------------------------------------------*/
const int get_widget_count(State_t screen) {
  switch (screen) {
    case STATE_RUN:
      return N_WIDGETS(widget_main);
    case STATE_CONFIG:
      return N_WIDGETS(widget_config);
    default:
      return 0;
  }
}

const Widget_t* get_widget(State_t screen) {
  switch (screen) {
    case STATE_RUN:
      return widget_main;
    case STATE_CONFIG:
      return widget_config;
    default:
      return NULL;
  }
}

const Widget_t* find_widget(State_t screen, const uint8_t* icon) {
  const int n = get_widget_count(screen);
  const Widget_t* widget = get_widget(screen);

  for (int i = 0; i < n; i++, widget++) {
    if (widget->img == icon) {
      return widget;
    }
  }

  return NULL;
}

/*--------------------------------------------------------------------------------
 * Draw the legend and icons at STATE_ON
 *--------------------------------------------------------------------------------*/
void widget_setup(void) {
  GFX_EXEC(fillScreen(BLACK));

  // Draw color bar
  const int n = sizeof(camColors) / sizeof(camColors[0]);
  const int w = dsp.box_size * dsp.interpolate_scale * MLX90640_COLS;
  int       y = dsp.box_size * dsp.interpolate_scale * MLX90640_ROWS + 3;
  for (int i = 0; i < n; i++) {
    int x = map(i, 0, n, 0, w);
    GFX_EXEC(fillRect(x, y, 1, FONT_HEIGHT, camColors[i]));
  }

  y += FONT_HEIGHT + 4;
  GFX_EXEC(setTextSize(2));
  GFX_EXEC(setTextColor(WHITE));
  gfx_printf(0,                      y, "%d", MINTEMP);
  gfx_printf(w / 2 - FONT_WIDTH * 2, y, "%3.1f", (float)(MINTEMP + MAXTEMP) / 2.0f);
  gfx_printf(w     - FONT_WIDTH * 2, y, "%d", MAXTEMP);

  GFX_EXEC(setTextSize(1));
  gfx_printf(260, LINE_HEIGHT * 0.0, "Resolution");
  gfx_printf(260, LINE_HEIGHT * 1.5, "FPS [Hz]");
  gfx_printf(260, LINE_HEIGHT * 3.0, "Input [ms]");
  gfx_printf(260, LINE_HEIGHT * 4.5, "Output[ms]");
  gfx_printf(260, LINE_HEIGHT * 6.0, "Sensor['C]");

  GFX_EXEC(setTextSize(2));
  gfx_printf(260 + FONT_WIDTH, LINE_HEIGHT * 0.5, "%2d:%d", dsp.interpolate_scale, dsp.box_size);

  DrawWidget(find_widget(STATE_RUN, icon_camera1));
  DrawWidget(find_widget(STATE_RUN, icon_config));
}

/*--------------------------------------------------------------------------------
 * Manage widget events
 *--------------------------------------------------------------------------------*/
void widget_event(const Widget_t *widgets, size_t size, EventPoint_t &ep) {
  for (int i = 0; i < size; i++) {

    // In case the touch event to be detected
    if ((widgets[i].event & ep.event) &&  widgets[i].callback) {

      // Find the widget where the event fired
      if (widgets[i].x <= ep.x && ep.x <= widgets[i].x + widgets[i].w &&
          widgets[i].y <= ep.y && ep.y <= widgets[i].y + widgets[i].h) {

        DBG_EXEC(printf("event = %d, x = %d, y = %d\n", ep.event, ep.x, ep.y));
        widgets[i].callback(ep);
      }
    }
  }
}

/*--------------------------------------------------------------------------------
 * State controller
 *--------------------------------------------------------------------------------*/
State_t widget_state(void) {
  return state;
}

void widget_control(void) {
  EventPoint_t ep;

  switch (state) {
    case STATE_ON:
      widget_setup();
      state = STATE_RUN;
      break;

    case STATE_RUN:
      if (touch_event(ep)) {
        // when icon 'config' is clicked then state becomes 'STATE_CONFIG'
        widget_event(widget_main, N_WIDGETS(widget_main), ep);
        if (state == STATE_RUN) {
          break;
        }
      }

    case STATE_CONFIG:
    case STATE_CONFIG_RUN:
      do {
        if (touch_event(ep)) {
          // when icon 'back' is clicked then state becomes 'STATE_ON' or 'STATE_CONFIG_RUN'
          widget_event(widget_config, N_WIDGETS(widget_config), ep);
        }
        delay(1); // reset wdt
      } while (state == STATE_CONFIG);
      break;
  }
}