/*
  * Provide access to the real time clock
  */
#include <WiFiUdp.h>
#include <driver/gpio.h>
#include <Wire.h>
#include <vector>

#include "JurasicParkGate.hpp"
#include "RtcMgr.hpp"
#include "WiFiMgr.hpp"
#include "NTPClient.h"

WiFiUDP m_Udp;
// NTPClient   timeClient (m_Udp, "time.nist.gov", 3600, 60000);
NTPClient   timeClient (m_Udp, "time.nist.gov");
TwoWire     & GhWire = Wire;

/*****************************************************************************/
/* global data                                                               */
/*****************************************************************************/
// instantiate the rtcMgr control object (singleton)
class c_rtcMgr RtcMgr;

static uint64_t prescaleCounter = 0;
static uint64_t prescaleValue   = 74000;

/*****************************************************************************/
/* Local Macros                                                              */
/*****************************************************************************/
#define M_SPRINTF sprintf

/*****************************************************************************/
/*  fsm                                                                      */
/*****************************************************************************/
static fsm_rtc_state_TestRTC fsm_rtc_state_TestRTC_imp;
static fsm_rtc_state_WaitForWiFiToComeUp fsm_rtc_state_WaitForWiFiToComeUp_imp;
static fsm_rtc_state_WaitBeforeSendingNextRequest fsm_rtc_state_WaitBeforeSendingNextRequest_imp;
static fsm_rtc_state_RefreshNtpBasedDate fsm_rtc_state_RefreshNtpBasedDate_imp;

/*****************************************************************************/
/* Code                                                                      */
/*****************************************************************************/
c_rtcMgr::c_rtcMgr (void) : c_Common (String ("RtcMgr"))
{
    uint32_t BussId = 0;
    for (auto & CurrentBussInfo : BussInfo)
    {
        CurrentBussInfo.BussId      = BussId++;
        CurrentBussInfo.NumDevices  = 0;
    }
}  // c_rtcMgr

/*****************************************************************************/
c_rtcMgr::~c_rtcMgr (void) {}

/*****************************************************************************/
void DEBUG_TracePulse (uint32_t numPulses)
{
    // return;
    // DEBUG_START;
    // DEBUG_V ("numPulses: " + String (numPulses));

    while (numPulses > 10)
    {
        digitalWrite (TracePulseGpio, HIGH);
        delayMicroseconds (30);
        digitalWrite (TracePulseGpio, LOW);
        delayMicroseconds (30);
        numPulses -= 10;
    }

    for (uint32_t count = 0;count < numPulses;++count)
    {
        digitalWrite (TracePulseGpio, HIGH);
        delayMicroseconds (10);
        digitalWrite (TracePulseGpio, LOW);
        delayMicroseconds (10);
    }

    // DEBUG_END;
}

/*****************************************************************************/
void c_rtcMgr::HardResetI2cMux ()
{
    // DEBUG_START;
    pinMode (I2CResetPin, OUTPUT);
    // DEBUG_V();
    digitalWrite (I2CResetPin, LOW);
    // DEBUG_V();
    delayMicroseconds (10);
    digitalWrite (I2CResetPin, HIGH);
    // DEBUG_V();
    delayMicroseconds (1);
    // DEBUG_END;
}  // HardResetI2cMux

/*****************************************************************************/
void c_rtcMgr::SetUpI2cChannels ()
{
    // DEBUG_START;
#ifdef SupportI2cMux
    uint8_t ChansToEnable = 0;
    for (uint8_t ChanToTest = 0x80;ChanToTest > 0;ChanToTest >>= 1)
    {
        HardResetI2cMux ();
        i2cmux.writeRegister (ChanToTest);
        if (i2cmux.readRegister () != ChanToTest)
        {
            // DEBUG_V (String ("Could not verify I2C Mux setting for 0x") + String (ChanToTest, HEX));
        }
        else
        {
            uint32_t numDevices = ScanI2Cbus (MUX_I2C_ADDRESS, MUX_I2C_ADDRESS);
            if (0 != numDevices)
            {
                // DEBUG_V (String ("Found bus: 0x") + String (ChanToTest, HEX) + " Num devices: " + String (numDevices));
                ChansToEnable |= ChanToTest;
            }
            else
            {
                // DEBUG_V (String ("Broken bus: 0x") + String (ChanToTest, HEX));
            }
        }
    }

    HardResetI2cMux ();
    // DEBUG_V (String ("Enable bus: 0x") + String (ChansToEnable, HEX));
    i2cmux.writeRegister (ChansToEnable);

    ScanI2Cbus (1, 127, true);
#endif // def SupportI2cMux

    // DEBUG_END;
}

