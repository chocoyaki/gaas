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
 * Revision 1.15  2006/10/31 21:14:04  ecaron
 * Correct wrong header
 *
 * Revision 1.14  2006/07/10 09:55:27  aamar
 * Workflow monitoring and client reordering objects added to
 * the CONTEXT array
 *
 * Revision 1.13  2006/04/14 14:15:33  aamar
 * Adding the value MA_DAG in object_type_t enumeration.
 *
 * Revision 1.12  2005/04/28 13:07:05  eboix
 *     Inclusion of CORBA.h substitued with omniORB4/CORBA.h. --- Injay 2461
 *
 * Revision 1.11  2004/03/01 18:38:10  rbolze
 * remove function getOID() end setOID(..)
 * change in signature of activate(..)
 * this change are provide by cpontvieux
 *
 * Revision 1.10  2003/10/06 10:04:00  cpontvie
 * Moving the interruption manager here
 * The current interruption is mapped on SIGINT (Ctrl+C)
 * The 'wait' function now return after the SIGINT
 *
 * Revision 1.9  2003/09/22 21:06:12  pcombes
 * Generalize the bind/unbindAgentToName and getAgentReference methods.
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

#include <omniORB4/CORBA.h>
#include <setjmp.h>
#include "DIET_config.h"

#define INTERRUPTION_MGR 0

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
  activate(PortableServer::ServantBase* obj,
           PortableServer::ObjectId_var* idVar_ptr = NULL);

  static int
  wait();

  typedef enum {
    AGENT = 0,
    DATAMGR,
    LOCMGR,
    LOGSERVICE,
    SED
    /**
     * Code not secure since the enum is used to index the CONTEXTS array *
     */
#ifdef HAVE_WORKFLOW 
    , MA_DAG
    , CLT_REO_MAN
    , WFLOGSERVICE
#endif // HAVE_WORKFLOW
  } object_type_t;

  static int
  bindObjToName(CORBA::Object_ptr obj, object_type_t type, const char* name);

  static int
  unbindObj(object_type_t type, const char* name);
  
  static CORBA::Object_ptr
  getObjReference(object_type_t type, const char* name);

  static char*
  getIORString(CORBA::Object_ptr obj);

  static CORBA::Object_ptr
  stringToObject(const char* IOR);

  static CORBA::ORB_ptr
  getORB();

  static PortableServer::POA_var
  getPOA();

  static PortableServer::POA_var
  getPOA_BIDIR();

private:

  static CORBA::ORB_ptr          ORB;
  static PortableServer::POA_var POA;
  static PortableServer::POA_var POA_BIDIR;

  static PortableServer::ObjectId_var OBJECT_ID;
  static const char* CONTEXTS[];

  static CosNaming::NamingContext_var
  getRootContext();
  
#if INTERRUPTION_MGR
  static sigjmp_buf buff_int;
  static void
  SigIntHandler(int sig);
#endif // INTERRUPTION_MGR
};


#endif // _ORBMGR_HH_
