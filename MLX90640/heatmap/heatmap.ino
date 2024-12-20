#include <Arduino.h>
#include <math.h>

/*--------------------------------------------------------------------------------
 * Inferno
 *--------------------------------------------------------------------------------*/
#define N_POINTS  25
float calcR(float x) { float y =   -0.0186f * pow(x, 3.0f) + 0.3123f * pow(x, 2.0f) + 11.9230f * x + 36.6580f; return y > 0.0 ? y : 0.0f; }
float calcG(float x) { float y =    0.0042f * pow(x, 3.0f) + 0.2183f * pow(x, 2.0f) +  1.0843f * x +  8.0676f; return y > 0.0 ? y : 0.0f; }
float calcB(float x) { float y =    0.0743f * pow(x, 3.0f) - 2.7407f * pow(x, 2.0f) + 23.1360f * x + 61.5370f; return y > 0.0 ? y : 0.0f; }
//float calcB(float x) { float y = -3.221e-5f * pow(x, 6.0f) + 0.0026f * pow(x, 5.0f) -  0.0780f * pow(x, 4.0f) + 1.0976f * pow(x, 3.0f) - 8.2067f * pow(x, 2.0f) + 30.074f * x + 68.001f; return y > 0.0 ? y : 0.0f; }

#define N_GRADATION 256
#define RGB565(r, g, b) ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

int printf(const char* fmt, ...) {
	char buf[256];

	va_list arg_ptr;
	va_start(arg_ptr, fmt);
	int len = vsnprintf(buf, sizeof(buf), fmt, arg_ptr);
	va_end(arg_ptr);

  len = Serial.print(buf);
  Serial.flush();
	return len;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();

  uint16_t R, G, B;
  uint16_t RGB;
  float r, g, b;
  float x = 1.0f;
  const float step = (float)(N_POINTS - 1) / (float)N_GRADATION;

  for (int i = 1; i <= N_GRADATION; i++, x += step) {
    r = calcR(x);
    g = calcG(x);
    b = calcB(x);

    R = (uint16_t)(r + 0.5f);
    G = (uint16_t)(g + 0.5f);
    B = (uint16_t)(b + 0.5f);
#if 1
    RGB = RGB565(R, G, B);
    printf("0x%4X,", RGB);

    if (i % 16 == 0) {
      printf("\n");
    }
#else
    printf("x: %.1f, R: %d, G: %d, B: %d\n", x, R, G, B);
#endif
  }
}

void loop() {}