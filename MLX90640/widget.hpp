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

/*--------------------------------------------------------------------------------
 * Widgets definition
 *--------------------------------------------------------------------------------*/
typedef enum {
  INSIDE = 0,
  INFO,
  SCALE,
  CAMERA1,
  CAMERA2,
  CONFIG,
} WidgetID_t;

void onInside (EventPoint_t &ep);
void onInform (EventPoint_t &ep);
void onColors (EventPoint_t &ep);
void onCamera (EventPoint_t &ep);
void onConfig (EventPoint_t &ep);

static const Widget_t widgets[] {
  {   0,   0, 256, 192, NULL, 0, EVENT_ALL, onInside },
  { 256,   0,  64, 140, NULL, 0, EVENT_ALL, onInform },
  {   0, 195, 256,  45, NULL, 0, EVENT_ALL, onColors },
  { 265, 135, ICON_WIDTH, ICON_HEIGHT, icon_camera1, sizeof(icon_camera1), EVENT_CLICK, onCamera },
  { 265, 135, ICON_WIDTH, ICON_HEIGHT, icon_camera2, sizeof(icon_camera2), EVENT_NONE,  nullptr  },
  { 265, 185, ICON_WIDTH, ICON_HEIGHT, icon_config,  sizeof(icon_config ), EVENT_CLICK, onConfig },
};

#define N_WIDGETS  (sizeof(widgets) / sizeof(widgets[0]))

/*--------------------------------------------------------------------------------
 * Event callback functions
 *--------------------------------------------------------------------------------*/
void DrawWidget(int id);

void onInside(EventPoint_t &ep) {
  DBG_EXEC(printf("onInside\n"));
}

void onInform(EventPoint_t &ep) {
  DBG_EXEC(printf("onInform\n"));
}

void onColors(EventPoint_t &ep) {
  DBG_EXEC(printf("onColors\n"));
}

void onCamera(EventPoint_t &ep) {
  DBG_EXEC(printf("onCamera\n"));
  DrawWidget(CAMERA2);
  sdcard_save();
  DrawWidget(CAMERA1);
}

void onConfig(EventPoint_t &ep) {
  DBG_EXEC(printf("onConfig\n"));
}

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

void DrawWidget(int id) {
  GFX_EXEC(startWrite());

  if (widgets[id].img) {
#if defined(_TFT_eSPIH_)
    DrawPNG((uint8_t*)widgets[id].img, widgets[id].size, widgets[id].x, widgets[id].y);
#elif defined(LOVYANGFX_HPP_)
    GFX_EXEC(drawPng((uint8_t*)widgets[id].img, widgets[id].size, widgets[id].x, widgets[id].y));
#endif
  }

  GFX_EXEC(endWrite());
}

void widget_setup(void) {
  DrawWidget(CAMERA1);
  DrawWidget(CONFIG);
}

void widget_event(EventPoint_t &ep) {
  for (int i = 0; i < N_WIDGETS; i++) {

    // In case the touch event to be detected
    if ((widgets[i].event & ep.event) &&  widgets[i].callback) {

      // Find the widget where the event fired
      if (widgets[i].x <= ep.x && ep.x <= widgets[i].x + widgets[i].w &&
          widgets[i].y <= ep.y && ep.y <= widgets[i].y + widgets[i].h) {
        DBG_EXEC(printf("event = %d, x = %d, y = %d\n", ep.event, ep.x, ep.y));
        widgets[i].callback(ep);
        delay(PERIOD_EVENT);
      }
    }
  }
}