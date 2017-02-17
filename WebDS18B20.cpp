
#include <ESP8266WiFi.h>

#include "WebCore.h"
#include "config.h"
#include "WirelessDS18B20.h"

#include "WebDS18B20.h"

WebDS18B20Bus::WebDS18B20Bus(uint8_t pinIn, uint8_t pinOut): OneWireDualPin( pinIn, pinOut) {};


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
// return True if s contain only hexadecimal figure
boolean WebDS18B20Bus::isROMCodeString(String s) {

  if (s.length() != 16) return false;
  for (int i = 0; i < 16; i++) {
    if (!isHexadecimalDigit(s[i])) return false;
  }
  return true;
}






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
    strcpy_P(buf, PSTR("HTTP/1.1 400 Temperature read failed\r\n\r\n"));
    c.write((uint8_t *)buf, strlen(buf));
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

  //Send client answer (build JSON structure while including temperature reading)
  //sprintf_P(buf, PSTR("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n\r\n"), .c_str());
  //c.write((uint8_t *)buf, strlen(buf));
  WebCore::SendHTTPShortAnswer(c, 200, WebCore::json, "{\r\n\t\"Temperature\": " + String((float)raw / 16.0, 2) + "\r\n}");
}
//------------------------------------------
// Get an array of DS18X20 sensor ROMCode and send it to Web Client
void WebDS18B20Bus::getRomCodeList(WiFiClient c) {

  byte nbFound = 0;

  uint8_t romCode[8];

  //prepare response header
  strcpy_P(buf, PSTR("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n"));
  //prepare JSON structure
  strcat_P(buf, PSTR("{\r\n\t\"TemperatureSensorList\": [\r\n"));

  reset_search();

  while (search(romCode) && nbFound < ((sizeof(buf) - 90/*(header+startOfJSON)*/) / 23 /*(nbchar per ROMCode)*/)) {

    //if ROM received is incorrect or not a Temperature sensor THEN continue to next device
    if ((crc8(romCode, 7) != romCode[7]) || (romCode[0] != 0x10 && romCode[0] != 0x22 && romCode[0] != 0x28)) continue;

    //populate JSON answer with romCode found
    sprintf_P(buf, PSTR("%s\t\t\%s\""), buf, nbFound == 0 ? "" : ",");
    for (byte i = 0; i < 8; i++) {
      sprintf_P(buf, PSTR("%s%02x"), buf, romCode[i]);
    }
    strcat_P(buf, PSTR("\"\r\n"));

    nbFound++;
  }

  //Finalize JSON structure
  strcat_P(buf, PSTR("\t]\r\n}\r\n"));

  c.write((uint8_t *)buf, strlen(buf));
}







//------------------------------------------
void WebDS18B20Bus::GetList(WiFiClient c, String req, byte nbOfBuses, uint8_t owBusesPins[][2]) {

  //LOG
  Serial.println(F("getList"));

  //check request structure
  if (req.indexOf('?') == -1 || req.indexOf(F("? ")) != -1 || req.indexOf(F(" HTTP/")) == -1) {
    strcpy_P(buf, PSTR("HTTP/1.1 400 missing parameter\r\n\r\n"));
    c.write((uint8_t *)buf, strlen(buf));
    return;
  }
  //keep only the part after '?' and before the final HTTP/1.1
  String getDatas = req.substring(req.indexOf('?') + 1, req.indexOf(F(" HTTP/")));

  //try to find busNumber
  String strBusNumber = WebCore::FindParameterInURLEncodedDatas(getDatas, F("bus"));
  //check string found
  if (strBusNumber.length() != 1 || strBusNumber[0] < 0x30 || strBusNumber[0] > 0x39) {
    strcpy_P(buf, PSTR("HTTP/1.1 400 Incorrect bus number\r\n\r\n"));
    c.write((uint8_t *)buf, strlen(buf));
    return;
  }

  //convert busNumber
  int busNumber = strBusNumber[0] - 0x30;

  //check busNumber
  if (busNumber >= nbOfBuses) {
    strcpy_P(buf, PSTR("HTTP/1.1 400 Wrong bus number\r\n\r\n"));
    c.write((uint8_t *)buf, strlen(buf));
    return;
  }

#if ESP01_PLATFORM
  Serial.flush();
  delay(5);
  Serial.end();
#endif

  //Search for OneWire Temperature sensor
  WebDS18B20Bus(owBusesPins[busNumber][0], owBusesPins[busNumber][1]).getRomCodeList(c);

#if ESP01_PLATFORM
  Serial.begin(SERIAL_SPEED);
#endif

}
//------------------------------------------
void WebDS18B20Bus::GetTemp(WiFiClient c, String req, byte nbOfBuses, uint8_t owBusesPins[][2]) {

  //LOG
  Serial.println(F("getTemp"));

  //check request structure
  if (req.indexOf('?') == -1 || req.indexOf(F("? ")) != -1 || req.indexOf(F(" HTTP/")) == -1) {
    strcpy_P(buf, PSTR("HTTP/1.1 400 missing parameter\r\n\r\n"));
    c.write((uint8_t *)buf, strlen(buf));
    return;
  }
  //keep only the part after '?' and before the final HTTP/1.1
  String getDatas = req.substring(req.indexOf('?') + 1, req.indexOf(F(" HTTP/")));

  //try to find busNumber
  String strBusNumber = WebCore::FindParameterInURLEncodedDatas(getDatas, F("bus"));
  //check string found
  if (strBusNumber.length() != 1 || strBusNumber[0] < 0x30 || strBusNumber[0] > 0x39) {
    strcpy_P(buf, PSTR("HTTP/1.1 400 Incorrect bus number\r\n\r\n"));
    c.write((uint8_t *)buf, strlen(buf));
    return;
  }

  //convert busNumber
  int busNumber = strBusNumber[0] - 0x30;

  //check busNumber
  if (busNumber >= nbOfBuses) {
    strcpy_P(buf, PSTR("HTTP/1.1 400 Wrong bus number\r\n\r\n"));
    c.write((uint8_t *)buf, strlen(buf));
    return;
  }

  //try to find ROMCode
  String strROMCode = WebCore::FindParameterInURLEncodedDatas(getDatas, F("ROMCode"));
  //check string found
  if (!isROMCodeString(strROMCode)) {
    strcpy_P(buf, PSTR("HTTP/1.1 400 Incorrect ROMCode\r\n\r\n"));
    c.write((uint8_t *)buf, strlen(buf));
    return;
  }

  //Parse ROMCode
  byte romCode[8];
  for (byte i = 0; i < 8; i++) {
    romCode[i] = (WebCore::AsciiToHex(strROMCode[i * 2]) * 0x10) + WebCore::AsciiToHex(strROMCode[(i * 2) + 1]);
  }

#if ESP01_PLATFORM
  Serial.flush();
  delay(5);
  Serial.end();
#endif

  //Read Temperature
  WebDS18B20Bus(owBusesPins[busNumber][0], owBusesPins[busNumber][1]).getTemp(c, romCode);

#if ESP01_PLATFORM
  Serial.begin(SERIAL_SPEED);
#endif


}

