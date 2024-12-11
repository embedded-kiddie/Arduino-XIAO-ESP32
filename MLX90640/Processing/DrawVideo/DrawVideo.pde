import java.io.*;
import java.nio.*;

String file = "./data/mlx0100.raw";

MLXVideo video;

int counter = 0;

void setup() {
  size(480, 400);
  frameRate(16);

  video = new MLXVideo(file);
  video.Rewind();
  video.SetFilterSize(0);
  video.SetInterpolation(false);
}

void draw() {
  video.Draw();
  if (!video.Next()) {
    video.Rewind();
    switch (++counter % 3) {
      case 0:
        video.SetFilterSize(0);
        video.SetInterpolation(false);
        break;
      case 1:
        video.SetInterpolation(true);
        break;
      case 2:
        video.SetFilterSize(7);
        video.SetInterpolation(false);
        break;
    }
  }
}
