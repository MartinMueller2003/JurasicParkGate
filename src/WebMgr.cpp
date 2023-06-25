/*
  * WebMgr.cpp - WEB UI Management class
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
#include <AsyncTCP.h>
#include <AsyncUDP.h>
#include "WiFiMgr.hpp"
#include "WebMgr.hpp"
#include "FileMgr.hpp"
#include <Int64String.h>
#include <time.h>
#include <sys/time.h>

const uint8_t HTTP_PORT = 80;                   ///< Default web server port

static EFUpdate efupdate;                       /// EFU Update Handler
static AsyncWebServer   webServer (HTTP_PORT);  // Web Server
static AsyncWebSocket   webSocket ("/ws");      // Web Socket Plugin

// -----------------------------------------------------------------------------
void PrettyPrint (JsonArray & jsonStuff, String Name)
{
    // DEBUG_V ("---------------------------------------------");
    LOG_PORT.println (String (F ("---- Pretty Print: '")) + Name + "'");
    serializeJson (jsonStuff, LOG_PORT);
    LOG_PORT.println ("");
    // DEBUG_V ("---------------------------------------------");
}  // PrettyPrint

// -----------------------------------------------------------------------------
void PrettyPrint (JsonObject & jsonStuff, String Name)
{
    // DEBUG_V ("---------------------------------------------");
    LOG_PORT.println (String (F ("---- Pretty Print: '")) + Name + "'");
    serializeJson (jsonStuff, LOG_PORT);
    LOG_PORT.println ("");
    // DEBUG_V ("---------------------------------------------");
}  // PrettyPrint

// -----------------------------------------------------------------------------
///< Start up the driver and put it into a safe mode
c_WebMgr::c_WebMgr () : c_Common (String (F ("WebMgr")))
{
    // this gets called pre-setup so there is nothing we can do here.
}  // c_WebMgr

// -----------------------------------------------------------------------------
///< deallocate any resources and put the output channels into a safe state
c_WebMgr::~c_WebMgr ()
{
    // DEBUG_START;

    // DEBUG_END;
}  // ~c_WebMgr

// -----------------------------------------------------------------------------
///< Start the module
void c_WebMgr::Init ()
{
    // DEBUG_START;

    if (WiFiMgr.IsWiFiConnected ())
    {
        init ();
    }

    // DEBUG_END;
}  // begin

// -----------------------------------------------------------------------------
// Configure and start the web server
void c_WebMgr::NetworkStateChanged (bool NewNetworkState)
{
    // DEBUG_START;

    if (true == NewNetworkState)
    {
        init ();
    }

    // DEBUG_END;
}  // NetworkStateChanged

bool filterApi (AsyncWebServerRequest * request)
{
    bool response = (-1 == request->url ().indexOf ("/api/"));

    // DEBUG_V(String("     url: ") + request->url());
    // DEBUG_V(String(" indexOf: ") + String(request->url().indexOf("/api/")));
    // DEBUG_V(String("response: ") + String(response));

    return response;
}

// -----------------------------------------------------------------------------
// Configure and start the web server
void c_WebMgr::init ()
{
    // DEBUG_START;
    // Add header for SVG plot support?
    DefaultHeaders::Instance ().addHeader (F ("Access-Control-Allow-Origin"),  "*");
    DefaultHeaders::Instance ().addHeader (F ("Access-Control-Allow-Headers"), "Content-Type, Content-Range, Content-Disposition, Content-Description, cache-control, x-requested-with");
    DefaultHeaders::Instance ().addHeader (F ("Access-Control-Allow-Methods"), "GET, PUT, POST, OPTIONS");

    // Setup WebSockets
    webSocket.onEvent (
        [this] (AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t * data, size_t len)
        {
            this->onWsEvent (server, client, type, arg, data, len);
        });
    webServer.addHandler (& webSocket);

    // Heap status handler
    webServer.on (
        "/heap",
        HTTP_GET,
        [] (AsyncWebServerRequest * request)
        {
            request->send (200, CN_textSLASHplain, String (ESP.getFreeHeap ()).c_str ());
        });

    // JSON Config Handler
    webServer.on (
        "/conf",
        HTTP_GET,
        [this] (AsyncWebServerRequest * request)
        {
            this->GetConfiguration ();
            request->send (200, "text/json", WebSocketFrameCollectionBuffer);
        });

    // Firmware upload handler - only in station mode
    webServer.on (
        "/updatefw",
        HTTP_POST,
        [] (AsyncWebServerRequest * request)
        {
            webSocket.textAll ("X6");
        },
        [] (AsyncWebServerRequest * request, String filename, size_t index, uint8_t * data, size_t len, bool final) {WebMgr.FirmwareUpload (request, filename, index, data, len,  final);}).setFilter (
        ON_STA_FILTER);

    // Static Handler
    webServer.serveStatic (CN_Slash, LittleFS, "/www/").setDefaultFile ("index.html").setFilter (filterApi);

    webServer.on (
        "/download",
        HTTP_GET,
        [] (AsyncWebServerRequest * request)
        {
            String filename = request->url ().substring (String ("/download").length ());
            // DEBUG_V (String ("filename: ") + String (filename));

            AsyncWebServerResponse * response = new AsyncFileResponse (SD, filename, "application/octet-stream", true);
            request->send (response);
            // DEBUG_V ("Send File Done");
        });

    // if the client posts to the upload page
    webServer.on (
        "/upload",
        HTTP_POST | HTTP_PUT | HTTP_OPTIONS,
        [] (AsyncWebServerRequest * request)
        {
            // DEBUG_V ("Got upload post request");
            if (true == FileMgr.SdCardIsInstalled ())
            {
                // Send status 200 (OK) to tell the client we are ready to receive
                request->send (200);
            }
            else
            {
                request->send (404, CN_textSLASHplain, CN_PageNotFound);
            }
        },

        [this] (AsyncWebServerRequest * request, String filename, size_t index, uint8_t * data, size_t len, bool final)
        {
            // DEBUG_V ("Got process FIle request");
            // DEBUG_V (String ("Got process File request: name: ")  + filename);
            // DEBUG_V (String ("Got process File request: index: ") + String (index));
            // DEBUG_V (String ("Got process File request: len:   ") + String (len));
            // DEBUG_V (String ("Got process File request: final: ") + String (final));
            if (true == FileMgr.SdCardIsInstalled ())
            {
                this->handleFileUpload (request, filename, index, data, len, final);  // Receive and save the file
            }
            else
            {
                request->send (404, CN_textSLASHplain, CN_PageNotFound);
            }
        },

        [this] (AsyncWebServerRequest * request, uint8_t * data, size_t len, size_t index, size_t total)
        {
            // DEBUG_V (String ("Got process Body request: index: ") + String (index));
            // DEBUG_V (String ("Got process Body request: len:   ") + String (len));
            // DEBUG_V (String ("Got process Body request: total: ") + String (total));
            request->send (404, CN_textSLASHplain, CN_PageNotFound);
        });

    webServer.onNotFound (
        [] (AsyncWebServerRequest * request)
        {
            // DEBUG_V ("onNotFound");
            request->send (404, CN_textSLASHplain, CN_PageNotFound);
        });

    webServer.begin ();
    LOG_INFO (String (F ("Web Server started on port ")) + HTTP_PORT);

    // DEBUG_END;
}

// -----------------------------------------------------------------------------
void c_WebMgr::handleFileUpload (AsyncWebServerRequest  * request,
                                 String                 filename,
                                 size_t                 index,
                                 uint8_t                * data,
                                 size_t                 len,
                                 bool final)
{
    // DEBUG_START;

    FileMgr.handleFileUpload (filename, index, data, len, final);

    // DEBUG_END;
}  // handleFileUpload

// -----------------------------------------------------------------------------
/*
  *     Gather config data from the various config sources and send it to the web page.
  */
