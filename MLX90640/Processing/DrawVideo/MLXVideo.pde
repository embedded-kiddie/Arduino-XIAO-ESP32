
int MLX90640_COLS = 32;
int MLX90640_ROWS = 24;
int MLX90640_SIZE = (MLX90640_COLS * MLX90640_ROWS * 4);

int DISPLAY_SCALE = 15;
int DISPLAY_COLS = (MLX90640_COLS * DISPLAY_SCALE);
int DISPLAY_ROWS = (MLX90640_ROWS * DISPLAY_SCALE);

int INTERPOLATE_SCALE = 8;
int INTERPOLATED_COLS = (MLX90640_COLS * INTERPOLATE_SCALE);
int INTERPOLATED_ROWS = (MLX90640_ROWS * INTERPOLATE_SCALE);

// Inferno
int N_POINTS = 25;
float calcR(float x) { float y = -0.0186f * pow(x, 3.0f) + 0.3123f * pow(x, 2.0f) + 11.9230f * x + 36.6580f; return constrain(y, 0.0f, 255.0f); }
float calcG(float x) { float y =  0.0042f * pow(x, 3.0f) + 0.2183f * pow(x, 2.0f) +  1.0843f * x +  8.0676f; return constrain(y, 0.0f, 255.0f); }
float calcB(float x) { float y =  0.0743f * pow(x, 3.0f) - 2.7407f * pow(x, 2.0f) + 23.1360f * x + 61.5370f; return constrain(y, 0.0f, 255.0f); }

int N_GRADATION = 1024;

public class ColorMap {
  public float[] r = new float[N_GRADATION];
  public float[] g = new float[N_GRADATION];
  public float[] b = new float[N_GRADATION];

  public ColorMap() {
    float x = 1.0f;
    float step = (float)(N_POINTS - 1) / (float)N_GRADATION;

    for (int i = 0; i < N_GRADATION; i++, x += step) {
      r[i] = calcR(x);
      g[i] = calcG(x);
      b[i] = calcB(x);
    }
  }
}

public class MLXVideo {
  private boolean autoRange = false;
  private ColorMap heatmap = null;
  private RandomAccessFile reader = null;
  private long frameNo = 0;
  private long frameCount = 0;
  private float[] data = new float[768];
  private int  [] temp = new int  [768];

  public MLXVideo(String filename) {
    noStroke();
    colorMode(RGB);
    heatmap = new ColorMap();

    try {
      // https://zawaworks.hatenablog.com/entry/2017/10/08/213602
      reader = new RandomAccessFile(sketchPath() + "/" + filename, "r");
      frameCount = reader.length() / MLX90640_SIZE;
      frameNo = 0;
      println("frameCount: " + frameCount);
    }
    catch (FileNotFoundException e) {
      e.printStackTrace();
      exit();
    }
    catch (IOException e) {
      e.printStackTrace();
      exit();
    }
  }

  public void read(long n) {
    try {
      reader.seek(n * MLX90640_SIZE);
      
      byte[] buffer = new byte[4];
      for (int i = 0; i < 768; i++) {
        if (reader.read(buffer) != 4) {
          throw new IOException("Unexpected End of Stream");
        }

        // https://www.jpcert.or.jp/java-rules/fio12-j.html
        data[i] = ByteBuffer.wrap(buffer).order(ByteOrder.LITTLE_ENDIAN).getFloat();
      }
    }
    catch (IOException e) {
      e.printStackTrace();
      exit();
    }
  }

  public void rewind() {
    frameNo = 0;
  }

  public boolean next() {
    if (frameNo < frameCount - 1) {
      frameNo++;
      return true;
    } else {
      return false;
    }
  }

  public boolean prev() {
    if (frameNo > 0) {
      frameNo--;
      return true;
    } else {
      return false;
    }
  }

  void draw() {
    read(frameNo);  // Read frame into data[]
    background(0);  // Clear the screen with a black background

    float maxTemp = -999.0f;
    float minTemp = +999.0f;

    if (autoRange) {
      // For each floating point value, double check that we've acquired a number,
      // then determine the min and max temperature values for this frame
      for (int i = 0; i < 768; i++) {
        if (Float.isNaN(data[i])) continue;
        if (data[i] > maxTemp) {
          maxTemp = data[i];
        } else if (data[i] < minTemp) {
          minTemp = data[i];
        }
      }
    } else {
      maxTemp = 35.0f;
      minTemp = 20.0f;
    }

    // for each of the 768 values, map the temperatures between min and max
    // to the blue through red portion of the color space
    for (int i = 0; i < 768; i++) {
      if (!Float.isNaN(data[i])) {
        int t = round(map(data[i], minTemp, maxTemp, 0.0f, (float)(N_GRADATION - 1)));
        temp[i] = constrain(t, 0, N_GRADATION - 1);
      } else {
        temp[i] = 0;
      }
    }

    // Draw heatmap
    int i = 0;
    for (int y = 0; y < DISPLAY_ROWS; y += DISPLAY_SCALE) {
      for (int x = 0; x < DISPLAY_COLS; x += DISPLAY_SCALE) {
        int t = temp[i];
        fill(heatmap.r[t], heatmap.g[t], heatmap.b[t]);
        rect(x, y, DISPLAY_SCALE, DISPLAY_SCALE);
        i++;
      }
    }

    // Add a gaussian blur to the canvas in order to create a rough
    // visual interpolation between pixels.
    filter(BLUR, 7);

    // Generate the legend on the bottom of the screen
    textSize(32);

    // Find the difference between the max and min temperatures in this frame
    float tempDif = maxTemp - minTemp;

    // Find 5 intervals between the max and min
    int legendInterval = round(tempDif / 5);

    // Set the first legend key to the min temp
    int legendTemp = round(minTemp);

    // Print each interval temperature in its corresponding heatmap color
    for (int intervals = 0; intervals < 6; intervals++) {
      int t = round(map(legendTemp, minTemp, maxTemp, 0.0f, (float)(N_GRADATION - 1)));
      t = constrain(t, 0, N_GRADATION - 1);
      fill(heatmap.r[t], heatmap.g[t], heatmap.b[t]);
      text(legendTemp + "°", 70 * intervals, 390);
      legendTemp += legendInterval;
    }
  }
}
