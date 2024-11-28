/*================================================================================
 * Functions for drawing widget
 *================================================================================*/
#include <Arduino.h>

/*--------------------------------------------------------------------------------
 * Functions prototyping
 *--------------------------------------------------------------------------------*/
void DrawColorRange(uint8_t flag);
static void DrawScreen(const Widget_t *widget);
static void DrawWidget(const Widget_t *widget, uint8_t offset = 0);
static void DrawButton(const Widget_t *widget, uint8_t offset = 0);
static void DrawToggle(const Widget_t *widget, bool check = false);
static void DrawCheck (const Widget_t *widget, bool check = false);
static void DrawPress (const Widget_t *widget, Event_t event = EVENT_INIT);
static void DrawSlider(const Widget_t *widget, int16_t pos, bool enable = true);
static void DrawRadio (const Widget_t *widget, uint8_t n_widget, uint8_t selected = 0);
static void DrawThumb (const Widget_t *widget, const char *path);

/*--------------------------------------------------------------------------------
 * Sprite object
 *--------------------------------------------------------------------------------*/
#if defined (LOVYANGFX_HPP_)

static LGFX_Sprite sprite(&lcd);

#elif defined (_TFT_eSPIH_)

static TFT_eSprite sprite(&tft);

#endif

/*--------------------------------------------------------------------------------
 * Draw colorbar and range
 *--------------------------------------------------------------------------------*/
void DrawColorRange(uint8_t flag) {
  const int n = sizeof(camColors) / sizeof(camColors[0]);
  const int w = mlx_cnf.box_size * mlx_cnf.interpolation * MLX90640_COLS;
  int       y = mlx_cnf.box_size * mlx_cnf.interpolation * MLX90640_ROWS + 3;
  
  GFX_EXEC(startWrite());
  
  // Draw color bar
  if (flag & 1) {
    for (int i = 0; i < n; i++) {
      int x = map(i, 0, n, 0, w);
#if defined (LOVYANGFX_HPP_)
      GFX_EXEC(writeFastVLine(x, y, FONT_HEIGHT, camColors[i]));
#else
      GFX_EXEC(drawFastVLine(x, y, FONT_HEIGHT, camColors[i]));
#endif
    }
  }

  // Draw thermal range
  if (flag & 2) {
    y += FONT_HEIGHT + 4;
    const uint8_t size = mlx_cnf.interpolation * mlx_cnf.box_size > 4 ? 2 : 1;
    const int font_w = (size == 2 ? FONT_WIDTH  : FONT_WIDTH  >> 1);
    const int font_h = (size == 2 ? FONT_HEIGHT : FONT_HEIGHT >> 1);
    GFX_EXEC(setTextSize(size));

    GFX_EXEC(setTextDatum(TL_DATUM));
    gfx_printf(0, y, "%d  ", mlx_cnf.range_min);

    GFX_EXEC(setTextDatum(TR_DATUM));
    gfx_printf(w, y, "  %d", mlx_cnf.range_max);

    if (mlx_cnf.interpolation * mlx_cnf.box_size > 1) {
      GFX_EXEC(setTextDatum(TC_DATUM));
      gfx_printf(w / 2, y, " %3.1f ", (float)(mlx_cnf.range_min + mlx_cnf.range_max) / 2.0f);
    }
  }

  // Draw min/max point
  if (flag & 4) {
    const int W = mlx_cnf.box_size * mlx_cnf.interpolation;
    GFX_EXEC(fillCircle((MLX90640_COLS - tmin.x - 1) * W, tmin.y * W, 3, BLACK));
    GFX_EXEC(fillCircle((MLX90640_COLS - tmax.x - 1) * W, tmax.y * W, 3, BLACK));
  }

  GFX_EXEC(endWrite());
}

#ifdef _TFT_eSPIH_
/*--------------------------------------------------------------------------------
 * Helper function for TFT_eSPI
 * Include the PNG decoder library, available via the IDE library manager
 * https://github.com/Bodmer/TFT_eSPI/tree/master/examples/PNG%20Images
 * pngDraw: Callback function to draw pixels to the display
 *--------------------------------------------------------------------------------*/
#include <PNGdec.h>

static PNG png; // PNG decoder instance

// Position variables must be global (PNGdec does not handle position coordinates)
static uint16_t xpos = 0;
static uint16_t ypos = 0;

/*--------------------------------------------------------------------------------
 * This next function will be called during decoding of the png file to
 * render each image line to the TFT. If you use a different TFT library
 * you will need to adapt this function to suit.
 *--------------------------------------------------------------------------------*/
static void pngDraw(PNGDRAW *pDraw) {
  uint16_t lineBuffer[TFT_WIDTH > TFT_HEIGHT ? TFT_WIDTH : TFT_HEIGHT];
  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
  GFX_EXEC(pushImage(xpos, ypos + pDraw->y, pDraw->iWidth, 1, lineBuffer));
}

