/*
  * InputButton.cpp
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
#include "InputButton.hpp"

/*****************************************************************************/
/*	Global Data                                                              */
/*****************************************************************************/

/*****************************************************************************/
/*	fsm                                                                      */
/*****************************************************************************/

fsm_InputButton_boot fsm_InputButton_boot_imp;
fsm_InputButton_off_state fsm_InputButton_off_state_imp;
fsm_InputButton_on_wait_long_state fsm_InputButton_on_wait_long_state_imp;
fsm_InputButton_wait_for_off_state fsm_InputButton_wait_for_off_state_imp;

/*****************************************************************************/
/* Code                                                                      */
/*****************************************************************************/
c_InputButton::c_InputButton ()
{
    // DEBUG_START;
    fsm_InputButton_boot_imp.Init (*this);  // currently redundant, but might Init() might do more ... so important to leave this

    // DEBUG_END;
}  // c_InputButton

/*****************************************************************************/
void c_InputButton::Begin ()
{
    // DEBUG_START;

    // DEBUG_V(String("Name: ") + Name);

    // DEBUG_END;
}

/*****************************************************************************/
void c_InputButton::GetConfig (JsonObject & JsonData)
{
    // DEBUG_START;

    JsonData[M_IO_ENABLED] = Enabled;
    JsonData[M_NAME]       = Name;
    JsonData[M_ID]         = GpioId;
    JsonData[M_POLARITY]   = (Polarity_t::ActiveHigh == polarity) ? CN_ActiveHigh : CN_ActiveLow;
    JsonData[CN_channels]  = TriggerChannel;
    JsonData[CN_long]      = LongPushDelayMS;

    // DEBUG_V (String ("GpioId: ") + String (GpioId));

    // DEBUG_END;
}  // GetConfig

/*****************************************************************************/
void c_InputButton::GetStatus (JsonObject & JsonData)
{
    // DEBUG_START;

    JsonData[M_NAME]  = Name;
    JsonData[M_ID]    = GpioId;
    JsonData[M_STATE] = ( ReadInput () ) ? CN_on : CN_off;

    // DEBUG_END;
}  // GetStatistics

/*****************************************************************************/
bool c_InputButton::SetConfig (JsonObject & JsonData)
{
    // DEBUG_START;

    String Polarity = (ActiveHigh == polarity) ? CN_ActiveHigh : CN_ActiveLow;

    uint32_t oldInputId = GpioId;

    setFromJSON (   Enabled,            JsonData,   M_IO_ENABLED);
    setFromJSON (   Name,               JsonData,   M_NAME);
    setFromJSON (   GpioId,             JsonData,   M_ID);
    setFromJSON (   Polarity,           JsonData,   M_POLARITY);
    setFromJSON (   TriggerChannel,     JsonData,   CN_channels);
    setFromJSON (   LongPushDelayMS,    JsonData,   CN_long);

    polarity = (String (CN_ActiveHigh) == Polarity) ? ActiveHigh : ActiveLow;

    if ( (oldInputId != GpioId) || (false == Enabled) )
    {
        pinMode (   oldInputId, INPUT);
        pinMode (   GpioId,     INPUT_PULLUP);
        fsm_InputButton_boot_imp.Init (*this);
    }

    // DEBUG_V (String ("GpioId: ") + String (GpioId));

    // DEBUG_END;
    return true;
}  // ProcessConfig

/*****************************************************************************/
void c_InputButton::Process (void)
{
    // _ DEBUG_START;

    CurrentFsmState->Poll (*this);

    // _ DEBUG_END;
}  // Poll

/*****************************************************************************/
bool c_InputButton::ReadInput (void)
{
    // read the input
    bool bInputValue = digitalRead (GpioId);

    // do we need to invert the input?
    if (Polarity_t::ActiveLow == polarity)
    {
        // invert the input value
        bInputValue = !bInputValue;
    }

    // DEBUG_V (String ("GpioId:    ") + String (GpioId));
    // DEBUG_V (String ("bInputValue: ") + String (bInputValue));
    return bInputValue;
}  // ReadInput

// -----------------------------------------------------------------------------
void c_InputButton::NetworkStateChanged (bool IsConnected) {
}                                                              // NetworkStateChanged

/*****************************************************************************/
/*	FSM                                                                      */
/*****************************************************************************/

