/*================================================================================
 * Functions for drawing widget
 *================================================================================*/
#include <Arduino.h>

/*--------------------------------------------------------------------------------
 * Functions prototyping
 *--------------------------------------------------------------------------------*/
static void DrawScreen(const Widget_t *widget);
static void DrawWidget(const Widget_t *widget, uint8_t offset = 0);
static void DrawButton(const Widget_t *widget, uint8_t offset = 0);
static void DrawSlider(const Widget_t *widget, int16_t offset = 0);
static void DrawToggle(const Widget_t *widget, bool check = false);
static void DrawCheck (const Widget_t *widget, bool check = false);
static void DrawPress (const Widget_t *widget, Event_t event = EVENT_INIT);
static void DrawRadio (const Widget_t *widget, uint8_t n_widget, uint8_t selected = 0);
static void DrawThumb (const Widget_t *widget, const char *path);

/*--------------------------------------------------------------------------------
 * Helper function for TFT_eSPI
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

    CHECK_POS(GFX_EXEC(drawRect(widget->x, widget->y, widget->w, widget->h, RED)));
    GFX_EXEC(endWrite());
  }
}

/*--------------------------------------------------------------------------------
 * Draw screen
 *--------------------------------------------------------------------------------*/
static void DrawScreen(const Widget_t *widget) {
  DrawWidget(widget, 0);
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

    CHECK_POS(GFX_EXEC(drawRect(widget->x, widget->y, widget->w, widget->h, RED)));
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

    CHECK_POS(GFX_EXEC(drawRect(widget->x, widget->y, widget->w, widget->h, RED)));
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
#define PRESSED_BUFFER_LEN  64  // [px]
#define PRESSED_OFFSET      1   // 1 or 2 (need a wider background image)

static void DrawPress(const Widget_t *widget, Event_t event /* = EVENT_INIT */) {
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

    CHECK_POS(GFX_EXEC(drawRect(widget->x, widget->y, widget->w, widget->h, RED)));
    GFX_EXEC(endWrite());
  }

  if (event == EVENT_RISING) {
    touch_clear();
  }
}

/*--------------------------------------------------------------------------------
 * Generate and draw thumbnail from a bitmap file
 * (LovyanGFX function requires SdFat instead of the standard SD library of ESP32)
 *--------------------------------------------------------------------------------*/
static void DrawThumb(const Widget_t *widget, const char *path) {
#if defined (LOVYANGFX_HPP_) && (!defined (ESP32) || defined (SdFat_h))

  GFX_EXEC(drawBmpFile(
    SD,
    path,
    widget->x, widget->y, widget->w, widget->h,
    0, 0,
    0.4, 0.4  // 320 x 240 --> 128 x 96
  ));

#endif
}