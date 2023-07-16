/*
 * InputMgr.cpp - Input Management class
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
 *   This is a factory class used to manage the Input channel. It creates and deletes
 *   the Input channel functionality as needed to support any new configurations
 *   that get sent from from the WebPage.
 *
 */

#include "JurasicParkGate.h"

// -----------------------------------------------------------------------------
// bring in driver definitions
#include "InputDisabled.hpp"
#include "InputEffectEngine.hpp"
#include "InputMQTT.h"
#include "InputAlexa.h"
#include "InputButtons.hpp"
#include "InputGateControl.hpp"
// needs to be last
#include "InputMgr.hpp"

static c_InputGateControl InputGateControl;

// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local Data definitions
// -----------------------------------------------------------------------------
typedef struct
{
    c_InputMgr::e_InputType id;
    String name;
    c_InputMgr::e_InputChannelIds ChannelId;
} InputTypeXlateMap_t;

static const InputTypeXlateMap_t InputTypeXlateMap[c_InputMgr::e_InputType::InputType_End] =
{
    {c_InputMgr::e_InputType::InputType_Buttons,  "Buttons",  c_InputMgr::e_InputChannelIds::InputPrimaryChannelId   },
    {c_InputMgr::e_InputType::InputType_Effects,  "Effects",  c_InputMgr::e_InputChannelIds::InputSecondaryChannelId },
    {c_InputMgr::e_InputType::InputType_MQTT,     "MQTT",     c_InputMgr::e_InputChannelIds::InputSecondaryChannelId },
    {c_InputMgr::e_InputType::InputType_Alexa,    "Alexa",    c_InputMgr::e_InputChannelIds::InputSecondaryChannelId },
    {c_InputMgr::e_InputType::InputType_Disabled, "Disabled", c_InputMgr::e_InputChannelIds::InputSecondaryChannelId }
};

// -----------------------------------------------------------------------------
// Methods
// -----------------------------------------------------------------------------
///< Start up the driver and put it into a safe mode
c_InputMgr::c_InputMgr ()
{
    ConfigFileName = String ( F ("/") ) + String (CN_input_config) + F (".json");

    // this gets called pre-setup so there is nothing we can do here.
    int pInputChannelDriversIndex = 0;
    for (auto & CurrentInput : InputChannelDrivers)
    {
        CurrentInput.pInputChannelDriver = nullptr;
        CurrentInput.DriverId            = pInputChannelDriversIndex;

        EffectEngineIsConfiguredToRun[pInputChannelDriversIndex] = false;
        ++pInputChannelDriversIndex;
    }
}  // c_InputMgr

// -----------------------------------------------------------------------------
///< deallocate any resources and put the Input channels into a safe state
c_InputMgr::~c_InputMgr ()
{
    // DEBUG_START;

    // delete pInputInstances;
    int pInputChannelDriversIndex = 0;
    for (auto & CurrentInput : InputChannelDrivers)
    {
        if (nullptr != CurrentInput.pInputChannelDriver)
        {
            // the drivers will put the hardware in a safe state
            delete CurrentInput.pInputChannelDriver;
            CurrentInput.pInputChannelDriver = nullptr;
        }

        pInputChannelDriversIndex++;
    }

    // DEBUG_END;
}  // ~c_InputMgr

// -----------------------------------------------------------------------------
///< Start the module
void c_InputMgr::Begin (uint32_t BufferSize)
{
    // DEBUG_START;

    InputDataBufferSize = BufferSize;
    // DEBUG_V (String("InputDataBufferSize: ") + String (InputDataBufferSize));

    // prevent recalls
    if (true == HasBeenInitialized) {return;}

    InputGateControl.Begin ();

    // make sure the pointers are set up properly
    for (auto & CurrentInput : InputChannelDrivers)
    {
        InstantiateNewInputChannel (e_InputChannelIds (CurrentInput.DriverId), e_InputType::InputType_Disabled);
        // DEBUG_V ("");
    }

    InputGateControl.SetBufferInfo(BufferSize);

    HasBeenInitialized = true;

    // load up the configuration from the saved file. This also starts the drivers
    LoadConfig ();

    // CreateNewConfig ();

    // DEBUG_END;
}  // begin

