/**
 * @file CltMan.cc
 *
 * @brief  DIET forwarder implementation - Workflow mngr forwarder implementation
 *
 * @author  Gael Le Mahec   (gael.le.mahec@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#include "DIETForwarder.hh"
#include "ORBMgr.hh"
#include <string>

::CORBA::Long
DIETForwarder::execNodeOnSed(const char *node_id,
                             const char *dag_id,
                             const char *seDName,
                             ::CORBA::ULong reqID,
                             ::corba_estimation_t &ev,
                             const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->execNodeOnSed(node_id, dag_id, seDName,
                                    reqID, ev, objString.c_str());
  }

  name = getName(objString);

  CltMan_var clt =
    ORBMgr::getMgr()->resolve<CltMan, CltMan_var>(WFMGRCTXT, name, this->name);
  return clt->execNodeOnSed(node_id, dag_id, seDName, reqID, ev);
} // execNodeOnSed

::CORBA::Long
DIETForwarder::execNode(const char *node_id,
                        const char *dag_id,
                        const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->execNode(node_id, dag_id, objString.c_str());
  }

  name = getName(objString);

  CltMan_var clt =
    ORBMgr::getMgr()->resolve<CltMan, CltMan_var>(WFMGRCTXT, name, this->name);
  return clt->execNode(node_id, dag_id);
} // execNode

char *
DIETForwarder::release(const char *dag_id,
                       ::CORBA::Boolean successful,
                       const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->release(dag_id, successful, objString.c_str());
  }

  name = getName(objString);

  CltMan_var clt =
    ORBMgr::getMgr()->resolve<CltMan, CltMan_var>(WFMGRCTXT, name, this->name);
  return clt->release(dag_id, successful);
} // release
