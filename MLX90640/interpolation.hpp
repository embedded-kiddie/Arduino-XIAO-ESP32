#include <Arduino.h>

inline float get_point(float *p, const int rows, const int cols, int x, int y) __attribute__((always_inline));
inline float get_point(float *p, const int rows, const int cols, int x, int y) {
  if (x < 0) { x = 0; }
  if (y < 0) { y = 0; }
  if (x >= cols) { x = cols - 1; }
  if (y >= rows) { y = rows - 1; }
  return p[y * cols + x];
}

#if 0

/*----------------------------------------------------------------------------------------------------
 * Bicubic interpolation
 * https://github.com/adafruit/Adafruit_AMG88xx/tree/master/examples/thermal_cam_interpolate
 *----------------------------------------------------------------------------------------------------*/

inline void set_point(float *p, const int rows, const int cols, int x, int y, float f) __attribute__((always_inline));
inline void set_point(float *p, const int rows, const int cols, int x, int y, float f) {
  if ((x < 0) || (x >= cols)) { return; }
  if ((y < 0) || (y >= rows)) { return; }
  p[y * cols + x] = f;
}

// src is rows*cols and dst is a 4-point array passed in already allocated!
inline void get_adjacents_1d(float *src, float *dst, const int rows, const int cols, int x, int y) __attribute__((always_inline));
inline void get_adjacents_1d(float *src, float *dst, const int rows, const int cols, int x, int y) {
  // Serial.print("("); Serial.print(x); Serial.print(", "); Serial.print(y);
  // Serial.println(")");
  // pick two items to the left
  dst[0] = get_point(src, rows, cols, x - 1, y);
  dst[1] = get_point(src, rows, cols, x, y);
  // pick two items to the right
  dst[2] = get_point(src, rows, cols, x + 1, y);
  dst[3] = get_point(src, rows, cols, x + 2, y);
}

// src is rows*cols and dst is a 16-point array passed in already allocated!
inline void get_adjacents_2d(float *src, float *dst, const int rows, const int cols, int x, int y) __attribute__((always_inline));
inline void get_adjacents_2d(float *src, float *dst, const int rows, const int cols, int x, int y) {
  // Serial.print("("); Serial.print(x); Serial.print(", "); Serial.print(y);
  // Serial.println(")");
  for (int delta_y = -1; delta_y < 3; delta_y++) { // -1, 0, 1, 2
    float *row = dst + 4 * (delta_y + 1); // index into each chunk of 4
    for (int delta_x = -1; delta_x < 3; delta_x++) { // -1, 0, 1, 2
      row[delta_x + 1] = get_point(src, rows, cols, x + delta_x, y + delta_y);
    }
  }
}

// p is a list of 4 points, 2 to the left, 2 to the right
inline float cubicInterpolate(float p[], float x) __attribute__((always_inline));
inline float cubicInterpolate(float p[], float x) {
  float r = p[1] + (0.5 * x *
                    (p[2] - p[0] +
                     x * (2.0 * p[0] - 5.0 * p[1] + 4.0 * p[2] - p[3] +
                          x * (3.0 * (p[1] - p[2]) + p[3] - p[0]))));
  /*
    Serial.print("interpolating: [");
    Serial.print(p[0],2); Serial.print(", ");
    Serial.print(p[1],2); Serial.print(", ");
    Serial.print(p[2],2); Serial.print(", ");
    Serial.print(p[3],2); Serial.print("] w/"); Serial.print(x); Serial.print("
    = "); Serial.println(r);
  */
  return r;
}

// p is a 16-point 4x4 array of the 2 rows & columns left/right/above/below
inline float bicubicInterpolate(float p[], float x, float y) __attribute__((always_inline));
inline float bicubicInterpolate(float p[], float x, float y) {
  float arr[4] = {0, 0, 0, 0};
  arr[0] = cubicInterpolate(p + 0, x);
  arr[1] = cubicInterpolate(p + 4, x);
  arr[2] = cubicInterpolate(p + 8, x);
  arr[3] = cubicInterpolate(p + 12, x);
  return cubicInterpolate(arr, y);
}

void interpolate_setup(const int dst_rows, const int dst_cols, int scale) {}

