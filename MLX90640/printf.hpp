/*
 * printf() to the serial console through the 'Serial'.
 */
#include <stdio.h>
#include <stdarg.h>

#define PRINTF_BUF_SIZE  256

int printf(const char* fmt, ...) {
	char buf[PRINTF_BUF_SIZE];

	va_list arg_ptr;
	va_start(arg_ptr, fmt);
	int len = vsnprintf(buf, PRINTF_BUF_SIZE, fmt, arg_ptr);
	va_end(arg_ptr);

  len = Serial.print(buf);
  Serial.flush();
	return len;
}