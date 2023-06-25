/*
  *	Create / manage history
  */

#include "History.hpp"
#include "FileMgr.hpp"
#include "CfgMgr.hpp"
#include "RtcMgr.hpp"

/*****************************************************************************/
/* global data                                                               */
/*****************************************************************************/

/*****************************************************************************/
/* Code                                                                      */
/*****************************************************************************/

/*****************************************************************************/
/*
  * Constructor
  */
c_History::c_History (const HistoryDefinition_t & HistoryDefinition)
{
    // DEBUG_START;

    m_Config = HistoryDefinition;

    // DEBUG_END;
}  // c_History

/*****************************************************************************/
c_History::c_History ()
{
    // DEBUG_START;

    // DEBUG_END;
}  // c_History

/*****************************************************************************/
c_History::~c_History (void) {}
/*****************************************************************************/

/*****************************************************************************/
/*
  *    Save the existing config for the History or crete a new config if no History defined
  *
  *    needs
  *        reference to the json node under which we will hang our data tree
  *    returns
  *        new data in the tree
  */
void c_History::GetConfig (JsonObject & JsonData)
{
    // DEBUG_START;

    JsonData[M_HISTORY_INTERVAL]    = m_Config.iHistoryIntervalSec.totalseconds ();
    JsonData[M_HISTORY_ENABLED]     = m_Config.EnableRecording;

    // DEBUG_V (String ("m_Config.EnableRecording: ") + String (m_Config.EnableRecording));

    // DEBUG_END;
}  // GetConfig

/*****************************************************************************/
void c_History::SetConfig (JsonObject & JsonData)
{
    // DEBUG_START;

    uint32_t temp = m_Config.iHistoryIntervalSec.totalseconds ();
    READ_JSON ( JsonData,   temp,                       M_HISTORY_INTERVAL, c_CfgMgr::ScheduleUpdate_t::ConfigUpdate);
    READ_JSON ( JsonData,   m_Config.EnableRecording,   M_HISTORY_ENABLED,  c_CfgMgr::ScheduleUpdate_t::ConfigUpdate);
    m_Config.iHistoryIntervalSec = TimeSpan (temp);

    // DEBUG_V (String ("EnableRecording: ") + String (m_Config.EnableRecording));

    // DEBUG_END;
}  // SetConfig

/*****************************************************************************/
/*
  *    Write a new history record to the file system
  *
  *    needs
  *        Reference to String to write
  *    returns
  *        nothing
  */
void c_History::WriteRecord (String & sNewRecord)
{
    // DEBUG_START;

    do  // once
    {
        if (false == m_Config.EnableRecording)
        {
            // DEBUG_V ("Recording Not Enabled");
            break;
        }  // we are NOT allowed to record data at the moment

        // DEBUG_V ("Recording Enabled");

        DateTime now = RtcMgr.now ();

        // Is it time to take a sample?
        if (now < m_NextSampleTime)
        {
            // DEBUG_V("Not time to take a sample yet");
            break;
        }  // not time yet

        // DEBUG_V ();

        // remember when this sample was taken in a global timestamp
        m_LastUpdateDate    = now;
        m_NextSampleTime    = now + m_Config.iHistoryIntervalSec;

        String FinalRecord;
        {
            // build a json record
            DynamicJsonDocument jsonDoc (1024);
            JsonObject jsonDocObject = jsonDoc.to <JsonObject>();
//             jsonDocObject[CN_name]      = ((c_Sensor *)this)->GetName ();
//             jsonDocObject[CN_type]      = ((c_Sensor *)this)->GetId ();
            jsonDocObject[CN_reading]   = sNewRecord;
            jsonDocObject[CN_time]      = String (now.unixtime ());

            String JsonRecordString;
            serializeJson (jsonDoc, JsonRecordString);
            // DEBUG_V (String ("JsonRecordString: ") + JsonRecordString);

            // convert the data to a String
            FinalRecord = String (",") + JsonRecordString + "]";
        }

        // DEBUG_V (String ("    FinalRecord: ") + FinalRecord);

        // write the result to disk
        SetUpCurrentHistoryFile ();

        // try to create a file for us to work with
        if (false == FileMgr.AppendSdFile (HistoryFilePath, FinalRecord, 1))
        {
            // DEBUG_V (String ("Could not Write: ") + HistoryFilePath);
            break;
        }  // write failed

        // DEBUG_V ();
    } while (false);

    // DEBUG_END;
}  // WriteRecord

/*****************************************************************************/
/*
  *    Write a new history record to the file system
  *
  *    needs
  *        Reference to json object to write
  *    returns
  *        nothing
  */
void c_History::WriteRecord (JsonObject & NewRecord)
{
    // DEBUG_START;

    do  // once
    {
        if (false == m_Config.EnableRecording)
        {
            break;
        }  // we are NOT allowed to record data at the moment

        String DataRecord;
        serializeJson (NewRecord, DataRecord);

        WriteRecord (DataRecord);
    } while (false);

    // DEBUG_END;
}  // WriteRecord

/*****************************************************************************/
void c_History::SetUpCurrentHistoryFile ()
{
    // DEBUG_START;

    do  // once
    {
        DateTime currentDate = RtcMgr.UtcDateTime ();

        if (true == FileMgr.GetSdFileExists (HistoryFilePath))
        {
            if ((currentDate.year () == HistoryFileDate.year ()) &&
                (currentDate.month () == HistoryFileDate.month ()))
            {
                // DEBUG_V ("No new name needed");
                break;
            }
        }

        // DEBUG_V ();
        HistoryFileDate = currentDate;

        String year, month;

        RtcMgr.currentYear (year);
        RtcMgr.currentMonth (month);

        HistoryFilePath = String (CN_Slash) + M_HISTORY_JSON_ROOT + CN_Slash + year + CN_Slash + month + ".hist";
        // DEBUG_V (String ("FileName: ") + HistoryFilePath);
    } while (false);

    if (false == FileMgr.GetSdFileExists (HistoryFilePath))
    {
        // DEBUG_V ();
        CreateNewHistoryFile ();
    }

    // DEBUG_END;
}  // SetUpCurrentHistoryFileName

/*****************************************************************************/
void c_History::CreateNewHistoryFile ()
{
    // DEBUG_START;

    DynamicJsonDocument doc (1024);
    JsonArray array = doc.to <JsonArray>();

    JsonObject Header = array.createNestedObject ();
    Header["0"] = 0;

    String newData;
    serializeJson (doc, newData);

    // DEBUG_V (String ("HistoryFilePath: ") + HistoryFilePath);
    // DEBUG_V (String ("        newData: ") + newData);
    FileMgr.SaveSdFile (HistoryFilePath, newData);

    // DEBUG_END;
}  // CreateNewHistoryFile