void c_WebMgr::GetConfiguration ()
{
    // DEBUG_START;

    CfgMgr.GetSystemConfig (WebSocketFrameCollectionBuffer, sizeof (WebSocketFrameCollectionBuffer));

    // DEBUG_END;
}  // GetConfiguration

// -----------------------------------------------------------------------------
/// Handle Web Service events
/** Handles all Web Service event types and performs initial parsing of Web Service event data.
  * Text messages that start with 'X' are treated as "Simple" format messages, else they're parsed as JSON.
  */
void c_WebMgr::onWsEvent (AsyncWebSocket        * server,
                          AsyncWebSocketClient  * client,
                          AwsEventType          type,
                          void                  * arg,
                          uint8_t               * data,
                          size_t                len)
{
    // DEBUG_START;
    // DEBUG_V (CN_Heap_colon + ESP.getFreeHeap ());

    switch (type)
    {
        case WS_EVT_DATA:
        {
            // DEBUG_V ("");

            AwsFrameInfo * MessageInfo = static_cast <AwsFrameInfo *> (arg);
            // DEBUG_V (String (F ("               len: ")) + len);
            // DEBUG_V (String (F ("MessageInfo->index: ")) + int64String (MessageInfo->index));
            // DEBUG_V (String (F ("  MessageInfo->len: ")) + int64String (MessageInfo->len));
            // DEBUG_V (String (F ("MessageInfo->final: ")) + String (MessageInfo->final));

            // only process text messages
            if (MessageInfo->opcode != WS_TEXT)
            {
                LOG_INFO (F ("Ignore binary message"));
                break;
            }

            // DEBUG_V ("");

            // is this a message start?
            if (0 == MessageInfo->index)
            {
                // clear the string we are building
                memset (WebSocketFrameCollectionBuffer, 0x0, sizeof (WebSocketFrameCollectionBuffer));
                // DEBUG_V ("");
            }

            // DEBUG_V ("");

            // will the message fit into our buffer?
            if (WebSocketFrameCollectionBufferSize < (MessageInfo->index + len))
            {
                // message wont fit. Dont save any of it
                LOG_ERROR (String (F ("WebIO::onWsEvent(): Incoming message is TOO long.")));
                break;
            }

            // add the current data to the aggregate message
            memcpy (& WebSocketFrameCollectionBuffer[MessageInfo->index], data, len);

            // is the message complete?
            if ((MessageInfo->index + len) != MessageInfo->len)
            {
                // DEBUG_V ("The message is not yet complete");
                break;
            }

            // DEBUG_V ("");

            // did we get the final part of the message?
            if (!MessageInfo->final)
            {
                // DEBUG_V ("message is not terminated yet");
                break;
            }

            // DEBUG_V (WebSocketFrameCollectionBuffer);
            // message is all here. Process it

            if (WebSocketFrameCollectionBuffer[0] == 'X')
            {
                // DEBUG_V ("");
                ProcessXseriesRequests (client);
                break;
            }

            if (WebSocketFrameCollectionBuffer[0] == 'V')
            {
                // DEBUG_V ("");
                ProcessVseriesRequests (client);
                break;
            }

            // convert the input data into a json structure (use json read only mode)
            size_t docSize = strlen ((const char *)(& WebSocketFrameCollectionBuffer[0])) * 3;
            DynamicJsonDocument webJsonDoc (docSize);
            DeserializationError error = deserializeJson (webJsonDoc, (const char *)(& WebSocketFrameCollectionBuffer[0]));

            // DEBUG_V ("");
            if (error)
            {
                LOG_ERROR ( String (F ("WebIO::onWsEvent(): Parse Error: ")) + error.c_str ());
                LOG_ERROR ( WebSocketFrameCollectionBuffer);
                break;
            }

            // DEBUG_V ("");

            ProcessReceivedJsonMessage (webJsonDoc, client);
            // DEBUG_V ("");

            break;
        }  // case WS_EVT_DATA:

        case WS_EVT_CONNECT:
        {
            LOG_INFO (String (F ("Web Socket Connect - ")) + client->id ());
            break;
        }  // case WS_EVT_CONNECT:

        case WS_EVT_DISCONNECT:
        {
            LOG_INFO (String (F ("Web Socket Disconnect - ")) + client->id ());
            break;
        }  // case WS_EVT_DISCONNECT:

        case WS_EVT_PONG:
        {
            LOG_INFO (F ("Web Socket PONG"));
            break;
        }  // case WS_EVT_PONG:

        case WS_EVT_ERROR:
        default:
        {
            LOG_ERROR (F ("Web Socket Generic Failure"));
            break;
        }
    }  // end switch (type)

    // DEBUG_V (CN_Heap_colon + ESP.getFreeHeap());

    // DEBUG_END;
}  // onEvent

