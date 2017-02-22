#include <ESP8266WiFi.h>

#include "WebCore.h"
#include "WirelessDS18B20.h"

#include "WebDS18B20.h"


//----------------------------------------------------------------------
// --- WebDS18B20Bus Class---
//----------------------------------------------------------------------

//-----------------------------------------------------------------------
// DS18X20 Read ScratchPad command
boolean WebDS18B20Bus::readScratchPad(byte addr[], byte data[]) {

  boolean crcScratchPadOK = false;

  //read scratchpad (if 3 failures occurs, then return the error
  for (byte i = 0; i < 3; i++) {
    // read scratchpad of the current device
    reset();
    select(addr);
    write(0xBE); // Read ScratchPad
    for (byte j = 0; j < 9; j++) { // read 9 bytes
      data[j] = read();
    }
    if (crc8(data, 8) == data[8]) {
      crcScratchPadOK = true;
      i = 3; //end for loop
    }
  }

  return crcScratchPadOK;
}
//------------------------------------------
// DS18X20 Write ScratchPad command
void WebDS18B20Bus::writeScratchPad(byte addr[], byte th, byte tl, byte cfg) {

  reset();
  select(addr);
  write(0x4E); // Write ScratchPad
  write(th); //Th 80°C
  write(tl); //Tl 0°C
  write(cfg); //Config
}
//------------------------------------------
// DS18X20 Copy ScratchPad command
void WebDS18B20Bus::copyScratchPad(byte addr[]) {

  reset();
  select(addr);
  write(0x48); //Copy ScratchPad
}
//------------------------------------------
// DS18X20 Start Temperature conversion
void WebDS18B20Bus::startConvertT(byte addr[]) {
  reset();
  select(addr);
  write(0x44); // start conversion
}

//------------------------------------------
// Constructor for WebDS18B20Bus that call constructor of parent class OneWireDualPin
WebDS18B20Bus::WebDS18B20Bus(uint8_t pinIn, uint8_t pinOut): OneWireDualPin( pinIn, pinOut) {};
//------------------------------------------
// Function to initialize DS18X20 sensor
void WebDS18B20Bus::setupTempSensors() {

  byte i, j;
  byte addr[8];
  byte data[9];
  boolean scratchPadReaded;

  //while we find some devices
  while (search(addr)) {

    //if ROM received is incorrect or not a DS1822 or DS18B20 THEN continue to next device
    if ((crc8(addr, 7) != addr[7]) || (addr[0] != 0x22 && addr[0] != 0x28)) continue;

    scratchPadReaded = readScratchPad(addr, data);
    //if scratchPad read failed then continue to next 1-Wire device
    if (!scratchPadReaded) continue;

    //if config is not correct
    if (data[2] != 0x50 || data[3] != 0x00 || data[4] != 0x5F) {

      //write ScratchPad with Th=80°C, Tl=0°C, Config 11bit resolution
      writeScratchPad(addr, 0x50, 0x00, 0x5F);

      scratchPadReaded = readScratchPad(addr, data);
      //if scratchPad read failed then continue to next 1-Wire device
      if (!scratchPadReaded) continue;

      //so we finally can copy scratchpad to memory
      copyScratchPad(addr);
    }
  }
}
//------------------------------------------
// function that get temperature from a DS18X20 and send it to Web Client (run convertion, get scratchpad then calculate temperature)
void WebDS18B20Bus::getTemp(WiFiClient c, byte addr[]) {

  byte i, j;
  byte data[12];

  startConvertT(addr);

  //wait for conversion end (DS18B20 are powered)
  while (read_bit() == 0) {
    delay(10);
  }

  //if read of scratchpad failed (3 times inside function) then return special fake value
  if (!readScratchPad(addr, data)) {
    WebCore::SendHTTPResponse(c, 400, WebCore::html, F("Temperature read failed"));
    return;
  }


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
  //result is (float)raw / 16.0;

  //make JSON
  String gtJSON(F("{\r\n\t\"Temperature\": "));
  gtJSON.reserve(28);
  gtJSON += String((float)raw / 16.0, 2) + F("\r\n}");

  //Send client answer (build JSON structure while including temperature reading)
  WebCore::SendHTTPResponse(c, 200, WebCore::json, gtJSON.c_str());
}
//------------------------------------------
// List DS18X20 sensor ROMCode and send it to Web Client
void WebDS18B20Bus::getRomCodeList(WiFiClient c) {

  bool first = true;
  uint8_t romCode[8];

  //prepare JSON structure
  String grclJSON(F("{\"TemperatureSensorList\": [\r\n"));

  reset_search();

  while (search(romCode)) {

    //if ROM received is incorrect or not a Temperature sensor THEN continue to next device
    if ((crc8(romCode, 7) != romCode[7]) || (romCode[0] != 0x10 && romCode[0] != 0x22 && romCode[0] != 0x28)) continue;

    //increase grclJSON size to limit heap fragment
    grclJSON.reserve(grclJSON.length() + 22);

    //populate JSON answer with romCode found
    if (!first) grclJSON += ',';
    else first = false;
    grclJSON += '\"';
    for (byte i = 0; i < 8; i++) {
      if (romCode[i] < 16)grclJSON += '0';
      grclJSON += String(romCode[i], HEX);
    }
    grclJSON += F("\"\r\n");
  }
  //Finalize JSON structure
  grclJSON += F("]\r\n}");

  WebCore::SendHTTPResponse(c, 200, WebCore::json, grclJSON.c_str());
}








//----------------------------------------------------------------------
// --- WebDS18B20Buses Class---
//----------------------------------------------------------------------

