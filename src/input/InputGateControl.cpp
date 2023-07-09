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
    DEBUG_START;

    if(nullptr != CurrentFsmState) { CurrentFsmState->Button_Open_Pressed(this); }

    DEBUG_END;
}

// -----------------------------------------------------------------------------
void c_InputGateControl::Button_Lights_Pressed () 
{
    DEBUG_START;

    if(nullptr != CurrentFsmState) { CurrentFsmState->Button_Lights_Pressed(this); }

    DEBUG_END;
}

// -----------------------------------------------------------------------------
void c_InputGateControl::Button_Play_Pressed () 
{
    DEBUG_START;

    if(nullptr != CurrentFsmState) { CurrentFsmState->Button_Play_Pressed(this); }

    DEBUG_END;
}

// -----------------------------------------------------------------------------
void c_InputGateControl::Button_Skip_Pressed () 
{
    DEBUG_START;

    if(nullptr != CurrentFsmState) { CurrentFsmState->Button_Skip_Pressed(this); }

    DEBUG_END;
}

// -----------------------------------------------------------------------------
void c_InputGateControl::Button_Stop_Pressed () 
{
    DEBUG_START;

    if(nullptr != CurrentFsmState) { CurrentFsmState->Button_Stop_Pressed(this); }

    DEBUG_END;
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

    // DEBUG_END;

}  // GetConfig

// -----------------------------------------------------------------------------
void c_InputGateControl::GetMqttEffectList (JsonObject & jsonConfig)
{
    DEBUG_START;
    JsonArray EffectsArray = jsonConfig.createNestedArray (CN_effect_list);

    DEBUG_END;
}  // GetMqttEffectList

// -----------------------------------------------------------------------------
void c_InputGateControl::GetMqttConfig (MQTTConfiguration_s & mqttConfig)
{
    DEBUG_START;

    DEBUG_END;
}  // GetMqttConfig

// -----------------------------------------------------------------------------
void c_InputGateControl::GetStatus (JsonObject & jsonStatus)
{
    // DEBUG_START;

    JsonObject Status = jsonStatus.createNestedObject ( F ("Gate Control") );

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

        // _ DEBUG_V ("Init OK");

        // _ DEBUG_V ("Pixel Count OK");

        CurrentFsmState->poll (this);

        // _ DEBUG_V ("Update output");
//        uint32_t wait = (this->*ActiveEffect->func)();
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

    } while (false);

    // DEBUG_END;
    return(true);

}  // SetConfig

// -----------------------------------------------------------------------------
void c_InputGateControl::SetMqttConfig (MQTTConfiguration_s & mqttConfig)
{
    DEBUG_START;

    DEBUG_END;
}  // SetConfig

// -----------------------------------------------------------------------------
void c_InputGateControl::validateConfiguration ()
{
    // DEBUG_START;

    // DEBUG_END;
}  // validateConfiguration

// -----------------------------------------------------------------------------
void c_InputGateControl::setBrightness (float brightness)
{
    DEBUG_START;

    DEBUG_END;
} // c_InputGateControl::setBrightness

// -----------------------------------------------------------------------------
// Yukky maths here. Input speeds from 1..10 get mapped to 17782..100
void c_InputGateControl::setSpeed (uint16_t speed)
{
    DEBUG_START;

    DEBUG_END;
}

// -----------------------------------------------------------------------------
void c_InputGateControl::setDelay (uint16_t delay)
{
    DEBUG_START;

    DEBUG_END;
}  // setDelay

// -----------------------------------------------------------------------------
void c_InputGateControl::setColor (String & NewColor)
{
    DEBUG_START;

    DEBUG_V ("NewColor: " + NewColor);

    // Parse the color string into rgb values

    uint32_t intValue = strtoul (NewColor.substring (1).c_str (), nullptr, 16);
    DEBUG_V (String ("intValue: ") + String (intValue, 16));
/*
    EffectColor.r = uint8_t ( (intValue >> 16) & 0xFF );
    EffectColor.g = uint8_t ( (intValue >> 8) & 0xFF );
    EffectColor.b = uint8_t ( (intValue >> 0) & 0xFF );
*/
    DEBUG_END;
}  // setColor

// -----------------------------------------------------------------------------
void c_InputGateControl::setPixel (uint16_t pixelId, CRGB color)
{
    DEBUG_START;

    DEBUG_V (String ("IsInputChannelActive: ") + String(IsInputChannelActive));
    DEBUG_V (String ("pixelId: ") + pixelId);
    DEBUG_V (String ("PixelCount: ") + PixelCount);

//    if ( (true == IsInputChannelActive) && (pixelId < PixelCount) )
    {
        uint8_t PixelBuffer[sizeof (CRGB) + 1];

        DEBUG_V (String ("color.r: ") + String (color.r));
        DEBUG_V (String ("color.g: ") + String (color.g));
        DEBUG_V (String ("color.b: ") + String (color.b));

        PixelBuffer[0] = color.r;
        PixelBuffer[1] = color.g;
        PixelBuffer[2] = color.b;

        OutputMgr.WriteChannelData (pixelId * 3, 3, PixelBuffer);
    }

    DEBUG_END;
}  // setPixel

