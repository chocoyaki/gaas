/****************************************************************************/
/* DIET forwarder implementation - Workflow mngr forwarder implementation   */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Gael Le Mahec   (gael.le.mahec@ens-lyon.fr)                         */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.4  2010/07/27 16:16:49  glemahec
 * Forwarders robustness
 *
 * Revision 1.3  2010/07/14 23:45:30  bdepardo
 * Header corrections
 *
 * Revision 1.2  2010/07/13 15:24:13  glemahec
 * Warnings corrections and some robustness improvements
 *
 * Revision 1.1  2010/07/12 16:08:56  glemahec
 * DIET 2.5 beta 1 - Forwarder implementations
 ****************************************************************************/

#include "DIETForwarder.hh"
#include "ORBMgr.hh"
#include <string>

using namespace std;


::CORBA::Long DIETForwarder::execNodeOnSed(const char* node_id,
					   const char* dag_id,
					   const char* seDName,
					   ::CORBA::ULong reqID,
					   ::corba_estimation_t& ev,
					   const char* objName)
{
  string objString(objName);
  string name;
        
  if (!remoteCall(objString)) {
    return getPeer()->execNodeOnSed(node_id, dag_id, seDName,
				    reqID, ev, objString.c_str());
  }
        
  name = getName(objString);
                
  CltMan_var clt = ORBMgr::getMgr()->resolve<CltMan, CltMan_var>(WFMGRCTXT, name,
								 this->name);
  return clt->execNodeOnSed(node_id, dag_id, seDName, reqID, ev);
}

::CORBA::Long DIETForwarder::execNode(const char* node_id,
				      const char* dag_id,
				      const char* objName)
{
  string objString(objName);
  string name;
        
  if (!remoteCall(objString)) {
    return getPeer()->execNode(node_id, dag_id, objString.c_str());
  }
        
  name = getName(objString);
        
  CltMan_var clt = ORBMgr::getMgr()->resolve<CltMan, CltMan_var>(WFMGRCTXT, name,
								 this->name);
  return clt->execNode(node_id, dag_id);
}

char* DIETForwarder::release(const char* dag_id,
			     ::CORBA::Boolean successful,
			     const char* objName)
{
  string objString(objName);
  string name;
        
  if (!remoteCall(objString)) {
    return getPeer()->release(dag_id, successful, objString.c_str());
  }
        
  name = getName(objString);
                
  CltMan_var clt = ORBMgr::getMgr()->resolve<CltMan, CltMan_var>(WFMGRCTXT, name,
								 this->name);
  return clt->release(dag_id, successful);
}
