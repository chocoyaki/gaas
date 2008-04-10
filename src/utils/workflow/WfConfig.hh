/****************************************************************************/
/* A utility class for workflow configuration parameters                    */
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

#ifndef _WFCONFIG_HH_
#define _WFCONFIG_HH_


#include "WfLogSrv.hh"

class WfConfig {
public:

  WfConfig();
  WfConfig(bool use_wf_log);

  static void 
  useLog(bool b, WfLogSrv_var logRef = NULL);

  static bool
  logUsed();

  static bool use_wf_log;
  static WfLogSrv_var myWfLogSrv;
  
private:
};


#endif   /* not defined _WFCONFIG_HH */



