#ifndef WebDS18B20_h
#define WebDS18B20_h

#include "OneWireDualPin.h"

class WebDS18B20Bus: public OneWireDualPin {
  private:
    boolean readScratchPad(byte addr[], byte data[]);
    void writeScratchPad(byte addr[], byte th, byte tl, byte cfg);
    void copyScratchPad(byte addr[]);
    void startConvertT(byte addr[]);

    static boolean isROMCodeString(String s);

  public:
    WebDS18B20Bus(uint8_t pinIn, uint8_t pinOut);
    void setupTempSensors();
    void getTemp(WiFiClient c, byte addr[]);
    void getRomCodeList(WiFiClient c);

    static void GetList(WiFiClient c, String req, byte nbOfBuses, uint8_t owBusesPins[][2]);
    static void GetTemp(WiFiClient c, String req, byte nbOfBuses, uint8_t owBusesPins[][2]);
};

#endif
