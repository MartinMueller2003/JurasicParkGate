#pragma once
/*
 * InputMgr.hpp - Input Management class
 *
 * Project: JurasicParkGate
 * Copyright (c) 2023 Martin Mueller
 * http://www.MartnMueller2003.com
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
 *   This is a factory class used to manage the input channels. It creates and deletes
 *   the input channel functionality as needed to support any new configurations
 *   that get sent from from the WebPage.
 *
 */

#include "JurasicParkGate.h"
#include "FileMgr.hpp"
#include "OutputMgr.hpp"

class c_InputCommon;  ///< forward declaration to the pure virtual Input class that will be defined later.

class c_InputMgr{
public:
// handles to determine which input channel we are dealing with
// Channel 1 = primary / show input; Channel 2 = service input
enum e_InputChannelIds
{
    InputPrimaryChannelId = 0,
    InputSecondaryChannelId = 1,
    InputChannelId_End,
    InputChannelId_Start = InputPrimaryChannelId,
    InputChannelId_ALL = InputChannelId_End,
    EffectsChannel = InputSecondaryChannelId
};

c_InputMgr ();
virtual~c_InputMgr ();

void Begin                (uint32_t BufferSize);
void LoadConfig           ();
void GetConfig            (byte*    Response,
 uint32_t                           maxlen);
void GetStatus            (JsonObject & jsonStatus);
void SetConfig            (const char* NewConfig);
void SetConfig            (ArduinoJson::JsonDocument & NewConfig);
void Process              ();
void SetBufferInfo        (uint32_t BufferSize);
void SetOperationalState  (bool Active);
void NetworkStateChanged  (bool IsConnected);
void DeleteConfig         ()
{
    FileMgr.DeleteConfigFile (ConfigFileName);
}
bool GetNetworkState      ()
{
    return(IsConnected);
}
void GetDriverName        (String & Name)
{
    Name = "InputMgr";
}
void RestartBlankTimer    (e_InputChannelIds Selector)
{
    BlankEndTime[int(Selector)].StartTimer (config.BlankDelay * 1000);
}
bool BlankTimerHasExpired (e_InputChannelIds Selector)
{
    return( BlankEndTime[int(Selector)].IsExpired () );
}
enum e_InputType
{
    InputType_Buttons = 0,
    InputType_Effects,
    InputType_MQTT,
    InputType_Alexa,
    InputType_Disabled,
    InputType_End,
    InputType_Start = InputType_Buttons,
    InputType_Default = InputType_Disabled
};

private:

void InstantiateNewInputChannel (e_InputChannelIds  InputChannelId,
 e_InputType                                        NewChannelType,
 bool                                               StartDriver = true);
void CreateNewConfig ();
struct DriverInfo_t
{
    uint32_t DriverId            = 0;
    c_InputCommon* pInputChannelDriver = nullptr;          ///< pointer(s) to the current active Input driver
};

DriverInfo_t InputChannelDrivers[InputChannelId_End];        ///< pointer(s) to the current active Input driver
uint32_t InputDataBufferSize = 0;
bool HasBeenInitialized  = false;
bool EffectEngineIsConfiguredToRun[InputChannelId_End];
bool IsConnected      = false;
bool configInProgress = false;
bool configLoadNeeded = false;

// configuration parameter names for the channel manager within the config file
bool ProcessJsonConfig           (JsonObject & jsonConfig);
void CreateJsonConfig            (JsonObject & jsonConfig);
bool ProcessJsonChannelConfig    (JsonObject &  jsonConfig,
 uint32_t                                       ChannelIndex);
bool InputTypeIsAllowedOnChannel (e_InputType   type,
 e_InputChannelIds                              ChannelId);

String ConfigFileName;
bool rebootNeeded = false;

FastTimer BlankEndTime[InputChannelId_End];

    #define IM_JSON_SIZE (5 * 1024)
}; // c_InputMgr

extern c_InputMgr InputMgr;
