#pragma once
/*
 * InputButtons.hpp
 *
 * Project: JurasicParkGate
 * Copyright (c) 2023 Martin Mueller
 * http://www.MartinMueller2003.com
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
#include "InputCommon.hpp"
#include "InputButton.hpp"
#include "FileMgr.hpp"

/*****************************************************************************/
class c_InputButtons : public c_InputCommon {
public:

c_InputButtons ();
void SetParms (c_InputMgr::e_InputChannelIds    NewInputChannelId,
 c_InputMgr::e_InputType                        NewChannelType,
 uint32_t                                       BufferSize);

enum InputValue_t
{
    off = 0,            // input is off
    shortOn,            // input was on for <N MS
    longOn              // input was on for <N MS
};

void Begin ();                                      ///< set up the operating environment based on the current config (or defaults)
bool SetConfig (JsonObject & jsonConfig);           ///< Set a new config in the driver
void GetConfig (JsonObject & jsonConfig);           ///< Get the current config used by the driver
void GetStatus (JsonObject & jsonStatus);
void Process ();                                    ///< Call from loop(),  renders Input data
void GetDriverName (String & sDriverName) { sDriverName = F ("Buttons"); }                                                   ///< get the name for the instantiated driver
void SetBufferInfo (uint32_t BufferSize);
void NetworkStateChanged (bool IsConnected);        // used by poorly designed rx functions
void RegisterButtonHandler(uint32_t ButtonId, void (*callback)(void*), void* context) {Buttons[ButtonId].RegisterButtonHandler(callback, context);}

    #define NumButtons 5

protected:

c_InputButton Buttons[NumButtons];
}; // c_InputButtons

extern c_InputButtons InputButtons;
