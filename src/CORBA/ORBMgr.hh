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
 * Revision 1.4  2003/06/02 08:53:16  cpera
 * Update api for asynchronize calls, manage bidir poa.
 *
 * Revision 1.3  2003/05/05 14:10:55  pcombes
 * Add destroy and stringToObject methods.
 *
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
  init(int argc, char** argv, bool init_POA, int DIET_ct = 1); 

  static void
  destroy();

  static int
  activate(PortableServer::ServantBase* obj);

  static void
  wait();

  static CORBA::Object_ptr
  getAgentReference(const char* agentName);
  
  static int
  bindAgentToName(CORBA::Object_ptr obj, const char* agentName);

  static char * 
  getIORString(CORBA::Object_ptr obj);

  static CORBA::Object_ptr
  stringToObject(const char* IOR);

private:
  static CORBA::ORB_ptr          ORB;
  static PortableServer::POA_var POA;
  static PortableServer::POA_var POA_BIDIR;
  static int DIET_ct;
};


#endif // _ORBMGR_HH_