// -----------------------------------------------------------------------------
void c_InputMgr::CreateJsonConfig (JsonObject & jsonConfig)
{
    // DEBUG_START;

    // add IM config parameters
    // DEBUG_V ("");

    // DEBUG_V ("");
    // extern void PrettyPrint (JsonObject & jsonStuff, String Name);

    // DEBUG_V ("");

    // add the channels header
    JsonObject InputMgrChannelsData;

    if ( true == jsonConfig.containsKey (CN_channels) )
    {
        // DEBUG_V ("");
        InputMgrChannelsData = jsonConfig[CN_channels];
    }
    else
    {
        // add our section header
        // DEBUG_V ("");
        InputMgrChannelsData = jsonConfig.createNestedObject (CN_channels);
    }

    // add the channel configurations
    // DEBUG_V ("For Each Input Channel");
    for (auto & CurrentChannel : InputChannelDrivers)
    {
        if (nullptr == CurrentChannel.pInputChannelDriver)
        {
            // DEBUG_V ("");
            continue;
        }

        // DEBUG_V (String("Create Section in Config file for the Input channel: '") + CurrentChannel.pInputChannelDriver->GetInputChannelId() + "'");
        // create a record for this channel
        JsonObject  ChannelConfigData;
        String      sChannelId = String ( CurrentChannel.pInputChannelDriver->GetInputChannelId () );

        // DEBUG_V (String("sChannelId: '") + String(sChannelId) + "'");
        if ( true == InputMgrChannelsData.containsKey (sChannelId) )
        {
            // DEBUG_V ("");
            ChannelConfigData = InputMgrChannelsData[sChannelId];
        }
        else
        {
            // add our section header
            // DEBUG_V ("");
            ChannelConfigData = InputMgrChannelsData.createNestedObject (sChannelId);
        }

        // save the name as the selected channel type
        uint32_t    DriverType    = int( CurrentChannel.pInputChannelDriver->GetInputType () );
        String      DriverTypeStr = String (DriverType);
        // DEBUG_V(String("DriverTypeId: ") + DriverTypeStr);
        String      DriverNameStr;
        CurrentChannel.pInputChannelDriver->GetDriverName (DriverNameStr);
        // DEBUG_V(String("DriverName: '") + DriverNameStr + "'");

        ChannelConfigData[CN_type] = DriverType;

        JsonObject ChannelConfigByTypeData;

        if ( true == ChannelConfigData.containsKey ( String (DriverTypeStr) ) )
        {
            ChannelConfigByTypeData = ChannelConfigData[DriverTypeStr];
            // DEBUG_V ("");
        }
        else
        {
            // add our section header
            // DEBUG_V ("");
            ChannelConfigByTypeData = ChannelConfigData.createNestedObject (DriverTypeStr);
        }

        // ask the channel to add its data to the record
        // DEBUG_V ("Add the Input channel configuration for type: " + DriverTypeId);

        // Populate the driver name
        String DriverName = "";
        CurrentChannel.pInputChannelDriver->GetDriverName (DriverName);
        ChannelConfigByTypeData[CN_type] = DriverName;

        CurrentChannel.pInputChannelDriver->GetConfig (ChannelConfigByTypeData);
        // DEBUG_V ("");
    }

    // smile. Your done
    // DEBUG_END;
}  // CreateJsonConfig

// -----------------------------------------------------------------------------
/*
 *     The running config is made from a composite of running and not instantiated
 *     objects. To create a complete config we need to start each Input type on
 *     each Input channel and collect the configuration at each stage.
 */
