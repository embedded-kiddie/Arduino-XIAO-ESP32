
int DISPLAY_SCALE = 15;
int DISPLAY_COLS = (MLX90640_COLS * DISPLAY_SCALE);
int DISPLAY_ROWS = (MLX90640_ROWS * DISPLAY_SCALE);

public class MLXVideo {
  RandomAccessFile reader = null;
  long frameNo = 0;
  long frameCount = 0;
  float[] data = new float[768];
  float[] temp = new float[768];

  public MLXVideo(String filename) {
    colorMode(HSB, 360, 100, 100);
    background(0);  // Clear the screen with a black background

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

    // For each floating point value, double check that we've acquired a number,
    // then determine the min and max temperature values for this frame
    for (int i = 0; i < 768; i++) {
      if (!Float.isNaN(data[i]) && data[i] > maxTemp) {
        maxTemp = data[i];
      } else if (!Float.isNaN(data[i]) && data[i] < minTemp) {
        minTemp = data[i];
      }
    }

    // for each of the 768 values, map the temperatures between min and max
    // to the blue through red portion of the color space
    for (int i = 0; i < 768; i++) {
      if (!Float.isNaN(data[i])) {
        temp[i] = constrain(map(data[i], minTemp, maxTemp, 180, 360), 160, 360);
      } else {
        temp[i] = 0.0f;
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
        fill(temp[i], 100, 100);
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
      fill(constrain(map(legendTemp, minTemp, maxTemp, 180, 360), 160, 360), 100, 100);
      text(legendTemp + "°", 70 * intervals, 390);
      legendTemp += legendInterval;
    }
  }
}
