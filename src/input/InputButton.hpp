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
class c_InputButton
{
public:
    c_InputButton ();

    void    Begin ();                               ///< set up the operating environment based on the current config (or defaults)
    bool    SetConfig (JsonObject & jsonConfig);    ///< Set a new config in the driver
    void    GetConfig (JsonObject & jsonConfig);    ///< Get the current config used by the driver
    void    GetStatus (JsonObject & jsonStatus);
    void    Process ();                             ///< Call from loop(),  renders Input data
    void    GetDriverName (String & sDriverName) {sDriverName = Name;} ///< get the name for the instantiated driver
    void    SetBufferInfo (uint32_t BufferSize);
    void    NetworkStateChanged (bool IsConnected); // used by poorly designed rx functions
    void    SetName (String & value) {Name = value;}
protected:

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
    #define M_ID            CN_id

    String Name;
    uint32_t                GpioId              = 0;
    uint32_t                TriggerChannel      = uint32_t (32);
    Polarity_t              polarity            = Polarity_t::ActiveLow;
    bool                    Enabled             = false;
    uint32_t                InputDebounceCount  = 0;
    FastTimer               InputHoldTimer;
    uint32_t                LongPushDelayMS     = 2000;
    fsm_InputButton_state   * CurrentFsmState   = nullptr;    // initialized in constructor

    friend class fsm_InputButton_boot;
    friend class fsm_InputButton_off_state;
    friend class fsm_InputButton_on_wait_long_state;
    friend class fsm_InputButton_wait_for_off_state;
}; // c_InputButton

/*****************************************************************************/
/*
  *	Generic fsm base class.
  */
/*****************************************************************************/
class fsm_InputButton_state
{
public:
    virtual void    Poll (c_InputButton & pInputButton) = 0;
    virtual void    Init (c_InputButton & pInputButton) = 0;
    virtual~fsm_InputButton_state () {}
private:
    #define MIN_INPUT_STABLE_VALUE 50
}; // fsm_InputButton_state

/*****************************************************************************/
// input is unknown and unreachable
//
class fsm_InputButton_boot final : public fsm_InputButton_state
{
public:
    void    Poll (c_InputButton & pInputButton) override;
    void    Init (c_InputButton & pInputButton) override;
    ~fsm_InputButton_boot () override{}
}; // fsm_InputButton_boot

/*****************************************************************************/
// input is off and stable
//
class fsm_InputButton_off_state final : public fsm_InputButton_state
{
public:
    void    Poll (c_InputButton & pInputButton) override;
    void    Init (c_InputButton & pInputButton) override;
    ~fsm_InputButton_off_state () override{}
}; // fsm_InputButton_off_state

/*****************************************************************************/
// input is always reported as on
//
class fsm_InputButton_on_wait_long_state final : public fsm_InputButton_state
{
public:
    void    Poll (c_InputButton & pInputButton) override;
    void    Init (c_InputButton & pInputButton) override;
    ~fsm_InputButton_on_wait_long_state () override{}
}; // fsm_InputButton_on_wait_long_state

/*****************************************************************************/
// input is always reported as on
//
class fsm_InputButton_wait_for_off_state final : public fsm_InputButton_state
{
public:
    void    Poll (c_InputButton & pInputButton) override;
    void    Init (c_InputButton & pInputButton) override;
    ~fsm_InputButton_wait_for_off_state () override{}
}; // fsm_InputButton_wait_for_off_state
