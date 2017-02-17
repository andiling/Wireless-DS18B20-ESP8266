#ifndef WebConfig_h
#define WebConfig_h

class WebConfig: public Config {

  public:
    void Get(WiFiClient c);
    void Post(WiFiClient c);
};

#endif
