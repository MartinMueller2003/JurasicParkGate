/*
  * config.cpp - Configuration management
  *
  * The config class manages the data in config file and provides
  * access to the data. Data is stored in the file in json format.
  *
  * Since the tiny json parser is designed to not use much ram and
  * modifies the json string when it parses the string, it is not
  * possible to use the parsed results outside the context of the function
  * that invoked the parser.
  *
  */
#include "JurasicParkGate.hpp"
#include "FileMgr.hpp"

/*****************************************************************************/
/* global data                                                               */
/*****************************************************************************/
// instantiate the config class as a singleton
class c_CfgMgr CfgMgr;

/*****************************************************************************/
/* Code                                                                      */
/*****************************************************************************/
/*****************************************************************************/
/* Main entry point. Ensures a valid set of defaults are present in eeprom
  *
  *  needs
  *    nothing
  *  returns
  *    nothing
  */
c_CfgMgr::c_CfgMgr (void) : c_Common (String (F ("CfgMgr")))
{}  // c_CfgMgr

void c_CfgMgr::Init (void)
{
    // DEBUG_START;

    // DEBUG_END;
}  // Init

/*****************************************************************************/
/*
  * Read the config data from eeprom and place it into our local saved image
  * Validates the data and distributes it to all consumers.
  *
  * This function should be called sparingly since it accesses the EEPROM which is a slow
  * operation.
  *
  *	needs
  *		nothing
  *  returns
  *		Nothing
  */
void c_CfgMgr::DistributeConfig (bool initialConfig)
{
    // DEBUG_START;

    do  // once
    {
        String  sConfigRoot;
        size_t  ConfigFileSize  = FileMgr.GetConfigFileSize (ConfigFileName);
        size_t  JsonDocSize     = max (ConfigFileSize * 3, size_t(CFGMGR_CFG_SIZE));
        // DEBUG_V (String ("  ConfigFileSize: ") + String (ConfigFileSize));
        if (0 == ConfigFileSize)
        {
            LOG_ERROR (F ("Could not find the config file. Using Default config"));
            ConfigRequiresSave = true;
            break;
        }

        DynamicJsonDocument jsonDoc (JsonDocSize);
        JsonObject JsonConfig = jsonDoc.to <JsonObject>();

        // DEBUG_V (   String ("  ConfigFileName: ") + String (ConfigFileName));
        // DEBUG_V (String ("jsonDoc.capacity: ") + String (jsonDoc.capacity ()));

        // get the existing values
        if (!GetSystemConfig (jsonDoc))
        {
            LOG_ERROR (F ("Could not read config file. Using Default config"));
            CommitConfig ();
            break;
        }

        SetSystemConfig (JsonConfig);

        if (initialConfig && !UsedSdConfig)
        {
            ConfigRequiresSave = false;
        }
    } while (false);

    // DEBUG_END;
}  // DistributeConfig

/*****************************************************************************/
/*
  *	Save the existing config or crete a new config if no object was defined
  *
  *	needs
  *		reference to the json node under which we will hang our data tree
  *	returns
  *		new data in the tree
  */
void c_CfgMgr::GetConfig (JsonObject & response)
{
    // DEBUG_START;

    response[M_CONFIG_REVISION_NAME] = M_CONFIG_REVISION;

    // DEBUG_END;
}  // GetConfig

/*****************************************************************************/
/*
  * Read the config data from eeprom and place it into our local saved image
  * Validates the data and clears all of the buffers if the data is not valid.
  *
  * This function should be called sparingly since it accesses the EEPROM which is a slow
  * operation.
  *
  *	needs
  *		String in which to leave the config JSON data
  *  returns
  *		json structure filled in
  */
void c_CfgMgr::GetSystemConfig (char * response, size_t maxlen)
{
    // DEBUG_START;

    if (!FileMgr.ReadConfigFile (ConfigFileName, response, maxlen))
    {
        LOG_ERROR (F ("Failed to read Flash config file. Using SD config file"));
        if (!FileMgr.ReadSdFile (ConfigFileName, (byte *)response, maxlen))
        {
            LOG_ERROR (F ("Failed to read SD config file"));
        }
        else
        {
            UsedSdConfig = true;
        }
    }

    // DEBUG_END;
}  // GetSystemConfig

/*****************************************************************************/
/*
  * Read the config data from eeprom and place it into our local saved image
  * Validates the data and clears all of the buffers if the data is not valid.
  *
  * This function should be called sparingly since it accesses the EEPROM which is a slow
  * operation.
  *
  *	needs
  *		String in which to leave the config JSON data
  *  returns
  *		json structure filled in
  */
