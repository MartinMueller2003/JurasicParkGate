/*
 * OutputMgr.cpp - Output Management class
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
 *   This is a factory class used to manage the output port. It creates and deletes
 *   the output channel functionality as needed to support any new configurations
 *   that get sent from from the WebPage.
 *
 */

#include "JurasicParkGate.h"
#include "FileMgr.hpp"


// -----------------------------------------------------------------------------
// bring in driver definitions
#include "OutputDisabled.hpp"
#include "OutputServoPCA9685.hpp"
// needs to be last
#include "OutputMgr.hpp"

#include "InputMgr.hpp"

#ifndef DEFAULT_RELAY_GPIO
    #define DEFAULT_RELAY_GPIO gpio_num_t::GPIO_NUM_1
#endif // ndef DEFAULT_RELAY_GPIO


// -----------------------------------------------------------------------------
// Local Data definitions
// -----------------------------------------------------------------------------
typedef struct
{
    c_OutputMgr::e_OutputType id;
    String name;
} OutputTypeXlateMap_t;

static const OutputTypeXlateMap_t OutputTypeXlateMap[c_OutputMgr::e_OutputType::OutputType_End] =
{
    {c_OutputMgr::e_OutputType::OutputType_Servo_PCA9685, "Servo_PCA9685" },
    // {c_OutputMgr::e_OutputType::OutputType_Disabled,      "Disabled"      },
};

// -----------------------------------------------------------------------------
typedef struct
{
    gpio_num_t GpioPin;
    uart_port_t PortId;
    c_OutputMgr::OM_PortType_t PortType;
} OutputChannelIdToGpioAndPortEntry_t;

// -----------------------------------------------------------------------------
static const OutputChannelIdToGpioAndPortEntry_t OutputChannelIdToGpioAndPort[] =
{
    {DEFAULT_RELAY_GPIO, uart_port_t (-1), c_OutputMgr::OM_PortType_t::Relay},
    {DEFAULT_RELAY_GPIO, uart_port_t (-1), c_OutputMgr::OM_PortType_t::Relay},
    {DEFAULT_RELAY_GPIO, uart_port_t (-1), c_OutputMgr::OM_PortType_t::Relay},
    {DEFAULT_RELAY_GPIO, uart_port_t (-1), c_OutputMgr::OM_PortType_t::Relay},
    {DEFAULT_RELAY_GPIO, uart_port_t (-1), c_OutputMgr::OM_PortType_t::Relay},
};

// -----------------------------------------------------------------------------
// Methods
// -----------------------------------------------------------------------------
///< Start up the driver and put it into a safe mode
c_OutputMgr::c_OutputMgr ()
{
    ConfigFileName = String ("/") + String (CN_output_config) + CN_Dotjson;

    // clear the input data buffer
    memset ( (char*)&OutputBuffer[0], 0, sizeof (OutputBuffer) );
}  // c_OutputMgr

// -----------------------------------------------------------------------------
///< deallocate any resources and put the output channels into a safe state
c_OutputMgr::~c_OutputMgr ()
{
    // DEBUG_START;

    // delete pOutputInstances;
    for (DriverInfo_t & CurrentOutput : OutputChannelDrivers)
    {
        // the drivers will put the hardware in a safe state
        delete CurrentOutput.pOutputChannelDriver;
    }

    // DEBUG_END;
}  // ~c_OutputMgr

// -----------------------------------------------------------------------------
///< Start the module
void c_OutputMgr::Begin ()
{
    // DEBUG_START;

    // IsBooting = false;
    // FileMgr.DeleteConfigFile(ConfigFileName);

    do  // once
    {
        // prevent recalls
        if (true == HasBeenInitialized)
        {
            break;
        }

        if (0 == OutputChannelId_End)
        {
            logcon ( F ("ERROR: No output Channels defined. Rebooting") );
            reboot = true;
            break;
        }

        HasBeenInitialized = true;

        #ifdef LED_FLASH_GPIO
            pinMode (LED_FLASH_GPIO, OUTPUT);
            digitalWrite (LED_FLASH_GPIO, LED_FLASH_OFF);
        #endif // def LED_FLASH_GPIO

        // make sure the pointers are set up properly
        uint8_t index = 0;
        for (DriverInfo_t & CurrentOutputChannelDriver : OutputChannelDrivers)
        {
            // DEBUG_V(String("init index: ") + String(index) + " Start");
            CurrentOutputChannelDriver.DriverId = e_OutputChannelIds (index++);
            InstantiateNewOutputChannel (CurrentOutputChannelDriver, e_OutputType::OutputType_Start);
            // DEBUG_V(String("init index: ") + String(index) + " Done");
        }

        // CreateNewConfig ();

        // load up the configuration from the saved file. This also starts the drivers
        // DEBUG_V();
        LoadConfig ();

        // CreateNewConfig ();

        // Preset the output memory
        memset ( (void*)&OutputBuffer[0], 0x00, sizeof (OutputBuffer) );
    } while (false);

    // DEBUG_END;
}  // begin