// -----------------------------------------------------------------------------
/// Process simple format 'X' messages
void c_WebMgr::ProcessXseriesRequests (AsyncWebSocketClient * client)
{
    // DEBUG_START;

    switch (WebSocketFrameCollectionBuffer[1])
    {
        case SimpleMessage::GET_STATUS:
        {
            // DEBUG_V ("");
            ProcessXJRequest (client);
            break;
        }  // end case SimpleMessage::GET_STATUS:

        case SimpleMessage::GET_ADMIN:
        {
            // DEBUG_V ("");
            ProcessXARequest (client);
            break;
        }  // end case SimpleMessage::GET_ADMIN:

        case SimpleMessage::DO_RESET:
        {
            // DEBUG_V ("");
            extern bool reboot;
            reboot = true;
            break;
        }  // end case SimpleMessage::DO_RESET:

        case SimpleMessage::DO_FACTORYRESET:
        {
            // DEBUG_V ("");
            CfgMgr.DeleteConfigFile ();

            // DEBUG_V ("");
            extern bool reboot;
            reboot = true;
            break;
        }  // end case SimpleMessage::DO_FACTORYRESET:

        case SimpleMessage::DO_STATSRESET:
        {
            // DEBUG_V ("DO_STATSRESET");
            LOG_INFO ("Reseting Statistics");

            extern listOfApplications_t g_listOfApplications;
            for (auto & currentApplication : g_listOfApplications)
            {
                // DEBUG_V ("");
                currentApplication->ResetStatistics ();
            }

            // DEBUG_V ("");
            break;
        }  // end case SimpleMessage::DO_STATSRESET:

        default:
        {
            LOG_ERROR (String (F ("Unhandled request: ")) + WebSocketFrameCollectionBuffer);
            client->text (String (F ("{\"Error\":\"Error\"}")).c_str ());
            break;
        }
    }  // end switch (data[1])

    // DEBUG_END;
}  // ProcessXseriesRequests

