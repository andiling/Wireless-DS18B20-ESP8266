#include <Arduino.h> //for byte type
#include <ESP8266WiFi.h>
#include <FS.h>

#include "WebCore.h"

//------------------------------------------
//simple function that convert an hexadecimal char to byte
byte WebCore::AsciiToHex(char c) {
  return (c < 0x3A) ? (c - 0x30) : (c > 0x60 ? c - 0x57 : c - 0x37);
}

//------------------------------------------
// Function that lookup in Datas to find a parameter and then return the corresponding decoded value
bool WebCore::FindParameterInURLEncodedDatas(const char* datas, const char* parameterToFind, char* returnBuf, size_t returnBufSize) {

  //can we find the param in the url
  char* param = strstr(datas, parameterToFind);
  //while posParam is not NULL
  while (param != NULL) {
    //if it's at beginning or previous char is & AND char just after is '='
    if ((param == datas || (param != datas && param[-1] == '&')) && param[strlen(parameterToFind)] == '=') {
      param += strlen(parameterToFind) + 1;
      break;
    }
    //look after the substr found
    param = strstr(param + strlen(parameterToFind), parameterToFind);
  }

  //if we didn't find it return false
  if (param == NULL) return false;

  //now we can copy the value and decode it at the same time
  int posParam = 0;
  int posReturnBuf = 0;
  returnBuf[0] = 0;
  while (param[posParam] && param[posParam] != '&' && posReturnBuf < returnBufSize) {

    if (param[posParam] == '+') returnBuf[posReturnBuf++] = ' ';
    else if (param[posParam] == '%') {
      returnBuf[posReturnBuf++] = (char)(AsciiToHex(param[posParam + 1]) * 0x10 + AsciiToHex(param[posParam + 2]));
      posParam += 2;
    }
    else returnBuf[posReturnBuf++] = param[posParam];

    returnBuf[posReturnBuf] = 0;
    posParam++;
  }
  if (posReturnBuf >= returnBufSize) return false;
  return true;
}
// Function that lookup in Datas to find a parameter and then return the corresponding decoded value
bool WebCore::FindParameterInURLEncodedDatas(const char* datas, const __FlashStringHelper* parameterToFind, char* returnBuf, size_t returnBufSize) {

  const char* _parameterToFind = (const char*)parameterToFind;

  //can we find the param in the url
  char* param = strstr_P(datas, _parameterToFind);
  //while posParam is not NULL
  while (param != NULL) {
    //if it's at beginning or previous char is & AND char just after is '='
    if ((param == datas || (param != datas && param[-1] == '&')) && param[strlen_P(_parameterToFind)] == '=') {
      param += strlen_P(_parameterToFind) + 1;
      break;
    }
    //look after the substr found
    param = strstr_P(param + strlen_P(_parameterToFind), _parameterToFind);
  }

  //if we didn't find it return false
  if (param == NULL) return false;

  //now we can copy the value and decode it at the same time
  int posParam = 0;
  int posReturnBuf = 0;
  returnBuf[0] = 0;
  while (param[posParam] && param[posParam] != '&' && posReturnBuf < returnBufSize) {

    if (param[posParam] == '+') returnBuf[posReturnBuf++] = ' ';
    else if (param[posParam] == '%') {
      returnBuf[posReturnBuf++] = (char)(AsciiToHex(param[posParam + 1]) * 0x10 + AsciiToHex(param[posParam + 2]));
      posParam += 2;
    }
    else returnBuf[posReturnBuf++] = param[posParam];

    returnBuf[posReturnBuf] = 0;
    posParam++;
  }
  if (posReturnBuf >= returnBufSize) return false;
  return true;
}