// -----------------------------------------------------------------------------
void c_OutputMgr::CreateJsonConfig (JsonObject & jsonConfig)
{
    // DEBUG_START;

    // extern void PrettyPrint (JsonObject&, String);
    // PrettyPrint (jsonConfig, String ("jsonConfig"));

    // add OM config parameters
    // DEBUG_V ();

    // add the channels header
    JsonObject OutputMgrChannelsData;

    if ( true == jsonConfig.containsKey (CN_channels) )
    {
        // DEBUG_V ();
        OutputMgrChannelsData = jsonConfig[CN_channels];
    }
    else
    {
        // add our section header
        // DEBUG_V ();
        OutputMgrChannelsData = jsonConfig.createNestedObject (CN_channels);
    }

    // add the channel configurations
    // DEBUG_V ("For Each Output Channel");
    for (auto & CurrentChannel : OutputChannelDrivers)
    {
        // DEBUG_V (String("Create Section in Config file for the output channel: '") + CurrentChannel.pOutputChannelDriver->GetOutputChannelId() + "'");
        // create a record for this channel
        JsonObject  ChannelConfigData;
        String      sChannelId = String ( CurrentChannel.pOutputChannelDriver->GetOutputChannelId () );

        if ( true == OutputMgrChannelsData.containsKey (sChannelId) )
        {
            // DEBUG_V ();
            ChannelConfigData = OutputMgrChannelsData[sChannelId];
        }
        else
        {
            // add our section header
            // DEBUG_V ();
            ChannelConfigData = OutputMgrChannelsData.createNestedObject (sChannelId);
        }

        // save the name as the selected channel type
        ChannelConfigData[CN_type] = int( CurrentChannel.pOutputChannelDriver->GetOutputType () );

        String      DriverTypeId = String ( int( CurrentChannel.pOutputChannelDriver->GetOutputType () ) );
        JsonObject  ChannelConfigByTypeData;

        if ( true == ChannelConfigData.containsKey ( String (DriverTypeId) ) )
        {
            ChannelConfigByTypeData = ChannelConfigData[DriverTypeId];
            // DEBUG_V ();
        }
        else
        {
            // add our section header
            // DEBUG_V ();
            ChannelConfigByTypeData = ChannelConfigData.createNestedObject (DriverTypeId);
        }

        // DEBUG_V ();
        // PrettyPrint (ChannelConfigByTypeData, String ("jsonConfig"));

        // ask the channel to add its data to the record
        // DEBUG_V ("Add the output channel configuration for type: " + DriverTypeId);

        // Populate the driver name
        String DriverName = "";
        CurrentChannel.pOutputChannelDriver->GetDriverName (DriverName);
        // DEBUG_V (String ("DriverName: ") + DriverName);

        ChannelConfigByTypeData[CN_type] = DriverName;

        // DEBUG_V ();
        // PrettyPrint (ChannelConfigByTypeData, String ("jsonConfig"));
        // DEBUG_V ();

        CurrentChannel.pOutputChannelDriver->GetConfig (ChannelConfigByTypeData);

        // DEBUG_V ();
        // PrettyPrint (ChannelConfigByTypeData, String ("jsonConfig"));
        // DEBUG_V ();
    }  // for each output channel

    // DEBUG_V ();
    // PrettyPrint (jsonConfig, String ("jsonConfig"));

    // smile. Your done
    // DEBUG_END;
}  // CreateJsonConfig

// -----------------------------------------------------------------------------
/*
 *     The running config is made from a composite of running and not instantiated
 *     objects. To create a complete config we need to start each output type on
 *     each output channel and collect the configuration at each stage.
 */
