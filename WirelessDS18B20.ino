#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <FS.h>

//Please, have a look at WirelessDS18B20.h for information and configuration of Arduino project

#include "WebCore.h"
#include "WebConfig.h"
#include "WebDS18B20.h"

#include "WirelessDS18B20.h"

//Config object
WebConfig webConfig;

WebDS18B20Buses webDSBuses;

//WiFiServer
WiFiServer server(80);


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
  String req = c.readStringUntil('\n');

  if (req.startsWith(F("GET /fw HTTP/1."))) WebCore::GetNativeContent(c, WebCore::fw);
  else if (req.startsWith(F("POST /fw HTTP/1."))) WebCore::PostFile(c, true); //true indicates that we upload a firmware
  else if (req.startsWith(F("GET /jquery-3.1.1.min.js HTTP/1."))) WebCore::GetNativeContent(c, WebCore::jquery);
  else if (req.startsWith(F("GET /config HTTP/1."))) WebCore::GetNativeContent(c, WebCore::config);
  else if (req.startsWith(F("GET /status HTTP/1."))) WebCore::GetNativeContent(c, WebCore::status);
  else if (req.startsWith(F("GET /gs0 HTTP/1."))) WebCore::GetSystemStatus(c);
#if DEVELOPPER_MODE
  else if (req.startsWith(F("GET /fwdev HTTP/1."))) WebCore::GetNativeContent(c, WebCore::fwdev);
  else if (req.startsWith(F("GET /fl HTTP/1."))) WebCore::GetFileList(c);
  else if (req.startsWith(F("POST /up HTTP/1."))) WebCore::PostFile(c);
  else if (req.startsWith(F("GET /rm"))) WebCore::GetRemoveFile(c, req);
#endif
  else if (req.startsWith(F("GET /gc HTTP/1."))) webConfig.Get(c);
  else if (req.startsWith(F("POST /sc HTTP/1."))) webConfig.Post(c);
  else if (req.startsWith(F("GET /getList?"))) webDSBuses.GetList(c, req);
  else if (req.startsWith(F("GET /getTemp?"))) webDSBuses.GetTemp(c, req);
  else if (req.startsWith(F("GET /gs1 HTTP/1."))) webDSBuses.GetStatus(c);
#if DEVELOPPER_MODE
  else if (req.startsWith(F("GET "))) WebCore::GetFile(c, req);
#endif
  else WebCore::SendHTTPResponse(c, 404);

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

  Serial.print(F("J6B Wireless DS18B20 ")); Serial.println(VERSION);
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
  pinMode(RESCUE_BTN_PIN, (RESCUE_BTN_PIN != 16) ? INPUT_PULLUP : INPUT);
  for (int i = 0; i < 100 && skipExistingConfig == false; i++) {
    if (digitalRead(RESCUE_BTN_PIN) == LOW) skipExistingConfig = true;
    delay(50);
  }
#endif

  Serial.print(F("Start Config"));

  //initialize config with default values
  webConfig.SetDefaultValues();
  
  //if skipExistingConfig is false then load the existing config
  if (!skipExistingConfig) {
    if (!webConfig.Load()) {
      Serial.println(F(" : Failed to load config!!!---------"));
    } else {
      Serial.println(F(" : OK"));
    }
  }
  else {
    Serial.println(F(" : OK (Config Skipped)"));
  }

#if DEVELOPPER_MODE
  Serial.print(F("Load SPIFFS"));
  if (SPIFFS.begin()) Serial.println(F(" : OK"));
  else {
    Serial.println(F(" : FAILED\r\nFormat SPIFFS"));
    if (SPIFFS.format()) Serial.println(F(" : OK"));
    else Serial.println(F(" : FAILED"));

    ESP.reset();
  }
#endif

  Serial.print(F("Start WiFi"));

  //disconnect first before reconfigure WiFi
  WiFi.disconnect();

  //setup hostname
  if (webConfig.hostname[0]) WiFi.hostname(webConfig.hostname);

  //Start Wifi
  if (webConfig.apMode) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(webConfig.ssid, webConfig.password);
    Serial.println(F(" : OK (AP mode 192.168.4.1)"));
  }
  else {
    WiFi.mode(WIFI_STA);
    WiFi.begin(webConfig.ssid, webConfig.password);
    for (int i = 0; i < 50 && (WiFi.status() != WL_CONNECTED); i++) {
      delay(500);
      Serial.print(".");
      if (i == 49) {
        Serial.println(F("FAILED\r\nRESTART"));
        ESP.restart();
      }
    }
    Serial.print(F(" : OK (Client mode ")); Serial.print(WiFi.localIP().toString()); Serial.println(")");
  }

#if OTA
  Serial.print(F("Start OTA"));

  //Setup and Start OTA
  ArduinoOTA.setPassword(webConfig.otaPassword);
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
  Serial.print(F(" : OK\r\n"));
#endif

  Serial.print(F("Start OneWire"));

#if ESP01_PLATFORM
  Serial.flush();
  delay(5);
  Serial.end();

  webConfig.numberOfBuses = 1;
  webConfig.owBusesPins[0][0] = 3;
  webConfig.owBusesPins[0][1] = 0;
#endif

  webDSBuses.Init(webConfig.numberOfBuses, webConfig.owBusesPins);

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

#if OTA
  ArduinoOTA.handle();
#endif

  // Check if a client has connected
  WiFiClient client = server.available();
  if (client) {
    handleWifiClient(client);
  }
  yield();
}


