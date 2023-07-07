/*
 * InputDisabled.cpp - InputDisabled NULL driver code for JurasicParkGate
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

#include "InputDisabled.hpp"
#include "InputCommon.hpp"

/*
 */
// ----------------------------------------------------------------------------
c_InputDisabled::c_InputDisabled (c_InputMgr::e_InputChannelIds NewInputChannelId,
 c_InputMgr::e_InputType                                        NewChannelType,
 uint32_t                                                       BufferSize) :
    c_InputCommon (NewInputChannelId, NewChannelType, BufferSize)
{
    // DEBUG_START;

    // DEBUG_END;
}  // c_InputDisabled

// ----------------------------------------------------------------------------
c_InputDisabled::~c_InputDisabled ()
{
    // DEBUG_START;

    // DEBUG_END;
}  // ~c_InputDisabled

// ----------------------------------------------------------------------------
// Use the current config to set up the Input port
void c_InputDisabled::Begin ()
{
    // DEBUG_START;

    // DEBUG_END;
}  // begin

// -----------------------------------------------------------------------------
void c_InputDisabled::GetStatus (JsonObject & jsonStatus)
{
    // _ DEBUG_START;

    JsonObject Status = jsonStatus.createNestedObject ( F ("disabled") );
    Status[CN_id] = InputChannelId;

    // _ DEBUG_END;
}  // GetStatus

// ----------------------------------------------------------------------------
/* Process the config
 *
 *   needs
 *       reference to string to process
 *   returns
 *       true - config has been accepted
 *       false - Config rejected. Using defaults for invalid settings
 */
bool c_InputDisabled::SetConfig (ArduinoJson::JsonObject & /* jsonConfig */)
{
    // DEBUG_START;

    // DEBUG_END;
    return(true);
}  // SetConfig

// ----------------------------------------------------------------------------
void c_InputDisabled::GetConfig (ArduinoJson::JsonObject & jsonConfig)
{
    // DEBUG_START;

    // DEBUG_END;
}  // GetConfig

// ----------------------------------------------------------------------------
void c_InputDisabled::Process ()
{
    // _ DEBUG_START;

    // _ DEBUG_END;
}  // Render
