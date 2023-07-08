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

// dCRGB red, green, blue 0->1.0
struct dCRGB
{
    double r;
    double g;
    double b;
    dCRGB operator= (dCRGB a)
    {
        r = a.r;
        g = a.g;
        b = a.b;

        return(a);
    } // =
};

// CRGB red, green, blue 0->255
struct CRGB
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

// dCHSV hue 0->360 sat 0->1.0 val 0->1.0
struct dCHSV
{
    double h;
    double s;
    double v;
};

typedef uint16_t(c_InputGateControl::* EffectFunc)(void);
typedef struct MQTTConfiguration_s
{
    String effect;
    bool mirror;
    bool allLeds;
    uint8_t brightness;
    bool whiteChannel;
    CRGB color;
} MQTTConfiguration_s;

// functions to be provided by the derived class
void Begin ();                                         ///< set up the operating environment based on the current config (or defaults)
bool SetConfig (JsonObject & jsonConfig);              ///< Set a new config in the driver
void SetMqttConfig (MQTTConfiguration_s & mqttConfig); ///< Set a new config in the driver
void GetConfig (JsonObject & jsonConfig);              ///< Get the current config used by the driver
void GetMqttConfig (MQTTConfiguration_s & mqttConfig); ///< Get the current config used by the driver
void GetMqttEffectList (JsonObject & jsonConfig);      ///< Get the current config used by the driver
void GetStatus (JsonObject & jsonStatus);
void Process ();                                       ///< Call from loop(),  renders Input data
void GetDriverName (String & sDriverName) { sDriverName = "Gate"; }                                                                           ///< get the name for the instantiated driver
void SetBufferInfo (uint32_t BufferSize);

protected:

    friend class FsmInputGateBooting ;
    friend class FsmInputGateIdle;
    friend class FsmInputGateOpening;
    friend class FsmInputGateOpen;
    friend class FsmInputGateClosing;
    friend class FsmInputGateLights;
    friend class FsmInputGatePlaying;
    friend class FsmInputGatePaused;
    FsmInputGateCommon * CurrentFsmState = nullptr;

void validateConfiguration ();

bool HasBeenInitialized = false;

    #define MIN_EFFECT_DELAY        10
    #define MAX_EFFECT_DELAY        65535
    #define DEFAULT_EFFECT_DELAY    1000

using timeType = decltype( millis () );

uint32_t PixelCount = 0;
CRGB FireColor         = {183, 0, 255};                    /* Externally controlled effect color */

void setPixel (uint16_t idx,
 CRGB                   color);
void GetPixel (uint16_t pixelId,
 CRGB &                 out);
void setRange (uint16_t first,
 uint16_t               len,
 CRGB                   color);
void clearRange (uint16_t   first,
 uint16_t                   len);
void setAll (CRGB color);

CRGB colorWheel (uint8_t pos);
dCHSV rgb2hsv (CRGB in);
CRGB hsv2rgb (dCHSV in);

void setColor (String & NewColor);
void setBrightness (float brightness);
void setSpeed (uint16_t speed);
void setDelay (uint16_t delay);
void PollFlash ();

void clearAll ();

}; // class c_InputGateControl

// -----------------------------------------------------------------------------
// ---------- FSM Definitions --------------------------------------------------
// -----------------------------------------------------------------------------
class FsmInputGateCommon
{
public:
    FsmInputGateCommon() {}
    virtual ~FsmInputGateCommon() {}
    virtual void init (c_InputGateControl * pParent) = 0;
    virtual void poll (c_InputGateControl * pParent) = 0;
    virtual String name () = 0;
    virtual void Button_Open_Pressed (c_InputGateControl * pParent) {}
    virtual void Button_Lights_Pressed (c_InputGateControl * pParent) {}
    virtual void Button_Play_Pressed (c_InputGateControl * pParent) {}
    virtual void Button_Skip_Pressed (c_InputGateControl * pParent) {}
    virtual void Button_Stop_Pressed (c_InputGateControl * pParent) {}

}; // FsmInputGateCommon

// -----------------------------------------------------------------------------
class FsmInputGateBooting final : public FsmInputGateCommon
{
public:
    void init (c_InputGateControl * pParent) override;
    void poll (c_InputGateControl * pParent) override;
    String name () {return F("Booting");}
}; // FsmInputGateBooting

