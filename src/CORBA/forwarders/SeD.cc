/**
 * @file SeD.cc
 *
 * @brief  DIET forwarder implementation - SeD forwarder implementation
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
DIETForwarder::checkContract(::corba_estimation_t &estimation,
                             const ::corba_pb_desc_t &pb,
                             const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->checkContract(estimation, pb, objString.c_str());
  }

  name = getName(objString);

  SeD_var sed =
    ORBMgr::getMgr()->resolve<SeD, SeD_var>(SEDCTXT, name, this->name);
  return sed->checkContract(estimation, pb);
} // checkContract

void
DIETForwarder::updateTimeSinceLastSolve(const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->updateTimeSinceLastSolve(objString.c_str());
  }

  name = getName(objString);

  SeD_var sed =
    ORBMgr::getMgr()->resolve<SeD, SeD_var>(SEDCTXT, name, this->name);
  return sed->updateTimeSinceLastSolve();
} // updateTimeSinceLastSolve

::CORBA::Long
DIETForwarder::solve(const char *path,
                     ::corba_profile_t &pb,
                     const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->solve(path, pb, objString.c_str());
  }

  name = getName(objString);

  SeD_var sed =
    ORBMgr::getMgr()->resolve<SeD, SeD_var>(SEDCTXT, name, this->name);
  return sed->solve(path, pb);
} // solve

void
DIETForwarder::solveAsync(const char *path,
                          const ::corba_profile_t &pb,
                          const char *volatileclientPtr,
                          const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->solveAsync(path, pb, volatileclientPtr, objString.c_str());
  }

  name = getName(objString);

  SeD_var sed =
    ORBMgr::getMgr()->resolve<SeD, SeD_var>(SEDCTXT, name, this->name);
  return sed->solveAsync(path, pb, volatileclientPtr);
} // solveAsync

char *
DIETForwarder::getDataMgrID(const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->getDataMgrID(objString.c_str());
  }

  name = getName(objString);

  SeD_var sed =
    ORBMgr::getMgr()->resolve<SeD, SeD_var>(SEDCTXT, name, this->name);
  return sed->getDataMgrID();
} // getDataMgrID

SeqCorbaProfileDesc_t *
DIETForwarder::getSeDProfiles(::CORBA::Long &length,
                              const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->getSeDProfiles(length, objString.c_str());
  }

  name = getName(objString);

  SeD_var sed =
    ORBMgr::getMgr()->resolve<SeD, SeD_var>(SEDCTXT, name, this->name);
  return sed->getSeDProfiles(length);
} // getSeDProfiles