void c_InputMgr::CreateNewConfig ()
{
    // DEBUG_START;

    logcon ( String ( F ("--- WARNING: Creating a new Input Manager configuration Data set ---") ) );

    // create a place to save the config
    // DEBUG_V(String("Heap: ") + String(ESP.getFreeHeap()));

    DynamicJsonDocument JsonConfigDoc (IM_JSON_SIZE);
    // DEBUG_V("");

    JsonObject JsonConfig = JsonConfigDoc.createNestedObject (CN_input_config);
    // DEBUG_V("");

    JsonConfig[CN_cfgver] = CurrentConfigVersion;

    // DEBUG_V ("for each Input type");
    for (int InputTypeId = int(InputType_Start);
     InputTypeId < int(InputType_End);
     ++InputTypeId)
    {
        // DEBUG_V(String("instantiate the Input type: ") + InputTypeId);
        // DEBUG_V("for each input channel");
        for (auto & CurrentInput : InputChannelDrivers)
        {
            // DEBUG_V(String("DriverId: ") + CurrentInput.DriverId);
            InstantiateNewInputChannel (e_InputChannelIds (CurrentInput.DriverId), e_InputType (InputTypeId), false);
        }  // end for each interface

        // DEBUG_V ("collect the config data");
        CreateJsonConfig (JsonConfig);

        // DEBUG_V ("");
    }  // end for each Input type

    // DEBUG_V ("leave the Inputs disabled");
    for (auto & CurrentInput : InputChannelDrivers)
    {
        InstantiateNewInputChannel (e_InputChannelIds (CurrentInput.DriverId), e_InputType::InputType_Disabled, false);
    }  // end for each interface

    // buttons are always enabled
    InstantiateNewInputChannel (e_InputChannelIds::InputChannelId_Start, e_InputType::InputType_Buttons, false);

    InputGateControl.GetConfig(JsonConfig);

    // DEBUG_V ("");

    // Record the default configuration
    CreateJsonConfig (JsonConfig);

    SetConfig (JsonConfigDoc);

    // logcon (String (F ("--- WARNING: Creating a new Input Manager configuration Data set - Done ---")));
    // DEBUG_END;
}  // CreateNewConfig

// -----------------------------------------------------------------------------
void c_InputMgr::GetConfig (byte* Response, uint32_t maxlen)
{
    // DEBUGSTART;

    FileMgr.ReadConfigFile (ConfigFileName, Response, maxlen);
    // DEBUGV (String ("TempConfigData: ") + TempConfigData);

    // DEBUGEND;
}  // GetConfig

// -----------------------------------------------------------------------------
void c_InputMgr::GetStatus (JsonObject & jsonStatus)
{
    // _ DEBUG_START;

    JsonArray InputStatus = jsonStatus.createNestedArray ( F ("input") );
    for (auto & CurrentInput : InputChannelDrivers)
    {
        JsonObject channelStatus = InputStatus.createNestedObject ();
        CurrentInput.pInputChannelDriver->GetStatus (channelStatus);
        // _ DEBUG_V("");
    }

    JsonObject GateStatus = InputStatus.createNestedObject ();
    InputGateControl.GetStatus(GateStatus);

    // _ DEBUG_END;
}  // GetStatus

// -----------------------------------------------------------------------------
/* Determine whether the input type is allowed on the desired input channel
 *
 *     needs
 *         channel type
 *         channel id
 *     returns
 *         true - input type is allowed on the input channel
 */
bool c_InputMgr::InputTypeIsAllowedOnChannel (e_InputType type, e_InputChannelIds ChannelId)
{
    // DEBUG_START;

    bool response = false;

    // DEBUG_V (String("type: ") + String(type));

    // find the input type
    for (InputTypeXlateMap_t currentInputType : InputTypeXlateMap)
    {
        // DEBUG_V ("");
        // is this the entry we are looking for?
        if (currentInputType.id == type)
        {
            // DEBUG_V ("");
            // is the input allowed on the desired channel?
            if ( (currentInputType.ChannelId == ChannelId) ||
             (currentInputType.ChannelId == InputChannelId_ALL) )
            {
                // DEBUG_V ("Allowed");
                response = true;
            }

            // DEBUG_V ("");

            break;
        }  // found the entry we are looking for

        // DEBUG_V ("");
    }  // loop entries

    // DEBUG_V (String ("response: ") + String (response));

    // DEBUG_END;
    return(response);
}  // InputTypeIsAllowedOnChannel

