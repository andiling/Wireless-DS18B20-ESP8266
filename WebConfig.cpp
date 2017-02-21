#include <ESP8266WiFi.h>

#include "WebConfig.h"

#include "WebCore.h"
#include "WirelessDS18B20.h"




//------------------------------------------
void WebConfig::Get(WiFiClient c) {

  //LOG
  Serial.println(F("handleGetConfig"));

  //{"a":"off","s":"Wifi","h":"TotoPC","n":1,"b0i":3,"b0o":0,"b":"1.4 (ESP01)","u":"5d23h50m","f":45875}

  String gc = F("{\"a\":\"");
  //there is a predefined special password (mean to keep already saved one)
  gc = gc + (APMode ? F("on") : F("off")) + F("\",\"s\":\"") + ssid + F("\",\"p\":\"ewcXoCt4HHjZUvY0\",\"h\":\"") + hostname + '\"';

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

  //temp variable for args
  bool tempAPMode = false;
  String tempSsid;
  String tempPassword = "";
  String tempHostname = "";
  byte tempNumberOfBuses = 0;
  uint8_t tempOwBusesPins[MAX_NUMBER_OF_BUSES][2];

  //LOG
  Serial.println(F("handleSubmit"));

  //bypass Header
  postedDatas = c.readStringUntil('\n');
  while (postedDatas != "\r") {
    postedDatas = c.readStringUntil('\n');
  }

  //read POSTed datas
  postedDatas = c.readStringUntil('\r');

  //Parse Parameters
  if (WebCore::FindParameterInURLEncodedDatas(postedDatas, F("a")) == "on") tempAPMode = true;
  tempSsid = WebCore::FindParameterInURLEncodedDatas(postedDatas, F("s"));
  if (tempSsid.length() == 0) {
    WebCore::SendHTTPResponse(c, 400, WebCore::html, F("Incorrect SSID"));
    return;
  }
  tempPassword = WebCore::FindParameterInURLEncodedDatas(postedDatas, F("p"));
  tempHostname = WebCore::FindParameterInURLEncodedDatas(postedDatas, F("h"));
#if !ESP01_PLATFORM
  if (WebCore::FindParameterInURLEncodedDatas(postedDatas, F("n")).length() == 0) {
    WebCore::SendHTTPResponse(c, 400, WebCore::html, F("Missing number of OW Buses"));
    return;
  }
  tempNumberOfBuses = WebCore::FindParameterInURLEncodedDatas(postedDatas, F("n")).toInt();
  if (tempNumberOfBuses < 1 || tempNumberOfBuses > MAX_NUMBER_OF_BUSES) {
    WebCore::SendHTTPResponse(c, 400, WebCore::html, F("Incorrect number of OW Buses"));
    return;
  }
  for (int i = 0; i < tempNumberOfBuses; i++) {
    if (WebCore::FindParameterInURLEncodedDatas(postedDatas, String("b") + i + "i").length() == 0) {
      WebCore::SendHTTPResponse(c, 400, WebCore::html, F("A PinIn value is missing"));
      return;
    }
    tempOwBusesPins[i][0] = WebCore::FindParameterInURLEncodedDatas(postedDatas, String("b") + i + "i").toInt();
    if (WebCore::FindParameterInURLEncodedDatas(postedDatas, String("b") + i + "o").length() == 0) {
      WebCore::SendHTTPResponse(c, 400, WebCore::html, F("A PinOut value is missing"));
      return;
    }
    tempOwBusesPins[i][1] = WebCore::FindParameterInURLEncodedDatas(postedDatas, String("b") + i + "o").toInt();
  }
#endif

  //config checked so copy
  APMode = tempAPMode;
  tempSsid.toCharArray(ssid, sizeof(ssid));
  //there is a predefined special password (mean to keep already saved one)
  if (tempPassword != F("ewcXoCt4HHjZUvY0")) tempPassword.toCharArray(password, sizeof(password));
  tempHostname.toCharArray(hostname, sizeof(hostname));

#if !ESP01_PLATFORM
  numberOfBuses = tempNumberOfBuses;
  for (int i = 0; i < tempNumberOfBuses; i++) {
    owBusesPins[i][0] = tempOwBusesPins[i][0];
    owBusesPins[i][1] = tempOwBusesPins[i][1];
  }
#else
  numberOfBuses = 1;
  owBusesPins[0][0] = 3;
  owBusesPins[0][1] = 0;
#endif

  //then save
  bool result = save();

  //Send client answer

  if (result) WebCore::SendHTTPResponse(c, 200);
  else WebCore::SendHTTPResponse(c, 500, WebCore::html, F("Configuration hasn't been saved"));

  delay(1000);

  //restart ESP to apply new config
  ESP.restart();
}
