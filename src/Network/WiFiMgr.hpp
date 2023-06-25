#pragma once
/*
  * WiFiMgr.hpp - Output Management class
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

#include "JurasicParkGate.hpp"

#include <WiFi.h>
#include <EventManager.h>
// #include "RtcMgr.hpp"

class fsm_WiFi_state;

class c_WiFiMgr : public c_Common
{
public:
    c_WiFiMgr ();
    virtual~c_WiFiMgr ();

    // Stop the compiler generating methods to copy the object
    c_WiFiMgr (c_WiFiMgr const & copy);
    c_WiFiMgr &operator=(c_WiFiMgr const & copy);

    void        Init            (); ///< set up the operating environment based on the current config (or defaults)
    int         ValidateConfig  ();
    IPAddress   getIpAddress    ()                      {return CurrentIpAddress;}
    void        setIpAddress    (IPAddress NewAddress)  {CurrentIpAddress = NewAddress;}
    IPAddress   getIpSubNetMask ()                      {return CurrentSubnetMask;}
    void        setIpSubNetMask (IPAddress NewAddress)  {CurrentSubnetMask = NewAddress;}
    void        GetStatus       (JsonObject & jsonStatus);
    void        GetConfig       (JsonObject & jsonConfig);
    void        SetConfig       (JsonObject & jsonConfig);

    void    connectWifi     (const String & ssid, const String & passphrase);
    void    reset           ();
    void    Poll ();

    void        SetFsmState     (fsm_WiFi_state * NewState) {pCurrentFsmState = NewState;}
    void        AnnounceState   ();
    void        SetFsmStartTime (uint32_t NewStartTime)     {FsmTimerWiFiStartTime = NewStartTime;}
    uint32_t    GetFsmStartTime (void)                      {return FsmTimerWiFiStartTime;}
    void        GetConfigPtr    (JsonObject & jsonStatus)   {}
    bool        IsWiFiConnected ()                          {return ReportedIsWiFiConnected;}
    void        SetIsWiFiConnected (bool value)             {ReportedIsWiFiConnected = value;}
    String      GetHostName     ()                          {return hostname;}
    uint32_t    GetStaTimeout   ()                          {return sta_timeout;}
    uint32_t    GetApTimeout    ()                          {return ap_timeout;}
    bool        GetApFallbackIsEnabled ()                   {return ap_fallbackIsEnabled;}
    bool        GetRebootOnWiFiFailureToConnect ()          {return RebootOnWiFiFailureToConnect;}

private:

    #define PollInterval    1000
    #define CLIENT_TIMEOUT  15      ///< In station/client mode try to connection for 15 seconds
    #define AP_TIMEOUT      120     ///< In AP mode, wait 60 seconds for a connection or reboot

    IPAddress CurrentIpAddress = IPAddress (0, 0, 0, 0);
    IPAddress       CurrentSubnetMask       = IPAddress (0, 0, 0, 0);
    uint32_t        NextPollTime            = 0;
    bool            ReportedIsWiFiConnected = false;
    DateTime        RssiLastUpdateTime;

    String          ssid;
    String          passphrase;
    String          hostname;
    IPAddress       ip                              = (uint32_t)0;
    IPAddress       netmask                         = (uint32_t)0;
    IPAddress       gateway                         = (uint32_t)0;
    bool            UseDhcp                         = true;             ///< Use DHCP?
    bool            ap_fallbackIsEnabled            = true;             ///< Fallback to AP if fail to associate?
    uint32_t        ap_timeout                      = AP_TIMEOUT;       ///< How long to wait in AP mode with no connection before rebooting
    uint32_t        sta_timeout                     = CLIENT_TIMEOUT;   ///< Timeout when connection as client (station)
    bool            RebootOnWiFiFailureToConnect    = true;

    void SetUpIp ();

    EventManager    EventManagerEvents;

    #define WiFiConnectEvent        EventManager::kEventUser0
    #define WiFiDisConnectEvent     EventManager::kEventUser1
    #define WiFiStaConnectEvent     EventManager::kEventUser2
    #define WiFiStaDisConnectEvent  EventManager::kEventUser3

private: void   onWiFiConnect         (const WiFiEvent_t event, const WiFiEventInfo_t info);
public:  void   onWiFiConnectEvent    (int event, int param);
private: void   onWiFiDisconnect      (const WiFiEvent_t event, const WiFiEventInfo_t info);
public:  void   onWiFiDisconnectEvent (int event, int param);

private: void   onWiFiStaConn         (const WiFiEvent_t event, const WiFiEventInfo_t info);
public:  void   onWiFiStaConnEvent    (int event, int param);
private: void   onWiFiStaDisc         (const WiFiEvent_t event, const WiFiEventInfo_t info);
public:  void   onWiFiStaDiscEvent    (int event, int param);

protected:
    friend class fsm_WiFi_state;
    friend class fsm_WiFi_state_Boot;
    friend class fsm_WiFi_state_ConnectingUsingConfig;
    friend class fsm_WiFi_state_ConnectingUsingDefaults;
    friend class fsm_WiFi_state_ConnectedToAP;
    friend class fsm_WiFi_state_ConnectingAsAP;
    friend class fsm_WiFi_state_ConnectedToSta;
    friend class fsm_WiFi_state_ConnectionFailed;
    friend class fsm_WiFi_state;
    fsm_WiFi_state  * pCurrentFsmState      = nullptr;
    uint32_t        FsmTimerWiFiStartTime   = 0;
};  // c_WiFiMgr
extern c_WiFiMgr WiFiMgr;


/*****************************************************************************/
/*
  *	Generic fsm base class.
  */
