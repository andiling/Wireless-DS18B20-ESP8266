#ifndef Config_h
#define Config_h

#define MAX_NUMBER_OF_BUSES 4

class Config {
  public:
    bool save();
    bool load();

    bool APMode = 1;
    char ssid[32 + 1] = "WirelessDS18B20"; 		 // SSID
    char password[64 + 1] = "PasswordDS18B20"; 		 // Pre shared key
    char hostname[24 + 1] = ""; // Hostname
    char otaPassword[24 + 1] = "DSPassword01";
    byte numberOfBuses = 0;
    uint8_t owBusesPins[MAX_NUMBER_OF_BUSES][2];

  private :
    uint16_t crc; ///!\ crc should always stay in last position
};


#endif

