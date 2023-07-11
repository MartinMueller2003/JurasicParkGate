/*
 * GateLights.cpp - Output Management class
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
#include <Int64String.h>

#include "GateLights.hpp"
#include "OutputMgr.hpp"

// -----------------------------------------------------------------------------
///< Start up the driver and put it into a safe mode
c_GateLights::c_GateLights ()
{}   // c_GateLights

// -----------------------------------------------------------------------------
///< deallocate any resources and put the output channels into a safe state
c_GateLights::~c_GateLights ()
{
    // DEBUG_START;

    // DEBUG_END;
}  // ~c_GateLights

// -----------------------------------------------------------------------------
///< Start the module
void c_GateLights::Begin ()
{
    // DEBUG_START;

    do  // once
    {
    } while (false);

    // DEBUG_END;
}  // begin

// -----------------------------------------------------------------------------
bool c_GateLights::SetConfig (JsonObject & json)
{
    // DEBUG_START;

    bool ConfigChanged = false;

    // DEBUG_END;

    return(ConfigChanged);
}  // SetConfig

// -----------------------------------------------------------------------------
void c_GateLights::GetConfig (JsonObject & json)
{
    // DEBUG_START;

    // json[CN_miso_pin]  = miso_pin;

    // DEBUG_END;
}  // GetConfig

// -----------------------------------------------------------------------------
void c_GateLights::GetStatus (JsonObject & json)
{
    // DEBUG_START;

//        json[F ("size")] = LittleFS.totalBytes ();

    // DEBUG_END;
}  // GetConfig

// -----------------------------------------------------------------------------
void c_GateLights::On ()
{
    // DEBUG_START;

    Enabled = true;

    // DEBUG_END;
}  // On

// -----------------------------------------------------------------------------
void c_GateLights::Off ()
{
    // DEBUG_START;

    // TODO - Restore Enabled = false;
    Enabled = true;
    clearAll();

    // DEBUG_END;
}  // Off

// -----------------------------------------------------------------------------
void c_GateLights::Poll ()
{
    // _ DEBUG_START;

    do // once
    {
        if ( !EffectDelayTimer.IsExpired () )
        {
            break;
        }

        if(!Enabled)
        {
            break;
        }

        byte    rev_intensity = 6;   // more=less intensive, less=more intensive
        byte    lum           = max ( EffectColor.r, max (EffectColor.g, EffectColor.b) ) / rev_intensity;

        for (uint16_t i = 0; i < PixelCount; i++)
        {
            uint8_t red = random (120) + 135;
            uint8_t grn = random (red/2);
            uint8_t blu = random (grn/2);
            setPixel (
                i,
                CRGB{
                uint8_t ( red ),
                uint8_t ( grn ),
                uint8_t ( blu ),
            });
        }

        EffectWait = EffectDelay/ 10;

        EffectDelayTimer.StartTimer (random(EffectWait)+75);

    } while(false);

    // _ DEBUG_END;
}  // Poll

// -----------------------------------------------------------------------------
void c_GateLights::clearAll ()
{
    clearRange (0, PixelCount);
} // clearAll

// -----------------------------------------------------------------------------
void c_GateLights::setRange (uint16_t FirstPixelId, uint16_t NumberOfPixels, CRGB color)
{
    for (uint16_t i = FirstPixelId; i < min (uint32_t (FirstPixelId + NumberOfPixels), PixelCount); i++)
    {
        setPixel (i, color);
    }
}  // setRange

// -----------------------------------------------------------------------------
void c_GateLights::setPixel (uint16_t pixelId, CRGB color)
{
    // DEBUG_START;

    // DEBUG_V (String ("IsInputChannelActive: ") + String(IsInputChannelActive));
    // DEBUG_V (String ("pixelId: ") + pixelId);
    // DEBUG_V (String ("PixelCount: ") + PixelCount);

    uint32_t StartChannel = pixelId * 3;

    if(pixelId > 3)
    {
        // skip the last 4 channels on the device
        StartChannel += 4;
    }

    if ( pixelId < PixelCount)
    {
        uint8_t PixelBuffer[sizeof (CRGB) + 1];

        // DEBUG_V(String("ChannelsPerPixel * pixelId: 0x") + String(uint(ChannelsPerPixel * pixelId), HEX));
        // DEBUG_V (String ("EffectBrightness: ") + String (EffectBrightness));
        // DEBUG_V (String ("color.r: ") + String (color.r));
        // DEBUG_V (String ("color.g: ") + String (color.g));
        // DEBUG_V (String ("color.b: ") + String (color.b));

        PixelBuffer[0] = color.r * EffectBrightness;
        PixelBuffer[1] = color.g * EffectBrightness;
        PixelBuffer[2] = color.b * EffectBrightness;

        OutputMgr.WriteChannelData (StartChannel, 3, PixelBuffer);
    }

    // DEBUG_END;
}  // setPixel

// -----------------------------------------------------------------------------
void c_GateLights::clearRange (uint16_t FirstPixelId, uint16_t NumberOfPixels)
{
    for (uint16_t i = FirstPixelId; i < min (uint32_t (FirstPixelId + NumberOfPixels), PixelCount); i++)
    {
        setPixel (i, {0, 0, 0});
    }
} // clearRange


// create a global instance of the Gate Audio
c_GateLights GateLights;