void c_OutputMgr::CreateNewConfig ()
{
    // DEBUG_START;
    // extern void PrettyPrint(JsonObject & jsonStuff, String Name);

    // DEBUG_V (String (("--- WARNING: Creating a new Output Manager configuration Data set ---")));

    BuildingNewConfig = true;

    // create a place to save the config
    DynamicJsonDocument JsonConfigDoc (OM_MAX_CONFIG_SIZE);
    // DEBUG_V ();

    JsonObject JsonConfig = JsonConfigDoc.createNestedObject (CN_output_config);
    // DEBUG_V ();

    JsonConfig[CN_cfgver]      = CurrentConfigVersion;
    JsonConfig[CN_MaxChannels] = sizeof (OutputBuffer);

    // Collect the all ports disabled config first
    CreateJsonConfig (JsonConfig);

    // DEBUG_V ("for each output type");
    for (auto CurrentOutputType : OutputTypeXlateMap)
    {
        // DEBUG_V (String ("instantiate output type: ") + String (CurrentOutputType.name));
        // DEBUG_V ("for each interface");
        for (DriverInfo_t & CurrentOutputChannelDriver : OutputChannelDrivers)
        {
            // DEBUG_V (String("DriverId: ") + String(CurrentOutputChannelDriver.DriverId));
            InstantiateNewOutputChannel (CurrentOutputChannelDriver, CurrentOutputType.id, false);
            // DEBUG_V ();
        }  // end for each interface

        if ( JsonConfigDoc.overflowed () )
        {
            logcon ("Error: Config size is too small. Cannot create an output config with the current settings.")
            break;
        }

        // PrettyPrint(JsonConfig, "Intermediate in OutputMgr");

        // DEBUG_V ("collect the config data for this output type");
        CreateJsonConfig (JsonConfig);
        // DEBUG_V (String("       Heap: ") + String(ESP.getFreeHeap()));
        // DEBUG_V (String(" overflowed: ") + String(JsonConfigDoc.overflowed()));
        // DEBUG_V (String("memoryUsage: ") + String(JsonConfigDoc.memoryUsage()));
        // PrettyPrint(JsonConfig, "Final Port in OutputMgr");

        // DEBUG_V ();
    }  // end for each output type

    // DEBUG_V ("leave the outputs disabled");
    for (auto & CurrentOutputChannelDriver : OutputChannelDrivers)
    {
        InstantiateNewOutputChannel (CurrentOutputChannelDriver, e_OutputType::OutputType_Start);
    }  // end for each interface

    // PrettyPrint(JsonConfig, "Complete OutputMgr");

    // DEBUG_V ("Outputs Are disabled");

    CreateJsonConfig (JsonConfig);

    // DEBUG_V (String ("       Heap: ") + String (ESP.getFreeHeap ()));
    // DEBUG_V (String (" overflowed: ") + String (JsonConfigDoc.overflowed()));
    // DEBUG_V (String ("memoryUsage: ") + String (JsonConfigDoc.memoryUsage()));

    SetConfig (JsonConfigDoc);

    // DEBUG_V (String (("--- WARNING: Creating a new Output Manager configuration Data set - Done ---")));

    BuildingNewConfig = true;

    // DEBUG_END;
}  // CreateNewConfig

// -----------------------------------------------------------------------------
void c_OutputMgr::GetConfig (String & Response)
{
    // DEBUG_START;

    FileMgr.ReadConfigFile (ConfigFileName, Response);

    // DEBUG_END;
}  // GetConfig

// -----------------------------------------------------------------------------
void c_OutputMgr::GetConfig (byte* Response, uint32_t maxlen)
{
    // DEBUG_START;

    FileMgr.ReadConfigFile (ConfigFileName, Response, maxlen);

    // DEBUG_END;
}  // GetConfig

// -----------------------------------------------------------------------------
void c_OutputMgr::GetStatus (JsonObject & jsonStatus)
{
    // DEBUG_START;

    #if defined (ARDUINO_ARCH_ESP32)
        // jsonStatus["PollCount"] = PollCount;
    #endif // defined(ARDUINO_ARCH_ESP32)

    JsonArray OutputStatus = jsonStatus.createNestedArray (CN_output);
    for (auto & CurrentOutput : OutputChannelDrivers)
    {
        // DEBUG_V ();
        JsonObject channelStatus = OutputStatus.createNestedObject ();
        CurrentOutput.pOutputChannelDriver->GetStatus (channelStatus);
        // DEBUG_V ();
    }

    // DEBUG_END;
}  // GetStatus

// -----------------------------------------------------------------------------
/* Create an instance of the desired output type in the desired channel
 *
 * WARNING:  This function assumes there is a driver running in the identified
 *           out channel. These must be set up and started when the manager is
 *           started.
 *
 *     needs
 *         channel ID
 *         channel type
 *     returns
 *         nothing
 */
