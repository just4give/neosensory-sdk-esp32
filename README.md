# Neosensory SDK for ESP32

An Arduino library that allows BLE communication to a Neosensory Buzz wristband via an ESP32 compatible board. This SDK is centered around the NeosensoryESP32 class. 

## Installation

Download the latest zip file from the [releases page](https://github.com/just4give/neosensory-sdk-esp32) and add it to Arduino following these [instructions for adding zip file libraries to Arduino](https://www.arduino.cc/en/guide/libraries#toc4).

## Dependencies



## Hardware

This library connects any Neosensory hardware (currently just Buzz) to a microcontroller with an ESP32 BLE chip. At this point, it has only been tested with ESP32 WROOM32 Dev board

## Documentation




## Examples

<img width="667" alt="Screen Shot 2020-11-23 at 12 09 34 PM" src="https://user-images.githubusercontent.com/9275193/99992768-c771f780-2d84-11eb-8063-803780bc4e19.png">


After you installed the library, go to Files -> Examples -> NeosensoryESP2 and select example files
- connect_and_vibrate.ino
This program will connect to your Buzz and vibrate randomly

- connect_and_control_wifi.ino
This program will connect to yuor Buzz and create a webserver. You can control the Buzz from your browser like below. You need to modify your WiFi SSID and Password in the program. This program also has dependency on other three libraries which you need to install seperately. 

  [ESPAsyncWebserver](https://github.com/me-no-dev/ESPAsyncWebServer)
  [AsyncTCP](https://github.com/me-no-dev/AsyncTCP)
  [ArduinoJson](https://github.com/bblanchon/ArduinoJson)

<img width="427" alt="Screen Shot 2020-11-23 at 12 02 17 PM" src="https://user-images.githubusercontent.com/9275193/99992404-4ca8dc80-2d84-11eb-9745-c9a9042800f0.png">

If your program cannot connect to Buzz, make sure you put your Buzz in pairing mode for the first time. 

## Paring 
Whether for the connect_and_vibrate.ino example or for your own project, you'll need to put Buzz into pairing mode the first time you connect to it. To do this, turn on your Buzz wristband and press and hold the plus and minus buttons on top of your Buzz. Buzz will show three blue LEDs and then a random pattern of LEDs (which is included in the advertising packet information in case you need to differentiate from several different Buzzes in pairing mode, but for most situations can be ignored).


## License

Please note that while this Neosensory SDK has an Apache 2.0 license, 
usage of the Neosensory API to interface with Neosensory products is 
still  subject to the Neosensory developer terms of service located at:
https://neosensory.com/legal/dev-terms-service.

See [LICENSE](https://github.com/neosensory/neosensory-sdk-esp32/blob/master/LICENSE).

## Want to contribute?
This library is open source and help developer get started with ESP32 and Neosensory Buzz. This library has some glitches and would need help from developer community. I would not able to maintain this repo actively due to time constraint and I would appreciate if anyone wants to contribute to this repo. Please feel free to send PR. 

## Author

Originally created by Mithun Das.
