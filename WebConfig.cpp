#include <ESP8266WiFi.h>

#include "WebCore.h"
#include "WirelessDS18B20.h"

#include "WebConfig.h"


//------------------------------------------
void WebConfig::Get(WiFiClient c) {

  //LOG
  Serial.println(F("handleGetConfig"));

  //{"s":"Wifi","h":"TotoPC","n":1,"b0i":3,"b0o":0,"b":"1.4 (ESP01)","u":"5d23h50m","f":45875}

  String gc = F("{\"s\":\"");
  //there is a predefined special password (mean to keep already saved one)
  gc = gc + ssid + F("\",\"p\":\"") + (__FlashStringHelper*)predefPassword + F("\",\"h\":\"") + hostname + '"';

#if !ESP01_PLATFORM
  gc = gc + F(",\"n\":") + numberOfBuses + F(",\"nm\":") + MAX_NUMBER_OF_BUSES;
  for (int i = 0; i < numberOfBuses; i++) {
    gc = gc + F(",\"b") + i + F("i\":") + owBusesPins[i][0] + F(",\"b") + i + F("o\":") + owBusesPins[i][1];
  }
#else
  gc += F(",\"e\":1,\"n\":1,\"nm\":1,\"b0i\":3,\"b0o\":0");
#endif

  gc += '}';

  WebCore::SendHTTPResponse(c, 200, WebCore::json, gc.c_str());
}


//------------------------------------------
void WebConfig::Post(WiFiClient c) {

  String postedDatas = "";

  //temp config
  Config tempConfig;


  //bypass Header
  postedDatas = c.readStringUntil('\n');
  while (postedDatas != "\r") {
    postedDatas = c.readStringUntil('\n');
  }

  //read POSTed datas
  postedDatas = c.readStringUntil('\r');

  //Parse Parameters
  char checkboxA[3]; //only on answer is interesting so 3
  if (!WebCore::FindParameterInURLEncodedDatas(postedDatas.c_str(), F("s"), tempConfig.ssid, sizeof(tempConfig.ssid))) {
    WebCore::SendHTTPResponse(c, 400, WebCore::html, F("SSID missing"));
    return;
  }
  WebCore::FindParameterInURLEncodedDatas(postedDatas.c_str(), F("p"), tempConfig.password, sizeof(tempConfig.password));
  WebCore::FindParameterInURLEncodedDatas(postedDatas.c_str(), F("h"), tempConfig.hostname, sizeof(tempConfig.hostname));


#if !ESP01_PLATFORM
  char tempNumberOfBusesA[2]; //only one char
  if (!WebCore::FindParameterInURLEncodedDatas(postedDatas.c_str(), F("n"), tempNumberOfBusesA, sizeof(tempNumberOfBusesA))) {
    WebCore::SendHTTPResponse(c, 400, WebCore::html, F("Missing number of OW Buses"));
    return;
  }
  tempConfig.numberOfBuses = atoi(tempNumberOfBusesA);
  if (tempConfig.numberOfBuses < 1 || tempConfig.numberOfBuses > MAX_NUMBER_OF_BUSES) {
    WebCore::SendHTTPResponse(c, 400, WebCore::html, F("Incorrect number of OW Buses"));
    return;
  }
  char busPinName[4] = {'b', '0', 'i', 0};
  for (int i = 0; i < tempConfig.numberOfBuses; i++) {
    char busPinA[4] = {0};
    busPinName[1] = '0' + i;
    busPinName[2] = 'i';
    if (!WebCore::FindParameterInURLEncodedDatas(postedDatas.c_str(), busPinName, busPinA, sizeof(busPinA)) || !busPinA[0]) {
      WebCore::SendHTTPResponse(c, 400, WebCore::html, F("A PinIn value is missing"));
      return;
    }
    if (atoi(busPinA) == 0 && (busPinA[0] != '0' || busPinA[1])) {
      WebCore::SendHTTPResponse(c, 400, WebCore::html, F("A PinIn value is incorrect"));
      return;
    }
    tempConfig.owBusesPins[i][0] = atoi(busPinA);

    busPinA[0] = 0;
    busPinName[2] = 'o';
    if (!WebCore::FindParameterInURLEncodedDatas(postedDatas.c_str(), busPinName, busPinA, sizeof(busPinA)) || !busPinA[0]) {
      WebCore::SendHTTPResponse(c, 400, WebCore::html, F("A PinOut value is missing"));
      return;
    }
    if (atoi(busPinA) == 0 && (busPinA[0] != '0' || busPinA[1])) {
      WebCore::SendHTTPResponse(c, 400, WebCore::html, F("A PinOut value is incorrect"));
      return;
    }
    tempConfig.owBusesPins[i][1] = atoi(busPinA);
  }
#endif

  //check for previous password ssid and apiKey (there is a predefined special password that mean to keep already saved one)
  if (!strcmp_P(tempConfig.password, predefPassword)) strcpy(tempConfig.password, password);

#if ESP01_PLATFORM
  tempConfig.numberOfBuses = 1;
  tempConfig.owBusesPins[0][0] = 3;
  tempConfig.owBusesPins[0][1] = 0;
#endif

  //then save
  bool result = tempConfig.Save();

  //Send client answer

  if (result) WebCore::SendHTTPResponse(c, 200);
  else WebCore::SendHTTPResponse(c, 500, WebCore::html, F("Configuration hasn't been saved"));

  delay(1000);

  //restart ESP to apply new config
  ESP.restart();
}
