/*
  * This is a common framework that makes the main loop functionality simpler to manage
  */

#include "JurasicParkGate.hpp"

/*****************************************************************************/
/* global data                                                               */
/*****************************************************************************/

/*****************************************************************************/
/* Code                                                                      */
/*****************************************************************************/
c_Common::c_Common (String sName)
{
    m_sName = sName;
}  // c_Common

/*****************************************************************************/
c_Common::~c_Common (void) {}

/*****************************************************************************/
void c_Common::Init (void)
{
    // DEBUG_START;
    // DEBUG_END;
}  // c_logging::Init

/*****************************************************************************/
void c_Common::GetConfig (JsonObject & jsonRoot)
{
    // DEBUG_START;
    // DEBUG_END;
}  // GetConfig
