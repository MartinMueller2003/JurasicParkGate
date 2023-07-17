#pragma once
/*
 * GateDoors.hpp - Output Management class
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

class FsmDoorStateCommon;
class c_GateDoors{
protected:

void SetChannelData(uint8_t);

uint8_t doorChannels[2] = {15, 31};
uint32_t TimeToOpenMS = 45000;
uint32_t TimeToCloseMS = 20000;
uint32_t TimeStartedMS = 0;
uint32_t TimeElapsedMS = 0;
uint8_t CurrentPosition = 0;

#define FULL_OPEN_VALUE 255
#define CLOSED_VALUE    0

friend class FsmDoorStateBooting;
friend class FsmDoorStateClosed;
friend class FsmDoorStateOpening;
friend class FsmDoorStateOpen;
friend class FsmDoorStateClosing;
FsmDoorStateCommon* CurrentFsmState = nullptr;

public:

c_GateDoors ();
virtual ~c_GateDoors ();

void Begin     ();
void Poll      ();
void GetConfig (JsonObject & json);
bool SetConfig (JsonObject & json);
void GetStatus (JsonObject & json);

void Open();
void Close();

bool IsOpen();
bool IsClosed();

void GetDriverName    (String & Name) {Name = "GateDoors";}

}; // c_GateDoors

// -----------------------------------------------------------------------------
// ---------- FSM Definitions --------------------------------------------------
// -----------------------------------------------------------------------------
class FsmDoorStateCommon{
public:
FsmDoorStateCommon() {}
virtual ~FsmDoorStateCommon() {}
virtual void init (c_GateDoors* pParent) = 0;
virtual void poll (c_GateDoors* pParent) = 0;
virtual String name () = 0;
virtual void Open (c_GateDoors* pParent) {}
virtual void Close (c_GateDoors* pParent) {}

}; // FsmDoorStateCommon

// -----------------------------------------------------------------------------
class FsmDoorStateBooting : public FsmDoorStateCommon {
public:
FsmDoorStateBooting() {}
virtual ~FsmDoorStateBooting() {}
virtual void init (c_GateDoors* pParent);
virtual void poll (c_GateDoors* pParent);
virtual String name () {return( F("Booting") );}
// virtual void Open (c_GateDoors * pParent) {}
// virtual void Close (c_GateDoors * pParent) {}

}; // class FsmDoorStateBooting

// -----------------------------------------------------------------------------
class FsmDoorStateClosed : public FsmDoorStateCommon {
public:
FsmDoorStateClosed() {}
virtual ~FsmDoorStateClosed() {}
virtual void init (c_GateDoors* pParent);
virtual void poll (c_GateDoors* pParent);
virtual String name () {return( F("Closed") );}
virtual void Open (c_GateDoors* pParent);
// virtual void Close (c_GateDoors * pParent);

}; // FsmDoorStateClosed

// -----------------------------------------------------------------------------
class FsmDoorStateOpening : public FsmDoorStateCommon {
public:
FsmDoorStateOpening() {}
virtual ~FsmDoorStateOpening() {}
virtual void init (c_GateDoors* pParent);
virtual void poll (c_GateDoors* pParent);
virtual String name () {return( F("Opening") );}
virtual void Open (c_GateDoors* pParent);
virtual void Close (c_GateDoors* pParent);

}; // FsmDoorStateOpening

// -----------------------------------------------------------------------------
class FsmDoorStateOpen : public FsmDoorStateCommon {
public:
FsmDoorStateOpen() {}
virtual ~FsmDoorStateOpen() {}
virtual void init (c_GateDoors* pParent);
virtual void poll (c_GateDoors* pParent);
virtual String name () {return( F("Open") );}
// virtual void Open (c_GateDoors * pParent);
virtual void Close (c_GateDoors* pParent);

}; // FsmDoorStateOpen

// -----------------------------------------------------------------------------
class FsmDoorStateClosing : public FsmDoorStateCommon {
public:
FsmDoorStateClosing() {}
virtual ~FsmDoorStateClosing() {}
virtual void init (c_GateDoors* pParent);
virtual void poll (c_GateDoors* pParent);
virtual String name () {return( F("Closing") );}
virtual void Open (c_GateDoors* pParent);
virtual void Close (c_GateDoors* pParent);

}; // FsmDoorStateClosing

extern c_GateDoors GateDoors;
