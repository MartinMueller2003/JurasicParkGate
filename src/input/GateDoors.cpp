/*
 * GateDoors.cpp - Output Management class
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

#include "GateDoors.hpp"
#include "OutputMgr.hpp"

    FsmDoorStateBooting FsmDoorStateBooting_Imp;
    FsmDoorStateClosed  FsmDoorStateClosed_Imp;
    FsmDoorStateOpening FsmDoorStateOpening_Imp;
    FsmDoorStateOpen    FsmDoorStateOpen_Imp;
    FsmDoorStateClosing FsmDoorStateClosing_Imp;

// -----------------------------------------------------------------------------
///< Start up the driver and put it into a safe mode
c_GateDoors::c_GateDoors ()
{
    FsmDoorStateBooting_Imp.init(this);
}   // c_GateDoors

// -----------------------------------------------------------------------------
///< deallocate any resources and put the output channels into a safe state
c_GateDoors::~c_GateDoors ()
{
    // DEBUG_START;

    // DEBUG_END;
}  // ~c_GateDoors

// -----------------------------------------------------------------------------
///< Start the module
void c_GateDoors::Begin ()
{
    // DEBUG_START;

    do  // once
    {

    } while (false);

    // DEBUG_END;
}  // begin

// -----------------------------------------------------------------------------
bool c_GateDoors::SetConfig (JsonObject & json)
{
    // DEBUG_START;

    bool ConfigChanged = false;

    do // once
    {
        if(!json.containsKey(CN_doors))
        {
            logcon(F("No Door configuration. Using defaults"));
            break;
        }
        JsonObject jsonDoors = json[CN_doors];

        uint32_t TimeToOpenSec = TimeToOpenMS / 1000;
        ConfigChanged |= setFromJSON(TimeToOpenSec,  jsonDoors, CN_open);
        TimeToOpenMS = TimeToOpenSec * 1000;

        uint32_t TimeToCloseSec = TimeToCloseMS / 1000;
        ConfigChanged |= setFromJSON(TimeToCloseSec, jsonDoors, CN_close);
        TimeToCloseMS = TimeToCloseSec * 1000;

        if(!jsonDoors.containsKey(CN_channels))
        {
            logcon(F("No Door Channel configuration. Using defaults"));
            break;
        }
        JsonArray jsonDoorChannels = jsonDoors[CN_channels];

        for(auto CurrentChannel : jsonDoorChannels)
        {
            uint index = uint(-1);
            setFromJSON(index, jsonDoors, CN_id);

            if(index >= sizeof(doorChannels))
            {
                logcon (F("Invalid door channel index. skipping"));
                continue;
            }

            ConfigChanged |= setFromJSON(doorChannels[index], jsonDoors, CN_channel);
        }

    } while(false);

    // DEBUG_END;
    return(ConfigChanged);
}  // SetConfig

// -----------------------------------------------------------------------------
void c_GateDoors::GetConfig (JsonObject & json)
{
    // DEBUG_START;

    JsonObject jsonDoors = json.createNestedObject(CN_doors);

    jsonDoors[CN_open]  = TimeToOpenMS / 1000;
    jsonDoors[CN_close] = TimeToCloseMS / 1000;

    JsonArray jsonDoorChannels = jsonDoors.createNestedArray(CN_channels);
    uint index = 0;
    for(auto CurrentChannel : doorChannels)
    {
        JsonObject CurrentJsonDoorChannel = jsonDoorChannels.createNestedObject();
        CurrentJsonDoorChannel[CN_id] = index++;
        CurrentJsonDoorChannel[CN_channel] = uint8_t(CurrentChannel);
    }

    // DEBUG_END;
}  // GetConfig

// -----------------------------------------------------------------------------
void c_GateDoors::GetStatus (JsonObject & json)
{
    // DEBUG_START;

    JsonObject jsonDoors = json.createNestedObject(CN_doors);
    jsonDoors[CN_state] = CurrentFsmState->name();
    jsonDoors[CN_channel] = CurrentPosition;
    jsonDoors[CN_time] = millis() - TimeStartedMS;

    // DEBUG_END;
}  // GetConfig

// -----------------------------------------------------------------------------
void c_GateDoors::Poll ()
{
    // _ DEBUG_START;

    CurrentFsmState->poll(this);

    // _ DEBUG_END;
}  // Open

// -----------------------------------------------------------------------------
void c_GateDoors::Open ()
{
    // DEBUG_START;

    CurrentFsmState->Open(this);

    // DEBUG_END;
}  // Open

// -----------------------------------------------------------------------------
void c_GateDoors::Close ()
{
    // DEBUG_START;

    CurrentFsmState->Close(this);

    // DEBUG_END;
}  // Close

// -----------------------------------------------------------------------------
void c_GateDoors::SetChannelData(uint8_t value)
{
    // _ DEBUG_START;

    // _ DEBUG_V(String("value: ") + String(value));

    uint8_t buffer = value;
    for(auto CurrentChannel : doorChannels)
    {
        OutputMgr.WriteChannelData(CurrentChannel, 1, &buffer);
    }

    // _ DEBUG_END;
} // SetChannelData

// -----------------------------------------------------------------------------
bool c_GateDoors::IsOpen()
{
    return (CurrentFsmState == &FsmDoorStateOpen_Imp);

} // IsOpen

// -----------------------------------------------------------------------------
bool c_GateDoors::IsClosed()
{
    return (CurrentFsmState == &FsmDoorStateClosed_Imp);

} // IsClosed

// -----------------------------------------------------------------------------
// ------------------ FSM Definitions ------------------------------------------
// -----------------------------------------------------------------------------
void FsmDoorStateBooting::init(c_GateDoors * pParent)
{
    // DEBUG_START;

    pParent->CurrentFsmState = this;
    
    // DEBUG_END;
} // FsmDoorStateBooting::init

// -----------------------------------------------------------------------------
void FsmDoorStateBooting::poll(c_GateDoors * pParent)
{
    // _ DEBUG_START;

    FsmDoorStateClosed_Imp.init(pParent);

    // _ DEBUG_END;
} // FsmDoorStateBooting::init

// -----------------------------------------------------------------------------
void FsmDoorStateClosed::init(c_GateDoors * pParent)
{
    // DEBUG_START;

    pParent->CurrentFsmState = this;

    // write the closed door value to the input channels
    pParent->CurrentPosition = CLOSED_VALUE;
    pParent->SetChannelData(pParent->CurrentPosition);
    
    // DEBUG_END;
} // FsmDoorStateClosed::init

// -----------------------------------------------------------------------------
void FsmDoorStateClosed::poll(c_GateDoors * pParent)
{
    // _ DEBUG_START;

    // Nothing to do.

    // _ DEBUG_END;
} // FsmDoorStateClosed::poll

// -----------------------------------------------------------------------------
void FsmDoorStateClosed::Open(c_GateDoors * pParent)
{
    // DEBUG_START;

    FsmDoorStateOpening_Imp.init(pParent);

    // DEBUG_END;
} // FsmDoorStateClosed::Open

// -----------------------------------------------------------------------------
void FsmDoorStateOpening::init(c_GateDoors * pParent)
{
    // DEBUG_START;

    pParent->CurrentFsmState = this;

    // Determine time needed to move door from current postion to full open
    uint32_t TimeRemainingMS = map(pParent->CurrentPosition, CLOSED_VALUE, FULL_OPEN_VALUE, pParent->TimeToOpenMS, 0);
    pParent->TimeStartedMS = millis() - (TimeRemainingMS - pParent->TimeToOpenMS);
    // DEBUG_V(String("            now: ") + String(millis()));
    // DEBUG_V(String("  TimeStartedMS: ") + String(pParent->TimeStartedMS));
    // DEBUG_V(String("TimeRemainingMS: ") + String(TimeRemainingMS));
    
    // DEBUG_END;
} // FsmDoorStateOpening::init

// -----------------------------------------------------------------------------
void FsmDoorStateOpening::poll(c_GateDoors * pParent)
{
    // _ DEBUG_START;

    // Update the current door position
    uint32_t timeElapsed = min((uint32_t(millis()) - pParent->TimeStartedMS), pParent->TimeToOpenMS);
    pParent->CurrentPosition = map(timeElapsed, 0, pParent->TimeToOpenMS, CLOSED_VALUE, FULL_OPEN_VALUE);

    // _ DEBUG_V(String("            now: ") + String(millis()));
    // _ DEBUG_V(String("   TimeToOpenMS: ") + String(pParent->TimeToOpenMS));
    // _ DEBUG_V(String("    timeElapsed: ") + String(timeElapsed));
    // _ DEBUG_V(String("CurrentPosition: ") + String(pParent->CurrentPosition));

    // are we done?
    if(timeElapsed >= pParent->TimeToOpenMS)
    {
        FsmDoorStateOpen_Imp.init(pParent);
    }
    else
    {
        pParent->SetChannelData(pParent->CurrentPosition);
    }

    // _ DEBUG_END;
} // FsmDoorStateOpening::poll

// -----------------------------------------------------------------------------
void FsmDoorStateOpening::Open(c_GateDoors * pParent)
{
    // DEBUG_START;

    // go to full open
    FsmDoorStateOpen_Imp.init(pParent);

    // DEBUG_END;
} // FsmDoorStateOpening::Open

// -----------------------------------------------------------------------------
void FsmDoorStateOpening::Close(c_GateDoors * pParent)
{
    // DEBUG_START;

    // start colsing the door
    FsmDoorStateClosing_Imp.init(pParent);

    // DEBUG_END;
} // FsmDoorStateOpening::Close

// -----------------------------------------------------------------------------
void FsmDoorStateOpen::init(c_GateDoors * pParent)
{
    // DEBUG_START;

    pParent->CurrentFsmState = this;

    // output the full open value to the door channels
    pParent->CurrentPosition = FULL_OPEN_VALUE;
    pParent->SetChannelData(pParent->CurrentPosition);

    // DEBUG_END;
} // FsmDoorStateOpen::init

// -----------------------------------------------------------------------------
void FsmDoorStateOpen::poll(c_GateDoors * pParent)
{
    // _ DEBUG_START;

    // Nothing to do

    // _ DEBUG_END;
} // FsmDoorStateOpen::poll

// -----------------------------------------------------------------------------
void FsmDoorStateOpen::Close(c_GateDoors * pParent)
{
    // DEBUG_START;

    // Start closing the door
    FsmDoorStateClosing_Imp.init(pParent);

    // DEBUG_END;
} // FsmDoorStateOpen::Close

// -----------------------------------------------------------------------------
void FsmDoorStateClosing::init(c_GateDoors * pParent)
{
    // DEBUG_START;

    pParent->CurrentFsmState = this;

    // Determine time needed to move door from current postion to full closed
    uint32_t TimeRemainingMS = map(long(pParent->CurrentPosition), CLOSED_VALUE, FULL_OPEN_VALUE, 0, pParent->TimeToCloseMS);
    pParent->TimeStartedMS = millis() + (TimeRemainingMS - pParent->TimeToCloseMS);
    // DEBUG_V(String("CurrentPosition: ") + String(pParent->CurrentPosition));
    // DEBUG_V(String("            now: ") + String(millis()));
    // DEBUG_V(String("  TimeStartedMS: ") + String(pParent->TimeStartedMS));
    // DEBUG_V(String("TimeRemainingMS: ") + String(TimeRemainingMS));

    // DEBUG_END;
} // FsmDoorStateClosing::init

// -----------------------------------------------------------------------------
void FsmDoorStateClosing::poll(c_GateDoors * pParent)
{
    // _ DEBUG_START;

    // output current door position
    uint32_t timeElapsed = min((uint32_t(millis()) - pParent->TimeStartedMS), pParent->TimeToCloseMS);
    pParent-> CurrentPosition = map(timeElapsed, 0, pParent->TimeToCloseMS, FULL_OPEN_VALUE, CLOSED_VALUE);

    // _ DEBUG_V(String("            now: ") + String(millis()));
    // _ DEBUG_V(String("  TimeToCloseMS: ") + String(pParent->TimeToCloseMS));
    // _ DEBUG_V(String("    timeElapsed: ") + String(timeElapsed));
    // _ DEBUG_V(String("CurrentPosition: ") + String(pParent->CurrentPosition));

    // are we done?
    if(timeElapsed >= pParent->TimeToCloseMS)
    {
        FsmDoorStateClosed_Imp.init(pParent);
    }
    else
    {
        pParent->SetChannelData(pParent->CurrentPosition);
    }

    // _ DEBUG_END;
} // FsmDoorStateClosing::poll

// -----------------------------------------------------------------------------
void FsmDoorStateClosing::Open(c_GateDoors * pParent)
{
    // DEBUG_START;

    // start opening the door again
    FsmDoorStateOpening_Imp.init(pParent);

    // DEBUG_END;
} // FsmDoorStateClosing::Open

// -----------------------------------------------------------------------------
void FsmDoorStateClosing::Close(c_GateDoors * pParent)
{
    // DEBUG_START;

    // go to full Closed
    FsmDoorStateClosed_Imp.init(pParent);

    // DEBUG_END;
} // FsmDoorStateClosing::Close

// create a global instance of the Gate Doors
c_GateDoors GateDoors;