// -----------------------------------------------------------------------------
/* Create an instance of the desired Input type in the desired channel
 *
 * WARNING:  This function assumes there is a driver running in the identified
 *           out channel. These must be set up and started when the manager is
 *           started.
 */
void c_InputMgr::InstantiateNewInputChannel (e_InputChannelIds ChannelIndex, e_InputType NewInputChannelType, bool StartDriver)
{
    // DEBUG_START;
    // DEBUG_V (String ("       ChannelIndex: ") + String (ChannelIndex));
    // DEBUG_V (String ("NewInputChannelType: ") + String (NewInputChannelType));
    // DEBUG_V (String ("        StartDriver: ") + String (StartDriver));
    // DEBUG_V (String ("InputDataBufferSize: ") + String (InputDataBufferSize));

    do  // once
    {
        // is there an existing driver?
        if (nullptr != InputChannelDrivers[ChannelIndex].pInputChannelDriver)
        {
            // DEBUG_V(String("InputChannelDrivers[ChannelIndex].pInputChannelDriver->GetInputType () '") + InputChannelDrivers[ChannelIndex].pInputChannelDriver->GetInputType() + String("'"));
            // DEBUG_V (String("NewInputChannelType '") + int(NewInputChannelType) + "'");

            // DEBUG_V ("does the driver need to change?");
            if (InputChannelDrivers[ChannelIndex].pInputChannelDriver->GetInputType () == NewInputChannelType)
            {
                // DEBUG_V ("nothing to change");
                break;
            }

            String DriverName;
            InputChannelDrivers[ChannelIndex].pInputChannelDriver->GetDriverName (DriverName);
            rebootNeeded |= InputChannelDrivers[ChannelIndex].pInputChannelDriver->isShutDownRebootNeeded ();

            // DEBUG_V (String ("rebootNeeded: ") + String (rebootNeeded));
            if (!IsBooting)
            {
                logcon ( String ( F ("Shutting Down '") ) + DriverName + String ( F ("' on Input: ") ) + String (ChannelIndex) );
            }

            // do not delete the buttons object
            if( e_InputType::InputType_Buttons != InputChannelDrivers[ChannelIndex].pInputChannelDriver->GetInputType () )
            {
                // DEBUG_V ("Delete old driver instance");
                delete InputChannelDrivers[ChannelIndex].pInputChannelDriver;
            }
            InputChannelDrivers[ChannelIndex].pInputChannelDriver = nullptr;

            // DEBUG_V ("");
        }  // end there is an existing driver

        // DEBUG_V ("Set up the new driver");

        switch (NewInputChannelType)
        {
        case e_InputType::InputType_Disabled :
        {
            // DEBUG_V ("InputType_Disabled: Start");
            logcon (String ( F ("Disabled Input type for channel '") ) + ChannelIndex + "'.");
            InputChannelDrivers[ChannelIndex].pInputChannelDriver = new c_InputDisabled (ChannelIndex, InputType_Disabled, InputDataBufferSize);
            // DEBUG_V ("InputType_Disabled: End");
            break;
        }

        case e_InputType::InputType_Buttons :
        {
            // DEBUG_V ("InputType_Button: Start");
            if ( InputTypeIsAllowedOnChannel (InputType_Buttons, ChannelIndex) )
            {
                logcon (String ( F ("Starting Button for channel '") ) + ChannelIndex + "'.");
                // DEBUG_V ("c_InputButtons");
                InputChannelDrivers[ChannelIndex].pInputChannelDriver = &InputButtons;
                InputButtons.SetParms (ChannelIndex, InputType_Buttons, InputDataBufferSize);
                // DEBUG_V ("c_InputButtons");
            }
            else
            {
                // DEBUG_V("InputType_Button: Not allowed on channel");
                InputChannelDrivers[ChannelIndex].pInputChannelDriver = new c_InputDisabled (ChannelIndex, InputType_Disabled, InputDataBufferSize);
            }

            // DEBUG_V ("c_InputButton: End");
            break;
        }

        case e_InputType::InputType_Effects :
        {
            // DEBUG_V ("InputType_Effects: Start");
            if ( InputTypeIsAllowedOnChannel (InputType_Effects, ChannelIndex) )
            {
                logcon (String ( F ("Starting Effects Engine for channel '") ) + ChannelIndex + "'.");
                // DEBUG_V ("c_InputEffectEngine");
                InputChannelDrivers[ChannelIndex].pInputChannelDriver = new c_InputEffectEngine (ChannelIndex, InputType_Effects, InputDataBufferSize);
                // DEBUG_V ("");
            }
            else
            {
                // DEBUG_V("InputType_Effects: Not allowed on channel");
                InputChannelDrivers[ChannelIndex].pInputChannelDriver = new c_InputDisabled (ChannelIndex, InputType_Disabled, InputDataBufferSize);
            }

            // DEBUG_V ("InputType_Effects: End");
            break;
        }

        case e_InputType::InputType_MQTT :
        {
            // DEBUG_V ("InputType_MQTT: Start");
            if ( InputTypeIsAllowedOnChannel (InputType_MQTT, ChannelIndex) )
            {
                logcon (String ( F ("Starting MQTT for channel '") ) + ChannelIndex + "'.");
                InputChannelDrivers[ChannelIndex].pInputChannelDriver = new c_InputMQTT (ChannelIndex, InputType_MQTT, InputDataBufferSize);
                // DEBUG_V ("");
            }
            else
            {
                // DEBUG_V("InputType_MQTT: Not allowed on channel");
                InputChannelDrivers[ChannelIndex].pInputChannelDriver = new c_InputDisabled (ChannelIndex, InputType_Disabled, InputDataBufferSize);
            }

            // DEBUG_V ("InputType_MQTT: End");
            break;
        }

        case e_InputType::InputType_Alexa :
        {
            // DEBUG_V ("InputType_Alexa: Start");
            if ( InputTypeIsAllowedOnChannel (InputType_Alexa, ChannelIndex) )
            {
                logcon (String ( F ("Starting Alexa for channel '") ) + ChannelIndex + "'.");
                InputChannelDrivers[ChannelIndex].pInputChannelDriver = new c_InputAlexa (ChannelIndex, InputType_Alexa, InputDataBufferSize);
                // DEBUG_V ("");
            }
            else
            {
                // DEBUG_V("InputType_Alexa: Not allowed on channel");
                InputChannelDrivers[ChannelIndex].pInputChannelDriver = new c_InputDisabled (ChannelIndex, InputType_Disabled, InputDataBufferSize);
            }

            // DEBUG_V ("InputType_Alexa: End");
            break;
        }

        default :
        {
            logcon (CN_stars + String ( F (" Unknown Input type for channel '") ) + ChannelIndex + String ( F ("'. Using disabled. ") ) + CN_stars);
            InputChannelDrivers[ChannelIndex].pInputChannelDriver = new c_InputDisabled (ChannelIndex, InputType_Disabled, InputDataBufferSize);
            // DEBUG_V ("");
            break;
        }
        }  // end switch (NewChannelType)

        // DEBUG_V ("");
        // String sDriverName;
        // pInputChannelDrivers[ChannelIndex]->GetDriverName (sDriverName);
        // Serial.println (String (CN_stars) + " '" + sDriverName + F("' Initialization for input: '") + String(ChannelIndex) + "' " + CN_stars);
        if (StartDriver)
        {
            // DEBUG_V (String ("StartDriver: ") + String (StartDriver));
            InputChannelDrivers[ChannelIndex].pInputChannelDriver->Begin ();
            // DEBUG_V ("");
            InputChannelDrivers[ChannelIndex].pInputChannelDriver->SetBufferInfo (InputDataBufferSize);
        }

        // DEBUG_V ("");
    } while (false);

    // DEBUG_END;
}  // InstantiateNewInputChannel

