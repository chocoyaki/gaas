/**
 * @file MaDag.cc
 *
 * @brief  DIET forwarder implementation - MA DAG forwarder implementation
 *
 * @author  Gael Le Mahec   (gael.le.mahec@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#include "DIETForwarder.hh"
#include "ORBMgr.hh"
#include <string>

::CORBA::Long
DIETForwarder::processDagWf(const ::corba_wf_desc_t &dag_desc,
                            const char *cltMgrRef,
                            ::CORBA::Long wfReqId,
                            const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->processDagWf(dag_desc, cltMgrRef,
                                   wfReqId, objString.c_str());
  }

  name = getName(objString);

  MaDag_var agent =
    ORBMgr::getMgr()->resolve<MaDag, MaDag_var>(MADAGCTXT, name, this->name);
  return agent->processDagWf(dag_desc, cltMgrRef, wfReqId);
} // processDagWf

::CORBA::Long
DIETForwarder::processMultiDagWf(const ::corba_wf_desc_t &dag_desc,
                                 const char *cltMgrRef,
                                 ::CORBA::Long wfReqId,
                                 ::CORBA::Boolean release,
                                 const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->processMultiDagWf(dag_desc, cltMgrRef,
                                        wfReqId, release,
                                        objString.c_str());
  }

  name = getName(objString);

  MaDag_var agent =
    ORBMgr::getMgr()->resolve<MaDag, MaDag_var>(MADAGCTXT, name, this->name);
  return agent->processMultiDagWf(dag_desc, cltMgrRef, wfReqId, release);
} // processMultiDagWf

::CORBA::Long
DIETForwarder::getWfReqId(const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->getWfReqId(objString.c_str());
  }

  name = getName(objString);

  MaDag_var agent =
    ORBMgr::getMgr()->resolve<MaDag, MaDag_var>(MADAGCTXT, name, this->name);
  return agent->getWfReqId();
} // getWfReqId

void
DIETForwarder::releaseMultiDag(::CORBA::Long wfReqId, const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->releaseMultiDag(wfReqId, objString.c_str());
  }

  name = getName(objString);

  MaDag_var agent =
    ORBMgr::getMgr()->resolve<MaDag, MaDag_var>(MADAGCTXT, name, this->name);
  return agent->releaseMultiDag(wfReqId);
} // releaseMultiDag

void
DIETForwarder::cancelDag(::CORBA::Long dagId, const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->cancelDag(dagId, objString.c_str());
  }

  name = getName(objString);

  MaDag_var agent =
    ORBMgr::getMgr()->resolve<MaDag, MaDag_var>(MADAGCTXT, name, this->name);
  return agent->cancelDag(dagId);
} // cancelDag

void
DIETForwarder::setPlatformType(::MaDag::pfmType_t pfmType,
                               const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->setPlatformType(pfmType, objString.c_str());
  }

  name = getName(objString);

  MaDag_var agent =
    ORBMgr::getMgr()->resolve<MaDag, MaDag_var>(MADAGCTXT, name, this->name);
  return agent->setPlatformType(pfmType);
} // setPlatformType