void c_OutputMgr::InstantiateNewOutputChannel (DriverInfo_t & CurrentOutputChannelDriver, e_OutputType NewOutputChannelType, bool StartDriver)
{
    // DEBUG_START;
    // BuildingNewConfig = false;
    // IsBooting = false;
    do  // once
    {
        // is there an existing driver?
        if (nullptr != CurrentOutputChannelDriver.pOutputChannelDriver)
        {
            // DEBUG_V (String("OutputChannelDrivers[uint(CurrentOutputChannel.DriverId)]->GetOutputType () '") +
            //    String(OutputChannelDrivers[uint(CurrentOutputChannelDriver.DriverId)].pOutputChannelDriver->GetOutputType()) + String("'"));
            // DEBUG_V (String ("NewOutputChannelType '") + int(NewOutputChannelType) + "'");

            // DEBUG_V ("does the driver need to change?");
            if (CurrentOutputChannelDriver.pOutputChannelDriver->GetOutputType () == NewOutputChannelType)
            {
                // DEBUG_V ("nothing to change");
                break;
            }

            String DriverName;
            CurrentOutputChannelDriver.pOutputChannelDriver->GetDriverName (DriverName);

            if (!IsBooting)
            {
                logcon ( String (MN_12) + DriverName + MN_13 + String (CurrentOutputChannelDriver.DriverId) );
            }

            delete CurrentOutputChannelDriver.pOutputChannelDriver;
            CurrentOutputChannelDriver.pOutputChannelDriver = nullptr;
            // DEBUG_V ();
        }  // end there is an existing driver

        // DEBUG_V ();

        // get the new data and UART info
        CurrentOutputChannelDriver.GpioPin  = OutputChannelIdToGpioAndPort[CurrentOutputChannelDriver.DriverId].GpioPin;
        CurrentOutputChannelDriver.PortType = OutputChannelIdToGpioAndPort[CurrentOutputChannelDriver.DriverId].PortType;
        CurrentOutputChannelDriver.PortId   = OutputChannelIdToGpioAndPort[CurrentOutputChannelDriver.DriverId].PortId;

        // DEBUG_V (String("DriverId: ") + String(CurrentOutputChannelDriver.DriverId));
        // DEBUG_V (String(" GpioPin: ") + String(CurrentOutputChannelDriver.PortId));
        // DEBUG_V (String("PortType: ") + String(CurrentOutputChannelDriver.PortType));
        // DEBUG_V (String("  PortId: ") + String(CurrentOutputChannelDriver.PortId));

        switch (NewOutputChannelType)
        {
/*
        case e_OutputType::OutputType_Disabled :
        {
            // logcon (CN_stars + String (CN_Disabled) + MN_06 + CurrentOutputChannelDriver.DriverId + "'. " + CN_stars);
            CurrentOutputChannelDriver.pOutputChannelDriver = new c_OutputDisabled (
                CurrentOutputChannelDriver.DriverId,
                CurrentOutputChannelDriver.GpioPin,
                CurrentOutputChannelDriver.PortId,
                OutputType_Disabled);
            // DEBUG_V ();
            break;
        }
 */
        case e_OutputType::OutputType_Servo_PCA9685 :
        {
            // if (CurrentOutputChannelDriver.PortType == OM_PortType_t::Relay)
            {
                logcon (CN_stars + String ( F (" Starting Servo PCA9685 for channel '") ) + CurrentOutputChannelDriver.DriverId + "'. " + CN_stars);
                CurrentOutputChannelDriver.pOutputChannelDriver = new c_OutputServoPCA9685 (
                    CurrentOutputChannelDriver.DriverId,
                    CurrentOutputChannelDriver.GpioPin,
                    CurrentOutputChannelDriver.PortId,
                    OutputType_Servo_PCA9685);
                // DEBUG_V ();
                break;
            }
            // DEBUG_V ();

            if (!BuildingNewConfig)
            {
                logcon (CN_stars + String ( F (" Cannot Start Servo PCA9685 for channel '") ) + CurrentOutputChannelDriver.DriverId + "'. " + CN_stars);
            }
/*
            CurrentOutputChannelDriver.pOutputChannelDriver = new c_OutputDisabled (
                CurrentOutputChannelDriver.DriverId,
                CurrentOutputChannelDriver.GpioPin,
                CurrentOutputChannelDriver.PortId,
                OutputType_Disabled);
            // DEBUG_V ();
            break;
 */
        }

        default :
        {
            if (!IsBooting)
            {
                logcon (CN_stars + String (MN_09) + String (NewOutputChannelType) + MN_10 + CurrentOutputChannelDriver.DriverId + MN_11 + CN_stars);
            }

            CurrentOutputChannelDriver.pOutputChannelDriver = new c_OutputServoPCA9685 (
                CurrentOutputChannelDriver.DriverId,
                CurrentOutputChannelDriver.GpioPin,
                CurrentOutputChannelDriver.PortId,
                OutputType_Servo_PCA9685);
            // DEBUG_V ();
            break;
        }
        }  // end switch (NewChannelType)

        // DEBUG_V ();
        // DEBUG_V (String("pOutputChannelDriver: 0x") + String(uint32_t(CurrentOutputChannelDriver.pOutputChannelDriver),HEX));
        // DEBUG_V (String("heap: 0x") + String(uint32_t(ESP.getMaxFreeBlockSize()),HEX));

        String sDriverName;
        CurrentOutputChannelDriver.pOutputChannelDriver->GetDriverName (sDriverName);

        // DEBUG_V (String("Driver Name: ") + sDriverName);
        if (!IsBooting)
        {
            logcon ( "'" + sDriverName + MN_14 + String (CurrentOutputChannelDriver.DriverId) );
        }

        if (StartDriver)
        {
            // DEBUG_V ("Starting Driver");
            CurrentOutputChannelDriver.pOutputChannelDriver->Begin ();
        }
    } while (false);

    // DEBUG_END;
}  // InstantiateNewOutputChannel