static void pngSprite(PNGDRAW *pDraw) {
  uint16_t lineBuffer[TFT_WIDTH > TFT_HEIGHT ? TFT_WIDTH : TFT_HEIGHT];
  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
  sprite.pushImage(xpos, ypos + pDraw->y, pDraw->iWidth, 1, lineBuffer);
}

static void DrawPNG(const uint8_t *img, size_t size, uint16_t x, uint16_t y, PNG_DRAW_CALLBACK *draw) {
  xpos = x;
  ypos = y;

  if (png.openFLASH((uint8_t*)img, size, draw) == PNG_SUCCESS) {
    GFX_EXEC(startWrite());
    png.decode(NULL, 0);
    GFX_EXEC(endWrite());
//  png.close(); // Required for files, not needed for FLASH arrays
  }
}
#endif // _TFT_eSPIH_

/*--------------------------------------------------------------------------------
 * Converting byte order according to MCU architecture
 *--------------------------------------------------------------------------------*/
#if defined (__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)

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
 * Get width and height of PNG image
 *--------------------------------------------------------------------------------*/
#define PNG_HEADER_WIDTH    16 // PNG file signature + offset from chunk data
#define PNG_HEADER_HEIGHT   20 // PNG file signature + offset from chunk data

inline uint32_t get_width (const uint8_t *data) __attribute__((always_inline));
inline uint32_t get_height(const uint8_t *data) __attribute__((always_inline));
inline uint32_t get_width (const uint8_t *data) { return swap_endian(*(uint32_t*)(data + PNG_HEADER_WIDTH )); }
inline uint32_t get_height(const uint8_t *data) { return swap_endian(*(uint32_t*)(data + PNG_HEADER_HEIGHT)); }

/*--------------------------------------------------------------------------------
 * Draw widget
 *--------------------------------------------------------------------------------*/
static void DrawWidget(const Widget_t *widget, uint8_t offset /* = 0 */) {
  const Image_t *image = &widget->image[offset];

  if (image) {
    GFX_EXEC(startWrite());

#if   defined (LOVYANGFX_HPP_)

    GFX_EXEC(drawPng(image->data, image->size, widget->x, widget->y));

#elif defined (_TFT_eSPIH_)

    DrawPNG(image->data, image->size, widget->x, widget->y, pngDraw);

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
static void DrawButton(const Widget_t *widget, uint8_t offset /* = 0 */) {
  DrawWidget(widget, offset);
}

/*--------------------------------------------------------------------------------
 * Draw slider
 *--------------------------------------------------------------------------------*/
static void DrawSlider(const Widget_t *widget, int16_t pos, bool enable /* = true */) {
  const Image_t *bar  = &widget->image[0];
  const Image_t *knob = &widget->image[enable ? 1 : 2]; // [1]: enable, [2]: disable

  if (bar && knob) {
    GFX_EXEC(startWrite());

    sprite.createSprite(get_width(bar->data), get_height(bar->data));

#if   defined (LOVYANGFX_HPP_)

    sprite.drawPng(bar->data,  bar->size,  0,   0);
    sprite.drawPng(knob->data, knob->size, pos, 0);

#elif defined (_TFT_eSPIH_)

    DrawPNG(bar->data,  bar->size,  0,   0, pngSprite);
    DrawPNG(knob->data, knob->size, pos, 0, pngSprite);

#endif

    sprite.pushSprite(widget->x, widget->y);
    sprite.deleteSprite();

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

#if   defined (LOVYANGFX_HPP_)

  lgfx::rgb888_t rgb[PRESSED_BUFFER_LEN];

#elif defined (_TFT_eSPIH_)

  uint16_t rgb[PRESSED_BUFFER_LEN];

#endif

  if (event & EVENT_FALLING) {
    offset = +PRESSED_OFFSET;
  } else if (event & EVENT_RISING) {
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

  if (event & EVENT_RISING) {
    touch_clear();
  }
}

/*--------------------------------------------------------------------------------
 * Generate and draw thumbnail from a bitmap file
 * (LovyanGFX requires SD library header file before including <LovyanGFX.hpp>)
 *--------------------------------------------------------------------------------*/
static void DrawThumb(const Widget_t *widget, const char *path) {
  GFX_EXEC(fillRect(widget->x, widget->y, widget->w, widget->h, BLACK));

#if defined (LOVYANGFX_HPP_) && (defined (_SD_H_) || defined (SdFat_h))

  GFX_EXEC(drawBmpFile(
    SD,
    path,
    widget->x, widget->y, widget->w, widget->h,
    0, 0,
    0.4, 0.4  // 320 x 240 --> 128 x 96
  ));

#else
#warning TFT_eSPI support required
#endif
}