##  Adafruit RGB Matrix Bonnet for Raspberry Pi 

- [Pinout](https://learn.adafruit.com/adafruit-rgb-matrix-bonnet-for-raspberry-pi/pinouts)

Unused GPIO pins include: SCL, SDA, RX, TX, #25, MOSI, MISO, SCLK, CE0, CE1, #19.

GPIO25 can be used to reset Raspberry PI


## 64x32 Flexible RGB LED Matrix - 4mm Pitch

https://www.adafruit.com/product/3826

12 16-bit latches that allow you to drive the display with a 1:16 scan rate.
These panels require 13 digital pins (6 bit data, 7 bit control) and a good 5V supply, up to 4A per panel.


##  Adafruit RGB Matrix Bonnet for Raspberry Pi

Adafruit sells a board where they choose a different mapping. You can choose with the `--led-gpio-mapping` flag.

If you have a 64x32 display, you need to supply the flags `--led-cols=64 --led-rows=32` for instance.




https://learn.adafruit.com/adafruit-rgb-matrix-bonnet-for-raspberry-pi/driving-matrices

```bash
curl https://raw.githubusercontent.com/adafruit/Raspberry-Pi-Installer-Scripts/main/rgb-matrix.sh >rgb-matrix.sh
sudo bash rgb-matrix.sh
```

sudo ./demo -D0 --led-rows=32 --led-cols=64 # better
sudo ./demo -D0 --led-rows=32 --led-cols=64 --led-slowdown-gpio=0
sudo ./demo -D 1 runtext.ppm --led-rows=32 --led-cols=64


## Install

```bash

# examples-api-use/image-example.cc

# https://github.com/autorope/donkeydocs/issues/27#issuecomment-1001816493
sudo apt-get update --allow-releaseinfo-change
sudo apt-get install libgraphicsmagick++-dev libwebp-dev -y
```

### Compile

```bash
# scrolling-text-example.cc
make
./scrolling-text-example -s 1 -f ../fonts/7x14.bdf "Hello world"

# image-example.cc
make image-example
sudo ./image-example --led-rows=32 --led-cols=64 --led-gpio-mapping=adafruit-hat LinkedIn_32.jpg

# led-image-viewer.cc
make led-image-viewer
sudo ./led-image-viewer --led-no-hardware-pulse --led-rows=32 --led-cols=64 --led-gpio-mapping=adafruit-hat LinkedIn_32.jpg

# image text
make image-text
sudo ./image-text -f "tuyendl" -s "Open to work"

sudo ./image-text -f "tuyendl" -s "Open to work" -i LinkedIn_16.jpg

sudo ./main -f "tuyendl" -s "Open to work" -i LinkedIn_16.jpg
```

## Interesting
- https://github.com/hzeller/rpi-rgb-led-matrix/tree/master/examples-api-use
- https://github.com/hzeller/rpi-rgb-led-matrix/blob/master/include/threaded-canvas-manipulator.h
- https://github.com/search?q=repo%3Ahzeller%2Frpi-rgb-led-matrix+thread&type=issues&p=10


## Thread

Monitor thread and its children

```bash
export PID=1829
htop -p `pstree -p $PID | perl -ne 'push @t, /\((\d+)\)/g; END { print join ",", @t }'`
```

## Debug

```bash

valgrind --leak-check=yes ./main
valgrind --leak-check=full ./main
```

## Issues

https://rpi-rgb-led-matrix.discourse.group


---
[Multi thread safe](https://github.com/hzeller/rpi-rgb-led-matrix/issues/1494#issuecomment-1368440225)

I would not recommend using multiple threads to update the currently active canvas. Setting a pixel requires multiple writes internally and it can not be protected by a mutex, as it would interrupt the thread constantly refreshing the display.

Instead, make the granularity of updates for a full frame. This is commonly used as well in regular graphics to avoid tearing.

Create an off-screen canvas, fill it with the next frame and then `SwapOnVSync()` that in. That way, you will not have any glitches or tearing. Also, you don't need multiple threads to update the content, you can prepare the entire next frame of your animation in the main thread (draw the clock, draw the weather data, write the text to the next scroll position). Simplifies the code a lot.

You also will have exact timing as the `SwapOnVSync()` changes the off-screen canvas atomically when switching to the next frame.

This is true for C and Python (in particular Python, as that is way slower and thus emphasizes glitches)

---

[Raspberry Pi Zero W Raspbian Stretch System Stall with 16x32 Matrix Running rpi-rgb-led-matrix](https://github.com/hzeller/rpi-rgb-led-matrix/issues/1210)

I just finished switching from Raspbian Stretch Lite to DietPi. Fingers crossed!

---

How to make a thread that can change the data it displays. #1135

You do not want a start/stop matrix here. Your goal here is more related to how to deal with the thread.

If you want to have a text and replace it later, you should have exactly one thread, that you then pass a new text. Don't let loose of the thread, you need a reference to it to pass some data (ideally, you use a thread abstraction like the one provided in this project - that way, you have an object you can more easily abstract the passing data part).

Your goal is to run the thread and tell it when to show something different. You typically would do that by having a method on your thread SetNewText(std::string s) of sorts, which manipulates an instance variable. Inside the Run() method, you'd check with every loop if that text changed and do whatever is necessary to switch to the new display.

You must use a mutex for passing the thread boundary of course.

In the ThreadedCanvasManipulator you see an example how to pass some data safely with a mutex; in this case it is a running flag, but you'd do something similar
https://github.com/hzeller/rpi-rgb-led-matrix/blob/master/include/threaded-canvas-manipulator.h