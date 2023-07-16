#pragma once
/*
 * InputGateControl.cpp - Input Management class
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

#include "InputCommon.hpp"

class FsmInputGateCommon;

class c_InputGateControl : public c_InputCommon {
public:
c_InputGateControl ();
virtual~c_InputGateControl ();

// functions to be provided by the derived class
void Begin ();                                         ///< set up the operating environment based on the current config (or defaults)
bool SetConfig (JsonObject & jsonConfig);              ///< Set a new config in the driver
void GetConfig (JsonObject & jsonConfig);              ///< Get the current config used by the driver
void GetStatus (JsonObject & jsonStatus);
void Process ();                                       ///< Call from loop(),  renders Input data
void GetDriverName (String & sDriverName) { sDriverName = "Gate"; }                                                                           ///< get the name for the instantiated driver
void SetBufferInfo (uint32_t BufferSize);

void Button_Open_Pressed ();
void Button_Lights_Pressed ();
void Button_Play_Pressed ();
void Button_Skip_Pressed ();
void Button_Stop_Pressed ();

protected:

friend class FsmInputGateCommon;
friend class FsmInputGateBooting;
friend class FsmInputGateIdle;
friend class FsmInputGateOpeningIntro;
friend class FsmInputGateOpening;
friend class FsmInputGateOpen;
friend class FsmInputGateClosing;
friend class FsmInputGateLights;
friend class FsmInputGatePlaying;
friend class FsmInputGatePaused;
FsmInputGateCommon* CurrentFsmState = nullptr;

uint32_t fsmTimer = 0;

    #define OPEN_TIME 45000
private:

void validateConfiguration ();

bool HasBeenInitialized = false;

}; // class c_InputGateControl

// -----------------------------------------------------------------------------
// ---------- FSM Definitions --------------------------------------------------
// -----------------------------------------------------------------------------
class FsmInputGateCommon{
public:
FsmInputGateCommon() {}
virtual ~FsmInputGateCommon() {}
virtual void init (c_InputGateControl* pParent);
void init (c_InputGateControl*  pParent,
 String                         name);
virtual void poll (c_InputGateControl* pParent) = 0;
virtual String name () = 0;
void GetDriverName (String & sDriverName) { _pParent->GetDriverName(sDriverName); }                                                                               ///< get the name for the instantiated driver
virtual void Button_Open_Pressed (c_InputGateControl* pParent) {}
virtual void Button_Lights_Pressed (c_InputGateControl* pParent) {}
virtual void Button_Play_Pressed (c_InputGateControl* pParent) {}
virtual void Button_Skip_Pressed (c_InputGateControl* pParent) {}
virtual void Button_Stop_Pressed (c_InputGateControl* pParent) {}

protected:
c_InputGateControl* _pParent = nullptr;
}; // FsmInputGateCommon

// -----------------------------------------------------------------------------
class FsmInputGateBooting final : public FsmInputGateCommon {
public:
void init (c_InputGateControl* pParent) override;
void poll (c_InputGateControl* pParent) override;
String name () {return(F("Booting") );}
}; // FsmInputGateBooting

// -----------------------------------------------------------------------------
class FsmInputGateIdle final : public FsmInputGateCommon {
public:
void init (c_InputGateControl* pParent) override;
void poll (c_InputGateControl* pParent) override;
String name () {return(F("Idle") );}
void Button_Open_Pressed (c_InputGateControl* pParent) override;
void Button_Lights_Pressed (c_InputGateControl* pParent) override;
void Button_Play_Pressed (c_InputGateControl* pParent) override;
// void Button_Skip_Pressed (c_InputGateControl * pParent) override;
// void Button_Stop_Pressed (c_InputGateControl * pParent) override;

}; // FsmInputGateIdle

// -----------------------------------------------------------------------------
class FsmInputGateOpeningIntro final : public FsmInputGateCommon {
public:
void init (c_InputGateControl* pParent) override;
void poll (c_InputGateControl* pParent) override;
String name () {return(F("Intro") );}
void Button_Open_Pressed (c_InputGateControl* pParent) override;
// void Button_Lights_Pressed (c_InputGateControl * pParent) override;
// void Button_Play_Pressed (c_InputGateControl * pParent) override;
// void Button_Skip_Pressed (c_InputGateControl * pParent) override;
// void Button_Stop_Pressed (c_InputGateControl * pParent) override;

}; // FsmInputGateOpening

// -----------------------------------------------------------------------------
class FsmInputGateOpening final : public FsmInputGateCommon {
public:
void init (c_InputGateControl* pParent) override;
void poll (c_InputGateControl* pParent) override;
String name () {return(F("Opening") );}
void Button_Open_Pressed (c_InputGateControl* pParent) override;
// void Button_Lights_Pressed (c_InputGateControl * pParent) override;
// void Button_Play_Pressed (c_InputGateControl * pParent) override;
// void Button_Skip_Pressed (c_InputGateControl * pParent) override;
// void Button_Stop_Pressed (c_InputGateControl * pParent) override;

}; // FsmInputGateOpening

// -----------------------------------------------------------------------------
class FsmInputGateOpen final : public FsmInputGateCommon {
public:
void init (c_InputGateControl* pParent) override;
void poll (c_InputGateControl* pParent) override;
String name () {return(F("Open") );}
void Button_Open_Pressed (c_InputGateControl* pParent) override;
// void Button_Lights_Pressed (c_InputGateControl * pParent) override;
// void Button_Play_Pressed (c_InputGateControl * pParent) override;
// void Button_Skip_Pressed (c_InputGateControl * pParent) override;
// void Button_Stop_Pressed (c_InputGateControl * pParent) override;

}; // FsmInputGateOpen

// -----------------------------------------------------------------------------
class FsmInputGateClosing final : public FsmInputGateCommon {
public:
void init (c_InputGateControl* pParent) override;
void poll (c_InputGateControl* pParent) override;
String name () {return(F("Closing") );}
}; // FsmInputGateClosing

// -----------------------------------------------------------------------------
class FsmInputGateLights final : public FsmInputGateCommon {
public:
void init (c_InputGateControl* pParent) override;
void poll (c_InputGateControl* pParent) override;
String name () {return(F("Lights On") );}
void Button_Open_Pressed (c_InputGateControl* pParent) override;
void Button_Lights_Pressed (c_InputGateControl* pParent) override;
// void Button_Play_Pressed (c_InputGateControl * pParent) override;
// void Button_Skip_Pressed (c_InputGateControl * pParent) override;
// void Button_Stop_Pressed (c_InputGateControl * pParent) override;

}; // FsmInputGateLights

// -----------------------------------------------------------------------------
class FsmInputGatePlaying final : public FsmInputGateCommon {
public:
void init (c_InputGateControl* pParent) override;
void poll (c_InputGateControl* pParent) override;
String name () {return(F("Playing") );}
// void Button_Open_Pressed (c_InputGateControl * pParent) override;
// void Button_Lights_Pressed (c_InputGateControl * pParent) override;
void Button_Play_Pressed (c_InputGateControl* pParent) override;
void Button_Skip_Pressed (c_InputGateControl* pParent) override;
void Button_Stop_Pressed (c_InputGateControl* pParent) override;

}; // FsmInputGateLights

// -----------------------------------------------------------------------------
class FsmInputGatePaused final : public FsmInputGateCommon {
public:
void init (c_InputGateControl* pParent) override;
void poll (c_InputGateControl* pParent) override;
String name () {return(F("Paused") );}
// void Button_Open_Pressed (c_InputGateControl * pParent) override;
// void Button_Lights_Pressed (c_InputGateControl * pParent) override;
void Button_Play_Pressed (c_InputGateControl* pParent) override;
// void Button_Skip_Pressed (c_InputGateControl * pParent) override;
void Button_Stop_Pressed (c_InputGateControl* pParent) override;

}; // FsmInputGatePaused