// -----------------------------------------------------------------------------
/* Load and process the current configuration
 *
 *   needs
 *       Nothing
 *   returns
 *       Nothing
 */
void c_OutputMgr::LoadConfig ()
{
    // DEBUG_START;

    // try to load and process the config file
    if ( !FileMgr.LoadConfigFile (
         ConfigFileName,
         [this] (DynamicJsonDocument & JsonConfigDoc)
    {
        // extern void PrettyPrint(DynamicJsonDocument & jsonStuff, String Name);
        // PrettyPrint(JsonConfigDoc, "OM Load Config");

        // DEBUG_V ();
        JsonObject JsonConfig = JsonConfigDoc.as <JsonObject>();

        // extern void PrettyPrint(JsonObject & jsonStuff, String Name);
        // PrettyPrint(JsonConfig, "OM Load Config");
        // DEBUG_V ();
        this->ProcessJsonConfig (JsonConfig);
        // DEBUG_V ();
    }) )
    {
        if (!IsBooting)
        {
            logcon (CN_stars + String (MN_15) + CN_stars);
        }

        // create a config file with default values
        // DEBUG_V ();
        CreateNewConfig ();
    }

    // DEBUG_END;
}  // LoadConfig

// -----------------------------------------------------------------------------
/*
 *     check the contents of the config and send
 *     the proper portion of the config to the currently instantiated channels
 *
 *     needs
 *         ref to data from config file
 *     returns
 *         true - config was properly processes
 *         false - config had an error.
 */
bool c_OutputMgr::ProcessJsonConfig (JsonObject & jsonConfig)
{
    // DEBUG_START;
    bool Response = false;

    // DEBUG_V ();

    // extern void PrettyPrint(JsonObject & jsonStuff, String Name);
    // PrettyPrint(jsonConfig, "ProcessJsonConfig");

    do  // once
    {
        if ( false == jsonConfig.containsKey (CN_output_config) )
        {
            logcon (String (MN_16) + MN_18);
            break;
        }

        JsonObject OutputChannelMgrData = jsonConfig[CN_output_config];
        // DEBUG_V ();

        uint8_t TempVersion = !CurrentConfigVersion;
        setFromJSON (TempVersion, OutputChannelMgrData, CN_cfgver);

        // DEBUG_V (String ("TempVersion: ") + String (TempVersion));
        // DEBUG_V (String ("CurrentConfigVersion: ") + String (CurrentConfigVersion));
        // extern void PrettyPrint (JsonObject & jsonStuff, String Name);
        // PrettyPrint (OutputChannelMgrData, "Output Config");
        if (TempVersion != CurrentConfigVersion)
        {
            logcon (MN_17);
            // break;
        }

        // do we have a channel configuration array?
        if ( false == OutputChannelMgrData.containsKey (CN_channels) )
        {
            // if not, flag an error and stop processing
            logcon (String (MN_16) + MN_18);
            break;
        }

        JsonObject OutputChannelArray = OutputChannelMgrData[CN_channels];
        // DEBUG_V ();

        // for each output channel
        for (auto & CurrentOutputChannelDriver : OutputChannelDrivers)
        {
            // get access to the channel config
            if ( false == OutputChannelArray.containsKey ( String (CurrentOutputChannelDriver.DriverId).c_str () ) )
            {
                // if not, flag an error and stop processing
                logcon (String (MN_16) + CurrentOutputChannelDriver.DriverId + MN_18);
                break;
            }

            JsonObject OutputChannelConfig = OutputChannelArray[String (CurrentOutputChannelDriver.DriverId).c_str ()];
            // DEBUG_V ();

            // extern void PrettyPrint (JsonObject& jsonStuff, String Name);
            // PrettyPrint(OutputChannelConfig, "ProcessJson Channel Config");

            // set a default value for channel type
            uint32_t ChannelType = uint32_t (OutputType_End);
            setFromJSON (ChannelType, OutputChannelConfig, CN_type);
            // DEBUG_V ();

            // is it a valid / supported channel type
            if ( ChannelType >= uint32_t (OutputType_End) )
            {
                // if not, flag an error and move on to the next channel
                logcon (String (MN_19) + ChannelType + MN_20 + CurrentOutputChannelDriver.DriverId + MN_03);
                InstantiateNewOutputChannel (CurrentOutputChannelDriver, e_OutputType::OutputType_Start);
                continue;
            }

            // DEBUG_V ();

            // do we have a configuration for the channel type?
            if ( false == OutputChannelConfig.containsKey ( String (ChannelType) ) )
            {
                // if not, flag an error and stop processing
                logcon (String (MN_16) + CurrentOutputChannelDriver.DriverId + MN_18);
                InstantiateNewOutputChannel (CurrentOutputChannelDriver, e_OutputType::OutputType_Start);
                continue;
            }

            // DEBUG_V ();
            // PrettyPrint(OutputChannelConfig, "ProcessJson Channel Config");

            JsonObject OutputChannelDriverConfig = OutputChannelConfig[String (ChannelType)];
            // DEBUG_V ();
            // PrettyPrint(OutputChannelDriverConfig, "ProcessJson Channel Driver Config before driver create");
            // DEBUG_V ();

            // make sure the proper output type is running
            InstantiateNewOutputChannel ( CurrentOutputChannelDriver, e_OutputType (ChannelType) );

            // DEBUG_V ();
            // PrettyPrint(OutputChannelDriverConfig, "ProcessJson Channel Driver Config");
            // DEBUG_V ();

            // send the config to the driver. At this level we have no idea what is in it
            CurrentOutputChannelDriver.pOutputChannelDriver->SetConfig (OutputChannelDriverConfig);
            // DEBUG_V ();
        }  // end for each channel

        // all went well
        Response = true;
    } while (false);

    // did we get a valid config?
    if (false == Response)
    {
        // save the current config since it is the best we have.
        // DEBUG_V ();
        CreateNewConfig ();
    }

    // DEBUG_V ();
    UpdateDisplayBufferReferences ();

    // DEBUG_V ();

    // DEBUG_END;
    return(Response);
}  // ProcessJsonConfig