bool c_CfgMgr::GetSystemConfig (DynamicJsonDocument & ConfigData)
{
    // DEBUG_START;

    bool response = false;

    if (false == (response = FileMgr.ReadConfigFile (ConfigFileName, ConfigData)))
    {
        LOG_ERROR (F ("Failed to read Flash config file. Using SD config file"));
        if (false == (response = FileMgr.ReadSdFile (ConfigFileName, ConfigData)))
        {
            LOG_ERROR (F ("Failed to read SD config file"));
        }
        else
        {
            UsedSdConfig = true;
        }
    }

    // DEBUG_END;
    return response;
}  // GetSystemConfig

/*****************************************************************************/
void c_CfgMgr::SetSystemConfig (JsonObject & NewConfig)
{
    // DEBUG_START;

    do  // once
    {
        // if we have a config string, write it to the FS
        // Init each of the applications
        for (auto & listOfApplicationsIterator : g_listOfApplications)
        {
            // DEBUG_V (String ("Set Config to: ") + listOfApplicationsIterator->GetName ());
            listOfApplicationsIterator->SetConfig (NewConfig);
            // DEBUG_V (String ("Done Set Config to: ") + listOfApplicationsIterator->GetName ());
        }  // end Init each application
    } while (false);  // do once

    // DEBUG_END;
}  // SetSystemConfig

/*****************************************************************************/
void c_CfgMgr::SetSystemConfig (const char * DataString)
{
    // DEBUG_START;

    do  // once
    {
        if (nullptr == DataString)
        {
            // DEBUG_V ("Nothing to save");
            break;
        }

        if (!FileMgr.SaveConfigFile (ConfigFileName, DataString))
        {
            LOG_ERROR (F ("Failed to save config file to flash."));
        }

        if (!FileMgr.SaveSdFile (ConfigFileName, DataString))
        {
            LOG_ERROR (F ("Failed to save config file to SD card."));
        }

        DistributeConfig (true);
    } while (false);  // do once

    // DEBUG_END;
}  // SetSystemConfig

/*****************************************************************************/
void c_CfgMgr::DeleteConfigFile ()
{
    // DEBUG_START;

    FileMgr.DeleteConfigFile (ConfigFileName);
    FileMgr.DeleteSdFile (ConfigFileName);

    // DEBUG_END;
}  // DeleteConfigFile

/*****************************************************************************/
/*
  * Write the known config data out to the internal FS
  */
void c_CfgMgr::CommitConfig (void)
{
    // DEBUG_START;

    do  // once
    {
        size_t  ConfigFileSize  = FileMgr.GetConfigFileSize (ConfigFileName);
        size_t  JsonDocSize     = max (ConfigFileSize * 3, size_t(CFGMGR_CFG_SIZE));
        DynamicJsonDocument jsonDoc (JsonDocSize);
        // DEBUG_V (String ("jsonDoc.capacity: ") + String (jsonDoc.capacity()));
        JsonObject JsonConfig = jsonDoc.to <JsonObject>();

        GetSystemConfig (jsonDoc);

        for (auto & currentApplication : g_listOfApplications)
        {
            currentApplication->GetConfig (JsonConfig);
        }

        if (jsonDoc.overflowed ())
        {
            LOG_ERROR (String (F ("JSON Config doc is too small.")));
        }
        else
        {
            // save the current settings
            LOG_INFO (F ("Saving current configuration."));
            FileMgr.SaveConfigFile (ConfigFileName, JsonConfig);
            FileMgr.WriteSdFile (ConfigFileName, JsonConfig);
        }
    } while (false);

    // DEBUG_END;
}  // CommitConfig

/*****************************************************************************/
/*
  * Write the known config data out to the internal spiffs
  */
void c_CfgMgr::CommitStatistics (void)
{
    // DEBUG_START;

    // DEBUG_END;
}  // commit

/*****************************************************************************/
/*
  *	Check to see if we are need to save the config
  *
  *	needs
  *		nothing
  *	returns
  *		nothing
  */
void c_CfgMgr::Poll (void)
{
    // DEBUG_START;

    // Has the ISR modified the config?
    if (true == ConfigRequiresSave)
    {
        // DEBUG_V ("Need to Commit Config");

        ConfigRequiresSave = false;

        // save the current config
        CommitConfig ();

        // DEBUG_V ("");
    }

    // Has the ISR modified the config?
    if (true == StatusRequiresSave)
    {
        StatusRequiresSave = false;
        // DEBUG_V ("");
        // save the current config
        CommitStatistics ();
        // DEBUG_V ("");
    }

    // DEBUG_END;
}  // Poll

// Cant inline these
void    c_CfgMgr::ScheduleConfigCommit (void)       {ConfigRequiresSave = true;}
void    c_CfgMgr::ScheduleStatisticsCommit (void)   {StatusRequiresSave = true;}
