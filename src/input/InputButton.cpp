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

fsm_InputButton_boot                fsm_InputButton_boot_imp;
fsm_InputButton_off_state           fsm_InputButton_off_state_imp;
fsm_InputButton_wait_for_off_state  fsm_InputButton_wait_for_off_state_imp;

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
    pinMode (GpioId, INPUT_PULLUP);

    // DEBUG_END;
} // c_InputButton::Begin

/*****************************************************************************/
void c_InputButton::GetConfig (JsonObject & JsonData)
{
    // DEBUG_START;

    JsonData[M_NAME]       = Name;
    JsonData[M_IO_ENABLED] = Enabled;
    JsonData[CN_GPIO]      = GpioId;
    JsonData[M_POLARITY]   = (Polarity_t::ActiveHigh == polarity)?CN_ActiveHigh : CN_ActiveLow;

    // DEBUG_V (String ("GpioId: ") + String (GpioId));

    // DEBUG_END;
}  // GetConfig

/*****************************************************************************/
void c_InputButton::GetStatus (JsonObject & JsonData)
{
    // DEBUG_START;

    JsonData[M_NAME]  = Name;
    JsonData[CN_GPIO] = GpioId;
    JsonData[M_STATE] = ( ReadInput () )?CN_on : CN_off;

    // DEBUG_END;
}  // GetStatistics

/*****************************************************************************/
bool c_InputButton::SetConfig (JsonObject & JsonData)
{
    // DEBUG_START;

    String      Polarity = (ActiveHigh == polarity)?CN_ActiveHigh : CN_ActiveLow;

    uint32_t    oldInputId = GpioId;

    setFromJSON (   Name,       JsonData,   M_NAME);
    setFromJSON (   Enabled,    JsonData,   M_IO_ENABLED);
    setFromJSON (   GpioId,     JsonData,   CN_GPIO);
    setFromJSON (   Polarity,   JsonData,   M_POLARITY);

    polarity = ( String (CN_ActiveHigh).equals (Polarity) )?ActiveHigh : ActiveLow;

    if ( (oldInputId != GpioId) )
    {
        pinMode (oldInputId, INPUT);
    }

    pinMode ( GpioId, INPUT_PULLUP);

    if (false == Enabled)
    {
        fsm_InputButton_boot_imp.Init (*this);
    }

    // DEBUG_V (String ("    Name: ") + Name);
    // DEBUG_V (String ("Polarity: ") + Polarity);
    // DEBUG_V (String (" Enabled: ") + String (Enabled));
    // DEBUG_V (String ("  GpioId: ") + String (GpioId));

    // DEBUG_END;
    return(true);
}  // ProcessConfig

/*****************************************************************************/
void c_InputButton::Process (void)
{
    // _ DEBUG_START;

    // _ DEBUG_V (String ("    Name: ") + Name);
    // _ DEBUG_V (String (" Enabled: ") + String (Enabled));
    // _ DEBUG_V (String ("  GpioId: ") + String (GpioId));

    if(nullptr != CurrentFsmState)
    {
        CurrentFsmState->Poll (*this);
    }

    // _ DEBUG_END;
}  // Poll

/*****************************************************************************/
bool c_InputButton::ReadInput (void)
{
    // DEBUG_START;

    // read the input
    bool bInputValue = digitalRead (GpioId);

    // do we need to invert the input?
    if (Polarity_t::ActiveLow == polarity)
    {
        // invert the input value
        bInputValue = !bInputValue;
    }

    // DEBUG_V (String ("       Name: ") + Name);
    // DEBUG_V (String ("    Enabled: ") + String(Enabled));
    // DEBUG_V (String ("     GpioId: ") + String (GpioId));
    // DEBUG_V (String ("bInputValue: ") + String (bInputValue));

    // DEBUG_END;

    return(bInputValue);
}  // ReadInput

// -----------------------------------------------------------------------------
void c_InputButton::NetworkStateChanged (bool IsConnected)
{
}                                                              // NetworkStateChanged