//------------------------------------------
//function that send to Web Client a standard answer (like error or simple success)
void WebCore::SendHTTPResponse(WiFiClient c, int code, ContentType ct, const char* content, uint16_t goToRefererTimeOut) {

  char goToRefererTimeOutA[7];
  uint16_t headerLength = 0;
  uint16_t contentLength = 0;
  char contentLengthA[7];

  //size calculations ---------------------

  //calculate ContentLength
  if (content) contentLength = strlen(content);
  if (goToRefererTimeOut) {
    itoa(goToRefererTimeOut, goToRefererTimeOutA, 10);
    contentLength += 103 + strlen(goToRefererTimeOutA); //103 is size of script function
  }

  //calculate headerlength
  headerLength = 9; // HTTP/1.1
  //200 OK or 404 Not Found ...
  switch (code) {
    case 200: headerLength += 6; break;
    case 400: headerLength += 15; break;
    case 404: headerLength += 13; break;
    case 500: headerLength += 25; break;
    default: return; //other code not accepted yet
  }

  //Content-Type
  if (ct != no) {
    headerLength += 16;
    switch (ct) {
      case html: headerLength += 9; break;
      case json: headerLength += 9; break;
    }
  }

  //rest of header (expires, content-length, etc...)
  headerLength += 53;
  itoa(contentLength, contentLengthA, 10); //convert contentLength to text
  headerLength += strlen(contentLengthA);

  //Sending response ---------------------

  char respBuf[std::max(headerLength, (uint16_t)(103 + strlen(goToRefererTimeOutA))) + 1]; //create buffer of sufficient size

  strcpy_P(respBuf, PSTR("HTTP/1.1 "));
  switch (code) {
    case 200: strcat_P(respBuf, PSTR("200 OK")); break;
    case 400: strcat_P(respBuf, PSTR("400 Bad Request")); break;
    case 404: strcat_P(respBuf, PSTR("404 Not Found")); break;
    case 500: strcat_P(respBuf, PSTR("500 Internal Server Error")); break;
    default: return; //other code not accepted yet
  }

  if (ct != no) {

    strcat_P(respBuf, PSTR("\r\nContent-Type: "));
    switch (ct) {
      case html:
        strcat_P(respBuf, PSTR("text/html"));
        break;
      case json:
        strcat_P(respBuf, PSTR("text/json"));
        break;
    }
  }
  sprintf_P(respBuf, PSTR("%s\r\nExpires: 0\r\nConnection: close\r\nContent-Length: %s\r\n\r\n"), respBuf, contentLengthA);
  c.write((uint8_t *)respBuf, strlen(respBuf));

  if (content && strlen(content)) c.write(content, strlen(content));
  if (goToRefererTimeOut) {
    sprintf_P(respBuf, PSTR("<script>setTimeout(function(){if('referrer' in document)window.location=document.referrer;},%s);</script>"), goToRefererTimeOutA);
    c.write((uint8_t *)respBuf, strlen(respBuf));
  }
}
//------------------------------------------
//function that send to Web Client a standard answer (like error or simple success)
void WebCore::SendHTTPResponse(WiFiClient c, int code, ContentType ct, const __FlashStringHelper* content, uint16_t goToRefererTimeOut) {

  const char* _content = (const char*)content;

  char goToRefererTimeOutA[7];
  uint16_t headerLength = 0;
  uint16_t contentLength = 0;
  char contentLengthA[7];

  //size calculations ---------------------

  //calculate ContentLength
  if (_content) contentLength = strlen_P(_content);
  if (goToRefererTimeOut) {
    itoa(goToRefererTimeOut, goToRefererTimeOutA, 10);
    contentLength += 103 + strlen(goToRefererTimeOutA); //103 is size of script function
  }

  //calculate headerlength
  headerLength = 9; // HTTP/1.1
  //200 OK or 404 Not Found ...
  switch (code) {
    case 200: headerLength += 6; break;
    case 400: headerLength += 15; break;
    case 404: headerLength += 13; break;
    case 500: headerLength += 25; break;
    default: return; //other code not accepted yet
  }

  //Content-Type
  if (ct != no) {
    headerLength += 16;
    switch (ct) {
      case html: headerLength += 9; break;
      case json: headerLength += 9; break;
    }
  }

  //rest of header (expires, content-length, etc...)
  headerLength += 53;
  itoa(contentLength, contentLengthA, 10); //convert contentLength to text
  headerLength += strlen(contentLengthA);

  //Sending response ---------------------

  //NEXT_ESP_VERSION
  //char respBuf[std::max(headerLength, (uint16_t)(103 + strlen(goToRefererTimeOutA))) + 1]; //create buffer of sufficient size
  char respBuf[std::max(std::max(headerLength, (uint16_t)(103 + strlen(goToRefererTimeOutA))), (uint16_t)strlen_P(_content)) + 1]; //create buffer of sufficient size

  strcpy_P(respBuf, PSTR("HTTP/1.1 "));
  switch (code) {
    case 200: strcat_P(respBuf, PSTR("200 OK")); break;
    case 400: strcat_P(respBuf, PSTR("400 Bad Request")); break;
    case 404: strcat_P(respBuf, PSTR("404 Not Found")); break;
    case 500: strcat_P(respBuf, PSTR("500 Internal Server Error")); break;
    default: return; //other code not accepted yet
  }

  if (ct != no) {

    strcat_P(respBuf, PSTR("\r\nContent-Type: "));
    switch (ct) {
      case html:
        strcat_P(respBuf, PSTR("text/html"));
        break;
      case json:
        strcat_P(respBuf, PSTR("text/json"));
        break;
    }
  }
  sprintf_P(respBuf, PSTR("%s\r\nExpires: 0\r\nConnection: close\r\nContent-Length: %s\r\n\r\n"), respBuf, contentLengthA);
  c.write((uint8_t *)respBuf, strlen(respBuf));

  //NEXT_ESP_VERSION
  //if (content && strlen_P(_content)) c.write(_content, strlen_P(_content));
  if (content && strlen_P(_content)) {
    strcpy_P(respBuf, _content);
    c.write((uint8_t *)respBuf, strlen(respBuf));
  }
  if (goToRefererTimeOut) {
    sprintf_P(respBuf, PSTR("<script>setTimeout(function(){if('referrer' in document)window.location=document.referrer;},%s);</script>"), goToRefererTimeOutA);
    c.write((uint8_t *)respBuf, strlen(respBuf));
  }
}