// -----------------------------------------------------------------------------
class FsmInputGateIdle final : public FsmInputGateCommon
{
public:
    void init (c_InputGateControl * pParent) override;
    void poll (c_InputGateControl * pParent) override;
    String name () {return F("Idle");}
    void Button_Open_Pressed (c_InputGateControl * pParent) override;
    void Button_Lights_Pressed (c_InputGateControl * pParent) override;
    void Button_Play_Pressed (c_InputGateControl * pParent) override;
    // void Button_Skip_Pressed (c_InputGateControl * pParent) override;
    // void Button_Stop_Pressed (c_InputGateControl * pParent) override;

}; // FsmInputGateIdle

// -----------------------------------------------------------------------------
class FsmInputGateOpening final : public FsmInputGateCommon
{
public:
    void init (c_InputGateControl * pParent) override;
    void poll (c_InputGateControl * pParent) override;
    String name () {return F("Opening");}
    void Button_Open_Pressed (c_InputGateControl * pParent) override;
    // void Button_Lights_Pressed (c_InputGateControl * pParent) override;
    // void Button_Play_Pressed (c_InputGateControl * pParent) override;
    // void Button_Skip_Pressed (c_InputGateControl * pParent) override;
    // void Button_Stop_Pressed (c_InputGateControl * pParent) override;

}; // FsmInputGateOpening

// -----------------------------------------------------------------------------
class FsmInputGateOpen final : public FsmInputGateCommon
{
public:
    void init (c_InputGateControl * pParent) override;
    void poll (c_InputGateControl * pParent) override;
    String name () {return F("Open");}
    void Button_Open_Pressed (c_InputGateControl * pParent) override;
    // void Button_Lights_Pressed (c_InputGateControl * pParent) override;
    // void Button_Play_Pressed (c_InputGateControl * pParent) override;
    // void Button_Skip_Pressed (c_InputGateControl * pParent) override;
    // void Button_Stop_Pressed (c_InputGateControl * pParent) override;

}; // FsmInputGateOpen

// -----------------------------------------------------------------------------
class FsmInputGateClosing final : public FsmInputGateCommon
{
public:
    void init (c_InputGateControl * pParent) override;
    void poll (c_InputGateControl * pParent) override;
    String name () {return F("Closing");}
}; // FsmInputGateClosing

// -----------------------------------------------------------------------------
class FsmInputGateLights final : public FsmInputGateCommon
{
public:
    void init (c_InputGateControl * pParent) override;
    void poll (c_InputGateControl * pParent) override;
    String name () {return F("Lights On");}
    void Button_Open_Pressed (c_InputGateControl * pParent) override;
    void Button_Lights_Pressed (c_InputGateControl * pParent) override;
    // void Button_Play_Pressed (c_InputGateControl * pParent) override;
    // void Button_Skip_Pressed (c_InputGateControl * pParent) override;
    // void Button_Stop_Pressed (c_InputGateControl * pParent) override;

}; // FsmInputGateLights

// -----------------------------------------------------------------------------
class FsmInputGatePlaying final : public FsmInputGateCommon
{
public:
    void init (c_InputGateControl * pParent) override;
    void poll (c_InputGateControl * pParent) override;
    String name () {return F("Playing");}
    // void Button_Open_Pressed (c_InputGateControl * pParent) override;
    // void Button_Lights_Pressed (c_InputGateControl * pParent) override;
    void Button_Play_Pressed (c_InputGateControl * pParent) override;
    void Button_Skip_Pressed (c_InputGateControl * pParent) override;
    void Button_Stop_Pressed (c_InputGateControl * pParent) override;

}; // FsmInputGateLights

// -----------------------------------------------------------------------------
class FsmInputGatePaused final : public FsmInputGateCommon
{
public:
    void init (c_InputGateControl * pParent) override;
    void poll (c_InputGateControl * pParent) override;
    String name () {return F("Paused");}
    // void Button_Open_Pressed (c_InputGateControl * pParent) override;
    // void Button_Lights_Pressed (c_InputGateControl * pParent) override;
    void Button_Play_Pressed (c_InputGateControl * pParent) override;
    // void Button_Skip_Pressed (c_InputGateControl * pParent) override;
    void Button_Stop_Pressed (c_InputGateControl * pParent) override;

}; // FsmInputGatePaused
