/*
  * WiFiMgr.cpp - Output Management class
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

#include <esp_wifi.h>

#include "WiFiMgr.hpp"

// -----------------------------------------------------------------------------
// Create secrets.h with a #define for SECRETS_SSID and SECRETS_PASS
// or delete the #include and enter the strings directly below.
#include "secrets.h"

#ifndef SECRETS_SSID
    #define SECRETS_SSID    "DEFAULT_SSID_NOT_SET"
    #define SECRETS_PASS    "DEFAULT_PASSPHRASE_NOT_SET"
#endif // ndef SECRETS_SSID

/* Fallback configuration if json is empty or fails */
const String    default_ssid        = SECRETS_SSID;
const String    default_passphrase  = SECRETS_PASS;

/*****************************************************************************/
/* FSM                                                                       */
/*****************************************************************************/
fsm_WiFi_state_Boot fsm_WiFi_state_Boot_imp;
fsm_WiFi_state_ConnectingUsingConfig fsm_WiFi_state_ConnectingUsingConfig_imp;
fsm_WiFi_state_ConnectingUsingDefaults fsm_WiFi_state_ConnectingDefault_imp;
fsm_WiFi_state_ConnectedToAP fsm_WiFi_state_ConnectedToAP_imp;
fsm_WiFi_state_ConnectingAsAP   fsm_WiFi_state_ConnectingAsAP_imp;
fsm_WiFi_state_ConnectedToSta   fsm_WiFi_state_ConnectedToSta_imp;
fsm_WiFi_state_ConnectionFailed fsm_WiFi_state_ConnectionFailed_imp;

// -----------------------------------------------------------------------------
///< Start up the driver and put it into a safe mode
c_WiFiMgr::c_WiFiMgr () : c_Common (String (F ("WiFiMgr")))
{
    // this gets called pre-setup so there is nothing we can do here.
    fsm_WiFi_state_Boot_imp.Init ();

    hostname = String ("JurasicParkGate-") + WiFi.macAddress ();

    EventManagerEvents. addListener (   WiFiConnectEvent,       [] (int one, int two)->void{WiFiMgr.onWiFiConnectEvent    (one, two);});
    EventManagerEvents. addListener (   WiFiDisConnectEvent,    [] (int one, int two)->void{WiFiMgr.onWiFiDisconnectEvent (one, two);});
    EventManagerEvents. addListener (   WiFiStaConnectEvent,    [] (int one, int two)->void{WiFiMgr.onWiFiStaConnEvent    (one, two);});
    EventManagerEvents. addListener (   WiFiStaDisConnectEvent, [] (int one, int two)->void{WiFiMgr.onWiFiStaDiscEvent    (one, two);});
}  // c_WiFiMgr

// -----------------------------------------------------------------------------
///< deallocate any resources and put the output channels into a safe state
c_WiFiMgr::~c_WiFiMgr ()
{
    // DEBUG_START;

    // DEBUG_END;
}  // ~c_WiFiMgr

