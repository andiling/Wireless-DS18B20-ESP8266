#ifndef WebConfig_h
#define WebConfig_h

#include "Config.h"

const char predefPassword[] PROGMEM = "ewcXoCt4HHjZUvY0";

class WebConfig: public Config {

  public:
    void Get(WiFiClient c);
    void Post(WiFiClient c);
};

#endif
