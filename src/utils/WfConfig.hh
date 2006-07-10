/****************************************************************************/
/* A utility class for workflow configuration parameters                    */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id:
 * $Log:
 ****************************************************************************/

#ifndef _WFCONFIG_HH_
#define _WFCONFIG_HH_


#include "WfLogService.hh"

class WfConfig {
public:

  WfConfig();
  WfConfig(bool use_wf_log);

  static void 
  useLog(bool b, WfLogService_var logRef = NULL);

  static bool
  logUsed();

  static bool use_wf_log;
  static WfLogService_var myWfLogService;
  
private:
};


#endif   /* not defined _WFCONFIG_HH */