// src is a grid src_rows * src_cols
// dst is a pre-allocated grid, dst_rows*dst_cols
void interpolate_image(float *src, const int src_rows, const int src_cols,
                       float *dst, const int dst_rows, const int dst_cols) {
  float mu_x = (src_cols - 1.0) / (dst_cols - 1.0);
  float mu_y = (src_rows - 1.0) / (dst_rows - 1.0);

  float adjacents[16]; // matrix for storing adjacents

  for (int y_idx = 0; y_idx < dst_rows; y_idx++) {
    for (int x_idx = 0; x_idx < dst_cols; x_idx++) {
      float x = x_idx * mu_x;
      float y = y_idx * mu_y;
      // Serial.print("("); Serial.print(y_idx); Serial.print(", ");
      // Serial.print(x_idx); Serial.print(") = "); Serial.print("(");
      // Serial.print(y); Serial.print(", "); Serial.print(x); Serial.print(")");
      //get_adjacents_1d(src, adjacents, src_rows, src_cols, x, y);
      get_adjacents_2d(src, adjacents, src_rows, src_cols, (int)x, (int)y);
      /*
      Serial.print("[");
      for (int i=0; i<16; i++) {
        Serial.print(adjacents[i]); Serial.print(", ");
      }
      Serial.println("]");
      */
      float frac_x = x - (int)x; // we only need the ~delta~ between the points
      float frac_y = y - (int)y; // we only need the ~delta~ between the points
      float t = bicubicInterpolate(adjacents, frac_x, frac_y);
      // Serial.print("\tInterp: "); Serial.println(t);
#if 1
      set_point(dst, dst_rows, dst_cols, x_idx, y_idx, t);
#else
      t = min((int)t, MAXTEMP);
      t = max((int)t, MINTEMP); 

      int colorIndex = map(t, MINTEMP, MAXTEMP, 0, 255);
      colorIndex = constrain(colorIndex, 0, 255);

      GFX_EXEC(fillRect(BOX_SIZE * (dst_cols - 1 - x_idx), BOX_SIZE * y_idx, BOX_SIZE, BOX_SIZE, camColors[colorIndex]));
#endif
    }
  }
}

#else

/*----------------------------------------------------------------------------------------------------
 * Bilinear interpolation
 * https://algorithm.joho.info/image-processing/bi-linear-interpolation/
 *----------------------------------------------------------------------------------------------------*/
static float table_ratio[INTERPOLATE_SCALE][2];

void interpolate_setup(const int scale) {
  for (int i = 0; i < INTERPOLATE_SCALE; i++) {
    table_ratio[i][0] = (float)i / (float)scale;
    table_ratio[i][1] = 1.0f - table_ratio[i][0];
  }
}

void interpolate_image(float *src, const int src_rows, const int src_cols, float *dst, const int dst_rows, const int dst_cols) {
  int X, Y;
  int scale = dst_rows / src_rows;
  float X0Y0, X1Y0, X0Y1, X1Y1;
  float x_ratio_lo, x_ratio_hi;
  float y_ratio_lo, y_ratio_hi;

  // Bilinear interpolation
  for (int y = 0; y < dst_rows; y++) {
    Y = y / scale;
    y_ratio_lo = table_ratio[y % scale][0];
    y_ratio_hi = table_ratio[y % scale][1];

    for (int x = 0; x < dst_cols; x += INTERPOLATE_SCALE) {
      X = x / scale;
      X0Y0 = get_point(src, src_rows, src_cols, X,     Y    );
      X1Y0 = get_point(src, src_rows, src_cols, X + 1, Y    );
      X0Y1 = get_point(src, src_rows, src_cols, X,     Y + 1);
      X1Y1 = get_point(src, src_rows, src_cols, X + 1, Y + 1);

      for (int i = 0; i < INTERPOLATE_SCALE; i++) {
        x_ratio_lo = table_ratio[i][0];
        x_ratio_hi = table_ratio[i][1];

        float t = y_ratio_hi * (x_ratio_hi * X0Y0 + x_ratio_lo * X1Y0) +
                  y_ratio_lo * (x_ratio_hi * X0Y1 + x_ratio_lo * X1Y1);

        dst[x + i + y * dst_cols] = t;
      }
    }
  }
}

#endif