// -----------------------------------------------------------------------------
/* Load and process the current configuration
 *
 *   needs
 *       Nothing
 *   returns
 *       Nothing
 */
void c_InputMgr::LoadConfig ()
{
    // DEBUG_START;

    // try to load and process the config file
    if ( !FileMgr.LoadConfigFile (
         ConfigFileName,
         [this] (DynamicJsonDocument & JsonConfigDoc)
    {
        // DEBUG_V ("");
        JsonObject JsonConfig = JsonConfigDoc.as <JsonObject>();
        // DEBUG_V ("");
        this->ProcessJsonConfig (JsonConfig);
        // DEBUG_V ("");
    }) )
    {
        logcon (CN_stars + String ( F (" Error loading Input Manager Config File ") ) + CN_stars);

        if (IsBooting)
        {
            // create a config file with default values
            // DEBUG_V ("");
            CreateNewConfig ();
        }
        else
        {
            reboot = true;
        }

    }

    // DEBUG_END;
}  // LoadConfig

// -----------------------------------------------------------------------------
///< Called from loop(), renders Input data
void c_InputMgr::Process ()
{
    // _ DEBUG_START;
    // do we need to save the current config?

    do  // once
    {
        if (configInProgress)
        {
            // prevent calls to process when we are doing a long operation
            break;
        }

        if (true == configLoadNeeded)
        {
            configLoadNeeded = false;
            configInProgress = true;
            // DEBUG_V ("Reload the config");
            LoadConfig ();
            // DEBUG_V ("End Save Config");
            configInProgress = false;
        }

        bool aBlankTimerIsRunning = false;
        for (auto & CurrentInput : InputChannelDrivers)
        {
            // _ DEBUG_V("");
            CurrentInput.pInputChannelDriver->Process ();

            if ( !BlankTimerHasExpired ( CurrentInput.pInputChannelDriver->GetInputChannelId () ) )
            {
                // _ DEBUG_V (String ("Blank Timer is running: ") + String (CurrentInput.pInputChannelDriver->GetInputChannelId ()));
                aBlankTimerIsRunning = true;
                break;
            }
        }

        InputGateControl.Process();

        if ( (false == aBlankTimerIsRunning) && (config.BlankDelay != 0) )
        {
            // _ DEBUG_V("Clear Input Buffer");
            OutputMgr.ClearBuffer ();
            RestartBlankTimer (InputSecondaryChannelId);
        }  // ALL blank timers have expired

        if (rebootNeeded)
        {
            // DEBUG_V("Requesting Reboot");
            reboot = true;
        }
    } while (false);

    // _ DEBUG_END;
}  // Process

