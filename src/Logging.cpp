/*
  * All activities of the state machines are logged to files in the
  * attached SD card using a FAT32 format. The logging system will make sure the
  * file system never gets more than 50% full (to allow for wear leveling). The
  * sd card must be formated by an external device. File management functions will
  * NOT be available to the user via any API. Some log information (current and
  * previous day) will be available via the web UI.
  */

#include "JurasicParkGate.hpp"
#include "RtcMgr.hpp"
#include "FileMgr.hpp"
#ifdef USE_DISPLAY
#include "DisplayMgr.hpp"
#endif // def USE_DISPLAY

/*****************************************************************************/
/* global data                                                               */
/*****************************************************************************/
// instantiate the logging control object (singleton)
class c_logging Logging;

/*****************************************************************************/
/* Code                                                                      */
/*****************************************************************************/
c_logging::c_logging (void) : c_Common (String (CN_logging))
{
    m_reasonCodes[logLevel_t::ERROR]    = CN_ERROR;
    m_reasonCodes[logLevel_t::WARN]     = CN_WARN;
    m_reasonCodes[logLevel_t::INFO]     = CN_INFO;
}  // c_logging

/*****************************************************************************/
c_logging::~c_logging (void) {}

/*****************************************************************************/
/*
  * read the config data and set up the logging
  */
void c_logging::Init (void)
{
    // DEBUG_START;

    m_bInitialized = true;

    // DEBUG_END;
}

/*****************************************************************************/
/*
  *	Process the configuration
  *
  * WARNING: This runs at the interrupt level. We can set things and cause
  *          future things to happen, but we cannot really do anything
  *
  *	needs
  *		Configuration record
  *	returns
  *		nothing
  */
void c_logging::SetConfig (JsonObject & jsonData)
{
    // DEBUG_START;

    String  sNameOfloggingRoot = M_logging_JSON_ROOT;
    String  sJsonConfigString;

    do  // once
    {
        // do we have an entry in this record that we can process

        if (false == jsonData.containsKey (sNameOfloggingRoot))
        {
            // DEBUG_V (F ("Could not find requested root in config data"));
            // the requested sub tree does not exist
            break;
        }  // failed to find the desired object

        JsonObject loggingData = jsonData[sNameOfloggingRoot];

        READ_JSON (loggingData, m_iMaxUsagePercent, M_MaxUsagePercent_JSON_NAME, c_CfgMgr::ScheduleUpdate_t::ConfigUpdate);
    } while (false);  // do once

    // DEBUG_END;
}  // c_logging::Init

/*****************************************************************************/
/*
  *	Save the existing config for the logging or crete a new config if no logging defined
  *
  *	needs
  *		reference to the json node under which we will hang our data tree
  *	returns
  *		new data in the tree
  */
void c_logging::GetConfig (JsonObject & jsonRoot)
{
    // DEBUG_START;

    JsonObject loggingData = jsonRoot.createNestedObject (M_logging_JSON_ROOT);
    loggingData[M_MaxUsagePercent_JSON_NAME] = m_iMaxUsagePercent;

    // DEBUG_END;
}  // GetConfig

/*****************************************************************************/
/*
  *	Write an entry in the log file.
  *
  *	needs
  *		log level
  *		module name
  *		message to output
  *	returns
  *		nothing
  */
void c_logging::writeLog (logLevel_t level, const String & sModuleName, const String & sMessage)
{
    // DEBUG_START;

    #define M_NAME_LEN 9

    String  sStatusMsg;
    String  sCurrentDateTime;
    RtcMgr.currentDateTime (sCurrentDateTime);

    size_t  neededSpaces    = (M_NAME_LEN - sModuleName.length ());
    size_t  trailingSpaces  = neededSpaces / 2;
    size_t  leadingSpaces   = neededSpaces - trailingSpaces;

    String sLeadingSpaces;
    while (0 != leadingSpaces--)
    {
        sLeadingSpaces += " ";
    }

    String sName = sLeadingSpaces + sModuleName;

    while (0 != trailingSpaces--)
    {
        sName += " ";
    }

    // send the message to the console
    String sOutput = m_reasonCodes[level] + sCurrentDateTime + " |" + sName + "| " + sMessage + "\n";
    LOG_PORT.print (sOutput);
    #ifdef USE_DISPLAY
        DisplayMgr.println (cDisplayMgr::eZone::LogZone, sOutput);
    #endif // def USE_DISPLAY

    if (FileMgr.SdCardIsInstalled ())
    {
        WriteLogEntryToSD (sOutput);
    }

    // DEBUG_END;
}  // writeLog

// void printme (const char * msg) {Serial.println (msg);}

/*****************************************************************************/
void c_logging::WriteLogEntryToSD (String & DataToWrite)
{
    // DEBUG_START;
    String year, month, day;

    RtcMgr.currentYear (year);
    RtcMgr.currentMonth (month);
    RtcMgr.currentDay (day);

    String FilePath = String (CN_Slash) + M_logging_JSON_ROOT + CN_Slash + year + CN_Slash + month + CN_Slash + day + ".log";

    if (FileMgr.GetSdFileExists (FilePath))
    {
        // DEBUG_V ("AppendSdFile");
        FileMgr.AppendSdFile (FilePath, DataToWrite);
    }
    else
    {
        // DEBUG_V ("WriteSdFile");
        FileMgr.WriteSdFile (FilePath, DataToWrite);
    }

    // DEBUG_END;
}  // WriteLogEntryToSD
