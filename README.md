# ArduinoEasy = Arduino Mega 2560 + Ethernet Shield

This is experimental project that may not be ready for production usage. This project is based on ESPEasy code backported to Arduino.

Please visit [discussion forum](https://www.letscontrolit.com/forum/viewtopic.php?f=18&t=2234) if you have questions and need help.
Binary image to try ArduinoEasy is available on [wiki page](https://www.letscontrolit.com/wiki/index.php/ArduinoEasy)

## Building
The latest sources can be build using Arduino 1.8.5 with recent libraries. Arduino will ask to rename src folder to ArduinoEasy once you open ArduinoEasy.ino file.
Make sure to add base64 library. Open Sketch -> Include library -> Add .ZIP library -> select folder /lib/Base64
Other libraries that should be also available
- Wire at version 1.0
- SPI at version 1.0
- SD at version 1.2.2
- Ethernet at version 1.1.2
- PubSubClient at version 2.6
- ArduinoJson at version 5.13.1

## Usage
MicroSD card is used to store configuration. Make sure to format microSD or microSDHC card to FAT and insert it into Arduino. Otherwise you will get following error in console.

    PID:0
    Version:0
    INIT : Incorrect PID or version!

## Mega Switch setup
Arduino Mega has lots of GPIOs and it is possible to control most of them using ArduinoEasy.  
List of GPIOs that can be controlled: 3, 5 - 9, 14 - 49, 56 - 69.  
56-69 pins are used for A2-A15 pins. A0 and A1 are required to work with SD card.

### MQTT control
To control GPIO via MQTT send "1" or "0" to corresponding pin number. Here is example of topic to control pin 39 on device named ArduinoEasyRocks.

    /ArduinoEasyRocks/gpio/39

### OpenHab configuration
Items configuration with switch defined

    Switch StairsUpHall  <light>  {mqtt=">[oh2mqtt:/ArduinoEasyRocks/gpio/39:command:ON:1],>[oh2mqtt:/ArduinoEasyRocks/gpio/39:command:OFF:0]"}

Sitemap with corresponding switch to show it on basic UI

    Switch item=StairsUpHall label="Upstairs hall"

Make sure to properly configure MQTT binding.
