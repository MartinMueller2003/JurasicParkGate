/*
 * GateAudio.cpp - Output Management class
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
 */

#include "JurasicParkGate.h"
#   include <soc/uart_reg.h>
#   include <driver/uart.h>
#   include <driver/gpio.h>

#include "GateAudio.hpp"

// -----------------------------------------------------------------------------
///< Start up the driver and put it into a safe mode
c_GateAudio::c_GateAudio ()
{}   // c_GateAudio

// -----------------------------------------------------------------------------
///< deallocate any resources and put the output channels into a safe state
c_GateAudio::~c_GateAudio ()
{
    DEBUG_START;

    DEBUG_END;
}  // ~c_GateAudio

// -----------------------------------------------------------------------------
///< Start the module
void c_GateAudio::Begin ()
{
    DEBUG_START;

    do  // once
    {
        ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2,
                             DEFAULT_UART_TX,
                             DEFAULT_UART_RX,
                             UART_PIN_NO_CHANGE,
                             UART_PIN_NO_CHANGE));
        Serial2.begin(9600, SERIAL_8N1, DEFAULT_UART_RX, DEFAULT_UART_TX, false);
        Player.setTimeOut(1000); //Set serial communictaion time out
        Player.begin(Serial2, false, false, true);
        uint8_t PlayerType = Player.readType();
        DEBUG_V(String("command: ") + String(Player._handleCommand, HEX));
        DEBUG_V(String("PlayerType: ") + String(PlayerType, HEX));
        if(DFPlayerCardOnline != PlayerType)
        {
            // readType() != DFPlayerCardOnline
            logcon(String(F("Failed to init the MP3 player. Error: ")) + String(Player.readType()));
            IsInstalled = false;
            break;
        }

        IsInstalled = true;

        Player.setTimeOut(500); //Set serial communictaion time out
          
        // Player.volume(10);  //Set volume value (0~30).
        Player.volume(10);  //Set volume value. From 0 to 30
        Player.EQ(DFPLAYER_EQ_NORMAL);
        Player.outputDevice(DFPLAYER_DEVICE_SD);

        NumFiles = Player.readFileCounts(DFPLAYER_DEVICE_SD);
        logcon(String(F("Player NumFiles: ")) + String(NumFiles));

    } while (false);

    DEBUG_END;
}  // begin

// -----------------------------------------------------------------------------
bool c_GateAudio::SetConfig (JsonObject & json)
{
    DEBUG_START;

    bool ConfigChanged = false;

    JsonObject jsonMp3 = json[CN_MP3];

    setFromJSON(randomize, jsonMp3, CN_randomize);

    DEBUG_END;

    return(ConfigChanged);
}  // SetConfig

// -----------------------------------------------------------------------------
void c_GateAudio::GetConfig (JsonObject & json)
{
    DEBUG_START;

    JsonObject jsonMp3 = json.createNestedObject(CN_MP3);
    jsonMp3[CN_randomize]  = randomize;

    DEBUG_END;
}  // GetConfig

// -----------------------------------------------------------------------------
void c_GateAudio::GetStatus (JsonObject & json)
{
    // _ DEBUG_START;
    JsonObject jsonMp3 = json.createNestedObject(CN_MP3);
    jsonMp3[F ("installed")] = IsInstalled;
    jsonMp3[F ("LastPlayerStatus")] = LastPlayerStatus;
    if(IsIdle())
    {
        jsonMp3[F ("playing")] = uint8_t(-1);
    }
    else
    {
        jsonMp3[F ("playing")] = FileNumberToPlay;
    }

    // _ DEBUG_END;
}  // GetConfig

// -----------------------------------------------------------------------------
void c_GateAudio::PlayIntro ()
{
    DEBUG_START;

    if(IsInstalled)
    {
        Player.advertise(1);
    }

    DEBUG_END;
}  // PlayIntro

// -----------------------------------------------------------------------------
void c_GateAudio::PlayCurrentSelection ()
{
    DEBUG_START;
    if(IsInstalled)
    {
        if(randomize)
        {
            FileNumberToPlay = uint8_t(random(NumFiles));
        }
        // Player.playFolder(1, FileNumberToPlay);
        FileNumberToPlay = 1;
        Player.play(FileNumberToPlay);
    }

    DEBUG_END;
}  // PlayCurrentSelection

// -----------------------------------------------------------------------------
void c_GateAudio::PausePlaying ()
{
    DEBUG_START;

    if(IsInstalled)
    {
        Player.pause(); 
    }

    DEBUG_END;
}  // PausePlaying

// -----------------------------------------------------------------------------
void c_GateAudio::StopPlaying ()
{
    DEBUG_START;

    if(IsInstalled)
    {
        Player.stop(); 
    }

    DEBUG_END;
}  // StopPlaying

// -----------------------------------------------------------------------------
void c_GateAudio::NextSong ()
{
    DEBUG_START;
    if(IsInstalled)
    {
        Player.next(); 
    }

    DEBUG_END;
}  // NextSong

// -----------------------------------------------------------------------------
void c_GateAudio::ResumePlaying ()
{
    DEBUG_START;

    if(IsInstalled)
    {
        Player.start();
    }

    DEBUG_END;
}  // ResumePlaying

// -----------------------------------------------------------------------------
bool c_GateAudio::IsIdle ()
{
    // _ DEBUG_START;

    if(IsInstalled)
    {
        int newPlayerStatus = Player.readState();
        printDetail(Player.readType(), Player.read()); //Print the detail message from DFPlayer to handle different errors and states.
        if(-1 != newPlayerStatus) // ignore failed status reads
        {
            if(LastPlayerStatus != newPlayerStatus)
            {
                DEBUG_V(String("newPlayerStatus: ") + String(newPlayerStatus));
            }
            LastPlayerStatus = newPlayerStatus;
        }
    }
    else
    {
        // show we are idle
        LastPlayerStatus = 0; 
    }

    // _ DEBUG_END;

    return 0 == LastPlayerStatus;
}  // ResumePlaying

// -----------------------------------------------------------------------------
void c_GateAudio::printDetail(uint8_t type, int value)
{
    // DEBUG_START;
  switch (type)
  {
    case TimeOut:
      logcon(F("Time Out!"));
      break;
    case WrongStack:
      logcon(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      logcon(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      logcon(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      logcon(F("Card Online!"));
      break;
    case DFPlayerUSBInserted:
      logcon("USB Inserted!");
      break;
    case DFPlayerUSBRemoved:
      logcon("USB Removed!");
      break;
    case DFPlayerPlayFinished:
      logcon(String(F("Number:")) + String(value));
      logcon(F(" Play Finished!"));
      break;
    case DFPlayerFeedBack:
      logcon(F(" Player Feedback!"));
      logcon(String(F("value: ")) + String(value));
      break;
    case DFPlayerError:
      logcon(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          logcon(F("Card not found"));
          break;
        case Sleeping:
          logcon(F("Sleeping"));
          break;
        case SerialWrongStack:
          logcon(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          logcon(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          logcon(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          logcon(F("Cannot Find File"));
          break;
        case Advertise:
          logcon(F("In Advertise"));
          break;
        default:
          logcon(String(F("type: value")) + String(value));
          break;
      }
      break;
    default:
          logcon(String(F(" type: ")) + String(type));
          logcon(String(F("value: ")) + String(value));
      break;
  }
  // DEBUG_END;
}

// create a global instance of the Gate Audio
c_GateAudio GateAudio;