// -----------------------------------------------------------------------------
///< Start the module
void c_WiFiMgr::Init ()
{
    // DEBUG_START;

    // Disable persistant credential storage and configure SDK params
    WiFi.persistent (false);

    esp_wifi_set_ps (WIFI_PS_NONE);

    // DEBUG_V ("");

    // Setup WiFi Handlers
    WiFi.   onEvent (   [this] (WiFiEvent_t event, arduino_event_info_t info) {this->onWiFiStaConn (event, info);}, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
    WiFi.   onEvent (   [this] (WiFiEvent_t event, arduino_event_info_t info) {this->onWiFiStaDisc (event, info);}, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

    WiFi.   onEvent (   [this] (WiFiEvent_t event, arduino_event_info_t info) {this->onWiFiConnect    (event, info);}, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.   onEvent (   [this] (WiFiEvent_t event, arduino_event_info_t info) {this->onWiFiDisconnect (event, info);}, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

    // set up the poll interval
    NextPollTime = millis () + PollInterval;

    #ifdef USE_DISPLAY
        DisplayMgr.println (cDisplayMgr::eZone::IpZone, "-.-.-.-");
    #endif // def USE_DISPLAY

    // DEBUG_END;
}  // begin

// -----------------------------------------------------------------------------
void c_WiFiMgr::GetConfig (JsonObject & jsonConfig)
{
    // DEBUG_START;
    JsonObject JsonNetworkConfig = jsonConfig.createNestedObject (CN_network);

    JsonNetworkConfig[CN_ssid]          = ssid;
    JsonNetworkConfig[CN_passphrase]    = passphrase;
    JsonNetworkConfig[CN_hostname]      = hostname;
    JsonNetworkConfig[CN_ip]            = ip.toString ();
    JsonNetworkConfig[CN_netmask]       = netmask.toString ();
    JsonNetworkConfig[CN_gateway]       = gateway.toString ();
    JsonNetworkConfig[CN_dhcp]          = UseDhcp;
    JsonNetworkConfig[CN_ap_fallback]   = ap_fallbackIsEnabled;
    JsonNetworkConfig[CN_ap_timeout]    = ap_timeout;
    JsonNetworkConfig[CN_sta_timeout]   = sta_timeout;
    JsonNetworkConfig[CN_ap_reboot]     = RebootOnWiFiFailureToConnect;

    // extern void PrettyPrint (JsonObject & jsonStuff, String Name);
    // PrettyPrint (jsonConfig, CN_network);

    // DEBUG_END;
}  // GetConfig

// -----------------------------------------------------------------------------
void c_WiFiMgr::GetStatus (JsonObject & response)
{
    // DEBUG_START;

    JsonObject jsonStatus = response.createNestedObject (CN_network);

    jsonStatus[CN_rssi]     = WiFi.RSSI ();
    jsonStatus[CN_ip]       = getIpAddress ().toString ();
    jsonStatus[CN_subnet]   = getIpSubNetMask ().toString ();
    jsonStatus[CN_mac]      = WiFi.macAddress ();
    jsonStatus[CN_hostname] = WiFi.getHostname ();
    jsonStatus[CN_ssid]     = WiFi.SSID ();

    // DEBUG_END;
}  // GetStatus

// -----------------------------------------------------------------------------
void c_WiFiMgr::SetConfig (JsonObject & jsonData)
{
    // DEBUG_START;

    do  // once
    {
        if (false == jsonData.containsKey (CN_network))
        {
            LOG_INFO (F ("No network config data found"));
            CfgMgr.ScheduleConfigCommit ();
            break;
        }

        JsonObject jsonConfig = jsonData[CN_network];

        // extern void PrettyPrint (JsonObject & jsonStuff, String Name);
        // PrettyPrint (jsonConfig, CN_network);

        String TempAddress;

        READ_JSON ( jsonConfig, ssid,       CN_ssid,        c_CfgMgr::ScheduleUpdate_t::ConfigUpdate);
        READ_JSON ( jsonConfig, passphrase, CN_passphrase,  c_CfgMgr::ScheduleUpdate_t::ConfigUpdate);
        // DEBUG_V (String ("      ssid: ") + ssid);
        // DEBUG_V (String ("passphrase: ") + passphrase);
        READ_JSON ( jsonConfig, hostname,   CN_hostname,    c_CfgMgr::ScheduleUpdate_t::ConfigUpdate);
        // DEBUG_V (String ("  hostname: ") + hostname);

        TempAddress = ip.toString ();
        READ_JSON (jsonConfig, TempAddress, CN_ip, c_CfgMgr::ScheduleUpdate_t::ConfigUpdate);
        ip.fromString (TempAddress);

        TempAddress = netmask.toString ();
        READ_JSON (jsonConfig, TempAddress, CN_netmask, c_CfgMgr::ScheduleUpdate_t::ConfigUpdate);
        netmask.fromString (TempAddress);

        TempAddress = gateway.toString ();
        READ_JSON (jsonConfig, TempAddress, CN_gateway, c_CfgMgr::ScheduleUpdate_t::ConfigUpdate);
        gateway.fromString (TempAddress);

        READ_JSON ( jsonConfig, UseDhcp,                        CN_dhcp,        c_CfgMgr::ScheduleUpdate_t::ConfigUpdate);
        READ_JSON ( jsonConfig, ap_fallbackIsEnabled,           CN_ap_fallback, c_CfgMgr::ScheduleUpdate_t::ConfigUpdate);
        READ_JSON ( jsonConfig, ap_timeout,                     CN_ap_timeout,  c_CfgMgr::ScheduleUpdate_t::ConfigUpdate);
        READ_JSON ( jsonConfig, sta_timeout,                    CN_sta_timeout, c_CfgMgr::ScheduleUpdate_t::ConfigUpdate);
        READ_JSON ( jsonConfig, RebootOnWiFiFailureToConnect,   CN_ap_reboot,   c_CfgMgr::ScheduleUpdate_t::ConfigUpdate);
    } while (false);

    // DEBUG_END;
}  // SetConfig

// -----------------------------------------------------------------------------
void c_WiFiMgr::connectWifi (const String & ssid, const String & passphrase)
{
    // DEBUG_START;

    // disconnect just in case
    WiFi.persistent (false);
    // DEBUG_V ("");
    WiFi.disconnect (true);

    // DEBUG_V (String ("config->hostname: ") + hostname);
    if (0 != hostname.length ())
    {
        // DEBUG_V (String ("Setting WiFi hostname: ") + config->hostname);
        WiFi.hostname (hostname);
    }

    // Switch to station mode
    WiFi.mode (WIFI_STA);
    // WiFi.enableLongRange(true);
    WiFi.setTxPower (WIFI_POWER_19_5dBm);    // Set WiFi RF power output to highest level
    WiFi.setSleep (false);

    // DEBUG_V ("");

    LOG_INFO (
        String (F ("Connecting to '")) +
        ssid +
        String (F ("' as '")) +
        hostname + "'");

    WiFi.begin (ssid.c_str (), passphrase.c_str ());

    // DEBUG_END;
}  // connectWifi

// -----------------------------------------------------------------------------
void c_WiFiMgr::reset ()
{
    // DEBUG_START;

    LOG_INFO (F ("Reset has been requested"));

    fsm_WiFi_state_Boot_imp.Init ();
    if (IsWiFiConnected ())
    {
        // InputMgr.NetworkStateChanged (false);
    }

    // DEBUG_END;
}  // reset

// -----------------------------------------------------------------------------
void c_WiFiMgr::SetUpIp ()
{
    // DEBUG_START;

    do  // once
    {
        if (true == UseDhcp)
        {
            LOG_INFO (F ("Connected with DHCP"));
            break;
        }

        IPAddress temp = (uint32_t)0;
        // DEBUG_V ("   temp: " + temp.toString ());
        // DEBUG_V ("     ip: " + ip.toString ());
        // DEBUG_V ("netmask: " + netmask.toString ());
        // DEBUG_V ("gateway: " + gateway.toString ());

        if (temp == ip)
        {
            LOG_ERROR (F ("STATIC SELECTED WITHOUT IP. Using DHCP assigned address"));
            break;
        }

        if ((ip == WiFi.localIP ()) &&
            (netmask == WiFi.subnetMask ()) &&
            (gateway == WiFi.gatewayIP ()))
        {
            // correct IP is already set
            break;
        }

        // We didn't use DNS, so just set it to our configured gateway
        WiFi.config (ip, gateway, netmask, gateway);

        LOG_INFO (F ("Connected with Static IP"));
    } while (false);

    // DEBUG_END;
}  // SetUpIp

// -----------------------------------------------------------------------------
void c_WiFiMgr::onWiFiStaConn (const WiFiEvent_t, const WiFiEventInfo_t)
{
    // DEBUG_V ("ESP has associated with the STA");
    EventManagerEvents.queueEvent (WiFiStaConnectEvent, 0);
}  // onWiFiStaConn

// -----------------------------------------------------------------------------
void c_WiFiMgr::onWiFiStaConnEvent (int, int)
{
    // DEBUG_V ("ESP has associated with the STA");
}  // onWiFiStaConn

// -----------------------------------------------------------------------------
void c_WiFiMgr::onWiFiStaDisc (const WiFiEvent_t, const WiFiEventInfo_t)
{
    // DEBUG_V ("ESP has disconnected from the STA");
    EventManagerEvents.queueEvent (WiFiStaDisConnectEvent, 0);
}  // onWiFiStaDisc

// -----------------------------------------------------------------------------
void c_WiFiMgr::onWiFiStaDiscEvent (int, int)
{
    // DEBUG_V ("ESP has disconnected from the STA");
}  // onWiFiStaDisc

// -----------------------------------------------------------------------------
void c_WiFiMgr::onWiFiConnect (const WiFiEvent_t, const WiFiEventInfo_t)
{
    // DEBUG_START;

    EventManagerEvents.queueEvent (WiFiConnectEvent, 0);

    // DEBUG_END;
}  // onWiFiConnect

// -----------------------------------------------------------------------------
void c_WiFiMgr::onWiFiConnectEvent (int, int)
{
    // DEBUG_START;

    pCurrentFsmState->OnConnect ();

    // DEBUG_END;
}  // onWiFiConnect

// -----------------------------------------------------------------------------
/// WiFi Disconnect Handler
void c_WiFiMgr::onWiFiDisconnect (const WiFiEvent_t, const WiFiEventInfo_t)
{
    // DEBUG_START;

    EventManagerEvents.queueEvent (WiFiConnectEvent, 0);

    // DEBUG_END;
}  // onWiFiDisconnect

// -----------------------------------------------------------------------------
/// WiFi Disconnect Handler
void c_WiFiMgr::onWiFiDisconnectEvent (int, int)
{
    // DEBUG_START;

    pCurrentFsmState->OnDisconnect ();

    // DEBUG_END;
}  // onWiFiDisconnect

// -----------------------------------------------------------------------------
int c_WiFiMgr::ValidateConfig ()
{
    // DEBUG_START;

    int response = 0;

    if (sta_timeout < 5)
    {
        sta_timeout = CLIENT_TIMEOUT;
        // DEBUG_V ();
        response++;
    }

    if (ap_timeout < 15)
    {
        ap_timeout = AP_TIMEOUT;
        // DEBUG_V ();
        response++;
    }

    // DEBUG_END;

    return response;
}  // ValidateConfig

// -----------------------------------------------------------------------------
void c_WiFiMgr::AnnounceState ()
{
    // DEBUG_START;

    String StateName;
    pCurrentFsmState->GetStateName (StateName);
    LOG_INFO (String (F ("Entering State: ")) + StateName);

    // DEBUG_END;
}  // AnnounceState

// -----------------------------------------------------------------------------
void c_WiFiMgr::Poll ()
{
    // DEBUG_START;

    EventManagerEvents.processAllEvents ();

    if (millis () > NextPollTime)
    {
        // DEBUG_V ("");
        NextPollTime += PollInterval;
        pCurrentFsmState->Poll ();
    }

    #ifdef USE_DISPLAY
        DateTime CurrentTime = RtcMgr.now ();
        if (CurrentTime.unixtime () != RssiLastUpdateTime.unixtime ())
        {
            // DEBUG_V(String("       CurrentTime: ") + String(CurrentTime.secondstime()));
            // DEBUG_V(String("RssiLastUpdateTime: ") + String(CurrentSensorUpdateTime.secondstime()));
            RssiLastUpdateTime = CurrentTime;

            DisplayMgr.println (cDisplayMgr::eZone::RssiZone, String (F ("RSSI: ")) + WiFi.RSSI () + F ("dBm"));
        }

    #endif // def USE_DISPLAY

    // DEBUG_END;
}  // Poll

/*****************************************************************************/
//  FSM Code
/*****************************************************************************/
/*****************************************************************************/
// Waiting for polling to start
void fsm_WiFi_state_Boot::Poll ()
{
    // DEBUG_START;

    // Start trying to connect to the AP
    fsm_WiFi_state_ConnectingUsingConfig_imp.Init ();

    // DEBUG_END;
}  // fsm_WiFi_state_boot

/*****************************************************************************/
// Waiting for polling to start
void fsm_WiFi_state_Boot::Init ()
{
    // DEBUG_START;

    WiFiMgr.SetFsmState (this);

    // This can get called before the system is up and running.
    // No log port available yet
    // WiFiMgr.AnnounceState ();

    // DEBUG_END;
}  // fsm_WiFi_state_Boot::Init

/*****************************************************************************/
/*****************************************************************************/
// Wait for events
void fsm_WiFi_state_ConnectingUsingConfig::Poll ()
{
    // DEBUG_START;

    // wait for the connection to complete via the callback function
    uint32_t CurrentTimeMS = millis ();

    if (WiFi.status () != WL_CONNECTED)
    {
        if (CurrentTimeMS - WiFiMgr.GetFsmStartTime () > (1000 * WiFiMgr.GetStaTimeout ()))
        {
            LOG_INFO (F ("WiFi Failed to connect using Configured Credentials"));
            fsm_WiFi_state_ConnectingDefault_imp.Init ();
        }
    }

    // DEBUG_END;
}  // fsm_WiFi_state_ConnectingUsingConfig::Poll

/*****************************************************************************/
// Wait for events
void fsm_WiFi_state_ConnectingUsingConfig::Init ()
{
    // DEBUG_START;

    if (0 == WiFiMgr.ssid.length ())
    {
        fsm_WiFi_state_ConnectingDefault_imp.Init ();
    }
    else
    {
        WiFiMgr.SetFsmState (this);
        WiFiMgr.AnnounceState ();
        WiFiMgr.SetFsmStartTime (millis ());

        WiFiMgr.connectWifi (WiFiMgr.ssid, WiFiMgr.passphrase);
    }

    // DEBUG_END;
}  // fsm_WiFi_state_ConnectingUsingConfig::Init

/*****************************************************************************/
// Wait for events
void fsm_WiFi_state_ConnectingUsingConfig::OnConnect ()
{
    // DEBUG_START;

    fsm_WiFi_state_ConnectedToAP_imp.Init ();

    // DEBUG_END;
}  // fsm_WiFi_state_ConnectingUsingConfig::OnConnect

/*****************************************************************************/
/*****************************************************************************/
// Wait for events
void fsm_WiFi_state_ConnectingUsingDefaults::Poll ()
{
    // DEBUG_START;

    // wait for the connection to complete via the callback function
    uint32_t CurrentTimeMS = millis ();

    if (WiFi.status () != WL_CONNECTED)
    {
        if (CurrentTimeMS - WiFiMgr.GetFsmStartTime () > (1000 * WiFiMgr.GetStaTimeout ()))
        {
            LOG_INFO (F ("WiFi Failed to connect using default Credentials"));
            fsm_WiFi_state_ConnectingAsAP_imp.Init ();
        }
    }

    // DEBUG_END;
}  // fsm_WiFi_state_ConnectingUsingDefaults::Poll

/*****************************************************************************/
// Wait for events
void fsm_WiFi_state_ConnectingUsingDefaults::Init ()
{
    // DEBUG_START;

    WiFiMgr.SetFsmState (this);
    WiFiMgr.AnnounceState ();
    WiFiMgr.SetFsmStartTime (millis ());

    // Switch to station mode and disconnect just in case
    // DEBUG_V ("");

    WiFiMgr.connectWifi (default_ssid, default_passphrase);

    // DEBUG_END;
}  // fsm_WiFi_state_ConnectingUsingDefaults::Init

/*****************************************************************************/
// Wait for events
void fsm_WiFi_state_ConnectingUsingDefaults::OnConnect ()
{
    // DEBUG_START;

    fsm_WiFi_state_ConnectedToAP_imp.Init ();

    // DEBUG_END;
}  // fsm_WiFi_state_ConnectingUsingDefaults::OnConnect

/*****************************************************************************/
/*****************************************************************************/
// Wait for events
void fsm_WiFi_state_ConnectingAsAP::Poll ()
{
    // DEBUG_START;

    if (0 != WiFi.softAPgetStationNum ())
    {
        fsm_WiFi_state_ConnectedToSta_imp.Init ();
    }
    else
    {
        if (millis () - WiFiMgr.GetFsmStartTime () > (1000 * WiFiMgr.GetApTimeout ()))
        {
            LOG_INFO (F ("WiFi STA Failed to connect"));
            fsm_WiFi_state_ConnectionFailed_imp.Init ();
        }
    }

    // DEBUG_END;
}  // fsm_WiFi_state_ConnectingAsAP::Poll

/*****************************************************************************/
// Wait for events
void fsm_WiFi_state_ConnectingAsAP::Init ()
{
    // DEBUG_START;

    WiFiMgr.SetFsmState (this);
    WiFiMgr.AnnounceState ();

    if (true == WiFiMgr.GetApFallbackIsEnabled ())
    {
        WiFi.mode (WIFI_AP);

        String ssid = String (F ("JurasicParkGate-")) + WiFiMgr.hostname;
        WiFi.softAP (ssid.c_str ());

        #ifdef USE_DISPLAY
            DisplayMgr.println (cDisplayMgr::eZone::IpZone, WiFi.localIP ().toString ());
        #endif // def USE_DISPLAY
        WiFiMgr.setIpAddress (WiFi.localIP ());
        WiFiMgr.setIpSubNetMask (WiFi.subnetMask ());

        LOG_INFO (String (F ("SOFTAP: IP Address: '")) + WiFiMgr.getIpAddress ().toString ());
    }
    else
    {
        LOG_INFO (String (F ("SOFTAP: Not enabled")));
        fsm_WiFi_state_ConnectionFailed_imp.Init ();
    }

    // DEBUG_END;
}  // fsm_WiFi_state_ConnectingAsAP::Init

/*****************************************************************************/
// Wait for events
void fsm_WiFi_state_ConnectingAsAP::OnConnect ()
{
    // DEBUG_START;

    fsm_WiFi_state_ConnectedToSta_imp.Init ();

    // DEBUG_END;
}  // fsm_WiFi_state_ConnectingAsAP::OnConnect

/*****************************************************************************/
/*****************************************************************************/
// Wait for events
void fsm_WiFi_state_ConnectedToAP::Poll ()
{
    // DEBUG_START;

    // did we get silently disconnected?
    if (WiFi.status () != WL_CONNECTED)
    {
        // DEBUG_V ("Handle Silent Disconnect");
        WiFi.reconnect ();
    }

    // DEBUG_END;
}  // fsm_WiFi_state_ConnectingAsAP::Poll

/*****************************************************************************/
// Wait for events
void fsm_WiFi_state_ConnectedToAP::Init ()
{
    // DEBUG_START;

    WiFiMgr.SetFsmState (this);
    WiFiMgr.AnnounceState ();

    WiFiMgr.SetUpIp ();

    #ifdef USE_DISPLAY
        DisplayMgr.println (cDisplayMgr::eZone::IpZone, WiFi.localIP ().toString ());
    #endif // def USE_DISPLAY
    WiFiMgr.setIpAddress (WiFi.localIP ());
    WiFiMgr.setIpSubNetMask (WiFi.subnetMask ());

    LOG_INFO (String (F ("Connected with IP: ")) + WiFiMgr.getIpAddress ().toString ());

    WiFiMgr.SetIsWiFiConnected (true);

    for (auto & CurrentProcess : g_listOfApplications)
    {
        CurrentProcess->NetworkStateChanged (true);
    }

    // DEBUG_END;
}  // fsm_WiFi_state_ConnectingAsAP::Init

/*****************************************************************************/
// Wait for events
void fsm_WiFi_state_ConnectedToAP::OnDisconnect ()
{
    // DEBUG_START;

    LOG_INFO (F ("Lost the connection to the AP"));
    fsm_WiFi_state_ConnectionFailed_imp.Init ();

    // DEBUG_END;
}  // fsm_WiFi_state_ConnectedToAP::OnDisconnect

/*****************************************************************************/
/*****************************************************************************/
// Wait for events
void fsm_WiFi_state_ConnectedToSta::Poll ()
{
    // DEBUG_START;

    // did we get silently disconnected?
    if (0 == WiFi.softAPgetStationNum ())
    {
        LOG_INFO (F ("Lost the connection to the STA"));
        fsm_WiFi_state_ConnectionFailed_imp.Init ();
    }

    // DEBUG_END;
}  // fsm_WiFi_state_ConnectedToSta::Poll

/*****************************************************************************/
// Wait for events
void fsm_WiFi_state_ConnectedToSta::Init ()
{
    // DEBUG_START;

    WiFiMgr.SetFsmState (this);
    WiFiMgr.AnnounceState ();

    WiFiMgr.SetUpIp ();

    #ifdef USE_DISPLAY
        DisplayMgr.println (cDisplayMgr::eZone::IpZone, WiFi.softAPIP ().toString ());
    #endif // def USE_DISPLAY
    WiFiMgr.setIpAddress (WiFi.softAPIP ());
    WiFiMgr.setIpSubNetMask (IPAddress (255, 255, 255, 0));

    LOG_INFO (String (F ("Connected to STA with IP: ")) + WiFiMgr.getIpAddress ().toString ());

    WiFiMgr.SetIsWiFiConnected (true);
    for (auto & CurrentProcess : g_listOfApplications)
    {
        CurrentProcess->NetworkStateChanged (true);
    }

    // DEBUG_END;
}  // fsm_WiFi_state_ConnectedToSta::Init

/*****************************************************************************/
// Wait for events
void fsm_WiFi_state_ConnectedToSta::OnDisconnect ()
{
    // DEBUG_START;

    LOG_INFO (F ("STA Disconnected"));
    fsm_WiFi_state_ConnectionFailed_imp.Init ();

    // DEBUG_END;
}  // fsm_WiFi_state_ConnectedToSta::OnDisconnect

/*****************************************************************************/
/*****************************************************************************/
// Wait for events
void fsm_WiFi_state_ConnectionFailed::Init ()
{
    // DEBUG_START;

    WiFiMgr.SetFsmState (this);
    WiFiMgr.AnnounceState ();

    if (WiFiMgr.IsWiFiConnected ())
    {
        WiFiMgr.SetIsWiFiConnected (false);
        for (auto & CurrentProcess : g_listOfApplications)
        {
            CurrentProcess->NetworkStateChanged (true);
        }
    }
    else
    {
        if (true == WiFiMgr.GetRebootOnWiFiFailureToConnect ())
        {
            extern bool reboot;
            LOG_INFO (F ("Requesting Reboot"));

            reboot = true;
        }
        else
        {
            LOG_INFO (F ("Reboot Disabled."));

            // start over
            fsm_WiFi_state_Boot_imp.Init ();
        }
    }

    // DEBUG_END;
}  // fsm_WiFi_state_ConnectionFailed::Init

// -----------------------------------------------------------------------------

// create a global instance of the WiFi Manager
c_WiFiMgr WiFiMgr;
