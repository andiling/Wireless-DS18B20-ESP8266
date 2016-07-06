#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>

#include "OneWireDualPin.h"

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

//Choose Serial Speed
#define SERIAL_SPEED 9600

//Choose Pin used to boot in Config Mode
#define CONFIG_BTN_PIN 2




#define VERSION_NUMBER F("1.2")

#define MAX_NUMBER_OF_BUSES 4
#define MAX_TEMP_SENSOR_PER_BUS 15
//Config -----------------
bool APMode = true; //1+1B
String ssid = "WirelessDS18B20"; //32+1B
String password = "Password01"; //64+1B
byte numberOfBuses = 0; //1+1B
uint8_t* owBusesPins; //2+1B * MAX_NUMBER_OF_BUSES

//EEPROM required size is sum of Config Settings + 1 additionnal byte
#define EEPROM_SIZE 115

//WiFiServer
WiFiServer server(80);


//-----------------------------------------------------------------------
// Config functions (config is stored in EEPROM)
//-----------------------------------------------------------------------
bool saveConfig() {
  int posEEPROM = 0;

  //Start EEPROM operation
  EEPROM.begin(EEPROM_SIZE);

  //write APMode
  EEPROM.write(posEEPROM, APMode ? '1' : '0');
  posEEPROM++;
  EEPROM.write(posEEPROM, 0);
  posEEPROM++;

  //write ssid
  for (int i = 0; i < ssid.length(); i++) {
    EEPROM.write(posEEPROM, ssid[i]);
    posEEPROM++;
  }
  EEPROM.write(posEEPROM, 0);
  posEEPROM++;

  //write password
  for (int i = 0; i < password.length(); i++) {
    EEPROM.write(posEEPROM, password[i]);
    posEEPROM++;
  }
  EEPROM.write(posEEPROM, 0);
  posEEPROM++;

  //write numberOfBuses
  EEPROM.write(posEEPROM, numberOfBuses);
  posEEPROM++;
  EEPROM.write(posEEPROM, 0);
  posEEPROM++;

  //write owBusesPins couples
  for (int i = 0; i < numberOfBuses; i++) {
    EEPROM.write(posEEPROM, owBusesPins[2 * i]);
    posEEPROM++;
    EEPROM.write(posEEPROM, owBusesPins[(2 * i) + 1]);
    posEEPROM++;
    EEPROM.write(posEEPROM, 0);
    posEEPROM++;
  }

  //write FINAL end char
  EEPROM.write(posEEPROM, 0);
  posEEPROM++;

  EEPROM.end();

  return true;
}
//------------------------------------------
bool loadConfig() {

  int posEEPROM = 0;

  //Start EEPROM operation
  EEPROM.begin(EEPROM_SIZE);

  //read APMode
  if (EEPROM.read(posEEPROM) == 0x31) APMode = true;
  else APMode = false;
  posEEPROM++;
  //check end char
  if (EEPROM.read(posEEPROM) != 0) {
    EEPROM.end();
    return false;
  }
  posEEPROM++;

  //read ssid
  ssid = "";
  while (EEPROM.read(posEEPROM) != 0 && posEEPROM < EEPROM_SIZE) {
    ssid += (char)EEPROM.read(posEEPROM);
    posEEPROM++;
  }
  //check that we didn't reach end of EEPROM
  if (posEEPROM == EEPROM_SIZE) {
    EEPROM.end();
    return false;
  }
  posEEPROM++;

  //read password
  password = "";
  while (EEPROM.read(posEEPROM) != 0 && posEEPROM < EEPROM_SIZE) {
    password += (char)EEPROM.read(posEEPROM);
    posEEPROM++;
  }
  //check that we didn't reach end of EEPROM
  if (posEEPROM == EEPROM_SIZE) {
    EEPROM.end();
    return false;
  }
  posEEPROM++;

  //read numberOfBuses
  numberOfBuses = EEPROM.read(posEEPROM);
  posEEPROM++;
  //check end char and numberOfBuses value
  if (EEPROM.read(posEEPROM) != 0 || numberOfBuses < 1 || numberOfBuses > MAX_NUMBER_OF_BUSES) {
    EEPROM.end();
    return false;
  }
  posEEPROM++;

  //read owBusesPins
  if (owBusesPins) free(owBusesPins);
  owBusesPins = new uint8_t[numberOfBuses * 2];
  for (int i = 0; i < numberOfBuses; i++) {
    owBusesPins[2 * i] = EEPROM.read(posEEPROM);
    posEEPROM++;
    owBusesPins[(2 * i) + 1] = EEPROM.read(posEEPROM);
    posEEPROM++;
    if (EEPROM.read(posEEPROM) != 0) {
      EEPROM.end();
      return false;
    }
    posEEPROM++;
  }

  //check FINAL end char
  if (EEPROM.read(posEEPROM) != 0) {
    EEPROM.end();
    return false;
  }

#if ESP01_PLATFORM
  numberOfBuses = 1;
  free(owBusesPins);
  owBusesPins = new uint8_t[2];
  owBusesPins[0] = 3;
  owBusesPins[1] = 0;
#endif

  EEPROM.end();
  return true;
}
//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
// DS18X20 Read ScratchPad command
boolean readScratchPad(OneWireDualPin ds, byte addr[], byte data[]) {

  boolean crcScratchPadOK;

  crcScratchPadOK = false;

  //read scratchpad (if 3 failures occurs, then return the error
  for (byte i = 0; i < 3; i++) {
    // read scratchpad of the current device
    ds.reset();
    ds.select(addr);
    ds.write(0xBE); // Read ScratchPad
    for (byte j = 0; j < 9; j++) { // read 9 bytes
      data[j] = ds.read();
    }
    if (OneWireDualPin::crc8(data, 8) == data[8]) {
      crcScratchPadOK = true;
      i = 3; //end for loop
    }
  }

  return crcScratchPadOK;
}
//------------------------------------------
// DS18X20 Write ScratchPad command
void writeScratchPad(OneWireDualPin ds, byte addr[], byte th, byte tl, byte cfg) {

  ds.reset();
  ds.select(addr);
  ds.write(0x4E); // Write ScratchPad
  ds.write(th); //Th 80°C
  ds.write(tl); //Tl 0°C
  ds.write(cfg); //Config
}
//------------------------------------------
// DS18X20 Copy ScratchPad command
void copyScratchPad(OneWireDualPin ds, byte addr[]) {

  ds.reset();
  ds.select(addr);
  ds.write(0x48); //Copy ScratchPad
}
//------------------------------------------
// DS18X20 Start Temperature conversion
void startConvertT(OneWireDualPin ds, byte addr[]) {
  ds.reset();
  ds.select(addr);
  ds.write(0x44); // start conversion
}
//------------------------------------------
// Function to initialize DS18X20 sensor
void setupTempSensors(OneWireDualPin ds) {

  byte i, j;
  byte addr[8];
  byte data[9];
  boolean scratchPadReaded;

  //while we find some devices
  while (ds.search(addr)) {

    //if ROM received is incorrect or not a DS1822 or DS18B20 THEN continue to next device
    if ((OneWireDualPin::crc8(addr, 7) != addr[7]) || (addr[0] != 0x22 && addr[0] != 0x28)) continue;

    scratchPadReaded = readScratchPad(ds, addr, data);
    //if scratchPad read failed then continue to next 1-Wire device
    if (!scratchPadReaded) continue;

    //if config is not correct
    if (data[2] != 0x50 || data[3] != 0x00 || data[4] != 0x5F) {

      //write ScratchPad with Th=80°C, Tl=0°C, Config 11bit resolution
      writeScratchPad(ds, addr, 0x50, 0x00, 0x5F);

      scratchPadReaded = readScratchPad(ds, addr, data);
      //if scratchPad read failed then continue to next 1-Wire device
      if (!scratchPadReaded) continue;

      //so we finally can copy scratchpad to memory
      copyScratchPad(ds, addr);
    }
  }
}
//------------------------------------------
// function that get temperature from a DS18X20 (run convertion, get scratchpad then calculate temperature)
float readTemp(OneWireDualPin ds, byte addr[]) {

  byte i, j;
  byte data[12];
  boolean scratchPadReaded;

  startConvertT(ds, addr);

  //wait for conversion end (DS18B20 are powered)
  while (ds.read_bit() == 0) {
    delay(10);
  }

  scratchPadReaded = readScratchPad(ds, addr, data);
  //if read of scratchpad failed 3 times then return special fake value
  if (!scratchPadReaded) return 12.3456;


  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (addr[0] == 0x10) { //type S temp Sensor
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  return (float)raw / 16.0;
}

//------------------------------------------
// convert an ascii hexa char to the byte value ('0'->0x0 ... 'F'->0x0F )
byte charToHexByte(char c) {
  return (c < 0x3A) ? (c - 0x30) : (c - 0x57);
}
//------------------------------------------
// Get an array of DS18X20 sensor ROMCode (stored in array param) and return number of ROMCode found
byte getTempRomCodeList(OneWireDualPin ds, byte array[][8]) {

  byte nbFound = 0;

  ds.reset_search();

  while (ds.search(array[nbFound]) && nbFound < MAX_TEMP_SENSOR_PER_BUS) {

    //if ROM received is incorrect or not a Temperature sensor THEN continue to next device
    if ((OneWireDualPin::crc8(array[nbFound], 7) != array[nbFound][7]) || (array[nbFound][0] != 0x10 && array[nbFound][0] != 0x22 && array[nbFound][0] != 0x28)) continue;

    nbFound++;
  }
  return nbFound;
}

//-----------------------------------------------------------------------
// Configuration Webpages functions
//-----------------------------------------------------------------------
byte asciiToHex(char c) {
  return (c < 0x3A) ? (c - 0x30) : (c > 0x60 ? c - 0x57 : c - 0x37);
}
//------------------------------------------
// Function that lookup in Datas to find a parameter and then return the corresponding decoded value
String findParameterInURLEncodedDatas(String datas, String parameterToFind) {

  String res = "";

  //can we find the param in the url
  int posParam = datas.indexOf(parameterToFind + "=");

  //if not then return empty string
  if (posParam == -1) return res;

  //if previous char is not a separator then lookup for the next match
  if (posParam != 0 && datas[posParam - 1] != '&') return findParameterInURLEncodedDatas(datas.substring(posParam + parameterToFind.length()), parameterToFind);

  //now we can extract the value and decode it at the same time
  //adujst position to the start of the value
  posParam += parameterToFind.length() + 1;
  while (posParam < datas.length() && datas[posParam] != '&') {
    if (datas[posParam] == '+') res += ' ';
    else if (datas[posParam] == '%') {
      res += (char)(asciiToHex(datas[posParam + 1]) * 0x10 + asciiToHex(datas[posParam + 2]));
      posParam++;
      posParam++;
    }
    else res += datas[posParam];
    posParam++;
  }
  return res;
}
//------------------------------------------
void handleGetConfig(WiFiClient c) {

  //DEBUG
  Serial.println(F("handleGetConfig"));

  c.print(F("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"));

  c.print(F("<html>\
      <head><title>J6B Wireless DS18B20</title></head>\
      <body style='background-color: #ffdb99; Color: #000000;font-family: Arial;'>\
      <h1>J6B Wireless DS18B20</h1>\
        <h2>Current Configuration</h2>"));

  c.print(F("APMode : ")); c.print(APMode ? F("on") : F("off"));
  c.print(F("<br>ssid : ")); c.print(ssid);
  c.print(F("<br>numberOfBuses : ")); c.print(numberOfBuses);
  for (int i = 0; i < numberOfBuses; i++) {
    c.print(F("<br>bus")); c.print(i); c.print(F(" : PinIn = ")); c.print(owBusesPins[2 * i]); c.print(F(" - PinOut = ")); c.print(owBusesPins[(2 * i) + 1]);
  }
  c.print(F("<br><br>build version : ")); c.print(VERSION_NUMBER); if (ESP01_PLATFORM) c.print(F(" (ESP01)"));
  c.print(F("</body></html>"));

}
//------------------------------------------
void handleConfig(WiFiClient c) {

  //DEBUG
  Serial.println(F("handleConfig"));

  //prepare submit url ip address
  String ipAddress;
  if (APMode) ipAddress = WiFi.softAPIP().toString();
  else ipAddress = WiFi.localIP().toString();

  c.print(F("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"));

  c.print(F("<html>\
    <head><title>J6B Wireless DS18B20</title></head>\
    <body style='background-color: #ffdb99; Color: #000000;font-family: Arial;'>\
    <h1>J6B Wireless DS18B20</h1>\
    <h2>Configuration WebPage</h2>\
    <form action='http://"));  c.print(ipAddress);  c.print(F("/submit' method='POST'>\
      APMode: <input type='checkbox' name='APMode'><br>\
      ssid: <input type='text' name='ssid' maxlength='32'><br>\
      password: <input type='password' name='password' maxlength='64'><br>"));
#if !ESP01_PLATFORM
  c.print(F("number Of OW Bus: <input type='number' min='1' max='")); c.print(MAX_NUMBER_OF_BUSES); c.print(F("' name='numberOfBuses'><br>"));
  for (int i = 0; i < MAX_NUMBER_OF_BUSES; i++) {
    c.print(F("bus")); c.print(i); c.print(F(": PinIn <input type='number' name='bus")); c.print(i); c.print(F("PinIn'> PinOut <input type='number' name='bus")); c.print(i); c.print(F("PinOut'><br>"));
  }
#endif
  c.print(F("<input type='submit' value='Submit Config'>\
    </form>\
    </body></html>"));
}

//------------------------------------------
void handleSubmit(WiFiClient c) {

  String postedDatas = "";

  //temp variable for args
  bool tempAPMode = false;
  String tempSsid;
  String tempPassword = "";
  byte tempNumberOfBuses = 0;
  uint8_t* tempOwBusesPins;

  //DEBUG
  Serial.println(F("handleSubmit"));

  //Find line with POSTed datas
  while (c.available() && !postedDatas.startsWith(F("\nssid=")) && !postedDatas.startsWith(F("\nAPMode="))) {
    postedDatas = c.readStringUntil('\r');
  }

  //If we didn't received it then return
  if (!postedDatas.startsWith(F("\nssid=")) && !postedDatas.startsWith(F("\nAPMode="))) return;
  else postedDatas = postedDatas.substring(1); //else remove the first \n

  //Parse Parameters
  if (findParameterInURLEncodedDatas(postedDatas, F("APMode")) == "on") tempAPMode = true;
  tempSsid = findParameterInURLEncodedDatas(postedDatas, F("ssid"));
  if (tempSsid.length() == 0) {
    c.print(F("HTTP/1.1 400 Bad Request1\r\n\r\n"));
    return;
  }
  tempPassword = findParameterInURLEncodedDatas(postedDatas, F("password"));
#if !ESP01_PLATFORM
  if (findParameterInURLEncodedDatas(postedDatas, F("numberOfBuses")).length() == 0) {
    c.print(F("HTTP/1.1 400 Bad Request2\r\n\r\n"));
    return;
  }
  tempNumberOfBuses = findParameterInURLEncodedDatas(postedDatas, F("numberOfBuses")).toInt();
  if (tempNumberOfBuses < 1 || tempNumberOfBuses > MAX_NUMBER_OF_BUSES) {
    c.print(F("HTTP/1.1 400 Bad Request3\r\n\r\n"));
    return;
  }
  tempOwBusesPins = new uint8_t[tempNumberOfBuses * 2];
  for (int i = 0; i < tempNumberOfBuses; i++) {
    if (findParameterInURLEncodedDatas(postedDatas, String("bus") + i + "PinIn").length() == 0) {
      c.print(F("HTTP/1.1 400 Bad Request4\r\n\r\n"));
      free(tempOwBusesPins);
      return;
    }
    tempOwBusesPins[2 * i] = findParameterInURLEncodedDatas(postedDatas, String("bus") + i + "PinIn").toInt();
    if (findParameterInURLEncodedDatas(postedDatas, String("bus") + i + "PinOut").length() == 0) {
      c.print(F("HTTP/1.1 400 Bad Request5\r\n\r\n"));
      free(tempOwBusesPins);
      return;
    }
    tempOwBusesPins[(2 * i) + 1] = findParameterInURLEncodedDatas(postedDatas, String("bus") + i + "PinOut").toInt();
  }
#endif

  //config checked so copy
  APMode = tempAPMode;
  ssid = tempSsid;
  password = tempPassword;

#if !ESP01_PLATFORM
  numberOfBuses = tempNumberOfBuses;
  free(owBusesPins);
  owBusesPins = tempOwBusesPins;
#endif

  //then save
  bool result = saveConfig();

  //Send client answer
  c.print(F("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"));

  c.print(F("<html>\
      <head><title>J6B Wireless TeleInfo</title></head>\
      <body style='background-color: #ffdb99; Color: #000000;font-family: Arial;'>\
      <h1>J6B Wireless DS18B20</h1>\
      <h2>Configuration submission</h2>"));
  c.print(F("APMode : ")); c.print(APMode ? F("on") : F("off"));
  c.print(F("<br>ssid : ")); c.print(ssid);
#if !ESP01_PLATFORM
  c.print(F("<br>numberOfBuses : ")); c.print(numberOfBuses);
  for (int i = 0; i < numberOfBuses; i++) {
    c.print(F("<br>bus")); c.print(i); c.print(F(" : PinIn = ")); c.print(owBusesPins[2 * i]); c.print(F(" - PinOut = ")); c.print(owBusesPins[(2 * i) + 1]);
  }
#endif
  c.print(F("<br><br>Save config result : ")); c.print(result ? F("OK") : F("FAILED!!!"));
  c.print(F("</body></html>"));

  //restart ESP to apply new config
  ESP.reset();
}
//------------------------------------------
void handleGetList(WiFiClient c, String req) {

  //DEBUG
  Serial.println(F("getList"));

  //check for ? in the url request
  if (req.indexOf('?') == -1 || req.indexOf('?') == (req.length() - 1)) {
    c.print(F("HTTP/1.1 400 Bad Request1\r\n\r\n"));
    return;
  }
  //keep only the part after '?' and before the final HTTP/1.1
  String getDatas = req.substring(req.indexOf('?') + 1);
  getDatas = getDatas.substring(0, getDatas.indexOf(' '));

  //try to find busNumber
  String strBusNumber = findParameterInURLEncodedDatas(getDatas, F("bus"));
  //check string found
  if (strBusNumber.length() != 1 || strBusNumber[0] < 0x30 || strBusNumber[0] > 0x39) {
    c.print(F("HTTP/1.1 400 Bad Request2\r\n\r\n"));
    return;
  }

  //convert busNumber
  int busNumber = strBusNumber[0] - 0x30;

  //check busNumber
  if (busNumber >= numberOfBuses) {
    c.print(F("HTTP/1.1 400 Bad Request3\r\n\r\n"));
    return;
  }

  //initializing vars for 1Wire module search
  byte romCodeList[MAX_TEMP_SENSOR_PER_BUS][8];
  byte nbOfRomCode;

#if ESP01_PLATFORM
  Serial.flush();
  delay(5);
  Serial.end();
#endif

  //Search for OneWire Temperature sensor
  nbOfRomCode = getTempRomCodeList(OneWireDualPin(owBusesPins[2 * busNumber], owBusesPins[(2 * busNumber) + 1]), romCodeList);

#if ESP01_PLATFORM
  Serial.begin(SERIAL_SPEED);
#endif

  //Send client answer
  c.print(F("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n"));

  //send JSON structure
  c.print(F("{\r\n\t\"TemperatureSensorList\": [\r\n"));
  //populate ROMCode in JSON structure
  for (byte i = 0; i < nbOfRomCode; i++) {
    c.print(F("\t\t\""));
    for (byte j = 0; j < 8; j++) {
      String s = String(romCodeList[i][j], HEX);
      c.print(s.length() == 1 ? String("0") + s : s);
    }
    if (i < nbOfRomCode - 1) c.print(F("\",\r\n"));
    else c.print(F("\"\r\n"));
  }
  //Finalize JSON structure
  c.print(F("\t]\r\n}\r\n"));
}
//------------------------------------------
// return True if s contain only hexadecimal figure
boolean isAlphaNumericString(String s) {

  if (s.length() == 0) return false;
  for (int i = 0; i < s.length(); i++) {
    if (s[i] < 0x30 || (s[i] > 0x39 && s[i] < 0x41) || (s[i] > 0x46 && s[i] < 0x61) || s[i] > 0x66) return false;
  }
  return true;
}
//------------------------------------------
void handleGetTemp(WiFiClient c, String req) {

  //DEBUG
  Serial.println(F("getTemp"));

  //check for ? in the url request
  if (req.indexOf('?') == -1 || req.indexOf('?') == (req.length() - 1)) {
    c.print(F("HTTP/1.1 400 Bad Request1\r\n\r\n"));
    return;
  }
  //keep only the part after '?' and before the final HTTP/1.1
  String getDatas = req.substring(req.indexOf('?') + 1);
  getDatas = getDatas.substring(0, getDatas.indexOf(' '));

  //try to find busNumber
  String strBusNumber = findParameterInURLEncodedDatas(getDatas, F("bus"));
  //check string found
  if (strBusNumber.length() != 1 || strBusNumber[0] < 0x30 || strBusNumber[0] > 0x39) {
    c.print(F("HTTP/1.1 400 Bad Request2\r\n\r\n"));
    return;
  }

  //convert busNumber
  int busNumber = strBusNumber[0] - 0x30;

  //check busNumber
  if (busNumber >= numberOfBuses) {
    c.print(F("HTTP/1.1 400 Bad Request3\r\n\r\n"));
    return;
  }

  //try to find ROMCode
  String strROMCode = findParameterInURLEncodedDatas(getDatas, F("ROMCode"));
  //check string found
  if (strROMCode.length() != 16 || !isAlphaNumericString(strROMCode)) {
    c.print(F("HTTP/1.1 400 Bad Request4\r\n\r\n"));
    return;
  }

  //Parse ROMCode
  byte romCode[8];
  for (byte i = 0; i < 8; i++) {
    romCode[i] = (asciiToHex(strROMCode[i * 2]) * 0x10) + asciiToHex(strROMCode[(i * 2) + 1]);
  }

#if ESP01_PLATFORM
  Serial.flush();
  delay(5);
  Serial.end();
#endif

  //Read Temperature
  float measuredTemperature = readTemp(OneWireDualPin(owBusesPins[2 * busNumber], owBusesPins[(2 * busNumber) + 1]), romCode);

#if ESP01_PLATFORM
  Serial.begin(SERIAL_SPEED);
#endif

  if (measuredTemperature == 12.3456F) {
    c.print(F("HTTP/1.1 400 Bad Request5\r\n\r\n"));
    return;
  }


  //Send client answer
  c.print(F("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n"));

  //build JSON structure while including temperature reading
  c.print(F("{\r\n\t\"Temperature\": "));
  c.print(String(measuredTemperature, 2));
  c.print(F("\r\n}\r\n"));

}

//------------------------------------------
void handleWifiClient(WiFiClient c) {

  Serial.println(F("handleWifiClient"));

  //wait for client to send datas
  int i = 0;
  while (!c.available()) {
    if (i == 10) {
      //the client did not sent request in timely fashion
      c.stop();
      return;
    }
    delay(100);
    i++;
  }

  //read first line request
  String req = c.readStringUntil('\r');

  if (req.startsWith(F("GET /getconfig HTTP/1."))) handleGetConfig(c);
  if (req.startsWith(F("GET /config HTTP/1."))) handleConfig(c);
  if (req.startsWith(F("POST /submit HTTP/1."))) handleSubmit(c);
  if (req.startsWith(F("GET /getList?"))) handleGetList(c, req);
  if (req.startsWith(F("GET /getTemp?"))) handleGetTemp(c, req);

  c.flush();
  c.stop();
}

//-----------------------------------------------------------------------
// Setup function
//-----------------------------------------------------------------------
void setup(void) {

  Serial.begin(SERIAL_SPEED);
  Serial.println("");
  delay(1000);

  Serial.print(F("J6B Wireless DS18B20 ")); Serial.print(VERSION_NUMBER); if (ESP01_PLATFORM) Serial.println(F(" (ESP01)")); else Serial.println("");
  Serial.println(F("---Booting---"));
  Serial.println(F("Wait skip config button for 5 seconds"));

#if ESP01_PLATFORM
  bool skipExistingConfig = false;
  pinMode(2, INPUT_PULLUP);
  for (int i = 0; i < 100 && skipExistingConfig == false; i++) {
    if (digitalRead(2) == LOW) skipExistingConfig = true;
    delay(50);
  }
#else
  bool skipExistingConfig = false;
  pinMode(CONFIG_BTN_PIN, INPUT_PULLUP);
  for (int i = 0; i < 100 && skipExistingConfig == false; i++) {
    if (digitalRead(CONFIG_BTN_PIN) == LOW) skipExistingConfig = true;
    delay(50);
  }
#endif

  Serial.print(F("Start Config"));

  //if skipExistingConfig is false then load the existing config
  if (!skipExistingConfig) {
    if (!loadConfig()) {
      Serial.println(F(" : Failed to load config!!!---------"));
    } else {
      Serial.println(F(" : OK"));
    }
  }
  else {
    Serial.println(F(" : OK (Config Skipped)"));
  }

  Serial.print(F("Start WiFi"));

  //Start Wifi
  if (APMode) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid.c_str(), password.c_str());
    Serial.println(F(" : OK (AP mode 192.168.4.1)"));
  }
  else {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    for (int i = 0; i < 50 && (WiFi.status() != WL_CONNECTED); i++) {
      delay(500);
      Serial.print(".");
      if (i == 49) {
        Serial.println(F("FAILED\r\nRESTART"));
        ESP.restart();
      }
    }
    Serial.println(F(" : OK (Client mode)"));
  }

  Serial.print(F("Start OTA"));

  //Setup and Start OTA
  ArduinoOTA.setPassword((const char *)"PasswordDS18B20");
  ArduinoOTA.onStart([]() {
    Serial.println(F("-OTAStart-"));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.print(F("-OTA Error : ")); Serial.println(error);
  });
  ArduinoOTA.onProgress([](unsigned int current, unsigned int total) {
    static byte lastProgress = 0;
    byte progress = ((current * 100 / total) / 5) * 5;
    if (lastProgress != progress) {
      lastProgress = progress;
      Serial.printf("%d%%\r\n", lastProgress);
    }
  });
  ArduinoOTA.begin();

  Serial.print(F(" : OK\r\nStart OneWire"));

#if ESP01_PLATFORM
  Serial.flush();
  delay(5);
  Serial.end();
#endif

  for (byte i = 0; i < numberOfBuses; i++) {
    setupTempSensors(OneWireDualPin(owBusesPins[2 * i], owBusesPins[(2 * i) + 1]));
  }

#if ESP01_PLATFORM
  Serial.begin(SERIAL_SPEED);
#endif

  Serial.print(F(" : OK\r\nStart WebServer"));

  server.begin();
  Serial.println(F(" : OK"));

  Serial.println(F("---End of setup()---"));
}

//-----------------------------------------------------------------------
// Main Loop function
//-----------------------------------------------------------------------
void loop(void) {
  ArduinoOTA.handle();

  // Check if a client has connected
  WiFiClient client = server.available();
  if (client) {
    handleWifiClient(client);
  }
  yield();
}



