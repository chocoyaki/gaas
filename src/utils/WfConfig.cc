/****************************************************************************/
/* [Documentation Here!]                                                    */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id: 
 * $Log: 
 ****************************************************************************/

#include "WfConfig.hh"

bool WfConfig::use_wf_log = false;
WfLogService_var WfConfig::myWfLogService = WfLogService::_nil();

WfConfig::WfConfig() {
  use_wf_log = false;
}

WfConfig::WfConfig(bool b) {
  use_wf_log = b;
}

void
WfConfig::useLog(bool b, WfLogService_var logRef) {
  use_wf_log = b;
  myWfLogService = logRef;
}

bool
WfConfig::logUsed() {
  return use_wf_log;
}
