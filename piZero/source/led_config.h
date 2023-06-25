#ifndef LED_CONFIG_H
#define LED_CONFIG_H

#include "led-matrix.h"

#include <math.h>
#include <stdio.h>
#include <unistd.h>

#include <exception>
#include <Magick++.h>
#include <magick/image.h>

#include "graphics.h"
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

// volatile bool interrupt_received;
// static void InterruptHandler(int signo);

using rgb_matrix::Canvas;
using rgb_matrix::RGBMatrix;
using rgb_matrix::FrameCanvas;

using ImageVector = std::vector<Magick::Image>;



#if 0
// Given the filename, load the image 
// // If this is an animated image, the resutlting vector will contain multiple.
static ImageVector LoadImage(const char *filename);

#endif

// Copy an image to a Canvas. Note, the RGBMatrix is implementing the Canvas
// interface as well as the FrameCanvas we use in the double-buffering of the
// animted image.
void CopyImageToCanvas(const Magick::Image &image, Canvas *canvas);


// An animated image has to constantly swap to the next frame.
// We're using double-buffering and fill an offscreen buffer first, then show.
void ShowAnimatedImage(const ImageVector &images, RGBMatrix *canvas);

bool usage(const char *progname);


class led_config
{
    public:
        // Initialize the RGB canvas with
        RGBMatrix::Options matrix_options;
        rgb_matrix::RuntimeOptions runtime_opt;

        const char *bdf_font_file = "7x14.bdf";
        int x_stop_point = 32; // end point for display
        const char *image_filename = NULL;
        const char* first_line = NULL;
        const char* second_line = NULL;
        float speed = 1.0f;
        // or e.g. "adafruit-hat" or "adafruit-hat-pwm"
        rgb_matrix::Color color {rgb_matrix::Color(255, 255, 0)};
        rgb_matrix::Color bg_color {rgb_matrix::Color(0, 0, 0)};
        /* x_origin is set by default just right of the screen */
        int x_default_start;
        int x_orig = 0;
        int y_orig = 0;
        int x = 0;
        int y = 0;
        int length = 0;

        int letter_spacing = 0;
        int loops = -1;
        ImageVector images;
        int image_width = 0;
        rgb_matrix::Font font;
        int delay_speed_usec = 1000000;

        static volatile bool interrupt_received;

        RGBMatrix *canvas;
        // Create a new canvas to be used with led_matrix_swap_on_vsync
        FrameCanvas *offscreen_canvas;

        // Make sure we can exit gracefully when Ctrl-C is pressed.
        // volatile bool interrupt_received = false;
        static void InterruptHandler(int signo) {
            interrupt_received = true;
        }

        led_config(int *argc, char ***argv);
        ~led_config();
        bool ParseOptionsFromFlags(int *argc, char ***argv);
        bool load_font(void);
        void load_image(void);
        void cal_delay_and_coordinate(void);

        void loop_display(void);
};

#endif