/*****************************************************************************/
void c_rtcMgr::Init (void)
{
    // DEBUG_START;

    do  // once
    {
        setenv ("TZ", "EST5EDT,M3.2.0,M11.1.0", 1);
        tzset ();
        struct timeval NewNow =
        {.tv_sec    = time_t (0),.tv_usec = time_t (0)};
        settimeofday (& NewNow, NULL);
        // DEBUG_V ();

        pinMode (TracePulseGpio, OUTPUT);
        // DEBUG_V ();
        digitalWrite (TracePulseGpio, LOW);
        // DEBUG_V ();
        // DEBUG_TracePulse (1);
        // DEBUG_V ();

        GhWire.end ();
        if (!GhWire.begin (I2CDataPin, I2CClockPin))
        {
            LOG_ERROR (F ("Failed to set up the two wire interface"));
            break;
        }
#ifdef SupportI2cMux

        // DEBUG_V ();

        // DEBUG_TracePulse (2);
        HardResetI2cMux ();
        // DEBUG_TracePulse (3);
        ScanI2Cbus (MUX_I2C_ADDRESS, MUX_I2C_ADDRESS);
        // DEBUG_TracePulse (4);

        SoftResetI2C ();
        // DEBUG_TracePulse (5);

        HardResetI2cMux ();
        // DEBUG_TracePulse (6);

        // delay(100);
        // HardResetI2cMux ();

        do
        {
            // DEBUG_TracePulse (7);

            // DEBUG_V (   String ("Detect Devices before mux begin, Found ") + String (ScanI2Cbus (8, 127)) + " devices.");
            // DEBUG_V ("Detect MUX");
            if (0 == ScanI2Cbus (MUX_I2C_ADDRESS, MUX_I2C_ADDRESS))
            {
                LOG_ERROR (F ("Could not detect the I2C MUX"));
                // delay(2000);
                break;
            }
            else
            {
                // DEBUG_TracePulse (8);
                // delay(2000);
                LOG_INFO (F ("I2C Mux Detected"));
            }
        } while (false);

        // DEBUG_V ("Begin MUX");
        // DEBUG_TracePulse (9);
        HardResetI2cMux ();
        i2cmux.begin (GhWire);

        // DEBUG_TracePulse (10);

        HardResetI2cMux ();
        if (i2cmux.readRegister () != 0)
        {
            LOG_ERROR (F ("Could not verify I2C Mux setting"));
        }
        else
        {
            // DEBUG_V("Verified I2C Mux setting");
        }

        // DEBUG_TracePulse (11);

        SetUpI2cChannels ();
        // DEBUG_TracePulse (12);
#endif // def SupportI2cMux

        // DEBUG_V (   String ("Detect Devices before RTC begin, Found ") + String (ScanI2Cbus (8, 127)) + " devices.");
        // DEBUG_V ("Detect RTC");
        if (0 == ScanI2Cbus (RTC_I2C_ADDRESS, RTC_I2C_ADDRESS))
        {
            // DEBUG_TracePulse (7);
            LOG_ERROR (F ("Couldn't find RTC"));
            break;
        }

        // DEBUG_TracePulse (13);

        // DEBUG_V (String (F ("rtcDevice.begin")));
        if (!rtcDevice.begin (& GhWire))
        {
            // DEBUG_TracePulse (9);
            LOG_ERROR (F ("Couldn't Start RTC - REBOOTING"));
            // reboot = true;
            foundRtc = false;
            break;
        }

        // DEBUG_TracePulse (14);

        // DEBUG_V ("Get the time");
        foundRtc = true;
        LOG_INFO (F ("RTC Is Present"));

        setCurrentDateTime (GetRtcTime ());
        // DEBUG_TracePulse (15);

        // DEBUG_V ("Check if RTC lost power");
        if (rtcDevice.lostPower ())
        {
            // DEBUG_TracePulse (13);
            LOG_WARN (F ("RTC lost power!"));
        }

        // DEBUG_TracePulse (16);

        // DEBUG_V(String("Detect Devices after RTC begin, Found ") + String(ScanI2Cbus(8, 127)) + " devices.");
    } while (false);

    fsm_rtc_state_TestRTC_imp.Init (this);

    // DEBUG_END;
}  // c_rtcMgr::Init

