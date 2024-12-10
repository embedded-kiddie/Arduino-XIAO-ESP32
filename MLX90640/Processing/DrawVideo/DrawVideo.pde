import java.io.*;
import java.nio.*;

String file = "./data/mlx0100.raw";

MLXVideo video;

void setup() {
  size(480, 400);
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
