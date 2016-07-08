# WirelessDS18B20
This project use an ESP8266 and some DS18B20 (a 1-wire bus) to provide temperatures in JSON format (Jeedom compatible).

###1-Wire bus
In order to get a wide 1-Wire bus (one that cover a house), Dallas provides some recommendations into their application note AN148.

I choosed this schematic to drive my bus : 

![Dallas AN148 ImprovedCPUBusInterface schema](https://raw.github.com/J6B/Jeedom-ESP8266-Wireless-DS18B20/master/img/AN148-ImprovedCPUBusInterface.jpg)

It requires a pin for reading the bus and another one as output to drive the bus low : I named it the "Dual Pin OneWire"

The global idea of this project is that any system (like Jeedom) that's able to do an HTTP GET request and interpret JSON will get list of DS18B20 ROMcode available on the bus or current temperature of a DS18B20 sensor.

The main schematic : 

TODO

Source code can be compiled for ESP-01 and so Pins usage is fixed like in schematic.
If you planned to use a different ESP-8266 model, you will be able to configure pins in configuration webpage

Usage : 

 - `http://IP/getList?bus=0` will return list of DS18B20 ROMCodes available on the first 1-Wire bus
 - `http://IP/getTemp?bus=0&ROMCode=0A1B2C3D4E5F6071` will return simple JSON with temperature from the sensor

Configuration : 

TODO explain initial configuration and pins setup for non ESP-01 components