// -----------------------------------------------------------------------------
/*
 *   Save the current configuration to the FS
 *
 *   needs
 *       pointer to the config data array
 *   returns
 *       Nothing
 */
void c_OutputMgr::SetConfig (const char* ConfigData)
{
    // DEBUG_START;

    // DEBUG_V (String ("ConfigData: ") + ConfigData);

    if ( true == FileMgr.SaveConfigFile (ConfigFileName, ConfigData) )
    {
        ConfigLoadNeeded = true;
    }  // end we got a config and it was good
    else
    {
        logcon (CN_stars + String (MN_21) + CN_stars);
    }

    // DEBUG_END;
}  // SaveConfig

// -----------------------------------------------------------------------------
/*
 *   Save the current configuration to the FS
 *
 *   needs
 *       Reference to the JSON Document
 *   returns
 *       Nothing
 */
void c_OutputMgr::SetConfig (ArduinoJson::JsonDocument & ConfigData)
{
    // DEBUG_START;

    // serializeJson(ConfigData, LOG_PORT);
    // DEBUG_V ();

    if ( true == FileMgr.SaveConfigFile (ConfigFileName, ConfigData) )
    {
        ConfigLoadNeeded = true;
    }  // end we got a config and it was good
    else
    {
        logcon (CN_stars + String (MN_21) + CN_stars);
    }

    // DEBUG_END;
}  // SaveConfig

// -----------------------------------------------------------------------------
///< Called from loop()
void c_OutputMgr::Poll ()
{
    // //DEBUG_START;

    #ifdef LED_FLASH_GPIO
        pinMode (LED_FLASH_GPIO, OUTPUT);
        digitalWrite (LED_FLASH_GPIO, LED_FLASH_OFF);
    #endif // def LED_FLASH_GPIO

    // do we need to save the current config?
    if (true == ConfigLoadNeeded)
    {
        ConfigLoadNeeded = false;
        LoadConfig ();
    }  // done need to save the current config

    if (false == IsOutputPaused)
    {
        // //DEBUG_V();
        for (DriverInfo_t & OutputChannel : OutputChannelDrivers)
        {
            // //DEBUG_V("Start a new channel");
            OutputChannel.pOutputChannelDriver->Poll ();
        }
    }

    // //DEBUG_END;
}  // Poll

