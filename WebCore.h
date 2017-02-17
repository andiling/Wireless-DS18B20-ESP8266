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
    static void SendHTTPShortAnswer(WiFiClient c, int code, ContentType ct = no, String content = "", int goToRefererTimeOut = 0);

    static void GetFirmwarePage(WiFiClient c);
    static void GetFileList(WiFiClient c);
    static void GetFile(WiFiClient c, String req);
    static void PostFile(WiFiClient c, bool firmwareUpdate = false);
    static void GetRemoveFile(WiFiClient c, String req);
  private:



    static String getContentType(String filename);
    static void ReadFile(String path, WiFiClient c);
    static void WriteFile(String path, long fileSize, WiFiClient c);
    static void WriteFirmware(long fileSize, WiFiClient c);
    static void DeleteFile(String filename, WiFiClient c);
};


//void handleOTAPassword(WiFiClient c, String req);
#endif
