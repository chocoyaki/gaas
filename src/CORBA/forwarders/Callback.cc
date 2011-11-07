/**
 * @file Callback.cc
 *
 * @brief  DIET forwarder implementation - Client forwarder implementation
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
DIETForwarder::notifyResults(const char *path, const ::corba_profile_t &pb,
                             ::CORBA::Long reqID, const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->notifyResults(path, pb, reqID, objString.c_str());
  }

  name = getName(objString);

  Callback_var cb =
    ORBMgr::getMgr()->resolve<Callback, Callback_var>(CLIENTCTXT, name,
                                                      this->name);
  return cb->notifyResults(path, pb, reqID);
} // notifyResults

::CORBA::Long
DIETForwarder::solveResults(const char *path,
                            const ::corba_profile_t &pb,
                            ::CORBA::Long reqID,
                            ::CORBA::Long result,
                            const char *objName) {
  std::string objString(objName);
  std::string name;

  if (!remoteCall(objString)) {
    return getPeer()->solveResults(path, pb, reqID, result, objString.c_str());
  }

  name = getName(objString);

  Callback_var cb =
    ORBMgr::getMgr()->resolve<Callback, Callback_var>(CLIENTCTXT, name,
                                                      this->name);
  return cb->solveResults(path, pb, reqID, result);
} // solveResults