// -----------------------------------------------------------------------------
void c_WebMgr::ProcessXARequest (AsyncWebSocketClient * client)
{
    // DEBUG_START;

    DynamicJsonDocument webJsonDoc (1024);
    JsonObject jsonAdmin = webJsonDoc.createNestedObject (F ("admin"));

    jsonAdmin["version"]        = VERSION;
    jsonAdmin["built"]          = BUILD_DATE;
    jsonAdmin["realflashsize"]  = String (ESP.getFlashChipSize ());
    jsonAdmin["flashchipid"]    = int64String (ESP.getEfuseMac (), HEX);

    strcpy (WebSocketFrameCollectionBuffer, "XA");
    size_t msgOffset = strlen (WebSocketFrameCollectionBuffer);
    serializeJson (webJsonDoc, & WebSocketFrameCollectionBuffer[msgOffset], (sizeof (WebSocketFrameCollectionBuffer) - msgOffset));
    // DEBUG_V (String (WebSocketFrameCollectionBuffer));

    client->text (WebSocketFrameCollectionBuffer);

    // DEBUG_END;
}  // ProcessXARequest

// -----------------------------------------------------------------------------
void c_WebMgr::ProcessXJRequest (AsyncWebSocketClient * client)
{
    // DEBUG_START;

    DynamicJsonDocument webJsonDoc (4096);
    JsonObject  status  = webJsonDoc.createNestedObject (F ("status"));
    JsonObject  system  = status.createNestedObject (F ("system"));

    system[F ("freeheap")]      = (String)ESP.getFreeHeap ();
    system[F ("uptime")]        = millis ();
    system[F ("SDinstalled")]   = FileMgr.SdCardIsInstalled ();

    // DEBUG_V ("");

    // Ask WiFi Stats
    // Poll each of the applications
    for (auto & listOfApplicationsIterator : g_listOfApplications)
    {
        // String sName;
        // listOfApplicationsIterator->GetName(sName);
        // DEBUG_V(String ("Poll an Application: Started ") + sName);

        listOfApplicationsIterator->GetStatus (status);

        // DEBUG_V(String ("Poll an Application: Done ") + sName);
    }  // end Poll each application

    // DEBUG_V ("");

    // DEBUG_V ("");

    String response;
    serializeJson (webJsonDoc, response);
    // DEBUG_V (response);

    client->text ((String (F ("XJ")) + response).c_str ());

    // DEBUG_END;
}  // ProcessXJRequest