// -----------------------------------------------------------------------------
/*
 *     check the contents of the config and send
 *     the proper portion of the config to the currently instantiated channels
 */
bool c_InputMgr::ProcessJsonConfig (JsonObject & jsonConfig)
{
    // DEBUG_START;
    bool Response = false;

    // DEBUG_V ("InputDataBufferSize: " + String (InputDataBufferSize));

    do  // once
    {
        if ( false == jsonConfig.containsKey (CN_input_config) )
        {
            logcon ( String ( F ("No Input Interface Settings Found. Using Defaults") ) );
            extern void PrettyPrint (JsonObject & jsonStuff, String Name);
            PrettyPrint ( jsonConfig, String ( F ("c_InputMgr::ProcessJsonConfig") ) );
            break;
        }

        JsonObject InputChannelMgrData = jsonConfig[CN_input_config];
        // DEBUG_V ("");

        uint8_t TempVersion = !InputChannelMgrData;
        setFromJSON (TempVersion, InputChannelMgrData, CN_cfgver);
        InputGateControl.SetConfig(InputChannelMgrData);

        // DEBUG_V (String ("TempVersion: ") + String (TempVersion));
        // DEBUG_V (String ("CurrentConfigVersion: ") + String (CurrentConfigVersion));
        // extern void PrettyPrint (JsonObject & jsonStuff, String Name);
        // PrettyPrint (InputChannelMgrData, "Output Config");

        if (TempVersion != CurrentConfigVersion)
        {
            logcon ( String ( F ("InputMgr: Incorrect Version found. Using existing/default config.") ) );
            // break;
        }

        // do we have a channel configuration array?
        if ( false == InputChannelMgrData.containsKey (CN_channels) )
        {
            // if not, flag an error and stop processing
            logcon ( String ( F ("No Input Channel Settings Found. Using Defaults") ) );
            break;
        }

        JsonObject InputChannelArray = InputChannelMgrData[CN_channels];
        // DEBUG_V ("");

        // for each Input channel
        for (uint32_t ChannelIndex = uint32_t (InputChannelId_Start);
         ChannelIndex < uint32_t (InputChannelId_End);
         ChannelIndex++)
        {
            // get access to the channel config
            if ( false == InputChannelArray.containsKey ( String (ChannelIndex) ) )
            {
                // if not, flag an error and stop processing
                logcon ( String ( F ("No Input Settings Found for Channel '") ) + ChannelIndex + String ( F ("'. Using Defaults") ) );
                continue;
            }

            JsonObject InputChannelConfig = InputChannelArray[String (ChannelIndex)];
            // DEBUG_V ("");

            // set a default value for channel type
            uint32_t ChannelType = uint32_t (InputType_Default);
            setFromJSON (ChannelType, InputChannelConfig, CN_type);
            // DEBUG_V ("");

            // is it a valid / supported channel type
            if ( /*(ChannelType < uint32_t (InputType_Start)) ||*/ ( ChannelType >= uint32_t (InputType_End) ) )
            {
                // if not, flag an error and move on to the next channel
                logcon (String ( F ("Invalid Channel Type in config '") ) + ChannelType + String ( F ("'. Specified for channel '") ) + ChannelIndex + "'. Disabling channel");
                InstantiateNewInputChannel (e_InputChannelIds (ChannelIndex), e_InputType::InputType_Disabled);
                continue;
            }

            // DEBUG_V ("");

            // do we have a configuration for the channel type?
            if ( false == InputChannelConfig.containsKey ( String (ChannelType) ) )
            {
                // if not, flag an error and stop processing
                logcon ( String ( F ("No Input Settings Found for Channel '") ) + ChannelIndex + String ( F ("'. Using Defaults") ) );
                InstantiateNewInputChannel (e_InputChannelIds (ChannelIndex), e_InputType::InputType_Disabled);
                continue;
            }

            JsonObject InputChannelDriverConfig = InputChannelConfig[String (ChannelType)];
            // DEBUG_V ("");

            // make sure the proper Input type is running
            InstantiateNewInputChannel ( e_InputChannelIds (ChannelIndex), e_InputType (ChannelType) );
            // DEBUG_V (String ("Response: ") + Response);

            EffectEngineIsConfiguredToRun[ChannelIndex] = ( e_InputType::InputType_Effects == InputChannelDrivers[ChannelIndex].pInputChannelDriver->GetInputType () )?true : false;

            // send the config to the driver. At this level we have no idea what is in it
            InputChannelDrivers[ChannelIndex].pInputChannelDriver->SetConfig (InputChannelDriverConfig);
            // DEBUG_V (String("Response: ") + Response);
        }  // end for each channel

        Response = true;
    } while (false);

    // did we get a valid config?
    if (false == Response)
    {
        // save the current config since it is the best we have.
        // DEBUG_V ("");
        CreateNewConfig ();
    }

    // DEBUG_V ("InputDataBufferSize: " + String (InputDataBufferSize));

    // DEBUG_END;
    return(Response);
}  // ProcessJsonConfig

