#ifndef WebConfig_h
#define WebConfig_h

#include "Config.h"

const char predefPassword[] PROGMEM = "ewcXoCt4HHjZUvY0";

class WebConfig: public Config {

  public:
    static bool FingerPrintS2A(byte* fingerPrintArray, const char* fingerPrintToDecode);
    static char* FingerPrintA2S(char* fpBuffer, byte* fingerPrintArray, char separator = 0);
    void Get(WiFiClient c);
    void Post(WiFiClient c);
};

#endif
