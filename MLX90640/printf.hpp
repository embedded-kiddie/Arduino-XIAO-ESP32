/*================================================================================
 * printf() to the serial console through the 'Serial'.
 *================================================================================*/
#include <stdio.h>
#include <stdarg.h>

#define BUF_SIZE  64

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