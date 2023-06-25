/*
  *    Name:		JurasicParkGate.ino
  *    Created:	4/7/2021 09:36:41
  *    Author:	mmueller
  */

#include "JurasicParkGate.hpp"       // config and control for the application interface
#include "GhMgr.hpp"            // config and control for the application interface
#include "FileMgr.hpp"          // config and control for the File interface
#include "WifiMgr.hpp"          // config and control for the WiFi interface
#include "WebMgr.hpp"           // config and control for the web user interface
// #include "SensorMgr.hpp"        // config and control for the Data collection objects
#ifdef USE_DISPLAY
    #include "OutputMgr.hpp"        // config and control for the Data collection objects
#endif // def USE_DISPLAY
#include "DisplayMgr.hpp"       // config and control for the TFT Display object

#include <Wire.h>

const String    VERSION     = "0.1-dev";
const String    BUILD_DATE  = String (__DATE__) + " - " + String (__TIME__);

// this is an ordered list. Be carefull how you modify it
listOfApplications_t g_listOfApplications
{
    #ifdef USE_DISPLAY
        & DisplayMgr,
    #endif // def USE_DISPLAY
    & FileMgr,
    & CfgMgr,
    & Logging,
    & WiFiMgr,
    & WebMgr,
    & OutputMgr,
    & JurasicParkGateMgr
};

bool reboot = false;

// the setup function runs once when you press reset or power the board
void setup ()
{
    Serial.begin (115200);
    delay (10);

    // DEBUG_V (F ("Boot"));

    // SaveCrash.print();
    // SaveCrash.clear();

    // DEBUG_V (F ("Init App Start"));

    // Init each of the applications
    for (auto & listOfApplicationsIterator : g_listOfApplications)
    {
        // String sName;
        // listOfApplicationsIterator->GetName (sName);
        // DEBUG_V (String ("Init App: ") + sName);
        // DisplayMgr.println (cDisplayMgr::eZone::LogZone, sName);

        listOfApplicationsIterator->Init ();
        // DEBUG_V (String ("Done Init App: ") + sName);
    }  // end Init each application

    // DEBUG_V (String ("Done Init All Apps "));

    // now distribute the config
    // DisplayMgr.println (cDisplayMgr::eZone::LogZone, "40");
    CfgMgr.DistributeConfig (true);
    // DisplayMgr.println (cDisplayMgr::eZone::LogZone, "50");

    // DEBUG_V (String ("Done Config All Apps "));

    // DEBUG_END;
}  // setup

// the loop function runs over and over again until power down or reset
void loop ()
{
    // DEBUG_START;

    // Poll each of the applications
    for (auto & listOfApplicationsIterator : g_listOfApplications)
    {
        // String sName;
        // listOfApplicationsIterator->GetName(sName);
        // DEBUG_V(String ("Poll an Application: Started ") + sName);

        listOfApplicationsIterator->Poll ();

        // DEBUG_V(String ("Poll an Application: Done ") + sName);
    }  // end Poll each application

    if (true == reboot)
    {
        LOG_PORT.println (String (CN_stars) + CN_minussigns + F ("Internal Reboot Requested. Rebooting Now"));
        delay (REBOOT_DELAYms);
        ESP.restart ();
    }

    // DEBUG_END;
}  // loop
