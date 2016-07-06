# WirelessDS18B20
This project use an ESP8266 and some DS18B20 (a 1-wire bus) to provide temperatures to Jeedom.

In order to get a wide 1-Wire bus (one that cover a house), Dallas provides some recommendations into their application note AN148.

I choosed this schematic to drive my bus : 
![Dallas AN148 ImprovedCPUBusInterface schema](https://raw.github.com/J6B/Jeedom-ESP8266-Wireless-DS18B20/master/img/AN148-ImprovedCPUBusInterface.jpg)

It requires a pin for reading the bus and another one as output to drive the bus low : I named it the "Dual Pin OneWire"
