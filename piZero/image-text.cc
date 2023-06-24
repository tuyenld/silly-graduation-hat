// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
//
// Example how to display an image, including animated images using
// ImageMagick. For a full utility that does a few more things, have a look
// at the led-image-viewer in ../utils
//
// Showing an image is not so complicated, essentially just copy all the
// pixels to the canvas. How to get the pixels ? In this example we're using
// the graphicsmagick library as universal image loader library that
// can also deal with animated images.
// You can of course do your own image loading or use some other library.
//
// This requires an external dependency, so install these first before you
// can call `make image-example`
//   sudo apt-get update
//   sudo apt-get install libgraphicsmagick++-dev libwebp-dev -y
//   make image-example

#include "led-matrix.h"

#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include <exception>
#include <Magick++.h>
#include <magick/image.h>

#include "graphics.h"
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

using rgb_matrix::Canvas;
using rgb_matrix::RGBMatrix;
using rgb_matrix::FrameCanvas;

// Make sure we can exit gracefully when Ctrl-C is pressed.
volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

static bool FullSaturation(const rgb_matrix::Color &c) {
  return (c.r == 0 || c.r == 255)
    && (c.g == 0 || c.g == 255)
    && (c.b == 0 || c.b == 255);
}

using ImageVector = std::vector<Magick::Image>;

// Given the filename, load the image 
// // If this is an animated image, the resutlting vector will contain multiple.
static ImageVector LoadImage(const char *filename) {
  ImageVector result;

  ImageVector frames;
  try {
    readImages(&frames, filename);
  } catch (std::exception &e) {
    if (e.what())
      fprintf(stderr, "%s\n", e.what());
    return result;
  }

  if (frames.empty()) {
    fprintf(stderr, "No image found.");
    return result;
  }

  // Animated images have partial frames that need to be put together
  if (frames.size() > 1) {
    Magick::coalesceImages(&result, frames.begin(), frames.end());
  } else {
    result.push_back(frames[0]); // just a single still image.
  }

  return result;
}


// Copy an image to a Canvas. Note, the RGBMatrix is implementing the Canvas
// interface as well as the FrameCanvas we use in the double-buffering of the
// animted image.
void CopyImageToCanvas(const Magick::Image &image, Canvas *canvas) {
  const int offset_x = 0, offset_y = 0;  // If you want to move the image.
  // Copy all the pixels to the canvas.
  for (size_t y = 0; y < image.rows(); ++y) {
    for (size_t x = 0; x < image.columns(); ++x) {
      const Magick::Color &c = image.pixelColor(x, y);
      if (c.alphaQuantum() < 256) {
        canvas->SetPixel(x + offset_x, y + offset_y,
                         ScaleQuantumToChar(c.redQuantum()),
                         ScaleQuantumToChar(c.greenQuantum()),
                         ScaleQuantumToChar(c.blueQuantum()));
      }
    }
  }
}

// An animated image has to constantly swap to the next frame.
// We're using double-buffering and fill an offscreen buffer first, then show.
void ShowAnimatedImage(const ImageVector &images, RGBMatrix *canvas) {
  FrameCanvas *offscreen_canvas = canvas->CreateFrameCanvas();
  while (!interrupt_received) {
    for (const auto &image : images) {
      if (interrupt_received) break;
      CopyImageToCanvas(image, offscreen_canvas);
      offscreen_canvas = canvas->SwapOnVSync(offscreen_canvas);
      usleep(image.animationDelay() * 10000);  // 1/100s converted to usec
    }
  }
}

int usage(const char *progname) {
  fprintf(stderr, "Usage: %s [led-canvas-options] <image-filename>\n",
          progname);
  rgb_matrix::PrintMatrixFlags(stderr);
  return 1;
}