// -----------------------------------------------------------------------------
/// Process simple format 'V' messages
void c_WebMgr::ProcessVseriesRequests (AsyncWebSocketClient * client)
{
    // DEBUG_START;

    // String Response;
    // serializeJson (webJsonDoc, response);
    switch (WebSocketFrameCollectionBuffer[1])
    {
        case '1':
        {
            // Diag screen is asking for real time output data
            // client->binary (OutputMgr.GetBufferAddress (), OutputMgr.GetBufferUsedSize ());
            break;
        }

        default:
        {
            client->text (F ("V Error"));
            LOG_ERROR (String (F ("Unsupported Web command V")) + WebSocketFrameCollectionBuffer[1]);
            break;
        }
    }  // end switch

    // DEBUG_END;
}  // ProcessVseriesRequests

// -----------------------------------------------------------------------------
// Process JSON messages
void c_WebMgr::ProcessReceivedJsonMessage (DynamicJsonDocument & webJsonDoc, AsyncWebSocketClient * client)
{
    // DEBUG_START;
    // LOG_PORT.printf_P( PSTR("ProcessReceivedJsonMessage heap / stack ZcppStats: %u:%u:%u:%u\n"), ESP.getFreeHeap(), ESP.getHeapFragmentation(), ESP.getMaxFreeBlockSize(), ESP.getFreeContStack());

    do  // once
    {
        // DEBUG_V ("");

        /** Following commands are supported:
          * - get: returns requested configuration
          * - set: receive and applies configuration
          * - opt: returns select option lists
          */
        if (webJsonDoc.containsKey ("cmd"))
        {
            // DEBUG_V ("cmd");
            {
                // JsonObject jsonCmd = webJsonDoc.as<JsonObject> ();
                // PrettyPrint (jsonCmd);
            }
            JsonObject jsonCmd = webJsonDoc["cmd"];
            processCmd (client, jsonCmd);
            break;
        }  // webJsonDoc.containsKey ("cmd")

        // DEBUG_V ("");
    } while (false);

    // DEBUG_END;
}  // ProcessReceivedJsonMessage

// -----------------------------------------------------------------------------
void c_WebMgr::processCmd (AsyncWebSocketClient * client, JsonObject & jsonCmd)
{
    // DEBUG_START;

    WebSocketFrameCollectionBuffer[0] = ((char)NULL);
    // PrettyPrint (jsonCmd);

    do  // once
    {
        // Process "GET" command - return requested configuration as JSON
        if (jsonCmd.containsKey ("get"))
        {
            // DEBUG_V ("get");
            // DEBUG_V ("");
            processCmdGet (jsonCmd);
            // strcpy (WebSocketFrameCollectionBuffer, "{\"get\":\"OK\"}");

            // DEBUG_V ("");
            break;
        }

        // Process "SET" command - return requested configuration as JSON
        if (jsonCmd.containsKey ("set"))
        {
            // DEBUG_V ("set");
            JsonObject jsonCmdSet = jsonCmd["set"];
            // DEBUG_V ("");
            processCmdSet (jsonCmdSet);
            strcpy (WebSocketFrameCollectionBuffer, "{\"set\":\"OK\"}");

            // DEBUG_V ("");
            break;
        }

        // Generate select option list data
        if (jsonCmd.containsKey ("delete"))
        {
            // DEBUG_V ("delete");
            JsonObject temp = jsonCmd["delete"];
            processCmdDelete (temp);
            strcpy (WebSocketFrameCollectionBuffer, "{\"get\":\"OK\"}");

            // DEBUG_V ("");
            break;
        }

        // log an error
        LOG_ERROR (F ("Unhandled cmd"));
        // PrettyPrint (jsonCmd, String (F ("ERROR: Unhandled cmd")));
        strcpy (WebSocketFrameCollectionBuffer, "{\"cmd\":\"Error\"}");
    } while (false);

    // DEBUG_V ("");
    // terminate the response
    // DEBUG_V (WebSocketFrameCollectionBuffer);
    client->text (WebSocketFrameCollectionBuffer);

    // DEBUG_END;
}  // processCmd

