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
 * Revision 1.6  2003/08/28 16:53:55  cpontvie
 * Add functions deactivate, unbindAgent, get_orb, get_poa, get_poa_bidir, get_oid, set_oid
 *
 * Revision 1.5  2003/07/04 09:47:53  pcombes
 * Remove useless setTraceLevel (since r1.2) and DIET_ct.
 *
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
  static int
  init(int argc, char** argv, bool init_POA = true);

  static void
  destroy();

  static int
  activate(PortableServer::ServantBase* obj);

  static int
  deactivate();

  static void
  wait();

  static CORBA::Object_ptr
  getAgentReference(const char* agentName);

  static int
  bindAgentToName(CORBA::Object_ptr obj, const char* agentName);

	static int
	unbindAgent(const char* agentName);

  static char *
  getIORString(CORBA::Object_ptr obj);

  static CORBA::Object_ptr
  stringToObject(const char* IOR);

	static CORBA::ORB_ptr
	get_orb();

	static PortableServer::POA_var
	get_poa();

	static PortableServer::POA_var
	get_poa_bidir();

	// To access to the ObjectID of the activate agent.
	static PortableServer::ObjectId_var
	get_oid();

	// To set the ObjectID of the activate agent.
	static void
	set_oid(PortableServer::ObjectId_var oid);

private:
  static CORBA::ORB_ptr          ORB;
  static PortableServer::POA_var POA;
  static PortableServer::POA_var POA_BIDIR;
	static PortableServer::ObjectId_var object_ID;
};


#endif // _ORBMGR_HH_