// -----------------------------------------------------------------------------
/* Sets the configuration for the current active ports:
 *
 *   WARNING: This runs in the Web server context and cannot access the File system
 */
void c_InputMgr::SetConfig (const char* NewConfigData)
{
    // DEBUG_START;

    if ( true == FileMgr.SaveConfigFile (ConfigFileName, NewConfigData) )
    {
        // DEBUG_V (String("NewConfigData: ") + NewConfigData);
        // FileMgr logs for us
        // logcon (CN_stars + String (F (" Saved Input Manager Config File. ")) + CN_stars);

        configLoadNeeded = true;
    }  // end we saved the config
    else
    {
        logcon (CN_stars + String ( F (" Error Saving Input Manager Config File ") ) + CN_stars);
    }

    // DEBUG_END;
}  // SetConfig

// -----------------------------------------------------------------------------
/* Sets the configuration for the current active ports:
 *
 *   WARNING: This runs in the Web server context and cannot access the File system
 */
void c_InputMgr::SetConfig (JsonDocument & NewConfigData)
{
    // DEBUG_START;

    if ( true == FileMgr.SaveConfigFile (ConfigFileName, NewConfigData) )
    {
        // FileMgr logs for us
        // logcon (CN_stars + String (F (" Saved Input Manager Config File. ")) + CN_stars);

        configLoadNeeded = true;
    }  // end we saved the config
    else
    {
        logcon (CN_stars + String ( F (" Error Saving Input Manager Config File ") ) + CN_stars);
    }

    // DEBUG_END;
}  // SetConfig