/*****************************************************************************/
void c_rtcMgr::SetConfig (JsonObject & jsonData)
{
    // DEBUG_START;

    do  // once
    {
        // do we have an entry in this record that we can process
        if (false == jsonData.containsKey (M_RTCMGR_JSON_ROOT))
        {
            // DEBUG_V (F ("Could not find requested root in config data"));
            // the requested sub tree does not exist
            break;
        }  // failed to find the desired object

        JsonObject JsonDataObject = jsonData[M_RTCMGR_JSON_ROOT];

        READ_JSON ( JsonDataObject, m_iTimeCorrcetionFactorHours,   M_TIME_CORRECTION_FACTOR_JSON_NAME, c_CfgMgr::ScheduleUpdate_t::ConfigUpdate);
        READ_JSON ( JsonDataObject, m_timeServer,                   M_TIME_SERVER_JSON_NAME,            c_CfgMgr::ScheduleUpdate_t::ConfigUpdate);

        setenv ("TZ", "EST5EDT,M3.2.0/2,M11.1.0", 1);
        tzset ();
    } while (false);

    // DEBUG_END;
}  // SetConfig

/*****************************************************************************/
void c_rtcMgr::GetConfig (JsonObject & jsonRoot)
{
    // DEBUG_START;

    JsonObject RtcMgrData = jsonRoot.createNestedObject (M_RTCMGR_JSON_ROOT);
    RtcMgrData[M_TIME_CORRECTION_FACTOR_JSON_NAME]  = m_iTimeCorrcetionFactorHours;
    RtcMgrData[M_TIME_SERVER_JSON_NAME]             = m_timeServer;

    // DEBUG_END;
}  // GetConfig

/*****************************************************************************/
void c_rtcMgr::GetStatus (JsonObject & jsonRoot)
{
    // DEBUG_START;

    JsonObject RtcMgrData = jsonRoot.createNestedObject (M_RTCMGR_JSON_ROOT);

    RtcMgrData["diff"]          = RtcDiff;
    RtcMgrData["time"]          = GetRtcTime ().unixtime ();
    RtcMgrData["local"]         = now ().unixtime ();
    RtcMgrData["bad"]           = BadRtcReadingCount;
    RtcMgrData["localupdates"]  = NumLocalTimeUpdates;
    RtcMgrData["rtcupdates"]    = NumRtcTimeUpdates;
    RtcMgrData[CN_present]      = foundRtc;

    // DEBUG_END;
}  // GetConfig

/*****************************************************************************/
TwoWire &c_rtcMgr::GetTwoWire () {return GhWire;}  // GetTwoWire

/*****************************************************************************/
void c_rtcMgr::Poll (void)
{
    // DEBUG_START;

    m_pCurrentFsmState->Poll (this);

    DateTime now = UtcDateTime ();

    if (now.minute () != LastDisplayTime)
    {
        LastDisplayTime =  now.minute ();
        String DisplayString;
        currentDateTime (DisplayString, false);  // ask for time without seconds
        #ifdef USE_DISPLAY
            DisplayMgr.println (cDisplayMgr::eZone::TimeZone, DisplayString);
        #endif // def USE_DISPLAY
    }

    // HardResetI2cMux();
    // GhWire.requestFrom(uint8_t(RTC_I2C_ADDRESS), size_t(1), true);
    // ScanI2Cbus(RTC_I2C_ADDRESS, RTC_I2C_ADDRESS);
    // SetUpI2cChannels();

    // DEBUG_END;
}  // Poll

/*****************************************************************************/
void c_rtcMgr::NetworkStateChanged (bool NewState) {m_pCurrentFsmState->NetworkStateChanged (this, NewState);}  // NetworkStateChanged

/*****************************************************************************/
/*
  *
  *	get the current date and time in a formatted string
  *
  *	needs
  *		string to put the result into
  *	returns
  *		nothing
  */
DateTime c_rtcMgr::UtcDateTime (void)
{
    struct timeval tv;
    gettimeofday (& tv, NULL);

    return DateTime (tv.tv_sec);
}  // currentDateTime

/*****************************************************************************/
DateTime c_rtcMgr::now ()
{
    // DEBUG_START;

    tm TimeInfo;

    // DEBUG_END;
    return now (TimeInfo);
}  // now

