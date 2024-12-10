
int DISPLAY_SCALE = 15;
int DISPLAY_COLS = (MLX90640_COLS * DISPLAY_SCALE);
int DISPLAY_ROWS = (MLX90640_ROWS * DISPLAY_SCALE);

int N_GRADATION = 1024;

int N_POINTS = 25;
float calcR(float x) { float y =   -0.0186f * pow(x, 3.0f) + 0.3123f * pow(x, 2.0f) + 11.9230f * x + 36.6580f; return y > 0.0 ? y : 0.0f; }
float calcG(float x) { float y =    0.0042f * pow(x, 3.0f) + 0.2183f * pow(x, 2.0f) +  1.0843f * x +  8.0676f; return y > 0.0 ? y : 0.0f; }
float calcB(float x) { float y = -3.221e-5f * pow(x, 6.0f) + 0.0026f * pow(x, 5.0f) -  0.0780f * pow(x, 4.0f) + 1.0976f * pow(x, 3.0f) - 8.2067f * pow(x, 2.0f) + 30.074f * x + 68.001f; return y > 0.0 ? y : 0.0f; }

public class ColorMap {
  public ColorMap(int[] r, int[] g, int[] b) {
    float R, G, B;
    float x = 1.0f;
    float step = (float)(N_POINTS - 1) / (float)N_GRADATION;

    for (int i = 0; i < N_GRADATION; i++, x += step) {
      R = calcR(x);
      G = calcG(x);
      B = calcB(x);
  
      r[i] = floor(R);
      g[i] = floor(G);
      b[i] = floor(B);
    }
  }
}

public class MLXVideo {
  private RandomAccessFile reader = null;
  private ColorMap cmap = null;
  private long frameNo = 0;
  private long frameCount = 0;
  private float[] data = new float[768];
  private int[] temp = new int[768];

  private int[] r = new int[N_GRADATION];
  private int[] g = new int[N_GRADATION];
  private int[] b = new int[N_GRADATION];

  public MLXVideo(String filename) {
//  colorMode(HSB, 360, 100, 100);
    colorMode(RGB);
    background(0);  // Clear the screen with a black background

    cmap = new ColorMap(r, g, b);

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
    read(frameNo);
    background(0);  // Clear the screen with a black background

    float maxTemp = -999.0f;
    float minTemp = +999.0f;
/*
    // For each floating point value, double check that we've acquired a number,
    // then determine the min and max temperature values for this frame
    for (int i = 0; i < 768; i++) {
      if (!Float.isNaN(data[i]) && data[i] > maxTemp) {
        maxTemp = data[i];
      } else if (!Float.isNaN(data[i]) && data[i] < minTemp) {
        minTemp = data[i];
      }
    }
//*/
//*
    maxTemp = 35.0f;
    minTemp = 20.0f;
//*/
    // for each of the 768 values, map the temperatures between min and max
    // to the blue through red portion of the color space
    for (int i = 0; i < 768; i++) {
      if (!Float.isNaN(data[i])) {
//      temp[i] = constrain(map(data[i], minTemp, maxTemp, 180, 360), 160, 360);
        temp[i] = (int)constrain(map(data[i], minTemp, maxTemp, 0, N_GRADATION - 1), 0, N_GRADATION - 1);
      } else {
        temp[i] = 0;
      }
    }

    // Prepare variables needed to draw our heatmap
    int x = 0;
    int y = 0;
    int i = 0;

    while (y < DISPLAY_ROWS) {
      // for each increment in the y direction, draw 8 boxes in the
      // x direction, creating a 64 pixel matrix
      while (x < DISPLAY_COLS) {
        // before drawing each pixel, set our paintcan color to the
        // appropriate mapped color value
        if (i >= temp.length) continue;
        int c = temp[i];
        fill(r[c], g[c], b[c]);
        rect(x, y, DISPLAY_SCALE, DISPLAY_SCALE);
        x = x + DISPLAY_SCALE;
        i++;
      }

      y = y + DISPLAY_SCALE;
      x = 0;
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
//    fill(constrain(map(legendTemp, minTemp, maxTemp, 180, 360), 160, 360), 100, 100);
      int c = (int)constrain(map(legendTemp, minTemp, maxTemp, 0, N_GRADATION - 1), 0, N_GRADATION - 1);
      fill(r[c], g[c], b[c]);
      text(legendTemp + "°", 70 * intervals, 390);
      legendTemp += legendInterval;
    }
  }
}
