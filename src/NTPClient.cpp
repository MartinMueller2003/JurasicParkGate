/**
  * The MIT License (MIT)
  * Copyright (c) 2015 by Fabrice Weinberg
  *
  * Permission is hereby granted, free of charge, to any person obtaining a copy
  * of this software and associated documentation files (the "Software"), to deal
  * in the Software without restriction, including without limitation the rights
  * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  * copies of the Software, and to permit persons to whom the Software is
  * furnished to do so, subject to the following conditions:
  * The above copyright notice and this permission notice shall be included in all
  * copies or substantial portions of the Software.
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  * SOFTWARE.
  */

#include "NTPClient.h"

NTPClient::NTPClient (UDP & udp)
{
    _udp =  & udp;
}

NTPClient::NTPClient (UDP & udp, long timeOffset)
{
    _udp        =  & udp;
    _timeOffset = timeOffset;
}

NTPClient::NTPClient (UDP & udp, const char * poolServerName)
{
    _udp            =  & udp;
    _poolServerName = poolServerName;
}

NTPClient::NTPClient (UDP & udp, const char * poolServerName, long timeOffset)
{
    _udp            =  & udp;
    _timeOffset     = timeOffset;
    _poolServerName = poolServerName;
}

NTPClient::NTPClient (UDP & udp, const char * poolServerName, long timeOffset, unsigned long updateInterval)
{
    _udp            =  & udp;
    _timeOffset     = timeOffset;
    _poolServerName = poolServerName;
    _updateInterval = updateInterval;
}

void NTPClient::begin () {begin (NTP_DEFAULT_LOCAL_PORT);}

void NTPClient::begin (int port)
{
    _port = port;

    _udp->begin (_port);

    _udpSetup = true;

    _lastUpdate = 0;
}

bool NTPClient::forceUpdate ()
{
    #ifdef DEBUG_NTPClient
        Serial.println ("Update from NTP Server");
    #endif // ifdef DEBUG_NTPClient

    bool response = false;

    unsigned long   startTime       = millis ();
    unsigned long   endTime         = 0;
    unsigned long   ExpirationTime  = startTime + 5000;

    sendNTPPacket ();

    do
    {
        endTime = millis ();
        if (true == (response = CheckForResponse ()))
        {
            _lastUpdate = endTime;

            break;
        }
    } while (millis () - ExpirationTime);

    return response;
}  // forceUpdate

bool NTPClient::CheckForResponse ()
{
    bool response = false;

    do  // once
    {
        // did we get a response?
        int cb = _udp->parsePacket ();
        if (0 == cb)
        {
            break;
        }

        _udp->read (_packetBuffer, NTP_PACKET_SIZE);

        unsigned long   highWord    = word (_packetBuffer[40], _packetBuffer[41]);
        unsigned long   lowWord     = word (_packetBuffer[42], _packetBuffer[43]);
        // combine the four bytes (two words) into a long integer
        // this is NTP time (seconds since Jan 1 1900):
        unsigned long secsSince1900 = highWord << 16 | lowWord;

        _currentEpoc = secsSince1900 - SEVENZYYEARS;

        response = true;
    } while (false);

    return response;
}  // CheckForResponse

bool NTPClient::update ()
{
    if ((millis () - _lastUpdate >= _updateInterval)    // Update after _updateInterval
        || (_lastUpdate == 0))                          // Update if there was no update yet.
    {
        if (!_udpSetup) begin ();                       // setup the UDP client if needed

        return forceUpdate ();
    }

    return true;
}

unsigned long NTPClient::getRawEpochTime () const
{
    return _timeOffset +    // User offset in seconds
           _currentEpoc;    // Time since last update
}

unsigned long NTPClient::getEpochTime () const
{
    return _timeOffset +                        // User offset in seconds
           _currentEpoc +                       // Epoc returned by the NTP server (seconds)
           ((millis () - _lastUpdate) / 1000);  // Time since last update
}

int NTPClient::getDay () const
{
    return ((getEpochTime () / 86400L) + 4) % 7;  // 0 is Sunday
}

int NTPClient::getHours () const
{
    return (getEpochTime () % 86400L) / 3600;
}

int NTPClient::getMinutes () const
{
    return (getEpochTime () % 3600) / 60;
}

int NTPClient::getSeconds () const
{
    return getEpochTime () % 60;
}

String NTPClient::getFormattedTime () const
{
    unsigned long   rawTime = getEpochTime ();
    unsigned long   hours   = (rawTime % 86400L) / 3600;
    String hoursStr         = hours < 10 ? "0" + String (hours) : String (hours);

    unsigned long minutes   = (rawTime % 3600) / 60;
    String minuteStr        = minutes < 10 ? "0" + String (minutes) : String (minutes);

    unsigned long seconds   = rawTime % 60;
    String secondStr        = seconds < 10 ? "0" + String (seconds) : String (seconds);

    return hoursStr + ":" + minuteStr + ":" + secondStr;
}

void NTPClient::end ()
{
    _udp->stop ();

    _udpSetup = false;
}

void NTPClient::setTimeOffset (int timeOffset) {_timeOffset = timeOffset;}

void NTPClient::setUpdateInterval (unsigned long updateInterval) {_updateInterval = updateInterval;}

void NTPClient::setPoolServerName (const char * poolServerName) {_poolServerName = poolServerName;}

void NTPClient::sendNTPPacket ()
{
    // set all bytes in the buffer to 0
    memset (_packetBuffer, 0, sizeof (_packetBuffer));
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)

    _packetBuffer[0]    = 0b11100011;   // LI, Version, Mode
    _packetBuffer[1]    = 0;            // Stratum, or type of clock
    _packetBuffer[2]    = 6;            // Polling Interval
    _packetBuffer[3]    = 0xEC;         // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    _packetBuffer[12]   = 49;
    _packetBuffer[13]   = 0x4E;
    _packetBuffer[14]   = 49;
    _packetBuffer[15]   = 52;

    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    _udp->beginPacket (_poolServerName, 123);  // NTP requests are to port 123
    _udp->write (_packetBuffer, NTP_PACKET_SIZE);
    _udp->endPacket ();
}
