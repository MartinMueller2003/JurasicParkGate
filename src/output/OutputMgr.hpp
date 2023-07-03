#pragma once
/*
 * OutputMgr.hpp - Output Management class
 *
 * Project: JurasicParkGate - An ESP8266 / ESP32 and E1.31 based pixel driver
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

#include "memdebug.h"
#include "FileMgr.hpp"

class c_OutputCommon;  ///< forward declaration to the pure virtual output class that will be defined later.

#ifdef UART_LAST
    #define NUM_UARTS UART_LAST
#else // ifdef UART_LAST
    #define NUM_UARTS 0
#endif // ifdef UART_LAST

class c_OutputMgr{
public:
c_OutputMgr ();
virtual~c_OutputMgr ();

void Begin             ();                                  ///< set up the operating environment based on the current config (or defaults)
void Poll            ();                                    ///< Call from loop(),  renders output data
void LoadConfig        ();                                  ///< Read the current configuration data from nvram
void GetConfig         (byte*   Response,
 uint32_t                       maxlen);
void GetConfig         (String & Response);
void SetConfig         (const char* NewConfig);                             ///< Save the current configuration data to nvram
void SetConfig         (ArduinoJson::JsonDocument & NewConfig);             ///< Save the current configuration data to nvram
void GetStatus         (JsonObject & jsonStatus);
void GetPortCounts     (uint16_t & PixelCount, uint16_t & SerialCount)
{
    PixelCount = uint16_t (OutputChannelId_End);
    SerialCount = uint16_t (NUM_UARTS);
}
uint8_t* GetBufferAddress  ()
{
    return(OutputBuffer);
}                                                                                                               ///< Get the address of the buffer into which the E1.31 handler will stuff data
uint32_t GetBufferUsedSize ()
{
    return(UsedBufferSize);
}                                                                                                               ///< Get the size (in intensities) of the buffer into which the E1.31 handler will
                                                                                                                ///<    stuff data
uint32_t GetBufferSize     ()
{
    return( sizeof (OutputBuffer) );
}                                                                 ///< Get the size (in intensities) of the buffer into which the E1.31 handler will
                                                                  ///<    stuff data
void DeleteConfig      ()
{
    FileMgr.DeleteConfigFile (ConfigFileName);
}
void PauseOutputs      (bool NewState);
void GetDriverName     (String & Name)
{
    Name = "OutputMgr";
}
void WriteChannelData  (uint32_t    StartChannelId,
 uint32_t                           ChannelCount,
 uint8_t*                           pData);
void ReadChannelData   (uint32_t    StartChannelId,
 uint32_t                           ChannelCount,
 uint8_t*                           pTargetData);
void ClearBuffer       ();

// handles to determine which output channel we are dealing with
enum e_OutputChannelIds
{
    OutputChannelId_Relay_1,
    OutputChannelId_Relay_2,

    OutputChannelId_End,      // must be last in the list
    OutputChannelId_Start = 0
};

// do NOT insert into the middle of this list. Always add new types to the end of the list
enum e_OutputType
{
    OutputType_Disabled = 0,
    OutputType_Servo_PCA9685,

    // Add new types here
    OutputType_End, // must be last
    OutputType_Start = OutputType_Disabled
};

    #define OM_MAX_NUM_CHANNELS (16 * 2)
    #define OM_MAX_CONFIG_SIZE  ( (uint32_t)(20 * 1024) )

enum OM_PortType_t
{
    Uart = 0,
    Rmt,
    Spi,
    Relay,
    Undefined
};

private:
struct DriverInfo_t
{
    uint32_t OutputBufferStartingOffset = 0;
    uint32_t OutputBufferDataSize       = 0;
    uint32_t OutputBufferEndOffset      = 0;

    uint32_t OutputChannelStartingOffset = 0;
    uint32_t OutputChannelSize           = 0;
    uint32_t OutputChannelEndOffset      = 0;

    gpio_num_t GpioPin              = gpio_num_t (-1);
    OM_PortType_t PortType             = OM_PortType_t::Undefined;
    uart_port_t PortId               = uart_port_t (-1);
    e_OutputChannelIds DriverId             = e_OutputChannelIds (-1);
    c_OutputCommon* pOutputChannelDriver = nullptr;
};

// pointer(s) to the current active output drivers
DriverInfo_t OutputChannelDrivers[OutputChannelId_End];

// configuration parameter names for the channel manager within the config file

bool HasBeenInitialized = false;
bool ConfigLoadNeeded   = false;
bool IsOutputPaused     = false;
bool BuildingNewConfig  = false;

bool ProcessJsonConfig (JsonObject & jsonConfig);
void CreateJsonConfig  (JsonObject & jsonConfig);
void UpdateDisplayBufferReferences (void);
void InstantiateNewOutputChannel (DriverInfo_t &    ChannelIndex,
 e_OutputType                                       NewChannelType,
 bool                                               StartDriver = true);
void CreateNewConfig ();

String ConfigFileName;

uint8_t OutputBuffer[OM_MAX_NUM_CHANNELS];
uint32_t UsedBufferSize      = 0;
gpio_num_t ConsoleTxGpio       = gpio_num_t::GPIO_NUM_1;
gpio_num_t ConsoleRxGpio       = gpio_num_t::GPIO_NUM_3;
bool ConsoleUartIsActive = true;
}; // c_OutputMgr

extern c_OutputMgr OutputMgr;
