/*================================================================================
 * printf() to the serial console through the 'Serial'.
 *================================================================================*/
#include <stdio.h>
#include <stdarg.h>

#define BUF_SIZE  64

#if     DEBUG

int printf(const char* fmt, ...) {
	char buf[BUF_SIZE];

	va_list arg_ptr;
	va_start(arg_ptr, fmt);
	int len = vsnprintf(buf, sizeof(buf), fmt, arg_ptr);
	va_end(arg_ptr);

  len = Serial.print(buf);
  Serial.flush();
	return len;
}

#endif

void gfx_printf(uint16_t x, uint16_t y, const char* fmt, ...) {
  int len = 0;
  char buf[16];

  va_list arg_ptr;
  va_start(arg_ptr, fmt);
  len = vsnprintf(buf, sizeof(buf), fmt, arg_ptr);
  va_end(arg_ptr);

  GFX_EXEC(fillRect(x, y, len * FONT_WIDTH, FONT_HEIGHT, BLACK));
  GFX_EXEC(setCursor(x, y));
  GFX_EXEC(print(buf));
}