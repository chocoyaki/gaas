/****************************************************************************/
/* $Id$ */
/* DIET ORB Manager header                                                  */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/*
 * $Log$
 * Revision 1.1  2003/02/04 09:58:13  pcombes
 * Unify ORBs interface with a manager class: ORBMgr
 *
 ****************************************************************************/

#ifndef _ORBMGR_HH_
#define _ORBMGR_HH_

#include "DIET_config.h"

#include <CORBA.h>

/**
 * This class unifies the interface to all ORBs supported by DIET.
 */

class ORBMgr
{
  
public:
  static void
  traceLevel(int level);

  static int
  init(int argc, char** argv, bool init_POA);

  static int
  activate(PortableServer::ServantBase* obj);

  static void
  wait();

  static CORBA::Object_ptr
  getAgentReference(const char* agentName);
  
  static CORBA::Boolean
  bindAgentToName(CORBA::Object_ptr obj, const char* agentName);

  static CORBA::String_var
  getIORstring(CORBA::Object_ptr obj);

};


#endif // _ORBMGR_HH_
