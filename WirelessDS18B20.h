#ifndef WirelessDS18B20_h
#define WirelessDS18B20_h


//J6B Informations
//Configuration Web Pages
//http://IP/getconfig
//http://IP/config
//DS18B20 Request Web Pages
//http://IP/getList?bus=0
//http://IP/getTemp?bus=0&ROMCode=0A1B2C3D4E5F6071


//Choose ESP-01 version or not
//For ESP-01, Pins used are restricted
//Pin 3 (RX) = 1Wire bus input
//Pin 0 = 1Wire bus output
//Pin 2 = config Mode button
//For other models, Pin Numbers and Buses are defined through Configuration Web Page
#define ESP01_PLATFORM 1

//Enable OTA or Not
#define OTA 1

//Choose Serial Speed
#define SERIAL_SPEED 115200

//Choose Pin used to boot in Rescue Mode
#define RESCUE_BTN_PIN 2

#define VERSION_NUMBER "2.0"

#if ESP01_PLATFORM
#define VERSION VERSION_NUMBER " (ESP-01)"
#else
#define VERSION VERSION_NUMBER
#endif

extern char buf[1024];

#endif


