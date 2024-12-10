import java.io.*;
import java.nio.*;

String file = "./data/mlx0100.raw";

int MLX90640_COLS = 32;
int MLX90640_ROWS = 24;
int MLX90640_SIZE = (MLX90640_COLS * MLX90640_ROWS * 4);

int INTERPOLATE_SCALE = 8;
int INTERPOLATED_COLS = (MLX90640_COLS * INTERPOLATE_SCALE);
int INTERPOLATED_ROWS = (MLX90640_ROWS * INTERPOLATE_SCALE);

MLXVideo video;

void setup() {
  size(480, 400);  // Size must be the first statement
  noStroke();
  frameRate(16);

  video = new MLXVideo(file);
  video.rewind();
}

void draw() {
  video.draw();
  if (!video.next()) {
    video.rewind();
  }
}
