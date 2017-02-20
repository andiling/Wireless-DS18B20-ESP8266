#ifndef WebCore_h
#define WebCore_h

class WebCore {
  public:

    typedef enum {
      no,
      html,
      json
    } ContentType;

    static byte AsciiToHex(char c); //Utils
    static String FindParameterInURLEncodedDatas(String datas, String parameterToFind);

    static void SendHTTPResponse(WiFiClient c, int code, ContentType ct = no, const char* content = NULL, uint16_t goToRefererTimeOut = 0);
    static void SendHTTPResponse(WiFiClient c, int code, ContentType ct, const __FlashStringHelper* content, uint16_t goToRefererTimeOut = 0);

    static void GetFirmwarePage(WiFiClient c);
    static void GetFileList(WiFiClient c);
    static void GetFile(WiFiClient c, String &req);
    static void PostFile(WiFiClient c, bool firmwareUpdate = false);
    static void GetRemoveFile(WiFiClient c, String &req);
    static void GetSystemStatus(WiFiClient c);
  private:



    static String getContentType(String &filename);
    static void ReadFile(String &path, WiFiClient c);
    static void WriteFile(String &path, long fileSize, WiFiClient c);
    static void WriteFirmware(long fileSize, WiFiClient c);
    static void DeleteFile(String &filename, WiFiClient c);
};


//void handleOTAPassword(WiFiClient c, String req);
#endif