// -----------------------------------------------------------------------------
void c_OutputMgr::UpdateDisplayBufferReferences (void)
{
    // DEBUG_START;

    /* Buffer vs virtual buffer.
     *     Pixels have a concept called groups. A group of pixels uses
     *     a single tupple of input channel data and replicates that
     *     data into N positions in the output buffer. For non pixel
     *     channels or for pixels with a group size of 1, the virtual
     *     buffer and the output buffers are the same.
     *     The virtual buffer size is the one we give to the input
     *     processing engine
     */
    uint32_t    OutputBufferOffset  = 0;    // offset into the raw data in the output buffer
    uint32_t    OutputChannelOffset = 0;    // Virtual channel offset to the output buffer.

    // DEBUG_V (String ("        BufferSize: ") + String (sizeof(OutputBuffer)));
    // DEBUG_V (String ("OutputBufferOffset: ") + String (OutputBufferOffset));

    for (auto & OutputChannel : OutputChannelDrivers)
    {
        // String DriverName;
        // OutputChannel.pOutputChannelDriver->GetDriverName(DriverName);
        // DEBUG_V(String("Name: ") + DriverName);
        // DEBUG_V(String("PortId: ") + String(OutputChannel.pOutputChannelDriver->GetOutputChannelId()) );

        OutputChannel.OutputBufferStartingOffset  = OutputBufferOffset;
        OutputChannel.OutputChannelStartingOffset = OutputChannelOffset;
        OutputChannel.pOutputChannelDriver->SetOutputBufferAddress (&OutputBuffer[OutputBufferOffset]);

        uint32_t    OutputBufferDataBytesNeeded        = OutputChannel.pOutputChannelDriver->GetNumOutputBufferBytesNeeded ();
        uint32_t    VirtualOutputBufferDataBytesNeeded = OutputChannel.pOutputChannelDriver->GetNumOutputBufferChannelsServiced ();

        uint32_t    AvailableChannels = sizeof (OutputBuffer) - OutputBufferOffset;

        if (AvailableChannels < OutputBufferDataBytesNeeded)
        {
            logcon ( MN_22 + String (OutputBufferDataBytesNeeded) );
            // DEBUG_V (String ("    ChannelsNeeded: ") + String (ChannelsNeeded));
            // DEBUG_V (String (" AvailableChannels: ") + String (AvailableChannels));
            // DEBUG_V (String ("ChannelsToAllocate: ") + String (ChannelsToAllocate));
            break;
        }

        // DEBUG_V (String ("    ChannelsNeeded: ") + String (OutputBufferDataBytesNeeded));
        // DEBUG_V (String (" AvailableChannels: ") + String (AvailableChannels));

        OutputBufferOffset                 += OutputBufferDataBytesNeeded;
        OutputChannel.OutputBufferDataSize  = OutputBufferDataBytesNeeded;
        OutputChannel.OutputBufferEndOffset = OutputBufferOffset - 1;
        OutputChannel.pOutputChannelDriver->SetOutputBufferSize (OutputBufferDataBytesNeeded);

        OutputChannelOffset                 += VirtualOutputBufferDataBytesNeeded;
        OutputChannel.OutputChannelSize      = VirtualOutputBufferDataBytesNeeded;
        OutputChannel.OutputChannelEndOffset = OutputChannelOffset;

        // DEBUG_V (String("OutputChannel.GetBufferUsedSize: ") + String(OutputChannel.pOutputChannelDriver->GetBufferUsedSize()));
        // DEBUG_V (String ("OutputBufferOffset: ") + String(OutputBufferOffset));
    }

    // DEBUG_V (String ("   TotalBufferSize: ") + String (OutputBufferOffset));
    UsedBufferSize = OutputBufferOffset;
    // DEBUG_V (String ("       OutputBuffer: 0x") + String (uint32_t (OutputBuffer), HEX));
    // DEBUG_V (String ("     UsedBufferSize: ") + String (uint32_t (UsedBufferSize)));
    InputMgr.SetBufferInfo (OutputChannelOffset);

    // DEBUG_END;
}  // UpdateDisplayBufferReferences

// -----------------------------------------------------------------------------
void c_OutputMgr::PauseOutputs (bool PauseTheOutput)
{
    // DEBUG_START;
    IsOutputPaused = PauseTheOutput;

    for (auto & CurrentOutput : OutputChannelDrivers)
    {
        CurrentOutput.pOutputChannelDriver->PauseOutput (PauseTheOutput);
    }

    // DEBUG_END;
}  // PauseOutputs

