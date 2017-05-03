#ifndef Config_h
#define Config_h

#define DEFAULT_AP_SSID "WirelessDS18B20"
#define DEFAULT_AP_PSK "PasswordDS18B20"
#define DEFAULT_OTAPASS "DSPassword01"

#define MAX_NUMBER_OF_BUSES 4

class Config {
  public:
    char ssid[32 + 1] = {0};
    char password[64 + 1] = {0};
    char hostname[24 + 1] = {0};
    char otaPassword[24 + 1] = {0};

    byte numberOfBuses = 0;
    uint8_t owBusesPins[MAX_NUMBER_OF_BUSES][2];

    void SetDefaultValues() {
      ssid[0] = 0;
      password[0] = 0;
      hostname[0] = 0;
      strcpy(otaPassword, DEFAULT_OTAPASS);
      numberOfBuses = 0;
      memset(owBusesPins, 0, MAX_NUMBER_OF_BUSES * 2);
    }
    bool Save();
    bool Load();
  private :
    uint16_t crc; ///!\ crc should always stay in last position
};


#endif