//------------------------------------------
//function that return to client a native Content (fw, JQuery, config or status) (fwdev also in developper mode)
//those are some big PROGMEM char array included (#include)
void WebCore::GetNativeContent(WiFiClient c, NativeContent p) {
  char respBuf[81];
  //send header
  strcpy_P(respBuf, PSTR("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nExpires: 0\r\nContent-Encoding: gzip\r\n\r\n"));
  c.write((uint8_t *)respBuf, strlen(respBuf));

  //send native content
  switch (p) {
    case fw: c.write_P(fwhtmlgz, sizeof(fwhtmlgz)); break;
    case jquery: c.write_P(jquery311minjsgz, sizeof(jquery311minjsgz)); break;
    case config: c.write_P(confightmlgz, sizeof(confightmlgz)); break;
    case status: c.write_P(statushtmlgz, sizeof(statushtmlgz)); break;
#if DEVELOPPER_MODE
    case fwdev: c.write_P(fwdevhtmlgz, sizeof(fwdevhtmlgz)); break;
#endif
  }
}
//------------------------------------------
//Manage HTTP discussion in order to receive a POSTed file
void WebCore::PostFile(WiFiClient c, bool firmwareUpdate) {
  String tmp = "";
  String header = "";
  byte boundarySize = 0;
  int i, j;
  long contentLength = 0;
  String filename = "";


  //read complete header (until empty line is found) -----
  tmp = c.readStringUntil('\n');
  while (tmp != "\r") {
    header += tmp;
    tmp = c.readStringUntil('\n');
  }

  //find boundary then it's size
  i = header.indexOf(F("boundary=")) + 9;
  j = i;
  while (header[j] != '\r' && j < header.length()) j++;
  boundarySize = j - i;


  //find Content-Length and extract it (converted to long)
  i = header.indexOf(F("Content-Length")) + 14;
  if (header[i] == ' ' || header[i] == ':') i++;
  if (header[i] == ' ' || header[i] == ':') i++;
  j = i;
  while (header[j] >= '0' && header[j] <= '9' && j < header.length()) j++;
  tmp = header.substring(i, j);
  contentLength = tmp.toInt();

  //end of page is marked with a CRLF, boundary with 4 minuses and another CRLF
  contentLength -= boundarySize + 2 + 4 + 2;

  //check for empty POST
  if (contentLength <= 0) {
    SendHTTPResponse(c, 400, html, F("Missing File Content!"));
    return;
  }

  //read complete boundary header (until empty line is found) -----
  tmp = c.readStringUntil('\n');
  contentLength -= tmp.length() + 1;
  while (tmp != "\r") {
    header += tmp;
    tmp = c.readStringUntil('\n');
    contentLength -= tmp.length() + 1;
  }

  if (!firmwareUpdate) {

    //look for filename
    i = header.indexOf("filename=") + 9;
    if (header[i] == '\"') i++;
    j = i;
    while (header[j] != '\"' && header[j] != '\r' && j < header.length()) j++;
    filename = header.substring(i, j);

    if (filename[0] != '/') filename = "/" + filename; //if first / is missing

    if (SPIFFS.exists(filename)) {
      SendHTTPResponse(c, 400, html, F("File Already exists"));
      return;
    }

    FSInfo fs_info;
    SPIFFS.info(fs_info);
    if (fs_info.totalBytes <= (fs_info.usedBytes + contentLength + 64)) {
      while (contentLength) {
        c.read(); //waste datas
        contentLength--;
      };
      SendHTTPResponse(c, 400, html, F("Not enough place on destination storage"));
      return;
    }
  }

  //empty String
  header = String();
  tmp = String();

  if (firmwareUpdate) WriteFirmware(contentLength, c);
#if DEVELOPPER_MODE
  else WriteFile(filename, contentLength, c);
#endif
}
//------------------------------------------
//return system Status
void WebCore::GetSystemStatus(WiFiClient c) {

  unsigned long minutes = millis() / 60000;

  String ssJSON(F("{\"b\":\""));
  ssJSON = ssJSON + VERSION + F("\",\"u\":\"") + (byte)(minutes / 1440) + 'd' + (byte)(minutes / 60 % 24) + 'h' + (byte)(minutes % 60) + F("m\",\"f\":") + ESP.getFreeHeap() + '}';

  SendHTTPResponse(c, 200, json, ssJSON.c_str());
}
#if DEVELOPPER_MODE
//------------------------------------------
//Provide a JSON list of file and infos about FS
void WebCore::GetFileList(WiFiClient c) {

  FSInfo fs_info;
  SPIFFS.info(fs_info);
  byte nbElement = 0;

  String flJSON = String(F("{\"t\":")) + fs_info.totalBytes + F(",\"u\":") + fs_info.usedBytes + F(",\"fl\":[");

  Dir dir = SPIFFS.openDir("/");
  bool first = true;

  while (dir.next()) {

    File entry = dir.openFile("r");
    flJSON += String(!first ? "," : "") + F("{\"t\":\"f\",\"n\":\"") + entry.name() + F("\",\"s\":") + entry.size() + F("}");
    entry.close();
    nbElement++;
    first = false;
  }
  flJSON = flJSON + F("],\"nb\":") + nbElement + '}';

  SendHTTPResponse(c, 200, json, flJSON.c_str());
}
//------------------------------------------
//Parse GET request then call handleFileRead to return the file to Web Client
void WebCore::GetFile(WiFiClient c, String &req) {
  req = req.substring(4, req.indexOf(F(" HTTP/1."))); //isolate filename
  req.replace(F("%20"), " "); //only url encoded space is accepted
  ReadFile(req, c);
}
//------------------------------------------
//Parse GET request to remove a file
void WebCore::GetRemoveFile(WiFiClient c, String &req) {

  int i, j;

  //check request structure
  if (req.indexOf('?') == -1 || req.indexOf(F("? ")) != -1 || req.indexOf(F(" HTTP/")) == -1) {
    SendHTTPResponse(c, 400, html, F("Missing parameter"));
    return;
  }
  //keep only the part after '?' and before the final HTTP/1.1
  String getDatas = req.substring(req.indexOf('?') + 1, req.indexOf(F(" HTTP/")));

  char filenameA[33] = {0};
  if (!FindParameterInURLEncodedDatas(getDatas.c_str(), F("n"), filenameA, sizeof(filenameA)) || !filenameA[0]) {
    SendHTTPResponse(c, 400, html, F("FileName is missing"));
    return;
  }

  //check filename passed
  if (filenameA[strlen(filenameA) - 1] == '/') {
    SendHTTPResponse(c, 400, html, F("Can't Remove Folder"));
    return;
  }

  //check that file exists
  if (!SPIFFS.exists(filenameA)) {
    SendHTTPResponse(c, 400, html, F("This file does not exist"));
    return;
  }

  //read complete header (until empty line is found) ---
  String tmp = c.readStringUntil('\n');
  while (tmp != "\r") tmp = c.readStringUntil('\n');

  tmp = String();

  DeleteFile(filename, c);
}
#endif










