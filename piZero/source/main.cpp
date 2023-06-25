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

#include "led_config.h"
#include "mqtt_control.h"
#include <signal.h>

int main(int argc, char *argv[]) {
  Magick::InitializeMagick(*argv);

  class led_config *pi_led_config;
  pi_led_config = new led_config(&argc, &argv);
  pi_led_config->load_font();
  pi_led_config->load_image();
  pi_led_config->cal_delay_and_coordinate();

  signal(SIGTERM, led_config::InterruptHandler);
  signal(SIGINT, led_config::InterruptHandler);
  printf("CTRL-C for exit.\n");

  /*
  * Control via MQTT
  **/
  class mqtt_control *led_control;
	mosqpp::lib_init();
	led_control = new mqtt_control("led_control", "piZero", "pihat", "104.248.243.162", 1883, "hat:ctrl");

	led_control->loop_start();
  pi_led_config->loop_display();

  printf("Exiting...\n");
  pi_led_config->canvas->Clear();
  delete pi_led_config;

  mosqpp::lib_cleanup();

  return 0;
}
