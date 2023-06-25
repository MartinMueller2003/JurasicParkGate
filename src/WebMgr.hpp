#pragma once
/*
  * WebMgr.hpp - Web Management class
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
#include "EFUpdate.hpp"
#include "CfgMgr.hpp"
#include <ESPAsyncWebServer.h>
// #include <EspalexaDevice.h>
#include <SD.h>

class c_WebMgr : public c_Common
{
public:
    c_WebMgr ();
    virtual~c_WebMgr ();

    // Stop the compiler generating methods to copy the object
    c_WebMgr (c_WebMgr const & copy);
    c_WebMgr &operator=(c_WebMgr const & copy);

    void    Init            (); ///< set up the operating environment based on the current config (or defaults)
    void    ValidateConfig  ();
    void    Poll            ();

    // void onAlexaMessage        (EspalexaDevice * pDevice);
    // void RegisterAlexaCallback (DeviceCallbackFunction cb);
    // bool IsAlexaCallbackValid  () { return (nullptr != pAlexaCallback); }
    void    FirmwareUpload        (AsyncWebServerRequest * request, String filename, size_t index, uint8_t * data, size_t len, bool final);
    void    handleFileUpload      (AsyncWebServerRequest * request, String filename, size_t index, uint8_t * data, size_t len, bool final);
    void    NetworkStateChanged   (bool NewNetworkState);
private:

    EFUpdate            efupdate;
    // DeviceCallbackFunction pAlexaCallback = nullptr;
    // EspalexaDevice *       pAlexaDevice   = nullptr;

    static const size_t WebSocketFrameCollectionBufferSize = c_CfgMgr::CFGMGR_CFG_SIZE;
    char                WebSocketFrameCollectionBuffer[WebSocketFrameCollectionBufferSize + 1];

    /// Valid "Simple" message types
    enum SimpleMessage
    {
        GET_STATUS  = 'J',
        GET_ADMIN   = 'A',
        DO_RESET = '6',
        DO_FACTORYRESET = '7',
        DO_STATSRESET = '8'
    };

    void    init ();
    void    onWsEvent                  (AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t * data, size_t len);
    void    ProcessVseriesRequests     (AsyncWebSocketClient * client);
    void    ProcessReceivedJsonMessage (DynamicJsonDocument & webJsonDoc, AsyncWebSocketClient * client);
    void    processCmd                 (AsyncWebSocketClient * client,  JsonObject & jsonCmd);
    void    processCmdGet              (JsonObject & jsonCmd);
    void    processCmdSet              (JsonObject & jsonCmd);
    void    processCmdOpt              (JsonObject & jsonCmd);
    void    processCmdDelete           (JsonObject & jsonCmd);
    void    processCmdSetTime          (JsonObject & jsonCmd);

    void    GetConfiguration           ();
    void    GetOptions                 ();
    void    ProcessXseriesRequests     (AsyncWebSocketClient * client);
    void    ProcessXARequest           (AsyncWebSocketClient * client);
    void    ProcessXJRequest           (AsyncWebSocketClient * client);

    void    GetDeviceOptions           ();
    void    GetInputOptions            ();
    void    GetOutputOptions           ();

protected:
};  // c_WebMgr

extern c_WebMgr WebMgr;