// -----------------------------------------------------------------------------
void c_OutputMgr::WriteChannelData (uint32_t StartChannelId, uint32_t ChannelCount, uint8_t* pSourceData)
{
    // DEBUG_START;

    do  // once
    {
        if ( ( (StartChannelId + ChannelCount) > UsedBufferSize ) || (0 == ChannelCount) )
        {
            // DEBUG_V (String("ERROR: Invalid parameters"));
            // DEBUG_V (String("StartChannelId: ") + String(StartChannelId, HEX));
            // DEBUG_V (String("  ChannelCount: ") + String(ChannelCount));
            // DEBUG_V (String("UsedBufferSize: ") + String(UsedBufferSize));
            break;
        }

        // DEBUG_V (String("&OutputBuffer[StartChannelId]: 0x") + String(uint(&OutputBuffer[StartChannelId]), HEX));
        uint32_t EndChannelId = StartChannelId + ChannelCount;
        // Serial.print('1');
        for (auto & currentOutputChannelDriver : OutputChannelDrivers)
        {
            // Serial.print('2');
            // does this output handle this block of data?
            if (StartChannelId < currentOutputChannelDriver.OutputChannelStartingOffset)
            {
                // we have gone beyond where we can put this data.
                // Serial.print('3');
                break;
            }

            if (StartChannelId > currentOutputChannelDriver.OutputChannelEndOffset)
            {
                // move to the next driver
                // Serial.print('4');
                continue;
            }

            // Serial.print('5');

            uint32_t    lastChannelToSet       = min (EndChannelId, currentOutputChannelDriver.OutputChannelEndOffset);
            uint32_t    ChannelsToSet          = lastChannelToSet - StartChannelId;
            uint32_t    RelativeStartChannelId = StartChannelId - currentOutputChannelDriver.OutputChannelStartingOffset;

            // DEBUG_V (String("               StartChannelId: 0x") + String(StartChannelId, HEX));
            // DEBUG_V (String("                 EndChannelId: 0x") + String(EndChannelId, HEX));
            // DEBUG_V (String("             lastChannelToSet: 0x") + String(lastChannelToSet, HEX));
            // DEBUG_V (String("                ChannelsToSet: 0x") + String(ChannelsToSet, HEX));
            if (ChannelsToSet)
            {
                currentOutputChannelDriver.pOutputChannelDriver->WriteChannelData (RelativeStartChannelId, ChannelsToSet, pSourceData);
            }

            StartChannelId += ChannelsToSet;
            pSourceData    += ChannelsToSet;
            // memcpy(&OutputBuffer[StartChannelId], pSourceData, ChannelCount);
        }
    } while (false);

    // DEBUG_END;
}  // WriteChannelData

// -----------------------------------------------------------------------------
void c_OutputMgr::ReadChannelData (uint32_t StartChannelId, uint32_t ChannelCount, byte* pTargetData)
{
    // DEBUG_START;

    do  // once
    {
        if ( (StartChannelId + ChannelCount) > UsedBufferSize )
        {
            // DEBUG_V (String("ERROR: Invalid parameters"));
            // DEBUG_V (String("StartChannelId: ") + String(StartChannelId, HEX));
            // DEBUG_V (String("  ChannelCount: ") + String(ChannelCount));
            // DEBUG_V (String("UsedBufferSize: ") + String(UsedBufferSize));
            break;
        }

        // DEBUG_V (String("&OutputBuffer[StartChannelId]: 0x") + String(uint(&OutputBuffer[StartChannelId]), HEX));
        uint32_t EndChannelId = StartChannelId + ChannelCount;
        // Serial.print('1');
        for (auto & currentOutputChannelDriver : OutputChannelDrivers)
        {
            // Serial.print('2');
            // does this output handle this block of data?
            if (StartChannelId < currentOutputChannelDriver.OutputChannelStartingOffset)
            {
                // we have gone beyond where we can put this data.
                // Serial.print('3');
                break;
            }

            if (StartChannelId > currentOutputChannelDriver.OutputChannelEndOffset)
            {
                // move to the next driver
                // Serial.print('4');
                continue;
            }

            // Serial.print('5');

            uint32_t    lastChannelToSet       = min (EndChannelId, currentOutputChannelDriver.OutputChannelEndOffset);
            uint32_t    ChannelsToSet          = lastChannelToSet - StartChannelId;
            uint32_t    RelativeStartChannelId = StartChannelId - currentOutputChannelDriver.OutputChannelStartingOffset;
            // DEBUG_V (String("               StartChannelId: 0x") + String(StartChannelId, HEX));
            // DEBUG_V (String("                 EndChannelId: 0x") + String(EndChannelId, HEX));
            // DEBUG_V (String("             lastChannelToSet: 0x") + String(lastChannelToSet, HEX));
            // DEBUG_V (String("                ChannelsToSet: 0x") + String(ChannelsToSet, HEX));
            currentOutputChannelDriver.pOutputChannelDriver->ReadChannelData (RelativeStartChannelId, ChannelsToSet, pTargetData);
            StartChannelId += ChannelsToSet;
            pTargetData    += ChannelsToSet;
            // memcpy(&OutputBuffer[StartChannelId], pTargetData, ChannelCount);
        }
    } while (false);

    // DEBUG_END;
}  // ReadChannelData

// -----------------------------------------------------------------------------
void c_OutputMgr::ClearBuffer ()
{
    // DEBUG_START;

    for (auto & currentOutputChannelDriver : OutputChannelDrivers)
    {
        if (nullptr != currentOutputChannelDriver.pOutputChannelDriver)
        {
            currentOutputChannelDriver.pOutputChannelDriver->ClearBuffer ();
        }
    }

    // DEBUG_END;
}  // ClearBuffer

// create a global instance of the output channel factory
c_OutputMgr OutputMgr;
