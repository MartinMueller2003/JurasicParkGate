/*
 * InputButtons.cpp
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
#include "InputButtons.hpp"
#include "FileMgr.hpp"
#include "InputMgr.hpp"

/*****************************************************************************/
/*	Global Data                                                              */
/*****************************************************************************/
static String DefaultButtonNames[]
{
    {"Open/Close"},  //
    {"Lights"},
    {"Pause/Play"},
    {"Skip"},
    {"Stop"},
};

/*****************************************************************************/
/* Code                                                                      */
/*****************************************************************************/

c_InputButtons::c_InputButtons () :
    c_InputCommon (c_InputMgr::e_InputChannelIds::InputChannelId_Start, c_InputMgr::e_InputType::InputType_Buttons, 32)
{
} // c_InputButtons

/*****************************************************************************/
void c_InputButtons::SetParms (c_InputMgr::e_InputChannelIds   NewInputChannelId,
 c_InputMgr::e_InputType                                        NewChannelType,
 uint32_t                                                       NewBufferSize)
 {
    // DEBUG_START;

    // DEBUG_V(String("NewInputChannelId: ") + String(int(NewInputChannelId)));
    // DEBUG_V(String("   NewChannelType: ") + String(int(NewChannelType)));

    InputChannelId = NewInputChannelId;
    ChannelType = NewChannelType;
    InputDataBufferSize = NewBufferSize;

    uint32_t index = 0;
    for (auto & CurrentButton : Buttons)
    {
        CurrentButton.SetName (DefaultButtonNames[index++]);
    }

    // DEBUG_END;
}  // SetParms

/*****************************************************************************/
void c_InputButtons::Begin ()
{
    // DEBUG_START;

    // DEBUG_V ("InputDataBufferSize: " + String(InputDataBufferSize));

    for (auto & CurrentButton : Buttons)
    {
        // send the config to the button
        CurrentButton.Begin ();
    }

    HasBeenInitialized = true;

    // DEBUG_END;
} // c_InputButtons::Begin

/*****************************************************************************/
void c_InputButtons::GetConfig (JsonObject & JsonData)
{
    // DEBUG_START;

    // make sure an array exists
    if ( false == JsonData.containsKey (CN_buttons) )
    {
        DEBUG_V ("Create Button array");
        JsonData.createNestedArray (CN_buttons);
    }

    JsonArray InputButtonArray = JsonData[CN_buttons];

    // remove the existing array
    InputButtonArray.clear ();

    // DEBUG_V ("");

    uint32_t index = 0;
    for (auto & CurrentButton : Buttons)
    {
        // Create an array entry
        JsonObject CurrentJsonData = InputButtonArray.createNestedObject ();

        CurrentJsonData[CN_device] = index++;

        // Get the config from the button
        CurrentButton.GetConfig (CurrentJsonData);
    }

    // DEBUG_END;
}  // GetConfig

/*****************************************************************************/
void c_InputButtons::GetStatus (JsonObject & JsonData)
{
    // DEBUG_START;

    JsonArray   InputStatus = JsonData.createNestedArray (CN_buttons);

    uint32_t    index = 0;
    for (auto & CurrentButton : Buttons)
    {
        JsonObject channelStatus = InputStatus.createNestedObject ();
        channelStatus[CN_device] = index++;
        CurrentButton.GetStatus (channelStatus);
    }

    // DEBUG_END;
}  // GetStatus

// -----------------------------------------------------------------------------
void c_InputButtons::SetBufferInfo (uint32_t BufferSize)
{
    // DEBUG_START;

    InputDataBufferSize = BufferSize;

    // DEBUG_V (String ("InputDataBufferSize: ") + String (InputDataBufferSize));

    // DEBUG_END;
}  // SetBufferInfo

/*****************************************************************************/
bool c_InputButtons::SetConfig (JsonObject & JsonData)
{
    // DEBUG_START;

    if ( false == JsonData.containsKey (CN_buttons) )
    {
        DEBUG_V ("Create Button array");
        JsonData.createNestedArray (CN_buttons);
    }

    JsonArray InputButtonArray = JsonData[CN_buttons];

    for (JsonObject CurrentButtonJsonData : InputButtonArray)
    {
        if ( false == CurrentButtonJsonData.containsKey (CN_device) )
        {
            logcon ( F ("Missing ID in button config. Skipping entry") );
            continue;
        }

        uint32_t index = CurrentButtonJsonData[CN_device];

        if (index >= NumButtons)
        {
            logcon ( String ( F ("Invalid button array entry ID: ") ) + String (index) );
            continue;
        }

        // send the config to the button
        Buttons[index].SetConfig (CurrentButtonJsonData);
    }

    // DEBUG_END;
    return(true);
}  // ProcessConfig

/*****************************************************************************/
void c_InputButtons::Process (void)
{
    // _ DEBUG_START;

    for (auto & CurrentButton : Buttons)
    {
        CurrentButton.Process ();
    }

    // _ DEBUG_END;
}  // Poll

/*****************************************************************************/
void c_InputButtons::NetworkStateChanged (bool IsConnected)
{
}                                                               // NetworkStateChanged

c_InputButtons InputButtons;
