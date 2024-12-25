/*================================================================================
 * printf() to the serial console through the 'Serial'.
 *================================================================================*/
#include <stdio.h>
#include <stdarg.h>

#ifndef BUF_SIZE
#define BUF_SIZE  64
#endif

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

#ifdef  GFX_EXEC

void gfx_printf(uint16_t x, uint16_t y, const char* fmt, ...) {
  int len = 0;
  char buf[BUF_SIZE];

  va_list arg_ptr;
  va_start(arg_ptr, fmt);
  len = vsnprintf(buf, sizeof(buf), fmt, arg_ptr);
  va_end(arg_ptr);

  // use setTextColor(foreground_color, background_color) and setTextDatum(textdatum_t::...)
  GFX_EXEC(drawString(buf, x, y));
}

#endif // GFX_EXEC