// -----------------------------------------------------------------------------
void c_WebMgr::processCmdGet (JsonObject & jsonCmd)
{
    // DEBUG_START;
    // PrettyPrint (jsonCmd, "processCmdGet");

    // DEBUG_V ("");

    do  // once
    {
        if ((jsonCmd[CN_get] == CN_config))
        {
            // DEBUG_V ("System Config");
            strcpy (WebSocketFrameCollectionBuffer, "{\"get\":{\"config\":");
            size_t  StartingLen     = strlen (WebSocketFrameCollectionBuffer);
            size_t  spaceRemaining  = (sizeof (WebSocketFrameCollectionBuffer) - StartingLen) - 2;
            CfgMgr.GetSystemConfig (& WebSocketFrameCollectionBuffer[StartingLen], spaceRemaining);
            strcat (WebSocketFrameCollectionBuffer, "}}");
            // DEBUG_V (WebSocketFrameCollectionBuffer);
            break;
        }

        if (jsonCmd[CN_get] == CN_files)
        {
            // DEBUG_V ("input");
            strcpy (WebSocketFrameCollectionBuffer, "{\"get\":");
            size_t  StartingLen     = strlen (WebSocketFrameCollectionBuffer);
            size_t  spaceRemaining  = (sizeof (WebSocketFrameCollectionBuffer) - StartingLen) - 2;
            FileMgr.GetListOfSdFiles (& WebSocketFrameCollectionBuffer[StartingLen], spaceRemaining);
            strcat (WebSocketFrameCollectionBuffer, "}");
            // DEBUG_V ("");
            break;
        }

        // log an error
        LOG_ERROR (F ("Unhandled Get Request"));
        // PrettyPrint (jsonCmd, String (F ("ERROR: Unhandled Get Request")));
        strcat (WebSocketFrameCollectionBuffer, "\"ERROR: Request Not Supported\"");
    } while (false);

    // DEBUG_V (WebSocketFrameCollectionBuffer);

    // DEBUG_END;
}  // processCmdGet

// -----------------------------------------------------------------------------
void c_WebMgr::processCmdSet (JsonObject & jsonCmd)
{
    // DEBUG_START;
    // PrettyPrint (jsonCmd);

    do  // once
    {
        if (jsonCmd.containsKey (CN_network))
        {
            // PrettyPrint (jsonCmd, String ("processCmdSet"));

            // DEBUG_V ("config");
            serializeJson (jsonCmd, WebSocketFrameCollectionBuffer, sizeof (WebSocketFrameCollectionBuffer));
            CfgMgr.SetSystemConfig (WebSocketFrameCollectionBuffer);

            // DEBUG_V ("config: Done");
            break;
        }

        if (jsonCmd.containsKey (CN_time))
        {
            // PrettyPrint (jsonCmd, String ("processCmdSet"));

            // DEBUG_V ("time");
            JsonObject otConfig = jsonCmd[CN_time];
            processCmdSetTime (otConfig);
            // DEBUG_V ("time: Done");
            break;
        }

        // DEBUG_V (WebSocketFrameCollectionBuffer);
        LOG_ERROR (F ("Unhandled Set request type"));
    } while (false);

    // DEBUG_V (WebSocketFrameCollectionBuffer);

    // DEBUG_END;
}  // processCmdSet