/*****************************************************************************/
void c_InputButton::RegisterButtonHandler(void (*_callback)(void*), void* _context)
{
    // DEBUG_START;

    callback = _callback;
    context = _context;

    // DEBUG_END;
} // RegisterButtonHandler

/*****************************************************************************/
void c_InputButton::generatateCallback()
{
    // DEBUG_START;

    if(nullptr != callback)
    {
        (*callback)(context);
    }

    // DEBUG_END;
} // c_InputButton::generatateCallback


/*****************************************************************************/
/*	FSM                                                                      */
/*****************************************************************************/

/*****************************************************************************/
// waiting for the system to come up
void fsm_InputButton_boot::Init (c_InputButton & pInputButton)
{
    // DEBUG_START;

    // DEBUG_V ("Entring BOOT State");
    pInputButton.CurrentFsmState = &fsm_InputButton_boot_imp;
    // dont do anything

    // DEBUG_END;
}  // fsm_InputButton_boot::Init

/*****************************************************************************/
// waiting for the system to come up
void fsm_InputButton_boot::Poll (c_InputButton & pInputButton)
{
    // _ DEBUG_START;

    // _ DEBUG_V (String ("    Name: ") + pInputButton.Name);
    // _ DEBUG_V (String (" Enabled: ") + String (pInputButton.Enabled));
    // _ DEBUG_V (String ("  GpioId: ") + String (pInputButton.GpioId));

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
    // DEBUG_V (String ("    Name: ") + pInputButton.Name);
    // DEBUG_V (String ("  GpioId: ") + String (pInputButton.GpioId));

    pInputButton.InputDebounceCount = MIN_INPUT_STABLE_VALUE;
    pInputButton.CurrentFsmState    = &fsm_InputButton_off_state_imp;

    // DEBUG_END;
}  // fsm_InputButton_off_state::Init

/*****************************************************************************/
// Input was off
void fsm_InputButton_off_state::Poll (c_InputButton & pInputButton)
{
    // DEBUG_START;

    // read the input
    bool bInputValue = pInputButton.ReadInput ();

    // If the input is "on"
    if (true == bInputValue)
    {
        // decrement the counter
        if (0 == --pInputButton.InputDebounceCount)
        {
            // we really are on
            fsm_InputButton_wait_for_off_state_imp.Init (pInputButton);
        }
    }
    else  // still off
    {
        // _ DEBUG_V ("reset the debounce counter");
        pInputButton.InputDebounceCount = MIN_INPUT_STABLE_VALUE;
    }

    // DEBUG_END;
}  // fsm_InputButton_off_state::Poll

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
// Input is on
void fsm_InputButton_wait_for_off_state::Init (c_InputButton & pInputButton)
{
    // DEBUG_START;

    // DEBUG_V ("Entring Wait OFF State");
    pInputButton.CurrentFsmState = &fsm_InputButton_wait_for_off_state_imp;
    pInputButton.generatateCallback();
    pInputButton.InputDebounceCount = MIN_INPUT_STABLE_VALUE;

    // DEBUG_END;
}  // fsm_InputButton_wait_for_off_state::Init

/*****************************************************************************/
// Input is on and is stable
void fsm_InputButton_wait_for_off_state::Poll (c_InputButton & pInputButton)
{
    // DEBUG_START;

    // read the input
    bool bInputValue = pInputButton.ReadInput ();

    // If the input is "on" then we wait for it to go off
    if (false == bInputValue)
    {
        if(0 == --pInputButton.InputDebounceCount)
        {
            // we really are off
            fsm_InputButton_off_state_imp.Init (pInputButton);
        }
    }
    else
    {
        pInputButton.InputDebounceCount = MIN_INPUT_STABLE_VALUE;
    }

    // DEBUG_END;
}  // fsm_InputButton_wait_for_off_state::Poll

/*****************************************************************************/