/*****************************************************************************/
// waiting for the system to come up
void fsm_InputButton_boot::Init (c_InputButton & pInputButton)
{
    // DEBUG_START;

    // DEBUG_V ("Entring BOOT State");
    pInputButton.CurrentFsmState = & fsm_InputButton_boot_imp;
    // dont do anything

    // DEBUG_END;
}  // fsm_InputButton_boot::Init

/*****************************************************************************/
// waiting for the system to come up
void fsm_InputButton_boot::Poll (c_InputButton & pInputButton)
{
    // _ DEBUG_START;

    // start normal operation
    if (pInputButton.Enabled)
    {
        fsm_InputButton_off_state_imp.Init (pInputButton);
    }

    // _ DEBUG_END;
}  // fsm_InputButton_boot::Poll

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
// Input is off and is stable
void fsm_InputButton_off_state::Init (c_InputButton & pInputButton)
{
    // DEBUG_START;

    // DEBUG_V ("Entring OFF State");
    pInputButton.InputDebounceCount = MIN_INPUT_STABLE_VALUE;
    pInputButton.CurrentFsmState    = & fsm_InputButton_off_state_imp;

    // DEBUG_END;
}  // fsm_InputButton_off_state::Init

/*****************************************************************************/
// Input was off
void fsm_InputButton_off_state::Poll (c_InputButton & pInputButton)
{
    // _ DEBUG_START;

    // read the input
    bool bInputValue = pInputButton.ReadInput ();

    // If the input is "on"
    if (true == bInputValue)
    {
        // decrement the counter
        if (0 == --pInputButton.InputDebounceCount)
        {
            // we really are on
            fsm_InputButton_on_wait_long_state_imp.Init (pInputButton);
        }
    }
    else  // still off
    {
        // _ DEBUG_V ("reset the debounce counter");
        pInputButton.InputDebounceCount = MIN_INPUT_STABLE_VALUE;
    }

    // _ DEBUG_END;
}  // fsm_InputButton_off_state::Poll

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
// Input is always on
void fsm_InputButton_on_wait_long_state::Init (c_InputButton & pInputButton)
{
    // DEBUG_START;

    // DEBUG_V ("Entring Wait Long State");
    pInputButton.InputHoldTimer.StartTimer (pInputButton.LongPushDelayMS);
    pInputButton.CurrentFsmState = & fsm_InputButton_on_wait_long_state_imp;

    // DEBUG_END;
}  // fsm_InputButton_on_wait_long_state::Init

/*****************************************************************************/
// Input is on and is stable
void fsm_InputButton_on_wait_long_state::Poll (c_InputButton & pInputButton)
{
    // _ DEBUG_START;

    // read the input
    bool bInputValue = pInputButton.ReadInput ();

    // If the input is "on"
    if (true == bInputValue)
    {
        // _ DEBUG_V("");
        // decrement the counter
        if ( pInputButton.InputHoldTimer.IsExpired () )
        {
            // we really are on
            fsm_InputButton_wait_for_off_state_imp.Init (pInputButton);
            // _ DEBUG_V("HadLongPush = true")
        }
    }
    else  // Turned off
    {
        // _ DEBUG_V("HadShortPush = true")
        fsm_InputButton_wait_for_off_state_imp.Init (pInputButton);
    }

    // _ DEBUG_END;
}  // fsm_InputButton_on_wait_long_state::Poll

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
// Input is always on
void fsm_InputButton_wait_for_off_state::Init (c_InputButton & pInputButton)
{
    // DEBUG_START;

    // DEBUG_V ("Entring Wait OFF State");
    pInputButton.CurrentFsmState = & fsm_InputButton_wait_for_off_state_imp;

    // DEBUG_END;
}  // fsm_InputButton_wait_for_off_state::Init

/*****************************************************************************/
// Input is on and is stable
void fsm_InputButton_wait_for_off_state::Poll (c_InputButton & pInputButton)
{
    // _ DEBUG_START;

    // read the input
    bool bInputValue = pInputButton.ReadInput ();

    // If the input is "on" then we wait for it to go off
    if (false == bInputValue)
    {
        fsm_InputButton_off_state_imp.Init (pInputButton);
    }

    // _ DEBUG_END;
}  // fsm_InputButton_wait_for_off_state::Poll

/*****************************************************************************/
