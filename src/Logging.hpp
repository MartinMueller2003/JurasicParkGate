/*****************************************************************************/
/*
  * This is the manager for the logging
  */
/*****************************************************************************/

// only allow the file to be included once.
#pragma once

#include "JurasicParkGate.hpp"
#ifdef USE_DISPLAY
#include "DisplayMgr.hpp"
#endif // def USE_DISPLAY
#include <stdint.h>
#include <map>

/*****************************************************************************/
class c_logging : public c_Common
{
public:
    c_logging (void);
    ~c_logging (void);

    // Stop the compiler generating methods of copy the object
    c_logging (c_logging const & copy);
    c_logging &operator=(c_logging const & copy);

    // read the config data and set up the logging
    void    Init (void);
    void    SetConfig (JsonObject & jsonData);

    // create a json config record with the current config data
    void GetConfig (JsonObject & jsonResult);

    typedef enum logLevel_e
    {
        ERROR = 0,
        WARN,
        INFO
    } logLevel_t;
    void writeLog (logLevel_t level, const String & sModuleId, const String & sOutput);

private:

    #define M_logging_JSON_ROOT         CN_logging
    #define M_MaxUsagePercent_JSON_NAME CN_MUP

    const int chipSelect = 16;

    uint8_t         m_iMaxUsagePercent = 40;
    typedef std::map <logLevel_t, String> ReasonCodeMap_t;
    ReasonCodeMap_t m_reasonCodes;

    void WriteLogEntryToSD (String & DataToWrite);
};  // class c_logging

/*****************************************************************************/

#define LOG_INFO(m)     Logging.writeLog (c_logging::logLevel_t::INFO,  GetName (), (m))
#define LOG_WARN(m)     Logging.writeLog (c_logging::logLevel_t::WARN,  GetName (), (m))
#define LOG_ERROR(m)    Logging.writeLog (c_logging::logLevel_t::ERROR, GetName (), (m))

#define MYFILE String (__FILE__).substring (String (__FILE__).lastIndexOf ("\\") + 1)

#define DEBUG_V(v)   \
{   \
    LOG_PORT.println (String ("------ ") + String (FPSTR (__func__)) + ":" + MYFILE + ":" + String (__LINE__) + ": " + String (v) + String (" ------"));   \
    LOG_PORT.flush ();   \
    /* DisplayMgr.println (cDisplayMgr::eZone::LogZone, String ("------ ") + String (FPSTR (__func__)) + ":" + MYFILE + ":" + String (__LINE__) + ": " + String (v) + String (" ------"));*/   \
}

#define DEBUG_START DEBUG_V (F ("Start"))
#define DEBUG_END   DEBUG_V (F ("End"))

// only one instance of the class is allowed and it is globally available
extern class c_logging Logging;
