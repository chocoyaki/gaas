/**
 * @file connect.cc
 * @brief  Example code for dynamically changing the parent of an element.
 *
 * @author Benjamin DEPARDON (Benjamin.Depardon@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#include "DIET_admin.h"
#include "SeD.hh"
#include "LocalAgent.hh"
#include "ORBMgr.hh"

int
diet_change_parent(dynamic_type_t type, const char *name,
                   const char *parent_name) {
  int argc = 0;
  char **argv = NULL;

  try {
    /* Initialize ORB */
    ORBMgr::init(argc, argv);

    switch (type) {
    case SED: {
      SeD_var sed = ORBMgr::getMgr()->resolve<SeD, SeD_ptr>(SEDCTXT, name);
      if (sed->bindParent(parent_name)) {
        return DIET_ADMIN_CALL_ERROR;
      }
      break;
    }
    case LA: {
      LocalAgent_var agent =
        ORBMgr::getMgr()->resolve<LocalAgent, LocalAgent_var>(AGENTCTXT, name);
      if (agent->bindParent(parent_name)) {
        return DIET_ADMIN_CALL_ERROR;
      }
      break;
    }
    default: {
      return DIET_UNKNOWN_ERROR;
    }
    } // end: switch (type)
  } catch (...) {
    return DIET_COMM_ERROR;
  }
  return DIET_NO_ERROR;
} // diet_change_parent
