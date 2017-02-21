#ifndef WebCore_h
#define WebCore_h

#include "WirelessDS18B20.h"


#include "data\fw.html.gz.h"
#include "data\config.html.gz.h"
#include "data\status.html.gz.h"
#include "data\jquery-3.1.1.min.js.gz.h"
#if DEVELOPPER_MODE
#include "data\fwdev.html.gz.h"
#endif


class WebCore {
  public:

    typedef enum {
      no,
      html,
      json
    } ContentType;

    typedef enum {
      fw,
      jquery,
      config,
      status
#if DEVELOPPER_MODE
      ,fwdev
#endif
    } NativeContent;

    static byte AsciiToHex(char c); //Utils
    static String FindParameterInURLEncodedDatas(String datas, String parameterToFind);

    static void SendHTTPResponse(WiFiClient c, int code, ContentType ct = no, const char* content = NULL, uint16_t goToRefererTimeOut = 0);
    static void SendHTTPResponse(WiFiClient c, int code, ContentType ct, const __FlashStringHelper* content, uint16_t goToRefererTimeOut = 0);

    static void GetNativeContent(WiFiClient c, NativeContent p);
    static void PostFile(WiFiClient c, bool firmwareUpdate = false);
    static void GetSystemStatus(WiFiClient c);
#if DEVELOPPER_MODE
    static void GetFileList(WiFiClient c);
    static void GetFile(WiFiClient c, String &req);
    static void GetRemoveFile(WiFiClient c, String &req);
#endif



  private:

    static void WriteFirmware(long fileSize, WiFiClient c);
#if DEVELOPPER_MODE
    static String getContentType(String &filename);
    static void ReadFile(String &path, WiFiClient c);
    static void WriteFile(String &path, long fileSize, WiFiClient c);
    static void DeleteFile(String &filename, WiFiClient c);
#endif
};


//void handleOTAPassword(WiFiClient c, String req);
#endif
