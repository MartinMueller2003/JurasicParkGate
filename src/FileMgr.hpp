#pragma once
/*
  * FileMgr.hpp - SD File Management class
  *
  *  This program is provided free for you to use in any way that you wish,
  *  subject to the laws and regulations where you are using it.  Due diligence
  *  is strongly suggested before using this code.  Please give credit where due.
  *
  *  The Author makes no warranty of any kind, express or implied, with regard
  *  to this program or the documentation contained in this document.  The
  *  Author shall not be liable in any event for incidental or consequential
  *  damages in connection with, or arising out of, the furnishing, performance
  *  or use of these programs.
  *
  */

#include "JurasicParkGate.hpp"
#include <LittleFS.h>
#include <SD.h>
#include <FS.h>
#include <map>
#include "RtcMgr.hpp"

class c_FileMgr : public c_Common
{
public:
    c_FileMgr ();
    virtual~c_FileMgr ();

    // Singleton
    c_FileMgr (c_FileMgr const & copy);
    c_FileMgr &operator=(c_FileMgr const & copy);

    void    Init             (void);
    void    Poll             (void);
    void    GetStatus        (JsonObject & jsonData);
    void    handleFileUpload (const String & filename, size_t index, uint8_t * data, size_t len, bool final);

    typedef enum
    {
        FileRead = 0,
        FileWrite,
        FileAppend
    } FileMode;
    typedef uint32_t FileId;

    void DeleteConfigFile (const String & FileName);

    size_t GetConfigFileSize (const String & FileName);

    bool    SaveConfigFile   (const String & FileName,   const char * FileData);
    bool    SaveConfigFile   (const String & FileName,   String & FileData);
    bool    SaveConfigFile   (const String & FileName,   JsonObject & FileData);

    bool    ReadConfigFile   (const String & FileName,   char * FileData, size_t maxLen);
    bool    ReadConfigFile   (const String & FileName,   String & FileData);
    bool    ReadConfigFile   (const String & FileName,   DynamicJsonDocument & FileData);

    bool    SdCardIsInstalled () {return SdCardInstalled;}
    FileId  CreateFileHandle  ();
    void    DeleteSdFile      (const String & FileName);
    void    DeleteSdFile      (File dir);
    bool    SaveSdFile        (const String & FileName,   String & FileData);
    bool    SaveSdFile        (const String & FileName,   const char * FileData);
    bool    SaveSdFile        (const String & FileName,   JsonVariant & FileData);
    bool    AppendSdFile      (const String & FileName,   String & FileData, uint32_t offsetFromEndOfFile = 0);
    bool    OpenSdFile        (const String & FileName,   FileMode Mode, FileId & FileHandle);
    size_t  ReadSdFile        (const FileId & FileHandle, byte * FileData, size_t NumBytesToRead);
    size_t  ReadSdFile        (const FileId & FileHandle, byte * FileData, size_t NumBytesToRead,  size_t StartingPosition);
    bool    ReadSdFile        (const String & FileName,   DynamicJsonDocument & FileData);
    bool    ReadSdFile        (const String & FileName,   String & FileData);
    bool    ReadSdFile        (const String & FileName,   byte * FileData, size_t NumBytesToRead);
    bool    WriteSdFile       (const String & FileHandle, JsonObject & FileData);
    bool    WriteSdFile       (const String & FileHandle, String & FileData);
    size_t  WriteSdFile       (const FileId & FileHandle, byte * FileData, size_t NumBytesToWrite);
    size_t  WriteSdFile       (const FileId & FileHandle, byte * FileData, size_t NumBytesToWrite, size_t StartingPosition);
    void    CloseSdFile       (const FileId & FileHandle);
    void    GetListOfSdFiles  (char * Response, size_t maxLen);
    void    AddFileToList     (File dir, JsonArray & ParentEntry, String & ParentFQN);
    size_t  GetSdFileSize     (const FileId & FileHandle);
    bool    GetSdFileExists   (const String & FileName);

private:

    void    SetSpiIoPins            ();
    void    listDir                 (fs::FS & fs, String dirname, uint8_t levels);
    void    DescribeSdCardToUser    ();
    void    handleFileUploadNewFile (const String & filename);
    void    printDirectory          (File dir, int numTabs);
    void    AdjustFileName          (const String & FileName, String & response);
    DateTime    LastTryTime;

    bool        SdCardInstalled = false;
    uint8_t     miso_pin        = SD_CARD_MISO_PIN;
    uint8_t     mosi_pin        = SD_CARD_MOSI_PIN;
    uint8_t     clk_pin         = SD_CARD_CLK_PIN;
    uint8_t     cs_pin          = SD_CARD_CS_PIN;
    #ifdef SD_CARD_POWER
        uint8_t SdPowerControlPin = SD_CARD_POWER;
        #define SD_CARD_OFF_TIME_ms     10
        #define SD_CARD_SETTLE_TIME_ms  1

    #endif // def SD_CARD_POWER
    #ifdef SD_CARD_DETECT
        uint8_t             SdCardDetectPin = SD_CARD_DETECT;
    #endif // def SD_CARD_DETECT

    FileId                  fsUploadFile;
    String                  fsUploadFileName;
    bool                    fsUploadFileSavedIsEnabled  = false;
    char                    XlateFileMode[3]            = {'r', 'w', 'w'};

    std::map <FileId, File> FileList;

protected:
};  // c_FileMgr

extern c_FileMgr FileMgr;
