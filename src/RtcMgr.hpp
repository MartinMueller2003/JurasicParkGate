/*****************************************************************************/
/*
  * This is the manager for the rtcMgr
  */
/*****************************************************************************/

// only allow the file to be included once.
#pragma once

#include "JurasicParkGate.hpp"
#include <map>
#include <RTClib.h>
// #include <TCA9548A.h>

/*****************************************************************************/
class fsm_rtc_state;
class c_rtcMgr : public c_Common
{
public:
    c_rtcMgr    (void);
    ~c_rtcMgr   (void);
    c_rtcMgr    (c_rtcMgr const & copy);
    c_rtcMgr    &operator =(c_rtcMgr const & copy);
    void        Init (void);
    void        Poll (void);
    void        GetConfig (JsonObject & jsonResult);
    void        GetStatus (JsonObject & jsonResult);
    void        SetConfig (JsonObject & jsonData);
    void        NetworkStateChanged (bool NewState);
    DateTime    UtcDateTime (void);
    void        currentDateTime (String & sOutput, bool IncludeSeconds = true);
    void        currentYear (String & sOutput);
    void        currentMonth (String & sOutput);
    void        currentDay (String & sOutput);
    int         sendNTPpacket (IPAddress & address);
    void        setCurrentDateTime (DateTime newData);
    DateTime    now (tm & _tm);
    DateTime    now ();
    DateTime    GetRtcTime ();
    double      GetRtcTemperature ();
    bool        IsRtcinstalled () {return foundRtc;}
    TwoWire     &GetTwoWire ();

protected:
    void StartUdp ();

private:
    void        SoftResetI2C ();
    uint32_t    ScanI2Cbus (int start, int end, bool DisplayDevices = false);
    void        HardResetI2cMux ();
    void        SetUpI2cChannels ();

    // strings used for config management
    const String    M_RTCMGR_JSON_ROOT                  = F ("rtcmgr");
    const String    M_TIME_CORRECTION_FACTOR_JSON_NAME  = F ("tcfh");
    const String    M_TIME_SERVER_JSON_NAME             = F ("tsrv");

    // how long (seconds) to wait after an NTP request is sent before sending another one
    const uint32_t  M_NTP_RESPONSE_TIMEOUT = 60;

    // shortest wait between NTP requests (60 minutes)
    const uint32_t  M_MIN_NTP_Poll_INTERVAL_sec = (60 * 60);

    // longest wait between NTP requests (2 days)
    const uint32_t  M_MAX_NTP_Poll_INTERVAL_sec = (60 * 60 * 24 * 2);

    // max time difference we allow between NTP and RTC before we change the NTP Poll rate
    const uint32_t  M_MAX_DRIFT_SEC = 3;

    const uint32_t  Poll_INCREMENT_SEC  = (M_MIN_NTP_Poll_INTERVAL_sec / 2);
    const uint32_t  NumNtpRetries       = 5;

    DateTime        m_lastDateTime                  = DateTime (uint32_t (0));
    uint16_t        m_localPort                     = 2390;                 // local port to listen for UDP packets
    int8_t          m_iTimeCorrcetionFactorHours    = -4;                   // number of hours to add / remove from utc time
    String          m_timeServer                    = F ("time.nist.gov");  // time.nist.gov NTP server
    DateTime        m_FsmNextNtpSendTime            = DateTime (uint32_t (0));
    DateTime        m_FsmNtpResponseDeadline        = DateTime (uint32_t (0));
    TimeSpan        m_NtpPollInterval               = TimeSpan (M_MIN_NTP_Poll_INTERVAL_sec * 2);
    bool            foundRtc                        = false;                 // true if we have seen the RTC on the bus
    fsm_rtc_state   * m_pCurrentFsmState            = nullptr;
    uint8_t         m_NtpRetryCount                 = NumNtpRetries;
    uint8_t         LastDisplayTime                 = 99;

    const uint16_t  MUX_I2C_ADDRESS = 112;
    // TCA9548A        i2cmux;

    const uint32_t  RTC_I2C_ADDRESS = 0x68;
    RTC_DS3231      rtcDevice;

    int32_t         RtcDiff                 = 0;
    DateTime        PreviousRtcReading      = DateTime (uint32_t (0));
    DateTime        LastRtcSystemTimeSync   = DateTime (uint32_t (0));
    uint32_t        BadRtcReadingCount      = 0;
    uint32_t        NumLocalTimeUpdates     = 0;
    uint32_t        NumRtcTimeUpdates       = 0;

    struct BussInfo_t
    {
        uint32_t    BussId;
        uint32_t    NumDevices;
    };

    BussInfo_t BussInfo[8];

    friend class fsm_rtc_state;
    friend class fsm_rtc_state_TestRTC;
    friend class fsm_rtc_state_WaitForWiFiToComeUp;
    friend class fsm_rtc_state_WaitBeforeSendingNextRequest;
    friend class fsm_rtc_state_RefreshNtpBasedDate;
};  // class c_rtcMgr
/*****************************************************************************/

// only one instance of the class is allowed and it is globally available
extern class c_rtcMgr RtcMgr;

/*****************************************************************************/
/*
  *  Generic fsm base class.
  */
/*****************************************************************************/
class fsm_rtc_state
{
public:
    virtual void    Poll (c_rtcMgr * rtcMgr)                                = 0;
    virtual void    Init (c_rtcMgr * rtcMgr)                                = 0;
    virtual void    NetworkStateChanged (c_rtcMgr * pRtcMgr, bool NewState) = 0;
};  // fsm_rtc_state

/*****************************************************************************/
// Wait for WiFi to be up
class fsm_rtc_state_TestRTC : public fsm_rtc_state
{
public:
    virtual void    Poll (c_rtcMgr * rtcMgr);
    virtual void    Init (c_rtcMgr * rtcMgr);
    virtual void    NetworkStateChanged (c_rtcMgr * pRtcMgr, bool NewState);

private:
    DateTime        RtcStartTime    = DateTime (uint32_t (0));
    uint32_t        TestEndTimeMS   = 0;
    const uint32_t  RtcTestDuration = 2000;
};  // fsm_rtc_state_Wait_WiFi_Up


/*****************************************************************************/
// Wait for WiFi to be up
class fsm_rtc_state_WaitForWiFiToComeUp : public fsm_rtc_state
{
public:
    virtual void    Poll (c_rtcMgr * rtcMgr);
    virtual void    Init (c_rtcMgr * rtcMgr);
    virtual void    NetworkStateChanged (c_rtcMgr * pRtcMgr, bool NewState);
};  // fsm_rtc_state_Wait_WiFi_Up

/*****************************************************************************/
// Wait to send an NTP request. Sending once a day
class fsm_rtc_state_WaitBeforeSendingNextRequest : public fsm_rtc_state
{
public:
    virtual void    Poll (c_rtcMgr * rtcMgr);
    virtual void    Init (c_rtcMgr * rtcMgr);
    virtual void    NetworkStateChanged (c_rtcMgr * pRtcMgr, bool NewState);
    String          GetName () {return RtcMgr.GetName ();}
};  // fsm_rtc_state_Wait_Send_Request

/*****************************************************************************/
// Waiting for a response to our NTP request. Restart the daily wait if we get a response
// Send a new request if there is no response
class fsm_rtc_state_RefreshNtpBasedDate : public fsm_rtc_state
{
public:
    virtual void    Poll (c_rtcMgr * rtcMgr);
    virtual void    Init (c_rtcMgr * rtcMgr);
    virtual void    NetworkStateChanged (c_rtcMgr * pRtcMgr, bool NewState);
};  // fsm_rtc_state_Wait_Response