// -----------------------------------------------------------------------------
void c_InputGateControl::GetPixel (uint16_t pixelId, CRGB & out)
{
    DEBUG_START;

    DEBUG_V (String ("IsInputChannelActive: ") + String(IsInputChannelActive));
    DEBUG_V (String ("pixelId: ") + pixelId);
    DEBUG_V (String ("PixelCount: ") + PixelCount);

//    if (pixelId < PixelCount)
    {
        byte PixelData[sizeof (CRGB)];
        OutputMgr.ReadChannelData (uint32_t (3 * pixelId), sizeof (PixelData), PixelData);

        out.r = PixelData[0];
        out.g = PixelData[1];
        out.b = PixelData[2];
    }

    DEBUG_END;
}  // getPixel

// -----------------------------------------------------------------------------
void c_InputGateControl::setRange (uint16_t FirstPixelId, uint16_t NumberOfPixels, CRGB color)
{
    for (uint16_t i = FirstPixelId; i < min (uint32_t (FirstPixelId + NumberOfPixels), PixelCount); i++)
    {
        setPixel (i, color);
    }
}  // setRange

// -----------------------------------------------------------------------------
void c_InputGateControl::clearRange (uint16_t FirstPixelId, uint16_t NumberOfPixels)
{
    for (uint16_t i = FirstPixelId; i < min (uint32_t (FirstPixelId + NumberOfPixels), PixelCount); i++)
    {
        setPixel (i, {0, 0, 0});
    }
} // c_InputGateControl::clearRange

// -----------------------------------------------------------------------------
void c_InputGateControl::setAll (CRGB color)
{
    setRange (0, PixelCount, color);
}                                                                                 // setAll

// -----------------------------------------------------------------------------
void c_InputGateControl::clearAll ()
{
    clearRange (0, PixelCount);
}                                                                    // clearAll

// -----------------------------------------------------------------------------
c_InputGateControl::CRGB c_InputGateControl::colorWheel (uint8_t pos)
{
    CRGB Response =
    {0, 0, 0};

    pos = 255 - pos;

    if (pos < 85)
    {
        Response =
        {uint8_t (255 - pos * 3), 0, uint8_t (pos * 3)};
    }
    else
    if (pos < 170)
    {
        pos     -= 85;
        Response =
        {0, uint8_t (pos * 3), uint8_t (255 - pos * 3)};
    }
    else
    {
        pos     -= 170;
        Response =
        {uint8_t (pos * 3), uint8_t (255 - pos * 3), 0};
    }

    return(Response);
}  // colorWheel

/*
// -----------------------------------------------------------------------------
uint16_t c_InputGateControl::effectFireFlicker ()
{
    DEBUG_START;

    byte    rev_intensity = 6;   // more=less intensive, less=more intensive
    byte    lum           = max ( EffectColor.r, max (EffectColor.g, EffectColor.b) ) / rev_intensity;

    for (uint16_t i = 0; i < PixelCount; i++)
    {
        uint8_t flicker = random (lum);
        setPixel (
            i,
            CRGB{
            uint8_t (   max (EffectColor.r - flicker, 0) ),
            uint8_t (   max (EffectColor.g - flicker, 0) ),
            uint8_t (   max (EffectColor.b - flicker, 0) )
        });
    }

    EffectStep = (1 + EffectStep) % PixelCount;

    DEBUG_END;
    return(EffectDelay / 10);
}  // effectFireFlicker
*/