// -----------------------------------------------------------------------------
void c_WebMgr::processCmdSetTime (JsonObject & jsonCmd)
{
    // DEBUG_START;

    // PrettyPrint (jsonCmd, String ("processCmdSetTime"));

    // DateTime TimeToSet = DateTime(uint32_t(0));
    // READ_JSON (jsonCmd, TimeToSet, F ("time_t"), c_CfgMgr::ScheduleUpdate_t::NoUpdate);

    // struct timeval now = { .tv_sec = TimeToSet.unixtime() };

    // settimeofday ( & now, NULL);

    // DEBUG_V (String ("TimeToSet: ") + String (TimeToSet));
    // DEBUG_V (String ("TimeToSet: ") + String (ctime( & TimeToSet)));

    strcat (WebSocketFrameCollectionBuffer, "{\"OK\" : true}");

    // DEBUG_END;
}  // ProcessXTRequest

// -----------------------------------------------------------------------------
void c_WebMgr::processCmdOpt (JsonObject & jsonCmd)
{
    // DEBUG_START;
    // PrettyPrint (jsonCmd);

    do  // once
    {
        // log error
        // PrettyPrint (jsonCmd, String (F ("ERROR: Unhandled 'opt' Request: ")));
    } while (false);

    // DEBUG_END;
}  // processCmdOpt

// -----------------------------------------------------------------------------
void c_WebMgr::processCmdDelete (JsonObject & jsonCmd)
{
    // DEBUG_START;

    do  // once
    {
        if (jsonCmd.containsKey (CN_files))
        {
            // DEBUG_V ("");

            JsonArray JsonFilesToDelete = jsonCmd[CN_files];

            // DEBUG_V (WebSocketFrameCollectionBuffer);
            for (JsonObject JsonFile : JsonFilesToDelete)
            {
                String FileToDelete = JsonFile[CN_name];

                // DEBUG_V ("FileToDelete: " + FileToDelete);
                FileMgr.DeleteSdFile (FileToDelete);
            }

            strcpy (WebSocketFrameCollectionBuffer, "{\"cmd\": { \"delete\": \"OK\"}}");

            break;
        }

        LOG_ERROR (F ("Unsupported Delete command"));
        // PrettyPrint (jsonCmd, String (F ("* Unsupported Delete command: ")));
        strcat (WebSocketFrameCollectionBuffer, "Page Not found");
    } while (false);

    // DEBUG_END;
}  // processCmdDelete

// -----------------------------------------------------------------------------
void c_WebMgr::FirmwareUpload (AsyncWebServerRequest    * request,
                               String                   filename,
                               size_t                   index,
                               uint8_t                  * data,
                               size_t                   len,
                               bool final)
{
    // DEBUG_START;

    do  // once
    {
        // make sure we are in AP mode
        if (0 == WiFi.softAPgetStationNum ())
        {
            // DEBUG_V("Not in AP Mode");

            // we are not talking to a station so we are not in AP mode
            // break;
        }

        // DEBUG_V ("");

        // is the first message in the upload?
        if (0 == index)
        {
            #ifdef ARDUINO_ARCH_ESP8266
                WiFiUDP::stopAll ();
            #else // ifdef ARDUINO_ARCH_ESP8266
                // this is not supported for ESP32
            #endif // ifdef ARDUINO_ARCH_ESP8266
            LOG_INFO (String (F ("Upload Started: ")) + filename);
            efupdate.begin ();
        }

        // DEBUG_V ("");

        if (!efupdate.process (data, len))
        {
            LOG_ERROR (String (F ("UPDATE ERROR: ")) + String (efupdate.getError ()));
        }

        if (efupdate.hasError ())
        {
            // DEBUG_V ("efupdate.hasError");
            request->send (200, CN_textSLASHplain, (String (F ("Update Error: ")) + String (efupdate.getError ())).c_str ());
            break;
        }

        // DEBUG_V ("");

        if (final)
        {
            request->send (200, CN_textSLASHplain, (String (F ("Update Finished: ")) + String (efupdate.getError ())).c_str ());
            LOG_INFO (F ("Upload Finished."));
            efupdate.end ();
            LittleFS.begin ();
        }
    } while (false);

    // DEBUG_END;
}  // onEvent

// -----------------------------------------------------------------------------
/*
  * This function is called as part of the Arudino "loop" and does things that need
  * periodic poking.
  *
  */
void c_WebMgr::Poll () {}  // Process

// -----------------------------------------------------------------------------
// create a global instance of the WEB UI manager
c_WebMgr WebMgr;