/*****************************************************************************/
// forward declaration
/*****************************************************************************/
class fsm_WiFi_state
{
public:
    virtual void    Poll (void)                     = 0;
    virtual void    Init (void)                     = 0;
    virtual void    GetStateName (String & sName)   = 0;
    virtual void    OnConnect (void)                = 0;
    virtual void    OnDisconnect (void)             = 0;
    String          GetName () {return WiFiMgr.GetName ();}
};  // fsm_WiFi_state

/*****************************************************************************/
// Wait for polling to start.
class fsm_WiFi_state_Boot : public fsm_WiFi_state
{
public:
    virtual void    Poll (void);
    virtual void    Init (void);
    virtual void    GetStateName (String & sName)   {sName = F ("Boot");}
    virtual void    OnConnect (void)                { /* ignore */}
    virtual void    OnDisconnect (void)             { /* ignore */}
};  // fsm_WiFi_state_Boot

/*****************************************************************************/
class fsm_WiFi_state_ConnectingUsingConfig : public fsm_WiFi_state
{
public:
    virtual void    Poll (void);
    virtual void    Init (void);
    virtual void    GetStateName (String & sName) {sName = F ("Connecting Using Config Credentials");}
    virtual void    OnConnect (void);
    virtual void    OnDisconnect (void) {}
};  // fsm_WiFi_state_ConnectingUsingConfig

/*****************************************************************************/
class fsm_WiFi_state_ConnectingUsingDefaults : public fsm_WiFi_state
{
public:
    virtual void    Poll (void);
    virtual void    Init (void);
    virtual void    GetStateName (String & sName) {sName = F ("Connecting Using Default Credentials");}
    virtual void    OnConnect (void);
    virtual void    OnDisconnect (void) {}
};  // fsm_WiFi_state_ConnectingUsingConfig

/*****************************************************************************/
class fsm_WiFi_state_ConnectedToAP : public fsm_WiFi_state
{
public:
    virtual void    Poll (void);
    virtual void    Init (void);
    virtual void    GetStateName (String & sName)   {sName = F ("Connected To AP");}
    virtual void    OnConnect (void)                {}
    virtual void    OnDisconnect (void);
};  // fsm_WiFi_state_ConnectedToAP

/*****************************************************************************/
class fsm_WiFi_state_ConnectingAsAP : public fsm_WiFi_state
{
public:
    virtual void    Poll (void);
    virtual void    Init (void);
    virtual void    GetStateName (String & sName) {sName = F ("Connecting As AP");}
    virtual void    OnConnect (void);
    virtual void    OnDisconnect (void) {}
};  // fsm_WiFi_state_ConnectingAsAP

/*****************************************************************************/
class fsm_WiFi_state_ConnectedToSta : public fsm_WiFi_state
{
public:
    virtual void    Poll (void);
    virtual void    Init (void);
    virtual void    GetStateName (String & sName)   {sName = F ("Connected To STA");}
    virtual void    OnConnect (void)                {}
    virtual void    OnDisconnect (void);
};  // fsm_WiFi_state_ConnectedToSta

/*****************************************************************************/
class fsm_WiFi_state_ConnectionFailed : public fsm_WiFi_state
{
public:
    virtual void    Poll (void) {}
    virtual void    Init (void);
    virtual void    GetStateName (String & sName)   {sName = F ("Connection Failed");}
    virtual void    OnConnect (void)                {}
    virtual void    OnDisconnect (void)             {}
};  // fsm_WiFi_state_Rebooting
