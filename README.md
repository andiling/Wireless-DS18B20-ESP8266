# WirelessDS18B20
This project use an ESP8266 and some DS18B20 (a 1-wire bus) to provide temperatures in JSON format (Jeedom compatible).

###1-Wire bus
In order to get a wide 1-Wire bus (one that cover a house), Dallas provides some recommendations into their application note AN148.

I choosed this schematic to drive my bus : 

![Dallas AN148 ImprovedCPUBusInterface schema](https://raw.github.com/J6B/Jeedom-ESP8266-Wireless-DS18B20/master/img/AN148-ImprovedCPUBusInterface.jpg)

This one requires a pin for reading the bus state and another one as output to drive the bus low : I named it the "Dual Pin OneWire"

##The WirelessDS18B20 project

The global idea of this project is that any system (like Jeedom) that's able to do an HTTP GET request and interpret JSON will get list of DS18B20 ROMcode available on buses or current temperature of a DS18B20 sensor.

###Schematic

TODO

###Code
Source code can be compiled for ESP-01 and so Pins usage is fixed like in schematic.
If you planned to use a different ESP-8266 model, you will be able to configure pins in configuration webpage

###How does it work

Usage (answers are in JSON format): 

 - `http://IP/getList?bus=0` will return list of DS18B20 ROMCodes available on the 1-Wire bus number 0
 - `http://IP/getTemp?bus=0&ROMCode=0A1B2C3D4E5F6071` will return simple JSON with temperature from the sensor

Configuration : 

 - `http://IP/getconfig` return you the current configuration (Wifi, 1-Wire Buses and version) : ![getconfig screenshot](https://raw.github.com/J6B/Jeedom-ESP8266-Wireless-DS18B20/master/img/getconfig.jpg)
 - `http://IP/config` allow you to change configuration (Wifi and 1-Wire buses (only non-ESP01)) : ![config screenshot](https://raw.github.com/J6B/Jeedom-ESP8266-Wireless-DS18B20/master/img/config.jpg)

TODO explain initial configuration and pins setup for non ESP-01 components