//------------------------------------------
// return True if s contain only hexadecimal figure
boolean WebDS18B20Buses::isROMCodeString(char* s) {

  if (strlen(s) != 16) return false;
  for (int i = 0; i < 16; i++) {
    if (!isHexadecimalDigit(s[i])) return false;
  }
  return true;
}
//------------------------------------------
//Init function to store number of Buses and pins associated
void WebDS18B20Buses::Init(byte nbOfBuses, uint8_t owBusesPins[][2]) {
  _nbOfBuses = nbOfBuses;
  _owBusesPins = owBusesPins;
  _initialized = (nbOfBuses > 0);

  for (byte i = 0; i < _nbOfBuses; i++) {
    WebDS18B20Bus(_owBusesPins[i][0], _owBusesPins[i][1]).setupTempSensors();
  }
}
//------------------------------------------
//Parse GET request to list temp sensor on a bus
void WebDS18B20Buses::GetList(WiFiClient c, String &req) {

  //LOG
  Serial.println(F("getList"));

  //check WebDS18B20Buses is initialized
  if (!_initialized) {
    WebCore::SendHTTPResponse(c, 400, WebCore::html, F("Buses not Initialized"));
    return;
  }

  //check request structure
  if (req.indexOf('?') == -1 || req.indexOf(F("? ")) != -1 || req.indexOf(F(" HTTP/")) == -1) {
    WebCore::SendHTTPResponse(c, 400, WebCore::html, F("Missing parameter"));
    return;
  }
  //keep only the part after '?' and before the final HTTP/1.1
  String getDatas = req.substring(req.indexOf('?') + 1, req.indexOf(F(" HTTP/")));



  //try to find busNumber
  char busNumberA[2] = {0}; //length limited to 1 char
  if (!WebCore::FindParameterInURLEncodedDatas(getDatas.c_str(), F("bus"), busNumberA, sizeof(busNumberA))) {
    WebCore::SendHTTPResponse(c, 400, WebCore::html, F("Incorrect bus number"));
    return;
  }
  //check string found
  if (busNumberA[0] < 0x30 || busNumberA[0] > 0x39) {
    WebCore::SendHTTPResponse(c, 400, WebCore::html, F("Incorrect bus number"));
    return;
  }
  //convert busNumber
  int busNumber = busNumberA[0] - 0x30;

  //check busNumber
  if (busNumber >= _nbOfBuses) {
    WebCore::SendHTTPResponse(c, 400, WebCore::html, F("Wrong bus number"));
    return;
  }

#if ESP01_PLATFORM
  Serial.flush();
  delay(5);
  Serial.end();
#endif

  //Search for OneWire Temperature sensor
  WebDS18B20Bus(_owBusesPins[busNumber][0], _owBusesPins[busNumber][1]).getRomCodeList(c);

#if ESP01_PLATFORM
  Serial.begin(SERIAL_SPEED);
#endif

}
//------------------------------------------
//Parse GET request to read a Temperature
void WebDS18B20Buses::GetTemp(WiFiClient c, String &req) {

  //LOG
  Serial.println(F("getTemp"));

  //check WebDS18B20Buses is initialized
  if (!_initialized) {
    WebCore::SendHTTPResponse(c, 400, WebCore::html, F("Buses not Initialized"));
    return;
  }

  //check request structure
  if (req.indexOf('?') == -1 || req.indexOf(F("? ")) != -1 || req.indexOf(F(" HTTP/")) == -1) {
    WebCore::SendHTTPResponse(c, 400, WebCore::html, F("Missing parameter"));
    return;
  }
  //keep only the part after '?' and before the final HTTP/1.1
  String getDatas = req.substring(req.indexOf('?') + 1, req.indexOf(F(" HTTP/")));

  //try to find busNumber
  char busNumberA[2] = {0}; //length limited to 1 char
  if (!WebCore::FindParameterInURLEncodedDatas(getDatas.c_str(), F("bus"), busNumberA, sizeof(busNumberA))) {
    WebCore::SendHTTPResponse(c, 400, WebCore::html, F("Incorrect bus number"));
    return;
  }
  //check string found
  if (busNumberA[0] < 0x30 || busNumberA[0] > 0x39) {
    WebCore::SendHTTPResponse(c, 400, WebCore::html, F("Incorrect bus number"));
    return;
  }
  //convert busNumber
  int busNumber = busNumberA[0] - 0x30;

  //check busNumber
  if (busNumber >= _nbOfBuses) {
    WebCore::SendHTTPResponse(c, 400, WebCore::html, F("Wrong bus number"));
    return;
  }



  //try to find ROMCode
  char ROMCodeA[17] = {0};
  //check string found
  if (!WebCore::FindParameterInURLEncodedDatas(getDatas.c_str(), F("ROMCode"), ROMCodeA, sizeof(ROMCodeA)) || !isROMCodeString(ROMCodeA)) {
    WebCore::SendHTTPResponse(c, 400, WebCore::html, F("Incorrect ROMCode"));
    return;
  }

  //Parse ROMCode
  byte romCode[8];
  for (byte i = 0; i < 8; i++) {
    romCode[i] = (WebCore::AsciiToHex(ROMCodeA[i * 2]) * 0x10) + WebCore::AsciiToHex(ROMCodeA[(i * 2) + 1]);
  }

#if ESP01_PLATFORM
  Serial.flush();
  delay(5);
  Serial.end();
#endif

  //Read Temperature
  WebDS18B20Bus(_owBusesPins[busNumber][0], _owBusesPins[busNumber][1]).getTemp(c, romCode);

#if ESP01_PLATFORM
  Serial.begin(SERIAL_SPEED);
#endif
}
//------------------------------------------
//return OneWire Status
void WebDS18B20Buses::GetStatus(WiFiClient c) {
  //nothing to send yet
  WebCore::SendHTTPResponse(c, 200);
}
