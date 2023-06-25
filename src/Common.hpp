/*****************************************************************************/
/*
  * Common class to be included by all others. This brings in basic configuration
  * and Init/Poll
  */
/*****************************************************************************/

// only allow the file to be included once.
#pragma once

#include "JurasicParkGate.hpp"
#include <vector>

/*****************************************************************************/
class c_Common
{
public:
    c_Common (String sName);
    ~c_Common (void);

    // read the config data and set up the logging
    virtual void Init (void) = 0;

    // called once each pass through the main loop
    virtual void Poll (void) {}

    // Request to create a json config record with the current config data
    virtual void    GetConfig       (JsonObject & jsonData);
    virtual void    SetConfig       (JsonObject & jsonData) {}
    virtual void    GetStatus       (JsonObject & jsonData) {}
    virtual void    ResetStatistics ()                      {}

    // retrieve the name set at Init time
    virtual void    GetName (String & sName)    {sName = m_sName;}
    virtual String  GetName (void)              {return m_sName;}

    bool            IsInitialized ()                    {return m_bInitialized;}
    virtual void    NetworkStateChanged (bool NewState) {}

protected:
    bool    m_bInitialized = false;

private:

    String  m_sName;
};  // class c_Common

typedef std::vector <c_Common *> listOfApplications_t;
extern listOfApplications_t g_listOfApplications;