//------------------------------------------
//Receive and Write Firmware
void WebCore::WriteFirmware(long fileSize, WiFiClient c) {

  Serial.println(F("-Firmware Update Start-"));

  char reqBuf[1024];

  //begin Firmware Update process passing filesize
  if (!Update.begin(fileSize, U_FLASH)) {
    //if begin failed
    SendHTTPResponse(c, 500, html, F("Firmware Update Error : Begin failed"));
    return;
  }

  //while some byte need to be written
  while (fileSize > 0) {

    //define number of byte to read
    size_t len = (fileSize >= sizeof(reqBuf)) ? sizeof(reqBuf) : fileSize;

    //wait for enough byte in c buffer
    int waited = 1000;
    while (c.available() < len && waited--) delay(1);

    //if waited more than 1 second
    if (!waited) {
      //then fail
      SendHTTPResponse(c, 500, html, F("Firmware Update Error : Network Timeout"));
      Update.end();
      return;
    }

    //else we have enough byte in c buffer
    //read
    c.read((uint8_t*)reqBuf, len);
    //write
    Update.write((uint8_t*)reqBuf, len);
    //decrement fileSize
    fileSize -= len;
  }

  //if Update successfully end
  if (Update.end()) {
    //send success message then restart
    SendHTTPResponse(c, 200, html, F("Firmware Successfully Uploaded"), 10000);
    Serial.println(F("-Firmware Update Ended Successfully -> Reboot-"));
    yield();
    ESP.restart();
  }
  else {
    //Update.end failed
    SendHTTPResponse(c, 500, html, F("Firmware Update Error : End failed"));
  }
}
#if DEVELOPPER_MODE
//------------------------------------------
//Return Content-Type based on filename
String WebCore::getContentType(String &filename) {
  /*if (server.hasArg("download")) return F("application/octet-stream");
    else */if (filename.endsWith(".htm")) return F("text/html");
  else if (filename.endsWith(".html")) return F("text/html");
  else if (filename.endsWith(".css")) return F("text/css");
  else if (filename.endsWith(".js")) return F("application/javascript");
  //else if (filename.endsWith(".png")) return F("image/png");
  //else if (filename.endsWith(".gif")) return F("image/gif");
  //else if (filename.endsWith(".jpg")) return F("image/jpeg");
  else if (filename.endsWith(".ico")) return F("image/x-icon");
  //else if (filename.endsWith(".xml")) return F("text/xml");
  //else if (filename.endsWith(".pdf")) return F("application/x-pdf");
  //else if (filename.endsWith(".zip")) return F("application/x-zip");
  else if (filename.endsWith(".gz")) return F("application/x-gzip");
  return F("text/plain");
}
//------------------------------------------
//Open the file then return content to Web Client
void WebCore::ReadFile(String &path, WiFiClient c) {

  char respBuf[1400];

  //if no extension then add .html
  if (path.indexOf('.') == -1) path += F(".html");
  String contentType = getContentType(path);
  size_t sent = 0;
  if (SPIFFS.exists(path) || SPIFFS.exists(path + F(".gz"))) {
    if (SPIFFS.exists(path + F(".gz"))) path += F(".gz");

    File file = SPIFFS.open(path, "r");
    if (file) {

      //prepare and send header
      strcpy_P(respBuf, PSTR("HTTP/1.1 200 OK\r\nContent-Type: "));
      strcat(respBuf, contentType.c_str());
      if (String(file.name()).endsWith(".gz") && contentType != "application/x-gzip" && contentType != "application/octet-stream") {
        strcat_P(respBuf, PSTR("\r\nContent-Encoding: gzip"));
      }
      strcat_P(respBuf, PSTR("\r\n\r\n"));
      c.write((uint8_t *)respBuf, strlen(respBuf));


      int siz = file.size();
      while (siz > 0) {
        size_t len = std::min((int)(sizeof(respBuf) - 1), siz);
        file.read((uint8_t *)respBuf, len);
        sent += c.write((uint8_t *)respBuf, len);
        siz -= len;
      }

      //sent = c.write(file);
      file.close();
    }
  }

  if (sent == 0) {
    SendHTTPResponse(c, 404);
  }
}
//------------------------------------------
//Create file and fill it with Web Client datas
void WebCore::WriteFile(String &path, long fileSize, WiFiClient c) {

  char reqBuf[1024];

  File fsUploadFile = SPIFFS.open(path, "w");

  while (fileSize > 0) {
    size_t len = (fileSize >= sizeof(reqBuf)) ? sizeof(reqBuf) : fileSize;

    //wait for enough byte in c buffer
    int waited = 1000;
    while (c.available() < len && waited--) delay(1);

    //if waited more than 1 second
    if (!waited) {
      //then fail
      SendHTTPResponse(c, 500, html, F("File Upload Error : Network Timeout"));
      fsUploadFile.close();
      SPIFFS.remove(path);
      return;
    }

    c.read((uint8_t*)reqBuf, len);
    fsUploadFile.write((uint8_t*)reqBuf, len);
    fileSize -= len;
  }
  fsUploadFile.close();

  if (fileSize == 0) SendHTTPResponse(c, 200, html, F("File Uploaded successfully"), 1000);
  else {
    SPIFFS.remove(path);
    SendHTTPResponse(c, 500, html, F("File upload FAILED!!!"));
  }
}
//------------------------------------------
//Delete file based on passed filename
void WebCore::DeleteFile(String &filename, WiFiClient c) {

  //can't remove folder for the moment (answer already handle by GetRemoveFile (400 Error))
  if (filename[filename.length() - 1] == '/') return;

  //if file doesn't exists (answer already handle by GetRemoveFile (400 Error))
  if (!SPIFFS.exists(filename)) return;

  if (SPIFFS.remove(filename)) {
    SendHTTPResponse(c, 200, html, F("File Deleted successfully"));
  }
  else {
    SendHTTPResponse(c, 500, html, F("Deletion FAILED!!!"));
  }
}
#endif








//------------------------------------------
/*
  void handleOTAPassword(WiFiClient c, String req) {

  //check request structure
  if (req.indexOf('?') == -1 || req.indexOf(F("? ")) != -1 || req.indexOf(F(" HTTP/")) == -1) {
    strcpy_P(buf, PSTR("HTTP/1.1 400 missing parameter\r\n\r\n"));
    c.write((uint8_t *)buf, strlen(buf));
    return;
  }
  //keep only the part after '?' and before the final HTTP/1.1
  String getDatas = req.substring(req.indexOf('?') + 1, req.indexOf(F(" HTTP/")));

  String OTAPassword = findParameterInURLEncodedDatas(getDatas, F("pass"));

  if (OTAPassword.length() > 24) {
    strcpy_P(buf, PSTR("HTTP/1.1 400 Bad Request\r\n\r\n"));
    c.write((uint8_t *)buf, strlen(buf));
    return;
  }

  strcpy(config.otaPassword, OTAPassword.c_str());

  strcpy_P(buf, PSTR("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nExpires: 0\r\nConnection: close\r\n\r\nOK<script>setTimeout(function(){document.location=\"/config\";},1500);</script>"));
  c.write((uint8_t *)buf, strlen(buf));
  }
*/

