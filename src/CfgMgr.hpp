/*****************************************************************************/
/*
  * config.h - Configuration management
  *
  * The config class manages the data in littlefs and provides
  * access to the data.
  *
  */
/*****************************************************************************/

// only allow the file to be included once.
#pragma once

#include "JurasicParkGate.hpp"

/*****************************************************************************/
class c_CfgMgr : public c_Common
{
public:
    c_CfgMgr (void);

    // Stop the compiler generating methods to copy the object
    c_CfgMgr (c_CfgMgr const & copy);
    c_CfgMgr &operator=(c_CfgMgr const & copy);

    // set up the runtime config
    void    Init (void);
    void    GetConfig (JsonObject & response);

    void    GetSystemConfig (char * DataString, size_t maxLen);
    bool    GetSystemConfig (DynamicJsonDocument & NewConfig);

    void    SetSystemConfig (JsonObject & NewConfig);
    void    SetSystemConfig (const char * DataString);

    void    DistributeConfig (bool initialConfig);
    void    DeleteConfigFile (void);

    // check to see if a config save is needed
    void Poll (void);

    // set up a request so that next time through the main loop we do a commit
    // Cant be inlined since these are passed by reference to the READ_JSON macros
    void    ScheduleConfigCommit (void);
    void    ScheduleStatisticsCommit (void);

    typedef enum ScheduleUpdate_e
    {
        NoUpdate = 0,
        ConfigUpdate,
        StatsUpdate
    } ScheduleUpdate_t;

    static const size_t CFGMGR_CFG_SIZE = 30 * 1024;

private:
    #define M_CONFIG_REVISION       1
    #define M_CONFIG_REVISION_NAME  CN_CfgVer

    // create a json object that represents the current config and then save it to eeprom
    void    CommitConfig (void);
    void    CommitStatistics (void);

    // Validate the current configuration data
    bool validateData (void);

    bool    ConfigRequiresSave  = false;    // set to true when the ISR has processed a new config or another process needs a commit
    bool    StatusRequiresSave  = false;    // set to true when a process needs a commit of the statistics records
    bool    UsedSdConfig        = false;

    String  ConfigFileName = String ("/config.json");
};  // class c_CfgMgr
/*****************************************************************************/

// only one instance of the class is allowed and it is globally available
extern class c_CfgMgr CfgMgr;

template <typename jsonRecord, typename jsonVar, typename VarName, typename JsonAction>
void inline READ_JSON (jsonRecord & jsonData, jsonVar & resultValue, VarName name, JsonAction action)
{
    if (true == (jsonData).containsKey (name))
    {
        jsonVar tempResult = (jsonData)[name];
        if (resultValue != tempResult)
        {
            resultValue = tempResult;
            // Serial.println (String ("tempResult: ") + String (tempResult));
            // Serial.println (String ("resultValue: ") + String (resultValue));
            switch (action)
            {
                case c_CfgMgr::ScheduleUpdate_t::ConfigUpdate:
                {
                    CfgMgr.ScheduleConfigCommit ();
                    break;
                }

                case c_CfgMgr::ScheduleUpdate_t::StatsUpdate:
                {
                    CfgMgr.ScheduleStatisticsCommit ();
                    break;
                }

                case c_CfgMgr::ScheduleUpdate_t::NoUpdate:
                default:
                {
                    break;
                }
            }  /* end switch */
        }
    }
}
