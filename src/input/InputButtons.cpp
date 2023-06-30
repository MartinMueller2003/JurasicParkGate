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

/*****************************************************************************/
/* Code                                                                      */
/*****************************************************************************/

c_InputButtons::c_InputButtons (c_InputMgr::e_InputChannelIds NewInputChannelId,
                                c_InputMgr::e_InputType       NewChannelType,
                                uint32_t                      BufferSize) :
    c_InputCommon (NewInputChannelId, NewChannelType, BufferSize)
{
	DEBUG_START;
	
	DEBUG_END;

} // c_InputButtons

/*****************************************************************************/
void c_InputButtons::Begin() 
{
	DEBUG_START;

    // DEBUG_V ("InputDataBufferSize: " + String(InputDataBufferSize));

	for(auto & CurrentButton : Buttons)
	{
		// send the config to the button
		CurrentButton.Begin();
	}

	HasBeenInitialized = true;

    // DEBUG_END;
}

/*****************************************************************************/
void c_InputButtons::GetConfig (JsonObject & JsonData)
{
    // DEBUG_START;

	// if the array does not exist, then make it

	for(auto & CurrentButton : Buttons)
	{
		// Create an array entry

		// send the config to the button
	}

    // DEBUG_END;

} // GetConfig

/*****************************************************************************/
void c_InputButtons::GetStatus (JsonObject & JsonData)
{
	DEBUG_START;

	// create the button array

	for(auto & CurrentButton : Buttons)
	{
		// create a button status object

		CurrentButton.GetStatus(JsonData);
	}

	DEBUG_END;

} // GetStatistics

//-----------------------------------------------------------------------------
void c_InputButtons::SetBufferInfo (uint32_t BufferSize)
{
    // DEBUG_START;

    InputDataBufferSize = BufferSize;

    // DEBUG_V (String ("InputDataBufferSize: ") + String (InputDataBufferSize));

    // DEBUG_END;

} // SetBufferInfo

/*****************************************************************************/
bool c_InputButtons::SetConfig (JsonObject & JsonData)
{
	DEBUG_START;

	for(auto & CurrentButton : Buttons)
	{
		// locate the config

		// send the config to the button
	}

	DEBUG_END;
	return true;
} // ProcessConfig

/*****************************************************************************/
void c_InputButtons::Process (void)
{
	DEBUG_START;
	
	for(auto & CurrentButton : Buttons)
	{
		CurrentButton.Process();
	}

	DEBUG_END;

} // Poll

//-----------------------------------------------------------------------------
void c_InputButtons::NetworkStateChanged (bool IsConnected)
{
} // NetworkStateChanged
