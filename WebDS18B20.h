#ifndef WebDS18B20_h
#define WebDS18B20_h

#include "OneWireDualPin.h"

class WebDS18B20Bus: public OneWireDualPin {
  private:
    boolean readScratchPad(byte addr[], byte data[]);
    void writeScratchPad(byte addr[], byte th, byte tl, byte cfg);
    void copyScratchPad(byte addr[]);
    void startConvertT(byte addr[]);

  public:
    WebDS18B20Bus(uint8_t pinIn, uint8_t pinOut);
    void setupTempSensors();
    void getTemp(WiFiClient c, byte addr[]);
    void getRomCodeList(WiFiClient c);
    boolean getLastResult();
};

class WebDS18B20Buses {
  private:
    bool _initialized = false;
    byte _nbOfBuses;
    uint8_t (*_owBusesPins)[2];

    boolean isROMCodeString(char* s);

  public:
    void Init(byte nbOfBuses, uint8_t owBusesPins[][2]);
    void GetList(WiFiClient c, String &req);
    void GetTemp(WiFiClient c, String &req);
    void GetStatus(WiFiClient c);
};

#endif