/*****************************************************************************/
DateTime c_rtcMgr::now (tm & TimeInfo)
{
    // DEBUG_START;

    memset ((void *)& TimeInfo, 0x00, sizeof (tm));
    getLocalTime (& TimeInfo, 5);

    // DEBUG_END;
    return DateTime (TimeInfo.tm_year + 1900, TimeInfo.tm_mon + 1, TimeInfo.tm_mday, TimeInfo.tm_hour, TimeInfo.tm_min, TimeInfo.tm_sec);
}  // now

/*****************************************************************************/
/*
  *
  *	get the current date and time in a formatted string
  *
  *	needs
  *		string to put the result into
  *	returns
  *		nothing
  */
void c_rtcMgr::currentDateTime (String & sOutput, bool IncludeSeconds)
{
    // DEBUG_START;

    // DEBUG_V (   String ("TimeStamp: ") + now.timestamp ());

    DateTime _now = now ();
    char buffer[50];

    if (IncludeSeconds)
    {
        M_SPRINTF (
            buffer,
            "%04d-%02d-%02d %02d:%02d:%02d",
            _now.year (),
            _now.month (),
            _now.day (),
            _now.hour (),
            _now.minute (),
            _now.second ());
    }
    else
    {
        M_SPRINTF (
            buffer,
            "%04d-%02d-%02d %02d:%02d",
            _now.year (),
            _now.month (),
            _now.day (),
            _now.hour (),
            _now.minute ());
    }

    sOutput = String (buffer);

    // DEBUG_END;
}  // currentDateTime

/*****************************************************************************/
/*
  *
  *	get the current Year in a formatted string
  *
  *	needs
  *		string to put the result into
  *	returns
  *		nothing
  */
void c_rtcMgr::currentYear (String & sOutput)
{
    // DEBUG_START;

    char buffer[25];
    M_SPRINTF (buffer, "%04d", UtcDateTime ().year ());
    sOutput = String (buffer);

    // DEBUG_END;
}  // currentDateTime

/*****************************************************************************/
/*
  *
  *	get the current month in a formatted string
  *
  *	needs
  *		string to put the result into
  *	returns
  *		nothing
  */
void c_rtcMgr::currentMonth (String & sOutput)
{
    // DEBUG_START;
    char buffer[25];
    M_SPRINTF (buffer, "%02d", UtcDateTime ().month ());
    sOutput = String (buffer);

    // DEBUG_END;
}  // currentDateTime

/*****************************************************************************/
/*
  *
  *	get the current day in a formatted string
  *
  *	needs
  *		string to put the result into
  *	returns
  *		nothing
  */
void c_rtcMgr::currentDay (String & sOutput)
{
    // DEBUG_START;

    char buffer[25];
    M_SPRINTF (buffer, "%02d", UtcDateTime ().day ());
    sOutput = String (buffer);

    // DEBUG_END;
}  // currentDateTime

/*****************************************************************************/
double c_rtcMgr::GetRtcTemperature ()
{
    // DEBUG_START;
    double Response = 0.0;
    if (foundRtc)
    {
        Response = rtcDevice.getTemperature ();
    }

    // DEBUG_END;
    return Response;
}

/*****************************************************************************/
void c_rtcMgr::SoftResetI2C ()
{
    // DEBUG_START;

    digitalWrite (  I2CClockPin,    HIGH);
    digitalWrite (  I2CDataPin,     HIGH);

    if (!GhWire.end ())
    {
        // DEBUG_V ("Failed to Stop the two wire interface");
    }

    pinMode (   I2CClockPin,    OUTPUT);
    pinMode (   I2CDataPin,     OUTPUT);

    digitalWrite (  I2CClockPin,    HIGH);
    digitalWrite (  I2CDataPin,     HIGH);

    // digitalWrite(I2CClockPin, HIGH);
    // digitalWrite (I2CDataPin, HIGH);
    delayMicroseconds (6);

    for (int count = 0;count < 18;++count)
    {
        digitalWrite (I2CClockPin, ((0 == (count & 0x1)) ? LOW : HIGH));
        delayMicroseconds (6);
    }

    digitalWrite (I2CClockPin, HIGH);
    delayMicroseconds (6);
    digitalWrite (I2CDataPin, LOW);
    delayMicroseconds (6);
    digitalWrite (I2CDataPin, HIGH);
    delayMicroseconds (10);

    if (!GhWire.begin (I2CDataPin, I2CClockPin))
    {
        // DEBUG_V ("Failed to Start the two wire interface");
    }

    // DEBUG_END;
}

