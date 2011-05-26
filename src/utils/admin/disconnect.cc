/****************************************************************************/
/* Dynamic hierarchy management                                             */
/* Example code for dynamically disconnecting an element from its parent.   */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Benjamin DEPARDON (Benjamin.Depardon@ens-lyon.fr)                   */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/

#include "DIET_admin.h"
#include "SeD.hh"
#include "LocalAgent.hh"
#include "ORBMgr.hh"


int
diet_disconnect_from_hierarchy(dynamic_type_t type, const char *name) {
  int argc = 0;
  char **argv = NULL;

  try {
    /* Initialize ORB */
    ORBMgr::init(argc, argv);

    switch (type) {
    case SED: {
      SeD_var sed = ORBMgr::getMgr()->resolve<SeD, SeD_ptr>(SEDCTXT, name);
      if (sed->disconnect()) {
	return DIET_ADMIN_CALL_ERROR;
      }
      break;
    }
    case LA: {
      LocalAgent_var agent = ORBMgr::getMgr()->resolve<LocalAgent, LocalAgent_var>(AGENTCTXT, name);
      if (agent->disconnect()) {
	return DIET_ADMIN_CALL_ERROR;
      }
      break;
    }
    default: {
      return DIET_UNKNOWN_ERROR;
    }
    } // end: switch (type)

  } catch(...) {
    return DIET_COMM_ERROR;
  }
  return DIET_NO_ERROR;
}
