# Neosensory SDK for ESP32

An Arduino library that allows BLE communication to a Neosensory Buzz wristband via an ESP32 compatible board. This SDK is centered around the NeosensoryESP32 class. 

## Installation

Download the latest zip file from the [releases page](https://github.com/just4give/neosensory-sdk-esp32) and add it to Arduino following these [instructions for adding zip file libraries to Arduino](https://www.arduino.cc/en/guide/libraries#toc4).

## Dependencies

This library depends on Adafruit's Bluefruit library, included in the [Adafruit Board Support Package (BSP) for nRF52 Boards](https://github.com/adafruit/Adafruit_nRF52_Arduino#bsp-installation) (make sure to go through the install instructions thoroughly and update your bootloader) and on [adamvr's base64 library](https://github.com/adamvr/arduino-base64).

## Hardware

This library connects any Neosensory hardware (currently just Buzz) to a microcontroller with an ESP32 BLE chip. At this point, it has only been tested with ESP32 WROOM32 Dev board

## Documentation



## Examples



## License

Please note that while this Neosensory SDK has an Apache 2.0 license, 
usage of the Neosensory API to interface with Neosensory products is 
still  subject to the Neosensory developer terms of service located at:
https://neosensory.com/legal/dev-terms-service.

See [LICENSE](https://github.com/neosensory/neosensory-sdk-for-bluefruit/blob/master/LICENSE).

## Author

Originally created by Mithun Das.