/*****************************************************************************/
uint32_t c_rtcMgr::ScanI2Cbus (int start, int end, bool DisplayDevices)
{
    // DEBUG_START;
    // DEBUG_V (String ("I2C scanner. Scanning ") + String (start) + " - " + String (end));
    uint32_t count = 0;

    for (byte i = start;i <= end;i++)
    {
        // DEBUG_V (String ("Testing address: ") + String (i, DEC) + " (0x" + String (i, HEX) + ")");
        delayMicroseconds (10);
        GhWire.beginTransmission (i);           // Begin I2C transmission Address (i)
        if (GhWire.endTransmission (true) == 0) // Receive 0 = success (ACK response)
        {
            if (DisplayDevices)
            {
                LOG_INFO (String ("Found device at address: ") + String (i, DEC) + " (0x" + String (i, HEX) + ")");
            }

            count++;
        }
    }

    // DEBUG_V (String ("Found ") + String (count, DEC) + " device(s).");
    // DEBUG_END;
    return count;
}

/*****************************************************************************/
/*
  * Set a new UTC time value
  *
  * needs
  *    nothing
  * returns
  *    nothing
  */
void c_rtcMgr::setCurrentDateTime (DateTime newDateTime)
{
    // DEBUG_START;

    DateTime OldDateTime = UtcDateTime ();
    // DEBUG_V (   String ("         Old Time: ") + String (OldDateTime.unixtime ()));
    // DEBUG_V (   String (" new NTP/RTC Time: ") + String (newDateTime.unixtime ()));

    do  // once
    {
        if (1 < abs (int32_t (OldDateTime.unixtime ()) - int32_t (newDateTime.unixtime ())))
        {
            // DEBUG_V ("Setting the system time");
            struct timeval NewNow =
            {.tv_sec    = time_t (newDateTime.unixtime ()),.tv_usec = time_t (0)};
            settimeofday (& NewNow, NULL);
            // DEBUG_V (   String ("     New system Time: ") + String (UtcDateTime ().unixtime ()));
            // DEBUG_V (   String ("             tv_sec: ") + String (NewNow.tv_sec));
            NumLocalTimeUpdates++;

            LOG_INFO (F ("Updated the system time from NTP/RTC."));
        }

        if (foundRtc)
        {
            DateTime RtcTime = GetRtcTime ();
            // DEBUG_V (   String ("     Old RTC Time: ") + String (RtcTime.unixtime ()));
            if (1 < abs (int32_t (RtcTime.unixtime ()) - int32_t (newDateTime.unixtime ())))
            {
                // DEBUG_V (   "Setting the RTC time");
                // DEBUG_V (   String ("Old Rtc DateTime Unix: ") + String (GetRtcTime ().unixtime ()));
                // DEBUG_V (   String ("new RTC DateTime Unix: ") + String (newDateTime.unixtime ()));
                rtcDevice.adjust (newDateTime);
                PreviousRtcReading = GetRtcTime ();
                LOG_INFO (F ("Updated the RTC time from NTP"));
            }
        }
    } while (false);

    // do we need to increase the Poll rate a bit
    if (abs ((int32_t)(OldDateTime.unixtime () - newDateTime.unixtime ())) > M_MAX_DRIFT_SEC)
    {
        // are we allowed to Poll faster?
        if (((uint64_t)M_MIN_NTP_Poll_INTERVAL_sec) < m_NtpPollInterval.totalseconds ())
        {
            // our rtc has introduced an error. We need to increase our NTP Poll rate
            LOG_INFO (F ("Increasing the NTP Poll rate."));
            m_NtpPollInterval = m_NtpPollInterval - Poll_INCREMENT_SEC;
            // DEBUG_V (String ("m_NtpPollInterval:") + String (m_NtpPollInterval.totalseconds ()));
        }   // end we can reduce the Poll interval
    }       // end we want to reduce the Poll interval

    // the Poll and rtc are in agreement. Can we reduce the Poll rate (increase Poll interval) a bit?
    else if (((uint64_t)M_MAX_NTP_Poll_INTERVAL_sec) > m_NtpPollInterval.totalseconds ())
    {
        // our RTC is spot on with the NTP server. We can decrease our Poll rate a bit.
        m_NtpPollInterval = m_NtpPollInterval + (Poll_INCREMENT_SEC * 2);
        LOG_INFO (F ("Decreasing the NTP Poll rate."));
        // DEBUG_V (String ("m_NtpPollInterval:") + String (m_NtpPollInterval.totalseconds ()));
    }  // end we can reduce the Poll rate

    // we are Polling as slow as we are going to go.
    else
    {
        // LOG_INFO (F ("Leaving the Poll rate unchanged."));
    }  // end no change in the Poll rate

    // DEBUG_END;
}  // setCurrentDateTime

