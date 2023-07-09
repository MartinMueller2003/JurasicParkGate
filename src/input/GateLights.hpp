#pragma once
/*
 * GateLights.hpp - Output Management class
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

class c_GateLights
{
private:

protected:

public:

c_GateLights ();
virtual ~c_GateLights ();

void Begin     ();
void Poll      () {}
void GetConfig (JsonObject & json);
bool SetConfig (JsonObject & json);
void GetStatus (JsonObject & json);

void On();
void Off();

void GetDriverName    (String & Name) {Name = "GateLights";}

}; // c_GateLights

extern c_GateLights GateLights;
