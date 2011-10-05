/****************************************************************************/
/* Remove a diet component                                                  */
/* Function code for dynamically killing an element and (potentially) its   */
/* underlying hierachy.                                                     */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Benjamin DEPARDON (Benjamin.Depardon@ens-lyon.fr)                   */
/*    - Kevin COULOMB (kevin.coulomb@-sysfera.fr)                           */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/


#include "DIET_admin.h"
#include "SeD.hh"
#include "Agent.hh"
#include "ORBMgr.hh"

int
diet_remove_from_hierarchy(dynamic_type_t type, const char *name, int recursive) {
  int argc = 0;
  char **argv = NULL;

  try {
    /* Initialize ORB */
    ORBMgr::init(argc, argv);

    switch (type) {
    case SED: {
      SeD_var sed = ORBMgr::getMgr()->resolve<SeD, SeD_ptr>(SEDCTXT, name);
      if (sed->removeElement()) {
        return DIET_ADMIN_CALL_ERROR;
      }
      break;
    }
    case AGENT:
      // Intentional fall through
    case MA:
      // Intentional fall through
    case LA: {
      Agent_var agent = ORBMgr::getMgr()->resolve<Agent, Agent_var>(AGENTCTXT, name);
      if (agent->removeElement(recursive)) {
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