// -----------------------------------------------------------------------------
// dCHSV hue 0->360 sat 0->1.0 val 0->1.0
c_InputGateControl::dCHSV c_InputGateControl::rgb2hsv (CRGB in_int)
{
    dCHSV   out;
    dCRGB   in =
    {double(in_int.r) / double(255.0), double(in_int.g) / double(255.0), double(in_int.b) / double(255.0)};
    double  min, max, delta;

    min = in.r < in.g?in.r : in.g;
    min = min < in.b?min : in.b;

    max = in.r > in.g?in.r : in.g;
    max = max > in.b?max : in.b;

    out.v = max;
    delta = max - min;

    if (delta < 0.00001)
    {
        out.s = 0;
        out.h = 0;   // undefined, maybe nan?

        return(out);
    }

    if (max > 0.0)              // NOTE: if Max is == 0, this divide would cause a crash
    {
        out.s = (delta / max);  // s
    }
    else
    {
        // if max is 0, then r = g = b = 0
        // s = 0, v is undefined
        out.s = 0.0;
        out.h = NAN;                              // its now undefined

        return(out);
    }

    if (in.r >= max)                            // > is bogus, just keeps compilor happy
    {
        out.h = (in.g - in.b) / delta;          // between yellow & magenta
    }
    else
    if (in.g >= max)
    {
        out.h = 2.0 + (in.b - in.r) / delta;  // between cyan & yellow
    }
    else
    {
        out.h = 4.0 + (in.r - in.g) / delta;  // between magenta & cyan
    }

    out.h *= 60.0;                              // degrees

    if (out.h < 0.0) out.h += 360.0;

    return(out);
} // c_InputGateControl::rgb2hsv

// -----------------------------------------------------------------------------
// dCHSV hue 0->360 sat 0->1.0 val 0->1.0
c_InputGateControl::CRGB c_InputGateControl::hsv2rgb (dCHSV in)
{
    double  hh, p, q, t, ff;
    long    i;
    dCRGB   out;
    CRGB    out_int =
    {0, 0, 0};

    if (in.s <= 0.0)       // < is bogus, just shuts up warnings
    {
        out.r = in.v;
        out.g = in.v;
        out.b = in.v;
    }
    else
    {
        hh = in.h;

        if (hh >= 360.0) hh = 0.0;

        hh /= 60.0;
        i   = (long)hh;
        ff  = hh - i;
        p   = in.v * (1.0 - in.s);
        q   = in.v * ( 1.0 - (in.s * ff) );
        t   = in.v * ( 1.0 - ( in.s * (1.0 - ff) ) );

        switch (i)
        {
        case 0 :
        {
            out.r = in.v;
            out.g = t;
            out.b = p;
            break;
        }

        case 1 :
        {
            out.r = q;
            out.g = in.v;
            out.b = p;
            break;
        }

        case 2 :
        {
            out.r = p;
            out.g = in.v;
            out.b = t;
            break;
        }

        case 3 :
        {
            out.r = p;
            out.g = q;
            out.b = in.v;
            break;
        }

        case 4 :
        {
            out.r = t;
            out.g = p;
            out.b = in.v;
            break;
        }

        case 5 :
        default :
        {
            out.r = in.v;
            out.g = p;
            out.b = q;
            break;
        }
        } // switch
    }

    out_int.r = min ( uint16_t (255), uint16_t (255 * out.r) );
    out_int.g = min ( uint16_t (255), uint16_t (255 * out.g) );
    out_int.b = min ( uint16_t (255), uint16_t (255 * out.b) );

    return(out_int);
} // c_InputGateControl::hsv2rgb

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
    DEBUG_START;

    pParent->CurrentFsmState = this;

    GateDoors.Close();
    GateAudio.StopPlaying();
    GateLights.Off();

    DEBUG_END;
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
    DEBUG_START;

    FsmInputGateOpeningIntro_Imp.init(pParent);

    DEBUG_END;
} // FsmInputGateIdle::Button_Open_Pressed

// -----------------------------------------------------------------------------
void FsmInputGateIdle::Button_Lights_Pressed (c_InputGateControl * pParent)
{
    DEBUG_START;

    FsmInputGateLights_Imp.init(pParent);

    DEBUG_END;
} // FsmInputGateIdle::Button_Lights_Pressed

// -----------------------------------------------------------------------------
void FsmInputGateIdle::Button_Play_Pressed (c_InputGateControl * pParent)
{
    DEBUG_START;

    FsmInputGatePlaying_Imp.init(pParent);

    DEBUG_END;
} // FsmInputGateIdle::Button_Play_Pressed

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void FsmInputGateOpeningIntro::init(c_InputGateControl * pParent)
{
    DEBUG_START;

    pParent->CurrentFsmState = this;

    GateDoors.Open();
    GateLights.On();
    GateAudio.PlayIntro();

    // TODO set up total cycle time

    DEBUG_END;
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
    DEBUG_START;

    // close the doors
    FsmInputGateClosing_Imp.init(pParent);

    DEBUG_END;
} // FsmInputGateOpeningIntro::Button_Open_Pressed

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void FsmInputGateOpening::init(c_InputGateControl * pParent)
{
    DEBUG_START;

    pParent->CurrentFsmState = this;
    GateAudio.PlayCurrentSelection();

    DEBUG_END;
} // FsmInputGateOpening::init

// -----------------------------------------------------------------------------
void FsmInputGateOpening::poll(c_InputGateControl * pParent)
{
    // _ DEBUG_START;

    // TODO Wait until total open time has expired, then move to closing the door

    // _ DEBUG_END;
} // FsmInputGateOpening::init

