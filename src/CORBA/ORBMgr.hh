/****************************************************************************/
/* DIET ORB Manager header                                                  */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2003/04/10 12:43:56  pcombes
 * Use the TRACE_LEVEL of the debug module. Uniformize return codes.
 *
 * Revision 1.1  2003/02/04 09:58:13  pcombes
 * Unify ORBs interface with a manager class: ORBMgr
 ****************************************************************************/

#ifndef _ORBMGR_HH_
#define _ORBMGR_HH_

#include <CORBA.h>
#include "DIET_config.h"


/**
 * This class unifies the interface to all ORBs supported by DIET.
 */

class ORBMgr
{
  
public:
  static void
  setTraceLevel();

  static int
  init(int argc, char** argv, bool init_POA);

  static int
  activate(PortableServer::ServantBase* obj);

  static void
  wait();

  static CORBA::Object_ptr
  getAgentReference(const char* agentName);
  
  static int
  bindAgentToName(CORBA::Object_ptr obj, const char* agentName);

  static CORBA::String_var
  getIORstring(CORBA::Object_ptr obj);

private:
  static CORBA::ORB_ptr          ORB;
  static PortableServer::POA_var POA;
};


#endif // _ORBMGR_HH_