/*****************************************************************************/
void c_rtcMgr::StartUdp ()
{
    // DEBUG_START;

    timeClient.begin ();
    // timeClient.setTimeOffset (long(m_iTimeCorrcetionFactorHours * 60 * 60));

    // DEBUG_END;
}  // StartUdp

/*****************************************************************************/
DateTime c_rtcMgr::GetRtcTime ()
{
    // DEBUG_START;
    DateTime response;
    int retry = 3;
    do
    {
        if (!foundRtc)
        {
            // DEBUG_V ("No RTC Detected");
            break;
        }

        response = rtcDevice.now ();
        --retry;
        DateTime newReading = rtcDevice.now ();
        if (response != newReading)
        {
            // DEBUG_V(String("Consecutive readings are not stable. Retry: ")+ String(retry));
            response    = newReading;
            retry       = 3;
        }
    } while (retry);

    // DEBUG_V(String("Response: ") + String(response));
    // DEBUG_END;
    return response;
}  // GetRtcTime

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
void fsm_rtc_state_TestRTC::Init (c_rtcMgr * pRtcMgr)
{
    // DEBUG_START;

    pRtcMgr->m_pCurrentFsmState =  & fsm_rtc_state_TestRTC_imp;

    RtcStartTime    = pRtcMgr->GetRtcTime ();
    TestEndTimeMS   = uint32_t (millis () + RtcTestDuration);

    // DEBUG_END;
}  // Init

/*****************************************************************************/
void fsm_rtc_state_TestRTC::Poll (c_rtcMgr * pRtcMgr)
{
    // DEBUG_START;

    do  // once
    {
        DateTime RtcEndTime = pRtcMgr->GetRtcTime ();
        if (RtcStartTime < RtcEndTime)
        {
            // DEBUG_V ("RTC is incrementing");
            fsm_rtc_state_WaitForWiFiToComeUp_imp.Init (pRtcMgr);
            break;
        }

        if (TestEndTimeMS < millis ())
        {
            // DEBUG_V (   String ("Could not validate the RTC device"));
            // DEBUG_V (   String ("RtcStartTime: ") + String (RtcStartTime.unixtime ()));
            // DEBUG_V (   String ("  RtcEndTime: ") + String (pRtcMgr->GetRtcTime ().unixtime ()));
            pRtcMgr->foundRtc = false;
            fsm_rtc_state_WaitForWiFiToComeUp_imp.Init (pRtcMgr);
            break;
        }
    } while (false);

    // DEBUG_END;
}  // Poll

/*****************************************************************************/
void fsm_rtc_state_TestRTC::NetworkStateChanged (c_rtcMgr * pRtcMgr, bool NewState)
{
    // DEBUG_START;

    // DEBUG_V ("ignore");

    // DEBUG_END;
}  // NetworkStateChanged

/*****************************************************************************/
/*****************************************************************************/
void fsm_rtc_state_WaitForWiFiToComeUp::Init (c_rtcMgr * pRtcMgr)
{
    // DEBUG_START;

    pRtcMgr->m_pCurrentFsmState =  & fsm_rtc_state_WaitForWiFiToComeUp_imp;

    // DEBUG_END;
}  // fsm_rtc_state_WaitForWiFiToComeUp::Init

/*****************************************************************************/
void fsm_rtc_state_WaitForWiFiToComeUp::Poll (c_rtcMgr * pRtcMgr)
{
    // DEBUG_START;

    if (true == WiFiMgr.IsWiFiConnected ())
    {
        // DEBUG_V (String (F ("WiFi is up.")));

        pRtcMgr->StartUdp ();

        fsm_rtc_state_RefreshNtpBasedDate_imp.Init (pRtcMgr);
    }  // end connection check

    // DEBUG_END;
}  // Poll

/*****************************************************************************/
void fsm_rtc_state_WaitForWiFiToComeUp::NetworkStateChanged (c_rtcMgr * pRtcMgr, bool NewState)
{
    // DEBUG_START;

    // DEBUG_END;
}  // NetworkStateChanged