// -----------------------------------------------------------------------------
void FsmInputGateOpening::Button_Open_Pressed (c_InputGateControl * pParent)
{
    DEBUG_START;

    // start closing the gate
    FsmInputGateClosing_Imp.init(pParent);

    DEBUG_END;
} // FsmInputGateOpening::Button_Open_Pressed

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void FsmInputGateOpen::init(c_InputGateControl * pParent)
{
    DEBUG_START;

    pParent->CurrentFsmState = this;
    // no actions

    DEBUG_END;
} // FsmInputGateOpen::init

// -----------------------------------------------------------------------------
void FsmInputGateOpen::poll(c_InputGateControl * pParent)
{
    // _ DEBUG_START;

    // TODO wait for the play timer to expire and then move to the closing state

    // _ DEBUG_END;
} // FsmInputGateOpen::init

// -----------------------------------------------------------------------------
void FsmInputGateOpen::Button_Open_Pressed (c_InputGateControl * pParent)
{
    DEBUG_START;

    // close the gate
    FsmInputGateClosing_Imp.init(pParent);

    DEBUG_END;
} // FsmInputGateOpen::Button_Open_Pressed

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void FsmInputGateClosing::init(c_InputGateControl * pParent)
{
    DEBUG_START;

    pParent->CurrentFsmState = this;

    GateDoors.Close();

    // TODO start a closing timer

    DEBUG_END;
} // FsmInputGateClosing::init

// -----------------------------------------------------------------------------
void FsmInputGateClosing::poll(c_InputGateControl * pParent)
{
    // _ DEBUG_START;

    // TOD when the closing timer expires, move to the idle state

    // _ DEBUG_END;
} // FsmInputGateClosing::init

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void FsmInputGateLights::init(c_InputGateControl * pParent)
{
    DEBUG_START;

    pParent->CurrentFsmState = this;
    // turn on the lights effect
    GateLights.On();

    DEBUG_END;
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
    DEBUG_START;

    // turn off lights and move to the opening Intro state
    GateLights.Off();
    FsmInputGateOpeningIntro_Imp.init(pParent);

    DEBUG_END;
} // FsmInputGateLights::init

// -----------------------------------------------------------------------------
void FsmInputGateLights::Button_Lights_Pressed (c_InputGateControl * pParent)
{
    DEBUG_START;

    // move to the idle state
    FsmInputGateIdle_Imp.init(pParent);

    DEBUG_END;
} // FsmInputGateLights::init

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void FsmInputGatePlaying::init(c_InputGateControl * pParent)
{
    DEBUG_START;

    pParent->CurrentFsmState = this;

    // tell the audio to start
    GateAudio.PlayCurrentSelection();

    DEBUG_END;
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
    DEBUG_START;

    FsmInputGatePaused_Imp.init(pParent);

    DEBUG_END;
} // FsmInputGatePlaying::init

// -----------------------------------------------------------------------------
void FsmInputGatePlaying::Button_Skip_Pressed (c_InputGateControl * pParent)
{
    DEBUG_START;

    // move the audio to the next song
    GateAudio.NextSong();

    DEBUG_END;
} // FsmInputGatePlaying::init

// -----------------------------------------------------------------------------
void FsmInputGatePlaying::Button_Stop_Pressed (c_InputGateControl * pParent)
{
    DEBUG_START;

    FsmInputGateIdle_Imp.init(pParent);

    DEBUG_END;
} // FsmInputGatePlaying::init

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void FsmInputGatePaused::init(c_InputGateControl * pParent)
{
    DEBUG_START;

    pParent->CurrentFsmState = this;

    // Tell audio to pause
    GateAudio.PausePlaying();

    DEBUG_END;
} // FsmInputGatePaused::init

// -----------------------------------------------------------------------------
void FsmInputGatePaused::poll(c_InputGateControl * pParent)
{
    // _ DEBUG_START;

    // no action

    // _ DEBUG_END;
} // FsmInputGatePaused::init

// -----------------------------------------------------------------------------
void FsmInputGatePaused::Button_Play_Pressed (c_InputGateControl * pParent)111
{
    DEBUG_START;

    // tell audio to play
    GateAudio.ResumePlaying();

    pParent->CurrentFsmState = &FsmInputGatePlaying_Imp;

    DEBUG_END;
} // FsmInputGatePaused::init

// -----------------------------------------------------------------------------
void FsmInputGatePaused::Button_Stop_Pressed (c_InputGateControl * pParent)
{
    DEBUG_START;

    FsmInputGateIdle_Imp.init(pParent);

    DEBUG_END;
} // FsmInputGatePaused::init

// -----------------------------------------------------------------------------
