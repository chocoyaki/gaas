/****************************************************************************/
/* [Documentation Here!]                                                    */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2006/11/27 10:15:12  aamar
 * Correct headers of files used in workflow support.
 *
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
