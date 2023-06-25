/*****************************************************************************/
/*
  *	Create / manage history
  */
/*****************************************************************************/

// only allow the file to be included once.
#pragma once

#include "JurasicParkGate.hpp"
#include "RtcMgr.hpp"

/*****************************************************************************/
class c_History
{
public:

    typedef double HistoryReading_t;

    typedef struct
    {
        TimeSpan    iHistoryIntervalSec = TimeSpan (uint32_t (3600));
        bool        EnableRecording     = false;
    } HistoryDefinition_t;

    c_History ();
    c_History (const HistoryDefinition_t & HistoryDefinition);
    ~c_History (void);

    // create a json config record with the current config data
    void    GetConfig (JsonObject & jsonResult);
    void    SetConfig (JsonObject & jsonResult);

    // write a new config record
    void    WriteRecord (String & jsonData);
    void    WriteRecord (JsonObject & jsonData);

    // String GetName () { return m_Name; }

protected:
    #define M_SENSOR_NAME       CN_name
    #define M_HISTORY_JSON_ROOT CN_History
    #define M_HISTORY_INTERVAL  CN_historyinterval
    #define M_HISTORY_ENABLED   CN_historyenabled

    void    SetUpCurrentHistoryFile ();
    void    CreateNewHistoryFile ();

    HistoryDefinition_t m_Config;
    // String              m_Name;
    // uint                m_typeId;
    DateTime            m_NextSampleTime    = DateTime (uint32_t (0));
    DateTime            m_LastUpdateDate    = DateTime (uint32_t (0));
    String              HistoryFilePath;
    DateTime            HistoryFileDate = DateTime (uint32_t (0));
};  // class c_History

/*****************************************************************************/
