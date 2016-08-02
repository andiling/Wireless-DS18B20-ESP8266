# WirelessDS18B20
This project use an ESP8266 and some DS18B20 sensors on a 1-Wire bus to provide temperatures in JSON format (Jeedom compatible).

The global idea of this project is that any system (like Jeedom) that's able to do an HTTP GET request and interpret JSON will get list of DS18B20 ROMcode available on buses or current temperature of a DS18B20 sensor.

##Build your 1-Wire bus
In order to get a wide 1-Wire bus (one that cover a house), Dallas provides some recommendations into their application note AN148.

This project use the Improved Interface bellow that is able to support a 200m bus: 

![Dallas AN148 ImprovedCPUBusInterface schema](https://raw.github.com/J6B/Jeedom-ESP8266-Wireless-DS18B20/master/img/AN148-ImprovedCPUBusInterface.jpg)

This one requires a pin for reading the bus state and another one as output to drive the bus low : I named it the "Dual Pin OneWire"

**Recommendation : If you build a large 1 Wire bus inside your house, keep in mind that some high voltage may appear on this one by induction. You need to have a good knowledge about electricity and associated risks!!!**

##Then build your WirelessDS18B20



###Schematic

![WirelessDS18B20 schematic](https://raw.github.com/J6B/Jeedom-ESP8266-Wireless-DS18B20/master/img/schematic.png)

###Code
Source code can be compiled for :

 - ESP-01 : Pin usage is fixed like in schematic.
 - other ESP8266 models : 1-Wire buses pins can be configured through the configuration webpage

###First Boot

###Configuration

Configuration : 

 - `http://IP/getconfig` return you the current configuration (Wifi, 1-Wire Buses and version) : ![getconfig screenshot](https://raw.github.com/J6B/Jeedom-ESP8266-Wireless-DS18B20/master/img/getconfig.jpg)
 - `http://IP/config` allow you to change configuration (Wifi and 1-Wire buses (only non-ESP01)) : ![config screenshot](https://raw.github.com/J6B/Jeedom-ESP8266-Wireless-DS18B20/master/img/config.jpg)

TODO explain initial configuration and pins setup for non ESP-01 components


###How does it work finally

Usage (answers are in JSON format): 

 - `http://IP/getList?bus=0` will return list of DS18B20 ROMCodes available on the 1-Wire bus number 0
 - `http://IP/getTemp?bus=0&ROMCode=0A1B2C3D4E5F6071` will return simple JSON with temperature from the sensor


##Jeedom Configuration

For this configuration you need the *Script* plugin installed from the market : 
![Script Icon](https://raw.github.com/J6B/Jeedom-ESP8266-Wireless-DS18B20/master/img/JeedomScriptIcon.png)
Go to script plugin then add a new equipment: 
![Script Add](https://raw.github.com/J6B/Jeedom-ESP8266-Wireless-DS18B20/master/img/JeedomScriptAdd.png)
Name it : 
![Script Name](https://raw.github.com/J6B/Jeedom-ESP8266-Wireless-DS18B20/master/img/JeedomScriptName.png)
Set refresh time (every minutes in my case) : 
![Script Refresh](https://raw.github.com/J6B/Jeedom-ESP8266-Wireless-DS18B20/master/img/JeedomScriptRefresh.png)
Then into command tab, add a script command : 
![Script Command](https://raw.github.com/J6B/Jeedom-ESP8266-Wireless-DS18B20/master/img/JeedomScriptCmd.png)
Then set it up with : 

 - command name : Temperature
 - script type : JSON
 - request : Temperature (JSON tag name)
 - URL : http://***IP***/getTemp?bus=***0***&ROMCode=***0A1B2C3D4E5F6071*** 
 - Unit : °C

![Script Command Config](https://raw.github.com/J6B/Jeedom-ESP8266-Wireless-DS18B20/master/img/JeedomScriptCmdConfig.png)

Now you're done and should get something like this : 
![Script Command Result](https://raw.github.com/J6B/Jeedom-ESP8266-Wireless-DS18B20/master/img/JeedomScriptResult.png)