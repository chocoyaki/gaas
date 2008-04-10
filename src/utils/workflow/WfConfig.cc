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
 * Revision 1.1  2008/04/10 08:38:50  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 * Revision 1.2  2006/11/27 10:15:12  aamar
 * Correct headers of files used in workflow support.
 *
 ****************************************************************************/

#include "WfConfig.hh"

bool WfConfig::use_wf_log = false;
WfLogSrv_var WfConfig::myWfLogSrv = WfLogSrv::_nil();

WfConfig::WfConfig() {
  use_wf_log = false;
}

WfConfig::WfConfig(bool b) {
  use_wf_log = b;
}

void
WfConfig::useLog(bool b, WfLogSrv_var logRef) {
  use_wf_log = b;
  myWfLogSrv = logRef;
}

bool
WfConfig::logUsed() {
  return use_wf_log;
}
