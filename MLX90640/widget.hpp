/*================================================================================
 * Wedget manager
 *================================================================================*/
#include <Arduino.h>
#include "icons.h"

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
 * Draw PNG
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

static void DrawWidget(const Widget_t &widget) {
  GFX_EXEC(startWrite());

  if (widget.img) {
#if defined(_TFT_eSPIH_)
    DrawPNG((uint8_t*)widget.img, widget.size, widget.x, widget.y);
#elif defined(LOVYANGFX_HPP_)
    GFX_EXEC(drawPng((uint8_t*)widget.img, widget.size, widget.x, widget.y));
#endif
  }

  GFX_EXEC(endWrite());
}

/*--------------------------------------------------------------------------------
 * Widgets definition
 *--------------------------------------------------------------------------------*/
// Screen Main
static void onInside (EventPoint_t &ep);
static void onOutside(EventPoint_t &ep);
static void onColors (EventPoint_t &ep);
static void onCamera (EventPoint_t &ep);
static void onConfig (EventPoint_t &ep);

const Widget_t widget_main[] {
  {   0,   0, 256,  92, NULL, 0, EVENT_ALL, onInside  },
  { 256,   0,  64, 140, NULL, 0, EVENT_ALL, onOutside },
  {   0, 195, 256,  45, NULL, 0, EVENT_ALL, onColors  },
  { 265, 135, ICON1_WIDTH, ICON1_HEIGHT, icon_camera1, sizeof(icon_camera1), EVENT_CLICK, onCamera },
  { 265, 135, ICON1_WIDTH, ICON1_HEIGHT, icon_camera2, sizeof(icon_camera2), EVENT_NONE,  nullptr  },
  { 265, 185, ICON1_WIDTH, ICON1_HEIGHT, icon_config,  sizeof(icon_config ), EVENT_CLICK, onConfig },
};

// Screen Config
static void onResolution (EventPoint_t &ep);
static void onThermograph(EventPoint_t &ep);
static void onFolder     (EventPoint_t &ep);
static void onCapturing  (EventPoint_t &ep);
static void onCalibration(EventPoint_t &ep);
static void onDevInfo    (EventPoint_t &ep);
static void onApply      (EventPoint_t &ep);

const Widget_t widget_config[] {
  {  22,  10, ICON2_WIDTH, ICON2_HEIGHT, icon_resolution,  sizeof(icon_resolution ), EVENT_ALL, onResolution  },
  { 124,  10, ICON2_WIDTH, ICON2_HEIGHT, icon_thermograph, sizeof(icon_thermograph), EVENT_ALL, onThermograph },
  { 226,  10, ICON2_WIDTH, ICON2_HEIGHT, icon_folder,      sizeof(icon_folder     ), EVENT_ALL, onFolder      },
  {  22, 100, ICON2_WIDTH, ICON2_HEIGHT, icon_capturing,   sizeof(icon_capturing  ), EVENT_ALL, onCapturing   },
  { 124, 100, ICON2_WIDTH, ICON2_HEIGHT, icon_calibration, sizeof(icon_calibration), EVENT_ALL, onCalibration },
  { 226, 100, ICON2_WIDTH, ICON2_HEIGHT, icon_devinfo,     sizeof(icon_devinfo    ), EVENT_ALL, onDevInfo     },
  { 134, 190, ICON1_WIDTH, ICON1_HEIGHT, icon_apply,       sizeof(icon_apply      ), EVENT_ALL, onApply       },
};

/*--------------------------------------------------------------------------------
 * Event callback functions
 *--------------------------------------------------------------------------------*/
int widget_event(const Widget_t *widgets, size_t size, EventPoint_t &ep);

static void onInside(EventPoint_t &ep) {
  DBG_EXEC(printf("onInside\n"));
}

static void onOutside(EventPoint_t &ep) {
  DBG_EXEC(printf("onOutside\n"));
}

static void onColors(EventPoint_t &ep) {
  DBG_EXEC(printf("onColors\n"));
}

static void onCamera(EventPoint_t &ep) {
  DBG_EXEC(printf("onCamera\n"));
  DrawWidget(widget_main[4]); // camera2
  sdcard_save();
  DrawWidget(widget_main[3]); // camera1
}

static void onConfig(EventPoint_t &ep) {
  DBG_EXEC(printf("onConfig\n"));
  GFX_EXEC(fillScreen(BLACK));
  for (int i = 0; i < N_WIDGETS(widget_config); i++) {
    DrawWidget(widget_config[i]);
  }

  while (true) {
    if (touch_event(ep)) {
      int id = widget_event(widget_config, N_WIDGETS(widget_config), ep);
      if (id >= 0 && widget_config[id].callback == onApply) {
        return;
      }
    }
    delay(1);
  }
}

static void onResolution(EventPoint_t &ep) {
  DBG_EXEC(printf("onResolution\n"));
}

static void onThermograph(EventPoint_t &ep) {
  DBG_EXEC(printf("onThermograph\n"));
}

static void onFolder(EventPoint_t &ep) {
  DBG_EXEC(printf("onFolder\n"));
}

static void onCapturing(EventPoint_t &ep) {
  DBG_EXEC(printf("onCapturing\n"));
}

static void onCalibration(EventPoint_t &ep) {
  DBG_EXEC(printf("onCalibration\n"));
}

static void onDevInfo(EventPoint_t &ep) {
  DBG_EXEC(printf("onDevice\n"));
}


static void onApply(EventPoint_t &ep) {
  DBG_EXEC(printf("onApply\n"));
}

static void widget_setup(void) {
  DrawWidget(widget_main[3]); // camera1
  DrawWidget(widget_main[5]); // config
}

/*--------------------------------------------------------------------------------
 * Widget API functions
 *--------------------------------------------------------------------------------*/
int widget_event(const Widget_t *widgets, size_t size, EventPoint_t &ep) {
  for (int i = 0; i < size; i++) {

    // In case the touch event to be detected
    if ((widgets[i].event & ep.event) &&  widgets[i].callback) {

      // Find the widget where the event fired
      if (widgets[i].x <= ep.x && ep.x <= widgets[i].x + widgets[i].w &&
          widgets[i].y <= ep.y && ep.y <= widgets[i].y + widgets[i].h) {

        DBG_EXEC(printf("event = %d, x = %d, y = %d\n", ep.event, ep.x, ep.y));
        widgets[i].callback(ep);
        return i;
      }
    }
  }

  return -1;
}