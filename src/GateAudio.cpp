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
#include <Int64String.h>

#include "GateAudio.hpp"

// -----------------------------------------------------------------------------
///< Start up the driver and put it into a safe mode
c_GateAudio::c_GateAudio ()
{}   // c_GateAudio

// -----------------------------------------------------------------------------
///< deallocate any resources and put the output channels into a safe state
c_GateAudio::~c_GateAudio ()
{
    // DEBUG_START;

    // DEBUG_END;
}  // ~c_GateAudio

// -----------------------------------------------------------------------------
///< Start the module
void c_GateAudio::Begin ()
{
    // DEBUG_START;

    do  // once
    {
        if(!Player.begin(Serial1))
        {
            logcon(F("Failed to init the MP3 player"));
            IsInstalled = false;
        }

        IsInstalled = true;

        Player.setTimeOut(500); //Set serial communictaion time out 500ms
          
        // Player.volume(10);  //Set volume value (0~30).
        Player.EQ(DFPLAYER_EQ_NORMAL);
        Player.outputDevice(DFPLAYER_DEVICE_SD);

    } while (false);

    // DEBUG_END;
}  // begin

// -----------------------------------------------------------------------------
bool c_GateAudio::SetConfig (JsonObject & json)
{
    // DEBUG_START;

    bool ConfigChanged = false;

    JsonObject jsonMp3 = json[CN_MP3];

    bool temp;
    setFromJSON(temp, jsonMp3, F("hmm"));

    // DEBUG_END;

    return(ConfigChanged);
}  // SetConfig

// -----------------------------------------------------------------------------
void c_GateAudio::GetConfig (JsonObject & json)
{
    // DEBUG_START;

    JsonObject jsonMp3 = json.createNestedObject(CN_MP3);
    jsonMp3[F("hmm")]  = true;

    // DEBUG_END;
}  // GetConfig

// -----------------------------------------------------------------------------
void c_GateAudio::GetStatus (JsonObject & json)
{
    // DEBUG_START;
    JsonObject jsonMp3 = json.createNestedObject(CN_MP3);
    jsonMp3[F ("installed")] = IsInstalled;

    // DEBUG_END;
}  // GetConfig

// -----------------------------------------------------------------------------
void c_GateAudio::PlayIntro ()
{
    // DEBUG_START;

    if(IsInstalled)
    {
        Player.advertise(1);
    }

    // DEBUG_END;
}  // PlayIntro

// -----------------------------------------------------------------------------
void c_GateAudio::PlayCurrentSelection ()
{
    // DEBUG_START;
    if(IsInstalled)
    {
        long maxFilesCount = Player.readFileCountsInFolder(1);
        Player.play(random(maxFilesCount));
    }

    // DEBUG_END;
}  // PlayCurrentSelection

// -----------------------------------------------------------------------------
void c_GateAudio::PausePlaying ()
{
    // DEBUG_START;

    if(IsInstalled)
    {
        Player.pause(); 
    }

    // DEBUG_END;
}  // PausePlaying

// -----------------------------------------------------------------------------
void c_GateAudio::StopPlaying ()
{
    // DEBUG_START;

    // DEBUG_START;
    if(IsInstalled)
    {
        Player.stop(); 
    }

    // DEBUG_END;
}  // StopPlaying

// -----------------------------------------------------------------------------
void c_GateAudio::NextSong ()
{
    // DEBUG_START;
    if(IsInstalled)
    {
        Player.next(); 
    }

    // DEBUG_END;
}  // NextSong

// -----------------------------------------------------------------------------
void c_GateAudio::ResumePlaying ()
{
    // DEBUG_START;

    if(IsInstalled)
    {
        Player.start();
    }

    // DEBUG_END;
}  // ResumePlaying

// create a global instance of the Gate Audio
c_GateAudio GateAudio;