int main(int argc, char *argv[]) {
  Magick::InitializeMagick(*argv);

  // Initialize the RGB canvas with
  RGBMatrix::Options matrix_options;
  rgb_matrix::RuntimeOptions runtime_opt;

  const char *bdf_font_file = "../fonts/7x14.bdf";
  int x_stop_point = 32; // end point for display
  std::string first_line = "tuyendl";
  std::string second_line = "Open to work";
  float speed = 1.0f;
  matrix_options.hardware_mapping = "adafruit-hat";  // or e.g. "adafruit-hat" or "adafruit-hat-pwm"
  matrix_options.chain_length = 1;
  matrix_options.rows = 32;
  matrix_options.cols = 64;
  matrix_options.show_refresh_rate = true;

  if (!rgb_matrix::ParseOptionsFromFlags(&argc, &argv,
                                         &matrix_options, &runtime_opt)) {
    return usage(argv[0]);
  }

  if (argc != 2)
    return usage(argv[0]);
  const char *filename = argv[1];

  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);
  printf("CTRL-C for exit.\n");

  RGBMatrix *canvas = RGBMatrix::CreateFromOptions(matrix_options, runtime_opt);
  if (canvas == NULL)
    return 1;

  rgb_matrix::Color color(255, 255, 0);
  rgb_matrix::Color bg_color(0, 0, 0);

  /* x_origin is set by default just right of the screen */
  const int x_default_start = (matrix_options.chain_length
                               * matrix_options.cols) + 5;
  int x_orig = x_default_start;
  int y_orig = 0;
  int letter_spacing = 0;
  int loops = -1;

//   const bool all_extreme_colors = (matrix_options.brightness == 100)
//     && FullSaturation(color)
//     && FullSaturation(bg_color);
/* cause image background disappeared */
//   if (all_extreme_colors)
//     canvas->SetPWMBits(1);

  ImageVector images = LoadImage(filename);
  // switch (images.size()) {
  // case 0:   // failed to load image.
  //   break;
  // case 1:   // Simple example: one image to show
  //   CopyImageToCanvas(images[0], canvas);
  //   while (!interrupt_received) sleep(1000);  // Until Ctrl-C is pressed
  //   break;
  // default:  // More than one image: this is an animation.
  //   ShowAnimatedImage(images, canvas);
  //   break;
  // }

  if (images.size() != 1)
  {
    printf("Failed to load image. Exiting...\n");
    return 1;
  }
   // Create a new canvas to be used with led_matrix_swap_on_vsync
  FrameCanvas *offscreen_canvas = canvas->CreateFrameCanvas();
  /*
   * Load font. This needs to be a filename with a bdf bitmap font.
   */
  rgb_matrix::Font font;
  if (!font.LoadFont(bdf_font_file)) {
    fprintf(stderr, "Couldn't load font '%s'\n", bdf_font_file);
    return 1;
  }
  int delay_speed_usec = 1000000;
  if (speed > 0) {
    delay_speed_usec = 1000000 / speed / font.CharacterWidth('W');
  } else if (x_orig == x_default_start) {
    // There would be no scrolling, so text would never appear. Move to front.
    x_orig = 0;
  }

  int x = x_orig;
  int y = y_orig;
  int length = 0;

  

  while (!interrupt_received && loops != 0) {
    offscreen_canvas->Fill(bg_color.r, bg_color.g, bg_color.b);

    // length = holds how many pixels our text takes up
    length = rgb_matrix::DrawText(offscreen_canvas, font,
                                  images[0].size().width(), 0 + font.baseline(),
                                  color, nullptr,
                                  first_line.c_str(), letter_spacing);

    /* clear all the pixels before the end point */
    // if (x < x_stop_point)
    // {
    //   for (int i_y = y_orig; i_y < y_orig + matrix_options.rows; ++i_y) {
    //     for (int i_x = x_stop_point; i_x >= x; --i_x) {
    //         offscreen_canvas->SetPixel(i_x, i_y, 0, 0, 0);
    //       }
    //   }
    // }


    length = rgb_matrix::DrawText(offscreen_canvas, font,
                                  x, y + 16 + font.baseline(),
                                  color, nullptr,
                                  second_line.c_str(), letter_spacing);

    if (speed > 0 && --x + length < 0) {
      x = x_orig;
      if (loops > 0) --loops;
    }
    // Swap the offscreen_canvas with canvas on vsync, avoids flickering
    CopyImageToCanvas(images[0], offscreen_canvas);
    
    offscreen_canvas = canvas->SwapOnVSync(offscreen_canvas);
    usleep(delay_speed_usec);
  }

  canvas->Clear();
  delete canvas;

  return 0;
}
