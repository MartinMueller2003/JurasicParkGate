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
void Poll      ();
void GetConfig (JsonObject & json);
bool SetConfig (JsonObject & json);
void GetStatus (JsonObject & json);

void On();
void Off();

void GetDriverName    (String & Name) {Name = "GateLights";}

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

// typedef uint16_t(c_InputGateControl::* EffectFunc)(void);
typedef struct MQTTConfiguration_s
{
    String effect;
    bool mirror;
    bool allLeds;
    uint8_t brightness;
    bool whiteChannel;
    CRGB color;
} MQTTConfiguration_s;

    #define MIN_EFFECT_DELAY        10
    #define MAX_EFFECT_DELAY        65535
    #define DEFAULT_EFFECT_DELAY    1000

using timeType = decltype( millis () );

uint32_t PixelCount = 10;

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

CRGB EffectColor        = {255, 200, 10};
uint16_t EffectDelay        = 1000;
uint32_t EffectWait = 32;                                   /* How long to wait for the effect to run again */
FastTimer EffectDelayTimer;
bool Enabled = false;
uint32_t EffectBrightness = 1;

}; // c_GateLights

extern c_GateLights GateLights;
