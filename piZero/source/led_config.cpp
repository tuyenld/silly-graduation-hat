#include "led_config.h"

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
  while (!led_config::interrupt_received) {
    for (const auto &image : images) {
      if (led_config::interrupt_received) break;
      CopyImageToCanvas(image, offscreen_canvas);
      offscreen_canvas = canvas->SwapOnVSync(offscreen_canvas);
      usleep(image.animationDelay() * 10000);  // 1/100s converted to usec
    }
  }
}


bool usage(const char *progname) {
  fprintf(stderr, "Usage: %s [led-canvas-options] <image-filename>\n",
          progname);
  rgb_matrix::PrintMatrixFlags(stderr);
  return true;
}


/*
* Class led_config
*/

bool led_config::ParseOptionsFromFlags(int *argc, char ***argv)
{
  if (!rgb_matrix::ParseOptionsFromFlags(argc, argv,
                                         &matrix_options, &runtime_opt)) {
    return usage((*argv)[0]);
  }

  int opt;
  while ((opt = getopt(*argc, *argv, "i:f:s:")) != -1) {
    switch (opt) {
    case 'i': image_filename = strdup(optarg); break;
    /* first line */
    case 'f': first_line = strdup(optarg); break;
    /* second line */
    case 's': second_line = strdup(optarg); break;
    default:
      return usage((*argv)[0]);
    }
  }
  return true;
}


volatile bool led_config::interrupt_received = false;

led_config::led_config(int *argc, char ***argv)
{
    matrix_options.hardware_mapping = "adafruit-hat";  
    matrix_options.chain_length = 1;
    matrix_options.rows = 32;
    matrix_options.cols = 64;
    matrix_options.show_refresh_rate = true;

    ParseOptionsFromFlags(argc, argv);
    x_default_start = (matrix_options.chain_length
                                    * matrix_options.cols) + 5;
    canvas = RGBMatrix::CreateFromOptions(matrix_options, runtime_opt);
    if (canvas == NULL)
        printf("Initiation canvas failed\n");
    // Create a new canvas to be used with led_matrix_swap_on_vsync
    offscreen_canvas = canvas->CreateFrameCanvas();

}

bool led_config::load_font()
{
    /*
    * Load font. This needs to be a filename with a bdf bitmap font.
    */
    if (!font.LoadFont(bdf_font_file)) {
        fprintf(stderr, "Couldn't load font '%s'\n", bdf_font_file);
        return false;
    }
    else
    {
        return true;
    }
}

void led_config::load_image()
{
    if (image_filename == NULL)
    {
        printf("Running without image.\n");
    }
    else
    {
        images = LoadImage(image_filename);
        image_width = images[0].size().width();
    }
}

void led_config::cal_delay_and_coordinate(void)
{
    if (speed > 0) {
        delay_speed_usec = 1000000 / speed / font.CharacterWidth('W');
    } else if (x_orig == x_default_start) {
        // There would be no scrolling, so text would never appear. Move to front.
        x_orig = 0;
    }

    x = x_orig;
    y = y_orig;
    length = 0;
}

void led_config::loop_display()
{
    while (!interrupt_received && loops != 0) {
        offscreen_canvas->Fill(bg_color.r, bg_color.g, bg_color.b);

        // length = holds how many pixels our text takes up
        if (first_line != NULL)
        {
            length = rgb_matrix::DrawText(offscreen_canvas, font,
                                        image_width, 0 + font.baseline(),
                                        color, nullptr,
                                        first_line, letter_spacing);
        }

        /* clear all the pixels before the end point */
        // if (x < x_stop_point)
        // {
        //   for (int i_y = y_orig; i_y < y_orig + matrix_options.rows; ++i_y) {
        //     for (int i_x = x_stop_point; i_x >= x; --i_x) {
        //         offscreen_canvas->SetPixel(i_x, i_y, 0, 0, 0);
        //       }
        //   }
        // }

        if (second_line != NULL)
        {
            length = rgb_matrix::DrawText(offscreen_canvas, font,
                                        x, y + 16 + font.baseline(),
                                        color, nullptr,
                                        second_line, letter_spacing);
        }

        if (speed > 0 && --x + length < 0) {
            x = x_orig;
            if (loops > 0) --loops;
        }
        // Swap the offscreen_canvas with canvas on vsync, avoids flickering
        if (image_width > 0)
        {
            CopyImageToCanvas(images[0], offscreen_canvas);
        }

        offscreen_canvas = canvas->SwapOnVSync(offscreen_canvas);
        usleep(delay_speed_usec);
    }
}

led_config::~led_config()
{
    delete canvas;
}