/*****************************************************************************/
/*****************************************************************************/
void fsm_rtc_state_WaitBeforeSendingNextRequest::Init (c_rtcMgr * pRtcMgr)
{
    // DEBUG_START;

    pRtcMgr->m_FsmNextNtpSendTime   = RtcMgr.UtcDateTime () + pRtcMgr->m_NtpPollInterval;
    pRtcMgr->m_NtpRetryCount        = RtcMgr.NumNtpRetries;
    pRtcMgr->LastRtcSystemTimeSync  = RtcMgr.UtcDateTime ();

    // DEBUG_V (String ("   m_NtpPollInterval: ") + String (pRtcMgr->m_NtpPollInterval.totalseconds ()));
    // DEBUG_V (String ("m_FsmNextNtpSendTime: ") + String (pRtcMgr->m_FsmNextNtpSendTime.unixtime ()));
    // DEBUG_V (String ("  RtcMgr.UtcDateTime: ") + String (pRtcMgr->UtcDateTime().unixtime ()));

    pRtcMgr->m_pCurrentFsmState =  & fsm_rtc_state_WaitBeforeSendingNextRequest_imp;

    // DEBUG_END;
}  // Init

/*****************************************************************************/
void fsm_rtc_state_WaitBeforeSendingNextRequest::Poll (c_rtcMgr * pRtcMgr)
{
    // DEBUG_START;

    DateTime Now = RtcMgr.UtcDateTime ();
    do  // once
    {
        if (!pRtcMgr->foundRtc)
        {
            // no rtc
            break;
        }

        // has it been 10 minutes since the last RTC sync check?
        if (Now.unixtime () < (pRtcMgr->LastRtcSystemTimeSync + TimeSpan (60 * 10)).unixtime ())
        {
            // DEBUG_V("dont need to check the RTC now");
            break;
        }

        // DEBUG_V("Calc RTC Drift");
        // DEBUG_V(String("                  Now 0x") + String(Now.unixtime(), HEX));
        // DEBUG_V(String("LastRtcSystemTimeSync 0x") + String(pRtcMgr->LastRtcSystemTimeSync.unixtime(), HEX));
        // DEBUG_V(String("        Adjusted Sync 0x") + String((pRtcMgr->LastRtcSystemTimeSync + TimeSpan(60 * 10)).unixtime(), HEX));

        pRtcMgr->LastRtcSystemTimeSync = Now;
        DateTime CurrentRtcReading = pRtcMgr->GetRtcTime ();

        // calculate time drift in seconds
        pRtcMgr->RtcDiff = int32_t (CurrentRtcReading.unixtime ()) - int32_t (Now.unixtime ());

        if (abs (pRtcMgr->RtcDiff) < 60)
        {
            // DEBUG_V (F ("System Time and RTC are IN sync."));
            break;
        }

        /*
          * seeing an issue where on occasion the RTC time
          * seems to go backwards and then fixes itself
          */
        if (CurrentRtcReading < pRtcMgr->PreviousRtcReading)
        {
            // DEBUG_V ("Skip potentialy bad RTC reading");
            pRtcMgr->BadRtcReadingCount++;
            break;
        }

        LOG_INFO (F ("System Time and RTC are not in sync."));

        // DEBUG_V (   String ("         System Time: ") + String (Now.unixtime ()));
        // DEBUG_V (   String ("   CurrentRtcReading: ") + String (CurrentRtcReading.unixtime ()));
        // DEBUG_V (   String ("             rtcdiff: ") + String (pRtcMgr->RtcDiff));
        // DEBUG_V (   String ("  PreviousRtcReading: ") + String (pRtcMgr->PreviousRtcReading.unixtime ()));

        // DEBUG_V (   "Is the rtc stuck?");
        if (CurrentRtcReading == pRtcMgr->PreviousRtcReading)
        {
            LOG_INFO (F ("RTC has stopped."));
            // break;
        }

        // pRtcMgr->PreviousRtcReading = CurrentRtcReading;
        // pRtcMgr->setCurrentDateTime (CurrentRtcReading);

        LOG_INFO (F ("Requesting NTP update."));
        fsm_rtc_state_RefreshNtpBasedDate_imp.Init (pRtcMgr);
    } while (false);

    // have we waited long enough before sending an NTP request?
    if (Now.unixtime () > pRtcMgr->m_FsmNextNtpSendTime.unixtime ())
    {
        // DEBUG_V (String ("            System Time: ") + String (Now.unixtime ()));
        // DEBUG_V (String ("   m_FsmNextNtpSendTime: ") + String (pRtcMgr->m_FsmNextNtpSendTime.unixtime ()));
        fsm_rtc_state_RefreshNtpBasedDate_imp.Init (pRtcMgr);
    }  // we need to refresh the time

    // DEBUG_END;
}  // Poll

