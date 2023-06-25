/*
  * FileMgr.cpp
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
#include <Int64String.h>
#include "FileMgr.hpp"
#include <StreamUtils.h>
#include "SPI.h"

#define HTML_TRANSFER_BLOCK_SIZE 563
#ifdef ARDUINO_ARCH_ESP32
    #define NumBlocksToBuffer 21
#else // ifdef ARDUINO_ARCH_ESP32
    #define NumBlocksToBuffer 9
#endif // ifdef ARDUINO_ARCH_ESP32

static const uint32_t FileUploadBufferSize = HTML_TRANSFER_BLOCK_SIZE * NumBlocksToBuffer;
SPIClass SdSpi (HSPI);

// -----------------------------------------------------------------------------
///< Start up the driver and put it into a safe mode
c_FileMgr::c_FileMgr () : c_Common (String (F ("FileMgr")))
{}  // c_FileMgr

// -----------------------------------------------------------------------------
///< deallocate any resources and put the output channels into a safe state
c_FileMgr::~c_FileMgr ()
{
    // DEBUG_START;

    // DEBUG_END;
}  // ~c_FileMgr

// -----------------------------------------------------------------------------
///< Start the module
void c_FileMgr::Init ()
{
    // DEBUG_START;

    do  // once
    {
        if (!LittleFS.begin ())
        {
            LOG_WARN (F (" Flash File system did not initialize correctly "));
        }
        else
        {
            LOG_INFO (F ("Flash File system initialized."));
            listDir (LittleFS, String (CN_Slash), 3);
        }

        // DEBUG_V();
        SetSpiIoPins ();
    } while (false);

    // DEBUG_END;
}  // begin

// -----------------------------------------------------------------------------
void c_FileMgr::SetSpiIoPins ()
{
    // DEBUG_START;
    SdCardInstalled = false;

    // DEBUG_V (   String ("  miso: ") + String (miso_pin));
    // DEBUG_V (   String ("  mosi: ") + String (mosi_pin));
    // DEBUG_V (   String (" clock: ") + String (clk_pin));
    // DEBUG_V (   String ("    cs: ") + String (cs_pin));
    #ifdef SD_CARD_POWER
        // DEBUG_V (   String (" Power: ") + String (SdPowerControlPin));
    #endif // def SD_CARD_POWER
    #ifdef SD_CARD_DETECT
        // DEBUG_V (   String ("Detect: ") + String (SdCardDetecPin));
    #endif // def SD_CARD_DETECT

    // SD.end ();

    SdSpi.end ();

    #ifdef SD_CARD_POWER
        // make sure the SD Card is off
        digitalWrite (SdPowerControlPin, HIGH);
        pinMode (SdPowerControlPin, OUTPUT);
        digitalWrite (SdPowerControlPin, HIGH);
        // delay(SD_CARD_OFF_TIME_ms);
        // Turn SD card on
        // digitalWrite(SdPowerControlPin, LOW);
        // delay(SD_CARD_SETTLE_TIME_ms);
    #endif // def SD_CARD_POWER

    #ifdef SD_CARD_DETECT
        pinMode (SdCardDetectPin, INPUT_PULLUP);
    #endif // def SD_CARD_DETECT

    // DEBUG_V ();
        pinMode (   cs_pin,     OUTPUT);
    // DEBUG_V();
        pinMode (   miso_pin,   INPUT);
    // DEBUG_V();
    SdSpi.begin (clk_pin, miso_pin, mosi_pin, cs_pin);
    // DEBUG_V (String ("SdSpi.bus: 0x") + String (uint32_t (SdSpi.bus ()), HEX));

    // Do the common detection activity
    SdCardInstalled = false;
    Poll ();

    // DEBUG_END;
}  // SetSpiIoPins

// -----------------------------------------------------------------------------
void c_FileMgr::Poll ()
{
    do  // once
    {
        if (SdCardInstalled)
        {
            #ifdef SD_CARD_DETECT
                // Does the socket say the card is present
                if (0 != digitalRead (SdCardDetectPin))
                {
                    // DEBUG_V("card is not present.");
                    SdCardInstalled = false;
                    LOG_WARN (F ("SD Card is not present."));
                    #ifdef SD_CARD_POWER
                        // DEBUG_V("turn off the power to the socket");
                        digitalWrite (SdPowerControlPin, HIGH);
                    #endif // def SD_CARD_POWER
                    SdCardInstalled = false;
                }

            #endif // def SD_CARD_DETECT
            break;
        }

        DateTime    now         = RtcMgr.now ();
        uint32_t    deltaTime   = uint32_t (abs (int(now.unixtime ()) - int(LastTryTime.unixtime ())));

        // have we waited long enough
        if (deltaTime < 10)
        {
            // keep waiting
            break;
        }

        // DEBUG_V(String("        now: ") + String(now.unixtime()));
        // DEBUG_V(String("LastTryTime: ") + String(LastTryTime.unixtime()));
        // DEBUG_V(String("  deltaTime: ") + String(deltaTime));

        // remember the time for the last reconnect attempt
        LastTryTime = now;
        // DEBUG_V(String("LastTryTime: ") + String(LastTryTime.unixtime()));

        #ifdef SD_CARD_POWER
            // DEBUG_V("turn off the power to the socket");
            digitalWrite (SdPowerControlPin, HIGH);
        #endif // def SD_CARD_POWER

        #ifdef SD_CARD_DETECT
            // Does the socket say the card is present
            if (0 != digitalRead (SdCardDetectPin))
            {
                // DEBUG_V("no SD card is present. Leave the power off");
                break;
            }

            delay (SD_CARD_OFF_TIME_ms);
        #endif // def SD_CARD_DETECT

        // DEBUG_V("take control of the CS pin");
        SD.end ();
        SdSpi.end ();
        pinMode (cs_pin, OUTPUT);
            digitalWrite (cs_pin, HIGH);

        #ifdef SD_CARD_POWER
            // DEBUG_V("Turn SD card on");
            digitalWrite (SdPowerControlPin, LOW);
            delay (SD_CARD_SETTLE_TIME_ms);
        #endif // def SD_CARD_POWER

        SdSpi.begin (clk_pin, miso_pin, mosi_pin, cs_pin);

        // send 0xff bytes until we get 0xff back
        digitalWrite (cs_pin, LOW);
        // DEBUG_V();
        SdSpi.beginTransaction (SPISettings ());
        // DEBUG_V();
        uint32_t NumConsecutiveFFs = 0;
        for (uint32_t count = 20000;count > 0;--count)
        {
            delay (1);
            if (0xff == SdSpi.transfer (0xff))
            {
                if (++NumConsecutiveFFs > 200)
                {
                    break;
                }
            }
            else
            {
                NumConsecutiveFFs = 0;
            }
        }

        // DEBUG_V();
        SdSpi.endTransaction ();
        // DEBUG_V();
        digitalWrite (cs_pin, HIGH);
        // DEBUG_V();

        if (!SD.begin (cs_pin, SdSpi))
        {
            LOG_WARN (F ("No SD Card Detected"));
            SdCardInstalled = false;
            SD.end ();
        }
        else
        {
            SdCardInstalled = true;
            DescribeSdCardToUser ();
        }
    } while (false);
}  // poll

// -----------------------------------------------------------------------------
void c_FileMgr::AdjustFileName (const String & filename, String & response)
{
    // DEBUG_START;

    if (false == filename.startsWith (CN_Slash))
    {
        // DEBUG_V ();

        response = String (CN_Slash) + filename;
    }
    else
    {
        // DEBUG_V ();
        response = filename;
    }

    // DEBUG_END;
}  // AdjustFileName

// -----------------------------------------------------------------------------
void c_FileMgr::DeleteConfigFile (const String & FileName)
{
    // DEBUG_START;

    String AdjustedFileName;
    AdjustFileName (FileName, AdjustedFileName);
    LittleFS.remove (AdjustedFileName);

    // DEBUG_END;
}  // DeleteConfigFile

// -----------------------------------------------------------------------------
void c_FileMgr::listDir (fs::FS & fs, String dirname, uint8_t levels)
{
    // DEBUG_START;
    do  // once
    {
        if (!dirname.startsWith ("/"))
        {
            dirname = String ("/") + dirname;
        }

        LOG_PORT.println (String (F ("Listing directory: ")) + dirname);

        File CurrentDirectory = fs.open (dirname, CN_r);
        if (!CurrentDirectory)
        {
            LOG_PORT.println (String (CN_stars) + F ("failed to open directory: ") + dirname + CN_stars);
            break;
        }

        if (!CurrentDirectory.isDirectory ())
        {
            LOG_PORT.println (String (F ("Is not a directory: ")) + dirname);
            break;
        }

        File MyFile = CurrentDirectory.openNextFile ();

        while (MyFile)
        {
            if (MyFile.isDirectory ())
            {
                if (levels)
                {
                    String ChildName = dirname + "/" + MyFile.name ();
                    // DEBUG_V (String ("ChildName: ") + ChildName);
                    listDir (fs, ChildName, levels - 1);
                }
            }
            else
            {
                LOG_PORT.println ("'" + String (MyFile.name ()) + "': \t'" + String (MyFile.size ()) + "'");
            }

            MyFile = CurrentDirectory.openNextFile ();
        }
    } while (false);
}  // listDir

// -----------------------------------------------------------------------------
size_t c_FileMgr::GetConfigFileSize (const String & FileName)
{
    // DEBUG_START;

    size_t FileSize = 0;

    String AdjustedFileName;
    AdjustFileName (FileName, AdjustedFileName);

    fs::File file = LittleFS.open (AdjustedFileName.c_str (), "r");
    if (!file)
    {
        LOG_ERROR (String (F ("Could not open file for size check.")));
    }
    else
    {
        FileSize = file.size ();
        file.close ();
    }

    // DEBUG_END;
    return FileSize;
}  // GetConfigFileSize

// -----------------------------------------------------------------------------
bool c_FileMgr::SaveConfigFile (const String & FileName, String & FileData)
{
    // DEBUG_START;

    return SaveConfigFile (FileName, FileData.c_str ());

    // DEBUG_END;
}  // SaveConfigFile

// -----------------------------------------------------------------------------
bool c_FileMgr::SaveConfigFile (const String & FileName, const char * FileData)
{
    // DEBUG_START;

    String AdjustedFileName;
    AdjustFileName (FileName, AdjustedFileName);

    bool Response               = false;
    String CfgFileMessagePrefix = String (CN_Configuration_File_colon) + "'" + AdjustedFileName + "' ";
    // DEBUG_V (FileData);

    fs::File file = LittleFS.open (AdjustedFileName.c_str (), "w");
    if (!file)
    {
        LOG_ERROR (CfgFileMessagePrefix + String (F ("Could not open file for writing.")));
    }
    else
    {
        file.seek (0, SeekSet);
        file.print (FileData);
        file.close ();
        file = LittleFS.open (AdjustedFileName.c_str (), "r");
        LOG_INFO (CfgFileMessagePrefix + String (F ("saved ")) + String (file.size ()) + String (F (" bytes")));
        file.close ();

        Response = true;
    }

    // DEBUG_END;

    return Response;
}  // SaveConfigFile

// -----------------------------------------------------------------------------
bool c_FileMgr::SaveConfigFile (const String & FileName, JsonObject & FileData)
{
    // DEBUG_START;
    bool Response = false;

    String AdjustedFileName;
    AdjustFileName (FileName, AdjustedFileName);

    String CfgFileMessagePrefix = String (CN_Configuration_File_colon) + "'" + AdjustedFileName + "' ";
    fs::File file               = LittleFS.open (AdjustedFileName.c_str (), "w");
    if (!file)
    {
        LOG_ERROR (CfgFileMessagePrefix + String (F ("Could not open file for writing.")));
    }
    else
    {
        file.seek (0, SeekSet);

        size_t bytesWritten = serializeJson (FileData, file);
        file.flush ();
        file.close ();

        if (0 == bytesWritten)
        {
            LOG_ERROR (F ("Failed to write to config file"));
        }
        else
        {
            LOG_INFO (CfgFileMessagePrefix + String (F ("saved ")) + String (bytesWritten) + String (F (" bytes")));
            Response = true;
        }
    }

    // DEBUG_END;

    return Response;
}  // SaveConfigFile

// -----------------------------------------------------------------------------
bool c_FileMgr::ReadConfigFile (const String & FileName, char * FileData, size_t SpaceInTarget)
{
    // DEBUG_START;

    bool GotFileData = false;

    String AdjustedFileName;
    AdjustFileName (FileName, AdjustedFileName);

    // DEBUG_V (String ("File '") + FileName + "' is being opened.");
    fs::File file = LittleFS.open (AdjustedFileName.c_str (), CN_r);
    if (file)
    {
        // DEBUG_V (String ("File '") + FileName + "' is open.");
        size_t bytesToRead = file.size ();
        // DEBUG_V (String ("AdjustedFileName: ") + String (AdjustedFileName));
        // DEBUG_V (String ("     bytesToRead: ") + String (bytesToRead));
        // DEBUG_V (String ("   SpaceInTarget: ") + String (SpaceInTarget));

        if (0 == bytesToRead)
        {
            LOG_ERROR (String (CN_Configuration_File_colon) + "'" + AdjustedFileName + F ("' Source File is empty."));
        }
        else if (bytesToRead < SpaceInTarget)
        {
            file.seek (0, SeekSet);
            if (bytesToRead == file.readBytes (FileData, bytesToRead))
            {
                // DEBUG_V (String ("       bytesRead: ") + String (bytesToRead));
                GotFileData = true;
            }
            else
            {
                // DEBUG_V (String ("AdjustedFileName: ") + String (AdjustedFileName));
                // DEBUG_V (String ("     bytesToRead: ") + String (bytesToRead));
                // DEBUG_V (String ("   SpaceInTarget: ") + String (SpaceInTarget));
                LOG_ERROR ( String (CN_Configuration_File_colon) + "'" + AdjustedFileName + F ("' Failed to read the correct number of bytes from the config file."));
                LOG_ERROR ( F ("."));
            }
        }
        else
        {
            // DEBUG_V (String ("AdjustedFileName: ") + String (AdjustedFileName));
            // DEBUG_V (String ("     bytesToRead: ") + String (bytesToRead));
            // DEBUG_V (String ("   SpaceInTarget: ") + String (SpaceInTarget));
            LOG_ERROR (String (CN_Configuration_File_colon) + "'" + AdjustedFileName + F ("' Destination buffer is too small."));
        }

        file.close ();

        // DEBUG_V (FileData);
    }
    else
    {
        LOG_ERROR (String (CN_Configuration_File_colon) + "'" + AdjustedFileName + F ("' not found."));
    }

    // DEBUG_END;
    return GotFileData;
}  // ReadConfigFile

// -----------------------------------------------------------------------------
bool c_FileMgr::ReadConfigFile (const String & FileName, String & FileData)
{
    // DEBUG_START;

    bool GotFileData = false;

    String AdjustedFileName;
    AdjustFileName (FileName, AdjustedFileName);

    // DEBUG_V (String ("File '") + AdjustedFileName + "' is being opened.");
    fs::File file = LittleFS.open (AdjustedFileName.c_str (), CN_r);
    if (file)
    {
        // DEBUG_V (String ("File '") + AdjustedFileName + "' is open.");
        file.seek (0, SeekSet);
        FileData = file.readString ();
        file.close ();
        GotFileData = true;

        // DEBUG_V (FileData);
    }
    else
    {
        LOG_ERROR (String (CN_Configuration_File_colon) + "'" + AdjustedFileName + F ("' not found."));
    }

    // DEBUG_END;
    return GotFileData;
}  // ReadConfigFile

// -----------------------------------------------------------------------------
bool c_FileMgr::ReadConfigFile (const String & FileName, DynamicJsonDocument & FileData)
{
    // DEBUG_START;
    bool GotFileData = false;

    do  // once
    {
        String AdjustedFileName;
        AdjustFileName (FileName, AdjustedFileName);

        // DEBUG_V (String ("File '") + FileName + "' is being opened.");
        fs::File file = LittleFS.open (AdjustedFileName.c_str (), CN_r);
        if (file)
        {
            // DEBUG_V (String ("File '") + AdjustedFileName + "' is open.");
            size_t bytesToRead = file.size ();
            // DEBUG_V (String ("AdjustedFileName: ") + String (AdjustedFileName));
            // DEBUG_V (String ("     bytesToRead: ") + String (bytesToRead));

            file.seek (0, SeekSet);

            ReadBufferingStream bufferedFileRead
            {file, 128};
            DeserializationError error = deserializeJson (FileData, bufferedFileRead);
            // DEBUG_V ("Error Check");

            file.close ();

            if (error)
            {
                String CfgFileMessagePrefix = String (CN_Configuration_File_colon) + "'" + AdjustedFileName + "' ";
                LOG_ERROR (CfgFileMessagePrefix + String (F ("Deserialzation Error. Error code = ")) + error.c_str ());
                break;
            }

            // DEBUG_V ("Got config data");
            GotFileData = true;
        }
        else
        {
            LOG_ERROR (String (CN_Configuration_File_colon) + "'" + AdjustedFileName + F ("' not found."));
        }
    } while (false);

    // DEBUG_END;
    return GotFileData;
}  // ReadConfigFile

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
c_FileMgr::FileId c_FileMgr::CreateFileHandle ()
{
    // DEBUG_START;

    FileId FileHandle = millis ();

    if (FileList.end () != FileList.find (FileHandle))
    {
        ++FileHandle;
    }

    // DEBUG_V (String ("FileHandle: ") + String (FileHandle));

    // DEBUG_END;

    return FileHandle;
}  // CreateFileHandle

// -----------------------------------------------------------------------------
void c_FileMgr::DeleteSdFile (File dir)
{
    // DEBUG_START;

    File entry;

    while (true)
    {
        if (!SdCardInstalled)
        {
            break;
        }

        entry = dir.openNextFile ();

        if (!entry)
        {
            // no more files in this dir
            break;
        }

        String EntryName = entry.name ();

        if (entry.isDirectory ())
        {
            DeleteSdFile (entry);
            entry.close ();
            // DEBUG_V (String (" Delete Dir: ") + String (EntryName));
            SD.rmdir (EntryName);
        }
        else
        {
            entry.close ();
            // DEBUG_V (String ("Delete File: ") + String (EntryName));
            SD.remove (EntryName);
        }
    }

    // DEBUG_END;
}  // DeleteSdFile

// -----------------------------------------------------------------------------
void c_FileMgr::DeleteSdFile (const String & FileName)
{
    // DEBUG_START;

    String AdjustedFileName;
    AdjustFileName (FileName, AdjustedFileName);

    // DEBUG_V (String ("FileName: ") + String (FileName));

    if (SD.exists (AdjustedFileName))
    {
        File entry = SD.open (AdjustedFileName, "r");
        DeleteSdFile (entry);
        bool isDir = entry.isDirectory ();
        entry.close ();

        // DEBUG_V (String ("Deleting '") + FileName + "'");
        if (isDir)
        {
            // DEBUG_V (String (" Delete Dir: ") + String (AdjustedFileName));
            SD.rmdir (AdjustedFileName);
        }
        else
        {
            // DEBUG_V (String ("Delete File: ") + String (AdjustedFileName));
            SD.remove (AdjustedFileName);
        }
    }

    // DEBUG_END;
}  // DeleteSdFile

// -----------------------------------------------------------------------------
void c_FileMgr::DescribeSdCardToUser ()
{
    // DEBUG_START;

    String BaseMessage = String (CN_stars) + F (" Found SD Card ") + CN_stars;

    uint64_t cardSize = SD.cardSize () / (1024 * 1024);
    LOG_INFO (String (F ("SD Card Size: ")) + int64String (cardSize) + "MB");

    #ifdef PRINT_SD_DETAILS
        File root = SD.open (CN_Slash);
        printDirectory (root, 0);
        root.close ();
    #endif // def PRINT_SD_DETAILS

    // DEBUG_END;
}  // DescribeSdCardToUser

// -----------------------------------------------------------------------------
void c_FileMgr::AddFileToList (File dir, JsonArray & ParentEntry, String & ParentFQN)
{
    // DEBUG_START;

    File entry;

    while (true)
    {
        if (!SdCardInstalled)
        {
            break;
        }

        entry = dir.openNextFile ();

        if (!entry)
        {
            // no more files in this dir
            break;
        }

        String EntryName = entry.name ();
        // String  AdjustedName    = EntryName.substring (ParentFQN.length ());

        // DEBUG_V (String ("   ParentFQN: '") + String (ParentFQN) + "'");
        // DEBUG_V (String ("   EntryName: '") + String (EntryName) + "'");
        // DEBUG_V (String ("AdjustedName: '") + String (AdjustedName) + "'");

        // are we ignoring the dir?
        if (EntryName == F ("System Volume Information"))
        {
            entry.close ();
            continue;
        }

        JsonObject MyEntry = ParentEntry.createNestedObject ();
        MyEntry[CN_name]    = EntryName;
        MyEntry[CN_date]    = entry.getLastWrite ();
        MyEntry[CN_length]  = entry.size ();

        if (entry.isDirectory ())
        {
            // DEBUG_V (entry.name ());
            JsonArray ChildFileEntry = MyEntry.createNestedArray (CN_List);
            AddFileToList (entry, ChildFileEntry, EntryName);
        }

        entry.close ();
    }

    // DEBUG_END;
}  // AddFileToList

// -----------------------------------------------------------------------------
void c_FileMgr::GetStatus (JsonObject & jsonData)
{
    // DEBUG_START;
    jsonData[F ("SdCardPresent")] = SdCardIsInstalled ();
    #ifdef SD_CARD_DETECT
        jsonData[F ("SdCardDetectPin")] = digitalRead (SdCardDetectPin);
    #endif // def SD_CARD_DETECT

    // DEBUG_END;
}  // GetStatus

// -----------------------------------------------------------------------------
void c_FileMgr::GetListOfSdFiles (char * Response, size_t maxLen)
{
    // DEBUG_START;

    DynamicJsonDocument JsonDoc (4096);
    JsonObject  JsonFileList    = JsonDoc.to <JsonObject>();
    JsonArray   FileArray       = JsonFileList.createNestedArray (CN_files);

    do  // once
    {
        if (0 == JsonDoc.capacity ())
        {
            LOG_ERROR (F ("Failed to allocate memory for the GetListOfSdFiles web request."));
            break;
        }

        JsonFileList[F ("SdCardPresent")] = SdCardIsInstalled ();
        #ifdef SD_CARD_DETECT
            JsonFileList[F ("SdCardDetectPin")] = digitalRead (SdCardDetectPin);
        #endif // def SD_CARD_DETECT

        if (false == SdCardIsInstalled ())
        {
            break;
        }

        // DEBUG_V ();

        File dir;
        if (dir = SD.open (CN_Slash, CN_r))
        {
            // DEBUG_V ();
            String AdjustedName = "";
            // DEBUG_V(String("Name: ") + dir.name())
            AddFileToList (dir, FileArray, AdjustedName);
            dir.close ();
        }

        // DEBUG_V ();
    } while (false);

    // DEBUG_V ();
    serializeJson (JsonDoc, Response, maxLen);

    // DEBUG_V (Response);

    // DEBUG_END;
}  // GetListOfFiles

// -----------------------------------------------------------------------------
void c_FileMgr::printDirectory (File dir, int numTabs)
{
    while (true)
    {
        File entry = dir.openNextFile ();

        if (!entry)
        {
            // no more files
            break;
        }

        for (uint8_t i = 0;i < numTabs;i++)
        {
            // LOG_PORT.print ('\t');
        }

        if (entry.isDirectory ())
        {
            // LOG_PORT.println (CN_Slash);
            printDirectory (entry, numTabs + 1);
        }
        else
        {
            LOG_PORT.   print ( entry.name ());
            LOG_PORT.   print ( "\t\t");
            LOG_PORT.println (entry.size (), DEC);
        }

        entry.close ();
    }
}  // printDirectory

// -----------------------------------------------------------------------------
bool c_FileMgr::AppendSdFile (const String & FileName, String & FileData, uint32_t offsetFromEndOfFile)
{
    // DEBUG_START;

    bool response = false;

    do  // once
    {
        if (!SdCardInstalled)
        {
            break;
        }

        String AdjustedFileName;
        AdjustFileName (FileName, AdjustedFileName);

        FileId FileHandle = 0;
        if (false == OpenSdFile (AdjustedFileName, FileMode::FileAppend, FileHandle))
        {
            LOG_ERROR (String (F ("Could not open '")) + FileName + F ("' for writting."));
            break;
        }

        // DEBUG_V ("Set the position relative to the end of the file");

        uint32_t FileSize = FileList[FileHandle].size ();
        FileList[FileHandle].seek (FileSize - offsetFromEndOfFile, SeekSet);

        int WriteCount = WriteSdFile (FileHandle, (byte *)FileData.c_str (), (size_t)FileData.length ());
        // DEBUG_V (String (F ("Wrote '")) + FileName + F ("' ") + String (WriteCount));

        CloseSdFile (FileHandle);

        response = true;
    } while (false);

    // DEBUG_END;
    return response;
}  // AppendSdFile

// -----------------------------------------------------------------------------
bool c_FileMgr::SaveSdFile (const String & FileName, String & FileData)
{
    bool response = false;
    do  // once
    {
        if (!SdCardInstalled)
        {
            break;
        }

        String AdjustedFileName;
        AdjustFileName (FileName, AdjustedFileName);

        // DEBUG_V (String ("AdjustedFileName: ") + AdjustedFileName);

        FileId FileHandle = 0;
        if (false == OpenSdFile (AdjustedFileName, FileMode::FileWrite, FileHandle))
        {
            LOG_ERROR (String (F ("Could not open '")) + FileName + F ("' for writting."));
            break;
        }

        int WriteCount = WriteSdFile (FileHandle, (byte *)FileData.c_str (), (size_t)FileData.length ());

        CloseSdFile (FileHandle);

        if (WriteCount != FileData.length ())
        {
            LOG_ERROR (String (F ("Wrote '")) + FileName + F ("' ") + String (WriteCount));
            break;
        }

        LOG_INFO (String (F ("Wrote '")) + FileName + F ("' ") + String (WriteCount));
        response = true;
    } while (false);

    return response;
}  // SaveSdFile

// -----------------------------------------------------------------------------
bool c_FileMgr::SaveSdFile (const String & FileName, const char * FileData)
{
    bool response = false;
    do  // once
    {
        if (!SdCardInstalled)
        {
            break;
        }

        String AdjustedFileName;
        AdjustFileName (FileName, AdjustedFileName);

        size_t FileDataLength = strlen (FileData);

        // DEBUG_V (String ("AdjustedFileName: ") + AdjustedFileName);

        FileId FileHandle = 0;
        if (false == OpenSdFile (AdjustedFileName, FileMode::FileWrite, FileHandle))
        {
            LOG_ERROR (String (F ("Could not open '")) + FileName + F ("' for writting."));
            break;
        }

        int WriteCount = WriteSdFile (FileHandle, (byte *)FileData, FileDataLength);

        CloseSdFile (FileHandle);

        if (WriteCount != FileDataLength)
        {
            LOG_ERROR (String (F ("Failed to write to SD. Only wrote '")) + FileName + F ("' ") + String (WriteCount));
            break;
        }

        LOG_INFO (String (F ("Wrote '")) + FileName + F ("' ") + String (WriteCount));
        response = true;
    } while (false);

    return response;
}  // SaveSdFile

// -----------------------------------------------------------------------------
bool c_FileMgr::SaveSdFile (const String & FileName, JsonVariant & FileData)
{
    String Temp;
    serializeJson (FileData, Temp);

    return SaveSdFile (FileName, Temp);
}  // SaveSdFile

// -----------------------------------------------------------------------------
bool c_FileMgr::OpenSdFile (const String & FileName, FileMode Mode, FileId & FileHandle)
{
    // DEBUG_START;

    bool FileIsOpen = false;
    FileHandle = 0;
    char ReadWrite[2] =
    {XlateFileMode[Mode], 0};

    do  // once
    {
        if (!SdCardInstalled)
        {
            // DEBUG_V ("No SD card is installed");
            break;
        }

        // DEBUG_V ();

        String AdjustedFileName;
        AdjustFileName (FileName, AdjustedFileName);

        // DEBUG_V (String ("FIle: ") + AdjustedFileName);
        if (false == SD.exists (AdjustedFileName))
        {
            // DEBUG_V ();
            if (FileMode::FileRead == Mode)
            {
                LOG_ERROR (String (F ("Cannot open '")) + FileName + F ("' for reading. File does not exist."));
                break;
            }
            else  // make sure the write path exists
            {
                size_t  StartOfFileName         = AdjustedFileName.lastIndexOf ("/");
                size_t  SegmentStartPosition    = 0; // All paths start with a slash
                size_t  SegmentEndPosition      = 0;
                String  CurrentPathSegment;

                // DEBUG_V (String ("     StartOfFileName: ") + String (StartOfFileName));

                while (SegmentStartPosition < StartOfFileName)
                {
                    SegmentEndPosition  = AdjustedFileName.indexOf ("/", SegmentStartPosition + 1);
                    CurrentPathSegment  = AdjustedFileName.substring (0, SegmentEndPosition);
                    // DEBUG_V (String ("SegmentStartPosition: ") + String (SegmentStartPosition));
                    // DEBUG_V (String ("  SegmentEndPosition: ") + String (SegmentEndPosition));
                    // DEBUG_V (String ("  CurrentPathSegment: ") + String (CurrentPathSegment));
                    SegmentStartPosition = SegmentEndPosition;

                    if (false == SD.exists (CurrentPathSegment))
                    {
                        // DEBUG_V (String ("            creating: ") + String (CurrentPathSegment));
                        int mkdirResponse = SD.mkdir (CurrentPathSegment);
                        if (0 > mkdirResponse)
                        {
                            LOG_ERROR (String (F ("Failed to create '")) + CurrentPathSegment + "'. Error: " + String (mkdirResponse));
                        }
                    }   // create subdir
                }       // test if subdir path exists
            }           // writting file
        }               // file does not exist

        // DEBUG_V ();
        FileHandle = CreateFileHandle ();
        // DEBUG_V (String ("FileHandle: ") + String (FileHandle));

        FileList[FileHandle] = SD.open (AdjustedFileName, ReadWrite);

        // DEBUG_V ();
        if (FileMode::FileWrite == Mode)
        {
            // DEBUG_V ("Write Mode");
            FileList[FileHandle].seek (0, SeekSet);
        }

        // DEBUG_V ();

        FileIsOpen = true;
    } while (false);

    // DEBUG_END;
    return FileIsOpen;
}  // OpenSdFile

// -----------------------------------------------------------------------------
bool c_FileMgr::ReadSdFile (const String & FileName, String & FileData)
{
    // DEBUG_START;

    bool GotFileData    = false;
    FileId FileHandle   = 0;

    if (SdCardInstalled)
    {
        // DEBUG_V (String ("File '") + FileName + "' is being opened.");
        if (true == OpenSdFile (FileName, FileMode::FileRead, FileHandle))
        {
            // DEBUG_V (String ("File '") + FileName + "' is open.");
            FileList[FileHandle].seek (0, SeekSet);
            FileData = FileList[FileHandle].readString ();

            CloseSdFile (FileHandle);
            GotFileData = (0 != FileData.length ());

            // DEBUG_V (FileData);
        }
        else
        {
            CloseSdFile (FileHandle);
            LOG_ERROR (String (F ("SD file: '")) + FileName + String (F ("' not found.")));
        }
    }

    // DEBUG_END;
    return GotFileData;
}  // ReadSdFile

// -----------------------------------------------------------------------------
bool c_FileMgr::ReadSdFile (const String & FileName, DynamicJsonDocument & FileData)
{
    // DEBUG_START;

    bool GotFileData    = false;
    FileId FileHandle   = 0;

    if (SdCardInstalled)
    {
        // DEBUG_V (String ("File '") + FileName + "' is being opened.");
        if (true == OpenSdFile (FileName, FileMode::FileRead, FileHandle))
        {
            // DEBUG_V (String ("File '") + FileName + "' is open.");
            FileList[FileHandle].seek (0, SeekSet);

            ReadBufferingStream bufferedFileRead
            {FileList[FileHandle], 128};
            DeserializationError error = deserializeJson (FileData, bufferedFileRead);

            CloseSdFile (FileHandle);

            if (error)
            {
                String CfgFileMessagePrefix = String (CN_Configuration_File_colon) + "'" + FileName + "' ";
                LOG_ERROR (CfgFileMessagePrefix + String (F ("Deserialzation Error. Error code = ")) + error.c_str ());
                // DEBUG_V (String ("FileContents: ") + FileContents);
            }
            else
            {
                // DEBUG_V ("Got File Data");
                GotFileData = true;
            }
        }
        else
        {
            LOG_ERROR (String (F ("SD file: '")) + FileName + String (F ("' not found.")));
        }
    }

    // DEBUG_END;
    return GotFileData;
}  // ReadSdFile

// -----------------------------------------------------------------------------
size_t c_FileMgr::ReadSdFile (const FileId & FileHandle, byte * FileData, size_t NumBytesToRead, size_t StartingPosition)
{
    // DEBUG_START;

    size_t response = 0;

    // DEBUG_V (String ("      FileHandle: ") + String (FileHandle));
    // DEBUG_V (String ("  NumBytesToRead: ") + String (NumBytesToRead));
    // DEBUG_V (String ("StartingPosition: ") + String (StartingPosition));

    if (SdCardInstalled)
    {
        if (FileList.end () != FileList.find (FileHandle))
        {
            FileList[FileHandle].seek (StartingPosition, SeekSet);
            response = ReadSdFile (FileHandle, FileData, NumBytesToRead);
        }
        else
        {
            LOG_ERROR (String (F ("ReadSdFile::ERROR::Invalid File Handle: ")) + String (FileHandle));
        }
    }

    // DEBUG_END;
    return response;
}  // ReadSdFile

// -----------------------------------------------------------------------------
size_t c_FileMgr::ReadSdFile (const FileId & FileHandle, byte * FileData, size_t NumBytesToRead)
{
    // DEBUG_START;

    // DEBUG_V (String ("    FileHandle: ") + String (FileHandle));
    // DEBUG_V (String ("NumBytesToRead: ") + String (NumBytesToRead));

    size_t response = 0;
    if (SdCardInstalled)
    {
        if (FileList.end () != FileList.find (FileHandle))
        {
            response = FileList[FileHandle].read (FileData, NumBytesToRead);
        }
        else
        {
            LOG_ERROR (String (F ("ReadSdFile::ERROR::Invalid File Handle: ")) + String (FileHandle));
        }
    }

    // DEBUG_END;
    return response;
}  // ReadSdFile

// -----------------------------------------------------------------------------
bool c_FileMgr::ReadSdFile (const String & FileName, byte * FileData, size_t SpaceInTarget)
{
    // DEBUG_START;

    bool GotFileData = false;

    if (SdCardInstalled)
    {
        String AdjustedFileName;
        AdjustFileName (FileName, AdjustedFileName);

        FileId FileHandle = 0;

        // DEBUG_V (String ("File '") + FileName + "' is being opened.");
        if (true == OpenSdFile (FileName, FileMode::FileRead, FileHandle))
        {
            size_t bytesToRead = FileList[FileHandle].size ();
            // DEBUG_V (String ("SpaceInTarget: ") + String (SpaceInTarget));
            // DEBUG_V (String ("  bytesToRead: ") + String (bytesToRead));

            if (bytesToRead < SpaceInTarget)
            {
                // DEBUG_V (String ("File '") + FileName + "' is open.");
                FileList[FileHandle].seek (0, SeekSet);
                ReadBufferingStream bufferedFile
                {FileList[FileHandle], 128};

                size_t bytesRead = 0;
                if (0 != (bytesRead = bufferedFile.readBytes ((char *)FileData, bytesToRead)))
                {
                    // DEBUG_V (String ("    bytesRead: ") + String (bytesRead));
                    // DEBUG_V (String ("FileData: ") + String ((char*)FileData));
                    GotFileData = true;
                }
                else
                {
                    // DEBUG_V (String ("AdjustedFileName: ") + String (AdjustedFileName));
                    // DEBUG_V (String ("     bytesToRead: ") + String (bytesToRead));
                    // DEBUG_V (String ("   SpaceInTarget: ") + String (SpaceInTarget));
                    LOG_ERROR ( String ("'") + AdjustedFileName + F ("' Failed to read the correct number of bytes from the config file."));
                    LOG_ERROR ( F ("."));
                }
            }
            else
            {
                // DEBUG_V (String ("AdjustedFileName: ") + String (AdjustedFileName));
                // DEBUG_V (String ("     bytesToRead: ") + String (bytesToRead));
                // DEBUG_V (String ("   SpaceInTarget: ") + String (SpaceInTarget));
                LOG_ERROR (String ("'") + AdjustedFileName + F ("' Destination buffer is too small."));
            }

            CloseSdFile (FileHandle);
        }
        else
        {
            LOG_ERROR (String (F ("SD file: '")) + FileName + String (F ("' not found.")));
        }
    }

    // DEBUG_END;
    return GotFileData;
}  // ReadSdFile

// -----------------------------------------------------------------------------
void c_FileMgr::CloseSdFile (const FileId & FileHandle)
{
    // DEBUG_START;

    if (FileList.end () != FileList.find (FileHandle))
    {
        FileList[FileHandle].close ();
        FileList.erase (FileHandle);
    }
    else
    {
        LOG_ERROR (String (F ("CloseSdFile::ERROR::Invalid File Handle: ")) + String (FileHandle));
    }

    // DEBUG_END;
}  // CloseSdFile

// -----------------------------------------------------------------------------
bool c_FileMgr::WriteSdFile (const String & FileName, String & FileData)
{
    // DEBUG_START;

    bool response = false;

    do  // once
    {
        if (!SdCardInstalled)
        {
            break;
        }

        String AdjustedFileName;
        AdjustFileName (FileName, AdjustedFileName);

        FileId FileHandle = 0;
        if (false == OpenSdFile (AdjustedFileName, FileMode::FileWrite, FileHandle))
        {
            LOG_PORT.println (String (F ("Could not open '")) + FileName + F ("' for writting."));
            SdCardInstalled = false;
            break;
        }

        // DEBUG_V ("Set the position relative to the end of the file");

        // uint32_t FileSize = FileList[FileHandle].size ();
        FileList[FileHandle].seek (0, SeekSet);

        int bytesWritten = WriteSdFile (FileHandle, (byte *)FileData.c_str (), (size_t)FileData.length ());
        // DEBUG_V (FileName + F (": saved ") + String (bytesWritten) + F (" bytes to SD card."));

        CloseSdFile (FileHandle);

        if (bytesWritten != (size_t)FileData.length ())
        {
            LOG_PORT.println (AdjustedFileName + F ("Failed to write the expected number of bytes"));
            break;
        }

        response = true;
    } while (false);

    // DEBUG_END;
    return response;
}  // WriteSdFile

// -----------------------------------------------------------------------------
bool c_FileMgr::WriteSdFile (const String & FileName, JsonObject & FileData)
{
    // DEBUG_START;

    bool response = false;

    do  // once
    {
        if (!SdCardInstalled)
        {
            break;
        }

        String AdjustedFileName;
        AdjustFileName (FileName, AdjustedFileName);

        FileId FileHandle = 0;
        if (false == OpenSdFile (AdjustedFileName, FileMode::FileWrite, FileHandle))
        {
            LOG_PORT.println (String (F ("Could not open '")) + FileName + F ("' for writting."));
            break;
        }

        FileList[FileHandle].seek (0, SeekSet);

        size_t bytesWritten = serializeJson (FileData, FileList[FileHandle]);

        CloseSdFile (FileHandle);

        if (0 == bytesWritten)
        {
            LOG_PORT.println (FileName + F (": Failed to write to file."));
            SdCardInstalled = false;
            break;
        }

        // DEBUG_V (FileName + F (": saved ") + String (bytesWritten) + F (" bytes to SD card."));
        response = true;
    } while (false);

    // DEBUG_END;
    return response;
}  // WriteSdFile

// -----------------------------------------------------------------------------
size_t c_FileMgr::WriteSdFile (const FileId & FileHandle, byte * FileData, size_t NumBytesToWrite)
{
    size_t bytesWritten = 0;
    if (SdCardInstalled)
    {
        if (FileList.end () != FileList.find (FileHandle))
        {
            bytesWritten = FileList[FileHandle].write (FileData, NumBytesToWrite);
            if (bytesWritten == NumBytesToWrite)
            {
                // DEBUG_V (String (F ("saved ")) + String (bytesWritten) + F (" bytes to SD card."));
            }
            else
            {
                LOG_PORT.println (String (F ("WriteSdFile::ERROR::Could not write file to SD")));
                SdCardInstalled = false;
            }
        }
        else
        {
            LOG_PORT.println (String (F ("WriteSdFile::ERROR::Invalid File Handle: ")) + String (FileHandle));
        }
    }

    return bytesWritten;
}  // WriteSdFile

// -----------------------------------------------------------------------------
size_t c_FileMgr::WriteSdFile (const FileId & FileHandle, byte * FileData, size_t NumBytesToWrite, size_t StartingPosition)
{
    size_t response = 0;
    if (FileList.end () != FileList.find (FileHandle))
    {
        FileList[FileHandle].seek (StartingPosition, SeekSet);
        response = WriteSdFile (FileHandle, FileData, NumBytesToWrite);
    }
    else
    {
        LOG_PORT.println (String (F ("WriteSdFile::ERROR::Invalid File Handle: ")) + String (FileHandle));
    }

    return response;
}  // WriteSdFile

// -----------------------------------------------------------------------------
bool c_FileMgr::GetSdFileExists (const String & FileName) {return SD.exists (FileName);}  // GetSdFileExists

// -----------------------------------------------------------------------------
size_t c_FileMgr::GetSdFileSize (const FileId & FileHandle)
{
    size_t response = 0;
    if (FileList.end () != FileList.find (FileHandle))
    {
        response = FileList[FileHandle].size ();
    }
    else
    {
        LOG_ERROR (String (F ("GetSdFileSize::ERROR::Invalid File Handle: ")) + String (FileHandle));
    }

    return response;
}  // GetSdFileSize

// -----------------------------------------------------------------------------
void c_FileMgr::handleFileUpload (const String  & filename,
                                  size_t        index,
                                  uint8_t       * data,
                                  size_t        len,
                                  bool final)
{
    // DEBUG_START;
    if (0 == index)
    {
        handleFileUploadNewFile (filename);
    }

    if ((0 != len) && (0 != fsUploadFileName.length ()))
    {
        // Write data
        // DEBUG_V ("UploadWrite: " + String (len) + String (" bytes"));
        WriteSdFile (fsUploadFile, data, len);
        // LOG_PORT.print (String ("Writting bytes: ") + String (index) + '\r');
    }

    if ((true == final) && (0 != fsUploadFileName.length ()))
    {
        // DEBUG_V ("UploadEnd: " + String (index + len) + String (" bytes"));
        LOG_INFO (String (F ("Upload File: '")) + fsUploadFileName + String (F ("' Done")));

        FileMgr.CloseSdFile (fsUploadFile);
        fsUploadFileName = "";
    }

    // DEBUG_END;
}  // handleFileUpload

// -----------------------------------------------------------------------------
void c_FileMgr::handleFileUploadNewFile (const String & filename)
{
    // DEBUG_START;

    // save the filename
    // DEBUG_V ("UploadStart: " + filename);

    // are we terminating the previous download?
    if (0 != fsUploadFileName.length ())
    {
        LOG_ERROR (String (F ("Aborting Previous File Upload For: '")) + fsUploadFileName + String (F ("'")));
        FileMgr.CloseSdFile (fsUploadFile);
        fsUploadFileName = "";
    }

    // Set up to receive a file
    fsUploadFileName = filename;

    LOG_INFO (String (F ("Upload File: '")) + fsUploadFileName + String (F ("' Started")));

    FileMgr.DeleteSdFile (fsUploadFileName);

    // Open the file for writing
    FileMgr.OpenSdFile (fsUploadFileName, FileMode::FileWrite, fsUploadFile);

    // DEBUG_END;
}  // handleFileUploadNewFile

// create a global instance of the File Manager
c_FileMgr FileMgr;
