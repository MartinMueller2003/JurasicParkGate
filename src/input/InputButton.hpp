#pragma once
/*
 * InputButton.hpp
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

/*****************************************************************************/
class fsm_InputButton_state;
class c_InputButton{
public:
c_InputButton ();

void Begin ();
void SetGpio (gpio_num_t value) {GpioId = value;}
bool SetConfig (JsonObject & jsonConfig);
void GetConfig (JsonObject & jsonConfig);
void GetStatus (JsonObject & jsonStatus);
void Process ();
void GetDriverName (String & sDriverName) { sDriverName = Name; }                                                   ///< get the name for the instantiated driver
void SetBufferInfo (uint32_t BufferSize);
void NetworkStateChanged (bool IsConnected);
void SetName (String & value) { Name = value; }
void RegisterButtonHandler(void (*callback)(void *), void * context);

protected:
void generatateCallback();

enum Polarity_t
{
    ActiveHigh = 0,
    ActiveLow
};

// read the adjusted value of the input pin
bool ReadInput (void);

    #define M_NAME          CN_name
    #define M_IO_ENABLED    CN_enabled
    #define M_STATE         CN_state
    #define M_POLARITY      CN_polarity

String Name;
gpio_num_t GpioId           = gpio_num_t::GPIO_NUM_0;
uint32_t TriggerChannel     = uint32_t (32);
Polarity_t polarity         = Polarity_t::ActiveLow;
bool Enabled                = true;
uint32_t InputDebounceCount = 0;
FastTimer InputHoldTimer;
uint32_t LongPushDelayMS    = 2000;
fsm_InputButton_state* CurrentFsmState = nullptr;

void (* callback) (void *) = nullptr;
void * context = nullptr;

friend class fsm_InputButton_boot;
friend class fsm_InputButton_off_state;
friend class fsm_InputButton_wait_for_off_state;

}; // c_InputButton

/*****************************************************************************/
/*
 *	Generic fsm base class.
 */
/*****************************************************************************/
class fsm_InputButton_state{
public:
virtual void Poll (c_InputButton & pInputButton) = 0;
virtual void Init (c_InputButton & pInputButton) = 0;
virtual~fsm_InputButton_state () {}
private:
    #define MIN_INPUT_STABLE_VALUE 200
}; // fsm_InputButton_state

/*****************************************************************************/
class fsm_InputButton_boot final : public fsm_InputButton_state {
public:
void Poll (c_InputButton & pInputButton) override;
void Init (c_InputButton & pInputButton) override;
~fsm_InputButton_boot () override {}
}; // fsm_InputButton_boot

/*****************************************************************************/
// input is off and stable
class fsm_InputButton_off_state final : public fsm_InputButton_state {
public:
void Poll (c_InputButton & pInputButton) override;
void Init (c_InputButton & pInputButton) override;
~fsm_InputButton_off_state () override {}
}; // fsm_InputButton_off_state

/*****************************************************************************/
// input is always reported as on
//
class fsm_InputButton_wait_for_off_state final : public fsm_InputButton_state {
public:
void Poll (c_InputButton & pInputButton) override;
void Init (c_InputButton & pInputButton) override;
~fsm_InputButton_wait_for_off_state () override {}
}; // fsm_InputButton_wait_for_off_state