/*****************************************************************************/
void fsm_rtc_state_WaitBeforeSendingNextRequest::NetworkStateChanged (c_rtcMgr * pRtcMgr, bool NewState)
{
    // DEBUG_START;

    if (false == NewState)
    {
        // DEBUG_V ("Stopping");
        fsm_rtc_state_WaitForWiFiToComeUp_imp.Init (pRtcMgr);
    }

    // DEBUG_END;
}  // NetworkStateChanged

/*****************************************************************************/
/*****************************************************************************/
void fsm_rtc_state_RefreshNtpBasedDate::Init (c_rtcMgr * pRtcMgr)
{
    // DEBUG_START;

    do  // once
    {
        // DEBUG_V (F ("Sending an NTP Request."));

        DateTime    currentTime = RtcMgr.UtcDateTime ();
        TimeSpan    increment (RtcMgr.M_NTP_RESPONSE_TIMEOUT);
        pRtcMgr->m_FsmNtpResponseDeadline = currentTime + increment;
        // DEBUG_V (String ("         New currentTime: ") + String (currentTime.unixtime ()));
        // DEBUG_V (String ("m_FsmNtpResponseDeadline: ") + String (pRtcMgr->m_FsmNtpResponseDeadline.unixtime ()));
        pRtcMgr->m_pCurrentFsmState =  & fsm_rtc_state_RefreshNtpBasedDate_imp;

        timeClient.sendNTPPacket ();
    } while (false);

    // DEBUG_END;
}  // Init

/*****************************************************************************/
void fsm_rtc_state_RefreshNtpBasedDate::Poll (c_rtcMgr * pRtcMgr)
{
    // DEBUG_START;

    do  // once
    {
        if (true == timeClient.CheckForResponse ())
        {
            // DEBUG_V ("Refresh worked");
            // DEBUG_V (timeClient.getFormattedTime ());

            pRtcMgr->setCurrentDateTime (DateTime (timeClient.getRawEpochTime ()));

            // DEBUG_V (String ("CurrentDT.year: ")    + String (pRtcMgr->currentDateTime ().year()));
            // DEBUG_V (String ("CurrentDT.month: ")   + String (pRtcMgr->currentDateTime ().month()));
            // DEBUG_V (String ("CurrentDT.day: ")     + String (pRtcMgr->currentDateTime ().day ()));
            // DEBUG_V (String ("CurrentDT.hour: ")    + String (pRtcMgr->currentDateTime ().hour ()));
            // DEBUG_V (String ("CurrentDT.minutes: ") + String (pRtcMgr->currentDateTime ().minute ()));
            // DEBUG_V (String ("CurrentDT.Seconds: ") + String (pRtcMgr->currentDateTime ().second ()));

            fsm_rtc_state_WaitBeforeSendingNextRequest_imp.Init (pRtcMgr);
            break;
        }

        // have we waited long enough.
        if (RtcMgr.UtcDateTime () < pRtcMgr->m_FsmNtpResponseDeadline)
        {
            // keep waiting
            break;
        }  // we have timed out

        // DEBUG_V ("Refresh failed");
        // DEBUG_V (String ("         m_NtpRetryCount: ") + String (pRtcMgr->m_NtpRetryCount));
        // DEBUG_V (String ("m_FsmNtpResponseDeadline: ") + String (pRtcMgr->m_FsmNtpResponseDeadline.unixtime()));
        // DEBUG_V (String ("             UtcDateTime: ") + String (pRtcMgr->UtcDateTime().unixtime()));

        if (pRtcMgr->m_NtpRetryCount)
        {
            pRtcMgr->m_NtpRetryCount--;
            fsm_rtc_state_RefreshNtpBasedDate_imp.Init (pRtcMgr);
        }
        else
        {
            fsm_rtc_state_WaitBeforeSendingNextRequest_imp.Init (pRtcMgr);
        }
    } while (false);

    // DEBUG_END;
}  // Poll

/*****************************************************************************/
void fsm_rtc_state_RefreshNtpBasedDate::NetworkStateChanged (c_rtcMgr * pRtcMgr, bool NewState)
{
    // DEBUG_START;

    if (false == NewState)
    {
        // DEBUG_V ("Stopping");
        fsm_rtc_state_WaitForWiFiToComeUp_imp.Init (pRtcMgr);
    }

    // DEBUG_END;
}  // NetworkStateChanged
