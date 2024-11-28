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

static inline uint32_t get_width (const uint8_t *data) __attribute__((always_inline));
static inline uint32_t get_height(const uint8_t *data) __attribute__((always_inline));
static inline uint32_t get_width (const uint8_t *data) { return swap_endian(*(uint32_t*)(data + PNG_HEADER_WIDTH )); }
static inline uint32_t get_height(const uint8_t *data) { return swap_endian(*(uint32_t*)(data + PNG_HEADER_HEIGHT)); }

/*--------------------------------------------------------------------------------
 * Draw colorbar and range
 *--------------------------------------------------------------------------------*/
// icon-point.png
// https://lang-ship.com/tools/image2data/
// RAW File Dump
static constexpr unsigned char icon_point[270] = {
0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52, 
0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x3a, 0x98, 0xa0, 
0xbd, 0x00, 0x00, 0x00, 0xd5, 0x49, 0x44, 0x41, 0x54, 0x78, 0x01, 0x35, 0xce, 0x03, 0x52, 0x45, 
0x01, 0x00, 0x86, 0xd1, 0x2f, 0x73, 0x09, 0xcf, 0xd6, 0x30, 0xaf, 0x24, 0xd7, 0x4e, 0xb2, 0x07, 
0xd9, 0xe6, 0x20, 0xad, 0x23, 0x7b, 0x94, 0x5b, 0xc2, 0x33, 0xef, 0x9f, 0xcf, 0x0a, 0x0e, 0xdf, 
0xb0, 0x37, 0x8d, 0x8d, 0x35, 0xd9, 0xf9, 0x67, 0x59, 0x8e, 0xe8, 0x4b, 0x64, 0xd9, 0x02, 0xdf, 
0xaa, 0x9f, 0xf4, 0xb0, 0x96, 0xc9, 0xac, 0xdd, 0xeb, 0xb9, 0x1a, 0xc0, 0xfc, 0xa2, 0xc1, 0x4a, 
0x57, 0x26, 0xe3, 0xaa, 0x1c, 0xd4, 0x8b, 0x05, 0x58, 0xd5, 0x20, 0x04, 0xa5, 0x20, 0x0c, 0x6a, 
0x0d, 0xec, 0xd1, 0xfb, 0x62, 0x28, 0x9b, 0x9b, 0x2b, 0x83, 0xe2, 0xbb, 0x98, 0x9d, 0x26, 0x2d, 
0x7a, 0xdc, 0x45, 0x00, 0x45, 0x6e, 0xcf, 0x82, 0x9a, 0xd8, 0x56, 0x2a, 0x99, 0x98, 0x06, 0x98, 
0x4e, 0x24, 0x53, 0x3a, 0x60, 0x4f, 0x99, 0x4c, 0x7a, 0x01, 0x60, 0x21, 0x9d, 0xc9, 0xe8, 0x98, 
0x66, 0x2d, 0x85, 0x82, 0x25, 0x00, 0x25, 0xc1, 0xd0, 0x92, 0x9a, 0x71, 0xc6, 0x6f, 0x0a, 0xa1, 
0x74, 0x62, 0xa2, 0x14, 0x0a, 0xaf, 0xe3, 0x4e, 0xd8, 0x54, 0x0f, 0x04, 0xa4, 0x00, 0xf4, 0x68, 
0x0b, 0xb0, 0xbd, 0xab, 0xbb, 0xec, 0x3b, 0x56, 0xd6, 0xad, 0x0f, 0x1b, 0x40, 0xfd, 0xbb, 0xae, 
0x16, 0x32, 0x99, 0x85, 0x2b, 0xbd, 0x37, 0xc0, 0x37, 0xc7, 0x66, 0x42, 0x5f, 0x12, 0x5b, 0x0e, 
0xfe, 0x79, 0x3a, 0x4e, 0x4f, 0x3b, 0x3d, 0x7c, 0xfb, 0x04, 0x7f, 0xfb, 0x69, 0x12, 0x9f, 0xf2, 
0x94, 0x5a, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82, };

void DrawColorRange(uint8_t flag) {
  const int box = mlx_cnf.box_size * mlx_cnf.interpolation;
  const int w = box * MLX90640_COLS;
  int       y = box * MLX90640_ROWS + 3;

  GFX_EXEC(startWrite());
  
  // Draw color bar
  if (flag & 1) {
    const int n = sizeof(camColors) / sizeof(camColors[0]);
 
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
    const uint8_t size = (box > 4 ? 2 : 1);
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
  else if (flag & 4) {
    const int pw = get_width (icon_point);
    const int ph = get_height(icon_point);
    sprite.createSprite(pw, ph);

#if   defined (LOVYANGFX_HPP_)

    sprite.drawPng(icon_point, sizeof(icon_point), 0, 0);
    sprite.pushSprite(&lcd_sprite, (MLX90640_COLS - _tmin.x - 1) * box - (pw >> 1), _tmin.y * box - (ph >> 1), BLACK);
    sprite.pushSprite(&lcd_sprite, (MLX90640_COLS - _tmax.x - 1) * box - (pw >> 1), _tmax.y * box - (ph >> 1), BLACK);

#elif defined (_TFT_eSPIH_)

    DrawPNG(icon_point, sizeof(icon_point), 0, 0, pngSprite);
    sprite.pushSprite(&tft_sprite, (MLX90640_COLS - _tmin.x - 1) * box - (pw >> 1), _tmin.y * box - (ph >> 1), BLACK);
    sprite.pushSprite(&tft_sprite, (MLX90640_COLS - _tmax.x - 1) * box - (pw >> 1), _tmax.y * box - (ph >> 1), BLACK);

#endif

    sprite.deleteSprite();
  }

  GFX_EXEC(endWrite());
  GFX_EXEC(setTextSize(2));
  GFX_EXEC(setTextDatum(TL_DATUM));
}

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