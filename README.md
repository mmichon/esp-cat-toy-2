# ESP32/8266-based Motion Sensing App-controlled Pet Toy

This project implements a motion-sensing app-controlled single-axis pet toy on Arduino-based firmware and a Blynk-based frontend.

## Hardware Needed
* ESP8266 or ESP32 microcontroller, like a [Wemos D1 Mini](https://www.aliexpress.com/store/product/WEMOS-D1-mini-Pro-16M-bytes-external-antenna-connector-ESP8266-WIFI-Internet-of-Things-development-board/1331105_32724692514.html)
* Passive infrared sensor, like the ubiquitous [HC-SR501](https://smile.amazon.com/HC-SR501-Sensor-Module-Pyroelectric-Infrared/dp/B007XQRKD4?sa-no-redirect=1)
* [Blynk](https://www.blynk.cc/)
* Wifi connection
* [Platformio](https://platformio.org/) (optional for dependency management)

## Configuration

<img src="https://github.com/mmichon/esp-cat-toy-2/blob/master/circuit.png?raw=true" width="500">
<img src="https://github.com/mmichon/esp-cat-toy-2/blob/master/blynk.jpg?raw=true" height="500">

1. Wire up the circuit like [this](https://github.com/mmichon/esp-cat-toy-2/blob/master/circuit.png?raw=true) (or check out the pin config in the code and wire it up according to your components)
1. Configure your Blynk app like [this](https://github.com/mmichon/esp-cat-toy-2/blob/master/blynk.jpg?raw=true)
1. Modify `include/config.h` with your secrets
1. Compile and upload the firmware
1. Tune your servo settings in the constants at the top of `main.cpp`
