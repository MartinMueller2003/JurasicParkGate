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

#include "JurasicParkGate.h"
#include <Int64String.h>

#include "GateDoors.hpp"

// -----------------------------------------------------------------------------
///< Start up the driver and put it into a safe mode
c_GateDoors::c_GateDoors ()
{}   // c_GateDoors

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

    // DEBUG_END;

    return(ConfigChanged);
}  // SetConfig

// -----------------------------------------------------------------------------
void c_GateDoors::GetConfig (JsonObject & json)
{
    // DEBUG_START;

    // json[CN_miso_pin]  = miso_pin;

    // DEBUG_END;
}  // GetConfig

// -----------------------------------------------------------------------------
void c_GateDoors::GetStatus (JsonObject & json)
{
    // DEBUG_START;

//        json[F ("size")] = LittleFS.totalBytes ();

    // DEBUG_END;
}  // GetConfig

// -----------------------------------------------------------------------------
void c_GateDoors::Open ()
{
    // DEBUG_START;

    // DEBUG_END;
}  // Open

// -----------------------------------------------------------------------------
void c_GateDoors::Close ()
{
    // DEBUG_START;


    // DEBUG_END;
}  // Close

// create a global instance of the Gate Doors
c_GateDoors GateDoors;
