/*
 * InputGateControl.cpp
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
#include "InputGateControl.hpp"
#include "InputButtons.hpp"
#include "GateDoors.hpp"
#include "GateLights.hpp"
#include "GateAudio.hpp"

// -----------------------------------------------------------------------------
// Local Structure and Data Definitions
// -----------------------------------------------------------------------------
    FsmInputGateBooting         FsmInputGateBooting_Imp;
    FsmInputGateIdle            FsmInputGateIdle_Imp;
    FsmInputGateOpeningIntro    FsmInputGateOpeningIntro_Imp;
    FsmInputGateOpening         FsmInputGateOpening_Imp;
    FsmInputGateOpen            FsmInputGateOpen_Imp;
    FsmInputGateClosing         FsmInputGateClosing_Imp;
    FsmInputGateLights          FsmInputGateLights_Imp;
    FsmInputGatePlaying         FsmInputGatePlaying_Imp;
    FsmInputGatePaused          FsmInputGatePaused_Imp;

// -----------------------------------------------------------------------------
c_InputGateControl::c_InputGateControl () :
    c_InputCommon (c_InputMgr::e_InputChannelIds::InputPrimaryChannelId,
     c_InputMgr::e_InputType(-1), 0)
{
    // _ DEBUG_START;
    // set a default effect

    SetBufferInfo (0);
    FsmInputGateBooting_Imp.init(this);

    // _ DEBUG_END;
}  // c_InputGateControl

// -----------------------------------------------------------------------------
c_InputGateControl::~c_InputGateControl ()
{}   // ~c_InputGateControl

// -----------------------------------------------------------------------------
void c_InputGateControl::Begin ()
{
    // DEBUG_START;

    if (true == HasBeenInitialized)
    {
        return;
    }

    GateAudio.Begin();
    GateDoors.Begin();
    GateLights.Begin();

    // button handlers
    InputButtons.RegisterButtonHandler(0, 
        [] (void * UserInfo)
            {
                if (UserInfo)
                {
                    static_cast <c_InputGateControl *> (UserInfo)->Button_Open_Pressed ();
                }
            },
            this);

    InputButtons.RegisterButtonHandler(1, 
        [] (void * UserInfo)
            {
                if (UserInfo)
                {
                    static_cast <c_InputGateControl *> (UserInfo)->Button_Lights_Pressed ();
                }
            },
            this);

    InputButtons.RegisterButtonHandler(2, 
        [] (void * UserInfo)
            {
                if (UserInfo)
                {
                    static_cast <c_InputGateControl *> (UserInfo)->Button_Play_Pressed ();
                }
            },
            this);


    InputButtons.RegisterButtonHandler(3, 
        [] (void * UserInfo)
            {
                if (UserInfo)
                {
                    static_cast <c_InputGateControl *> (UserInfo)->Button_Skip_Pressed ();
                }
            },
            this);

    InputButtons.RegisterButtonHandler(4, 
        [] (void * UserInfo)
            {
                if (UserInfo)
                {
                    static_cast <c_InputGateControl *> (UserInfo)->Button_Stop_Pressed ();
                }
            },
            this);

    HasBeenInitialized = true;

    validateConfiguration ();

    // DEBUG_V ("");

    // DEBUG_END;
}  // Begin

// -----------------------------------------------------------------------------
void c_InputGateControl::Button_Open_Pressed () 
{
    // DEBUG_START;

    if(nullptr != CurrentFsmState) { CurrentFsmState->Button_Open_Pressed(this); }

    // DEBUG_END;
}

// -----------------------------------------------------------------------------
void c_InputGateControl::Button_Lights_Pressed () 
{
    // DEBUG_START;

    if(nullptr != CurrentFsmState) { CurrentFsmState->Button_Lights_Pressed(this); }

    // DEBUG_END;
}

// -----------------------------------------------------------------------------
void c_InputGateControl::Button_Play_Pressed () 
{
    // DEBUG_START;

    if(nullptr != CurrentFsmState) { CurrentFsmState->Button_Play_Pressed(this); }

    // DEBUG_END;
}

// -----------------------------------------------------------------------------
void c_InputGateControl::Button_Skip_Pressed () 
{
    // DEBUG_START;

    if(nullptr != CurrentFsmState) { CurrentFsmState->Button_Skip_Pressed(this); }

    // DEBUG_END;
}

// -----------------------------------------------------------------------------
void c_InputGateControl::Button_Stop_Pressed () 
{
    // DEBUG_START;

    if(nullptr != CurrentFsmState) { CurrentFsmState->Button_Stop_Pressed(this); }

    // DEBUG_END;
}

// -----------------------------------------------------------------------------
void c_InputGateControl::GetConfig (JsonObject & jsonConfig)
{
    // DEBUG_START;

    if ( false == jsonConfig.containsKey (CN_gate) )
    {
        JsonObject Config = jsonConfig.createNestedObject (CN_gate);
    }

    JsonObject Config = jsonConfig[CN_gate];
    Config["Foo"]  = "Bar";

    GateAudio.GetConfig(jsonConfig);
    GateDoors.GetConfig(jsonConfig);
    GateLights.GetConfig(jsonConfig);

    // DEBUG_END;

}  // GetConfig

// -----------------------------------------------------------------------------
void c_InputGateControl::GetStatus (JsonObject & jsonStatus)
{
    // DEBUG_START;

    JsonObject Status = jsonStatus.createNestedObject ( F ("GateControl") );

    Status[CN_state] = CurrentFsmState->name();

    GateAudio.GetStatus(Status);
    GateDoors.GetStatus(Status);
    GateLights.GetStatus(Status);

    // DEBUG_END;
}  // GetStatus

// -----------------------------------------------------------------------------
void c_InputGateControl::Process ()
{
    // _ DEBUG_START;

    // _ DEBUG_V (String ("HasBeenInitialized: ") + HasBeenInitialized);
    // _ DEBUG_V (String ("PixelCount: ") + PixelCount);

    do  // once
    {
        if (!HasBeenInitialized)
        {
            break;
        }

        CurrentFsmState->poll (this);

        GateAudio.Poll();
        GateDoors.Poll();
        GateLights.Poll();

        // _ DEBUG_V ("Update output");
        InputMgr.RestartBlankTimer ( GetInputChannelId () );

    } while (false);

    // _ DEBUG_END;
}  // process

// -----------------------------------------------------------------------------
void c_InputGateControl::SetBufferInfo (uint32_t BufferSize)
{
    // DEBUG_START;

    InputDataBufferSize = BufferSize;

    // DEBUG_V (String ("BufferSize: ") + String (BufferSize));

    // DEBUG_END;
}  // SetBufferInfo

// -----------------------------------------------------------------------------
bool c_InputGateControl::SetConfig (ArduinoJson::JsonObject & jsonConfig)
{
    // DEBUG_START;

    do // once
    {
        if ( false == jsonConfig.containsKey (CN_gate) )
        {
            logcon ( String ( F ("No Gate Control Settings Found. Using Defaults") ) );
            extern void PrettyPrint (JsonObject & jsonStuff, String Name);
            PrettyPrint ( jsonConfig, String ( F ("c_InputGateControl::SetConfig") ) );
            break;
        }
        JsonObject Config = jsonConfig[CN_gate];

        // setFromJSON (EffectSpeed, Config, CN_EffectSpeed);

        SetBufferInfo (InputDataBufferSize);

        validateConfiguration ();

        GateAudio.SetConfig(jsonConfig);
        GateDoors.SetConfig(jsonConfig);
        GateLights.SetConfig(jsonConfig);

    } while (false);

    // DEBUG_END;
    return(true);

}  // SetConfig

// -----------------------------------------------------------------------------
void c_InputGateControl::validateConfiguration ()
{
    // DEBUG_START;

    // DEBUG_END;
}  // validateConfiguration

// -----------------------------------------------------------------------------
// ------------------ FSM Definitions ------------------------------------------
// -----------------------------------------------------------------------------
void FsmInputGateBooting::init(c_InputGateControl * pParent)
{
    // _ DEBUG_START;

    pParent->CurrentFsmState = this;
    
    // _ DEBUG_END;
} // FsmInputGateBooting::init

// -----------------------------------------------------------------------------
void FsmInputGateBooting::poll(c_InputGateControl * pParent)
{
    // _ DEBUG_START;

    FsmInputGateIdle_Imp.init(pParent);

    // _ DEBUG_END;
} // FsmInputGateBooting::init

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void FsmInputGateIdle::init(c_InputGateControl * pParent)
{
    // DEBUG_START;

    pParent->CurrentFsmState = this;

    GateDoors.Close();
    GateAudio.StopPlaying();
    GateLights.Off();

    // DEBUG_END;
} // FsmInputGateIdle::init

// -----------------------------------------------------------------------------
void FsmInputGateIdle::poll(c_InputGateControl * pParent)
{
    // _ DEBUG_START;

    // No Actions

    // _ DEBUG_END;
} // FsmInputGateIdle::init

// -----------------------------------------------------------------------------
void FsmInputGateIdle::Button_Open_Pressed (c_InputGateControl * pParent)
{
    // DEBUG_START;

    FsmInputGateOpeningIntro_Imp.init(pParent);

    // DEBUG_END;
} // FsmInputGateIdle::Button_Open_Pressed

// -----------------------------------------------------------------------------
void FsmInputGateIdle::Button_Lights_Pressed (c_InputGateControl * pParent)
{
    // DEBUG_START;

    FsmInputGateLights_Imp.init(pParent);

    // DEBUG_END;
} // FsmInputGateIdle::Button_Lights_Pressed

// -----------------------------------------------------------------------------
void FsmInputGateIdle::Button_Play_Pressed (c_InputGateControl * pParent)
{
    // DEBUG_START;

    FsmInputGatePlaying_Imp.init(pParent);

    // DEBUG_END;
} // FsmInputGateIdle::Button_Play_Pressed

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void FsmInputGateOpeningIntro::init(c_InputGateControl * pParent)
{
    // DEBUG_START;

    pParent->CurrentFsmState = this;

    GateDoors.Open();
    GateLights.On();
    GateAudio.PlayIntro();

    // set up total cycle time
    pParent->fsmTimer = millis() + OPEN_TIME;

    // DEBUG_END;
} // FsmInputGateOpeningIntro::init

// -----------------------------------------------------------------------------
void FsmInputGateOpeningIntro::poll(c_InputGateControl * pParent)
{
    // _ DEBUG_START;

    if(GateAudio.IsIdle())
    {
        FsmInputGateOpening_Imp.init(pParent);
    }

    // _ DEBUG_END;
} // FsmInputGateOpeningIntro::init

// -----------------------------------------------------------------------------
void FsmInputGateOpeningIntro::Button_Open_Pressed (c_InputGateControl * pParent)
{
    // DEBUG_START;

    // close the doors
    FsmInputGateClosing_Imp.init(pParent);

    // DEBUG_END;
} // FsmInputGateOpeningIntro::Button_Open_Pressed

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void FsmInputGateOpening::init(c_InputGateControl * pParent)
{
    // DEBUG_START;

    pParent->CurrentFsmState = this;
    GateAudio.PlayCurrentSelection();

    // DEBUG_END;
} // FsmInputGateOpening::init

// -----------------------------------------------------------------------------
void FsmInputGateOpening::poll(c_InputGateControl * pParent)
{
    // _ DEBUG_START;

    // Wait until the gate is open
    if(GateDoors.IsOpen())
    {
        FsmInputGateOpen_Imp.init(pParent);
    }

    // _ DEBUG_END;
} // FsmInputGateOpening::init

// -----------------------------------------------------------------------------
void FsmInputGateOpening::Button_Open_Pressed (c_InputGateControl * pParent)
{
    // DEBUG_START;

    // start closing the gate
    FsmInputGateClosing_Imp.init(pParent);

    // DEBUG_END;
} // FsmInputGateOpening::Button_Open_Pressed

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void FsmInputGateOpen::init(c_InputGateControl * pParent)
{
    // DEBUG_START;

    pParent->CurrentFsmState = this;
    // no actions

    // DEBUG_END;
} // FsmInputGateOpen::init

// -----------------------------------------------------------------------------
void FsmInputGateOpen::poll(c_InputGateControl * pParent)
{
    // _ DEBUG_START;

    // wait for the play timer to expire and then move to the closing state
    if(millis() > pParent->fsmTimer)
    {
        FsmInputGateClosing_Imp.init(pParent);
    }

    // _ DEBUG_END;
} // FsmInputGateOpen::init

// -----------------------------------------------------------------------------
void FsmInputGateOpen::Button_Open_Pressed (c_InputGateControl * pParent)
{
    // DEBUG_START;

    // close the gate
    FsmInputGateClosing_Imp.init(pParent);

    // DEBUG_END;
} // FsmInputGateOpen::Button_Open_Pressed

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void FsmInputGateClosing::init(c_InputGateControl * pParent)
{
    // DEBUG_START;

    pParent->CurrentFsmState = this;

    GateDoors.Close();

    // DEBUG_END;
} // FsmInputGateClosing::init

// -----------------------------------------------------------------------------
void FsmInputGateClosing::poll(c_InputGateControl * pParent)
{
    // _ DEBUG_START;

    if(GateDoors.IsClosed())
    {
        FsmInputGateIdle_Imp.init(pParent);
    }

    // _ DEBUG_END;
} // FsmInputGateClosing::init

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void FsmInputGateLights::init(c_InputGateControl * pParent)
{
    // DEBUG_START;

    pParent->CurrentFsmState = this;
    // turn on the lights effect
    GateLights.On();

    // DEBUG_END;
} // FsmInputGateLights::init

// -----------------------------------------------------------------------------
void FsmInputGateLights::poll(c_InputGateControl * pParent)
{
    // _ DEBUG_START;

    // no actions

    // _ DEBUG_END;
} // FsmInputGateLights::init

// -----------------------------------------------------------------------------
void FsmInputGateLights::Button_Open_Pressed (c_InputGateControl * pParent)
{
    // DEBUG_START;

    // turn off lights and move to the opening Intro state
    GateLights.Off();
    FsmInputGateOpeningIntro_Imp.init(pParent);

    // DEBUG_END;
} // FsmInputGateLights::init

// -----------------------------------------------------------------------------
void FsmInputGateLights::Button_Lights_Pressed (c_InputGateControl * pParent)
{
    // DEBUG_START;

    // move to the idle state
    FsmInputGateIdle_Imp.init(pParent);

    // DEBUG_END;
} // FsmInputGateLights::init

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void FsmInputGatePlaying::init(c_InputGateControl * pParent)
{
    // DEBUG_START;

    pParent->CurrentFsmState = this;

    // tell the audio to start
    GateAudio.PlayCurrentSelection();

    // DEBUG_END;
} // FsmInputGatePlaying::init

// -----------------------------------------------------------------------------
void FsmInputGatePlaying::poll(c_InputGateControl * pParent)
{
    // _ DEBUG_START;

    // When audio completes, restart it
    if(GateAudio.IsIdle())
    {
        GateAudio.PlayCurrentSelection();
    }

    // _ DEBUG_END;
} // FsmInputGatePlaying::init

// -----------------------------------------------------------------------------
void FsmInputGatePlaying::Button_Play_Pressed (c_InputGateControl * pParent)
{
    // DEBUG_START;

    FsmInputGatePaused_Imp.init(pParent);

    // DEBUG_END;
} // FsmInputGatePlaying::init

// -----------------------------------------------------------------------------
void FsmInputGatePlaying::Button_Skip_Pressed (c_InputGateControl * pParent)
{
    // DEBUG_START;

    // move the audio to the next song
    GateAudio.NextSong();

    // DEBUG_END;
} // FsmInputGatePlaying::init

// -----------------------------------------------------------------------------
void FsmInputGatePlaying::Button_Stop_Pressed (c_InputGateControl * pParent)
{
    // DEBUG_START;

    FsmInputGateIdle_Imp.init(pParent);

    // DEBUG_END;
} // FsmInputGatePlaying::init

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void FsmInputGatePaused::init(c_InputGateControl * pParent)
{
    // DEBUG_START;

    pParent->CurrentFsmState = this;

    // Tell audio to pause
    GateAudio.PausePlaying();

    // DEBUG_END;
} // FsmInputGatePaused::init

// -----------------------------------------------------------------------------
void FsmInputGatePaused::poll(c_InputGateControl * pParent)
{
    // _ DEBUG_START;

    // no action

    // _ DEBUG_END;
} // FsmInputGatePaused::init

// -----------------------------------------------------------------------------
void FsmInputGatePaused::Button_Play_Pressed (c_InputGateControl * pParent)
{
    // DEBUG_START;

    // tell audio to play
    GateAudio.ResumePlaying();

    pParent->CurrentFsmState = &FsmInputGatePlaying_Imp;

    // DEBUG_END;
} // FsmInputGatePaused::init

// -----------------------------------------------------------------------------
void FsmInputGatePaused::Button_Stop_Pressed (c_InputGateControl * pParent)
{
    // DEBUG_START;

    FsmInputGateIdle_Imp.init(pParent);

    // DEBUG_END;
} // FsmInputGatePaused::init

// -----------------------------------------------------------------------------
