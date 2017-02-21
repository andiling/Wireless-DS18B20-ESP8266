#ifndef WebConfig_h
#define WebConfig_h

#include "Config.h"

class WebConfig: public Config {

  public:
    void Get(WiFiClient c);
    void Post(WiFiClient c);
};

#endif
