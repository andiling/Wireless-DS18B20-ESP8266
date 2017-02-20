
#include <ESP8266WiFi.h>

#include "Config.h"
#include "WebCore.h"
#include "WirelessDS18B20.h"

#include "WebConfig.h"


//------------------------------------------
void WebConfig::Get(WiFiClient c) {

  //LOG
  Serial.println(F("handleGetConfig"));

  //{"a":"off","s":"Wifi","h":"TotoPC","n":1,"b0i":3,"b0o":0,"b":"1.4 (ESP01)","u":"5d23h50m","f":45875}

  strcpy_P(buf, PSTR("HTTP/1.1 200 OK\r\nContent-Type: text/json\r\nExpires: 0\r\n\r\n"));

  sprintf_P(buf, PSTR("%s{\"a\":\"%s\",\"s\":\"%s\",\"h\":\"%s\""), buf, APMode ? "on" : "off", ssid, hostname);

#if !ESP01_PLATFORM
  sprintf_P(buf, PSTR("%s,\"n\":%d,\"nm\":%d"), buf, numberOfBuses, MAX_NUMBER_OF_BUSES);
  for (int i = 0; i < numberOfBuses; i++) {
    sprintf_P(buf, PSTR("%s,\"b%di\":%d,\"b%do\":%d"), buf, i, owBusesPins[i][0], i, owBusesPins[i][1]);
  }
#else
  strcat_P(buf, PSTR(",\"e\":1,\"n\":1,\"nm\":1,\"b0i\":3,\"b0o\":0"));
#endif

  unsigned long minutes = millis() / 60000;

  sprintf_P(buf, PSTR("%s,\"b\":\"%s\",\"u\":\"%dd%dh%dm\",\"f\":%d}"), buf, VERSION, (byte)(minutes / 1440), (byte)(minutes / 60 % 24), (byte)(minutes % 60), ESP.getFreeHeap());

  c.write((uint8_t *)buf, strlen(buf));
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
    WebCore::SendHTTPShortAnswer(c, 400, WebCore::html, F("Incorrect SSID"));
    return;
  }
  tempPassword = WebCore::FindParameterInURLEncodedDatas(postedDatas, F("p"));
  tempHostname = WebCore::FindParameterInURLEncodedDatas(postedDatas, F("h"));
#if !ESP01_PLATFORM
  if (WebCore::FindParameterInURLEncodedDatas(postedDatas, F("n")).length() == 0) {
    WebCore::SendHTTPShortAnswer(c, 400, WebCore::html, F("Missing number of OW Buses"));
    return;
  }
  tempNumberOfBuses = WebCore::FindParameterInURLEncodedDatas(postedDatas, F("n")).toInt();
  if (tempNumberOfBuses < 1 || tempNumberOfBuses > MAX_NUMBER_OF_BUSES) {
    WebCore::SendHTTPShortAnswer(c, 400, WebCore::html, F("Incorrect number of OW Buses"));
    return;
  }
  for (int i = 0; i < tempNumberOfBuses; i++) {
    if (WebCore::FindParameterInURLEncodedDatas(postedDatas, String("b") + i + "i").length() == 0) {
      WebCore::SendHTTPShortAnswer(c, 400, WebCore::html, F("A PinIn value is missing"));
      return;
    }
    tempOwBusesPins[i][0] = WebCore::FindParameterInURLEncodedDatas(postedDatas, String("b") + i + "i").toInt();
    if (WebCore::FindParameterInURLEncodedDatas(postedDatas, String("b") + i + "o").length() == 0) {
      WebCore::SendHTTPShortAnswer(c, 400, WebCore::html, F("A PinOut value is missing"));
      return;
    }
    tempOwBusesPins[i][1] = WebCore::FindParameterInURLEncodedDatas(postedDatas, String("b") + i + "o").toInt();
  }
#endif

  //config checked so copy
  APMode = tempAPMode;
  tempSsid.toCharArray(ssid, sizeof(ssid));
  tempPassword.toCharArray(password, sizeof(password));
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

  if (result) WebCore::SendHTTPShortAnswer(c, 200);
  else WebCore::SendHTTPShortAnswer(c, 500, WebCore::html, PSTR("Configuration hasn't been saved"));

  delay(1000);

  //restart ESP to apply new config
  ESP.restart();
}