// -----------------------------------------------------------------------------
void c_InputMgr::SetBufferInfo (uint32_t BufferSize)
{
    // DEBUG_START;

    InputDataBufferSize = BufferSize;

    // DEBUG_V ("InputDataBufferSize: " + String (InputDataBufferSize));

    InputGateControl.SetBufferInfo(BufferSize);

    // pass through each active interface and set the buffer info
    // for each Input channel
    for (int ChannelIndex = int(InputChannelId_Start);
     ChannelIndex < int(InputChannelId_End);
     ChannelIndex++)
    {
        if (nullptr != InputChannelDrivers[ChannelIndex].pInputChannelDriver)
        {
            InputChannelDrivers[ChannelIndex].pInputChannelDriver->SetBufferInfo (InputDataBufferSize);
        }
    }  // end for each channel

    // DEBUG_END;
}  // SetBufferInfo

// -----------------------------------------------------------------------------
void c_InputMgr::SetOperationalState (bool ActiveFlag)
{
    // DEBUG_START;

    // pass through each active interface and set the active state
    for (auto & InputChannel : InputChannelDrivers)
    {
        // DEBUG_V ("");
        if (nullptr != InputChannel.pInputChannelDriver)
        {
            InputChannel.pInputChannelDriver->SetOperationalState (ActiveFlag);
            // DEBUG_V ("");
        }
    }
    InputGateControl.SetOperationalState(ActiveFlag);

    // DEBUG_END;
}  // SetOperationalState

// -----------------------------------------------------------------------------
void c_InputMgr::NetworkStateChanged (bool _IsConnected)
{
    // DEBUG_START;

    IsConnected = _IsConnected;

    if (HasBeenInitialized)
    {
        // pass through each active interface and notify WiFi changed state
        for (auto & InputChannel : InputChannelDrivers)
        {
            // DEBUG_V (String ("pInputChannel: 0x") + String (uint32_t (), HEX));
            InputChannel.pInputChannelDriver->NetworkStateChanged (IsConnected);
        }
    }
    InputGateControl.NetworkStateChanged(_IsConnected);
    // DEBUG_END;
}  // NetworkStateChanged

// create a global instance of the Input channel factory
c_InputMgr InputMgr;
