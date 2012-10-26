/**
 * @file  CallbackImpl.cc
 *
 * @brief   POA callback implementation
 *
 * @author   Christophe PERA (christophe.pera@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */




#include <iostream>
#include <CallbackImpl.hh>
#define TRACE_LEVEL 50
#include "debug.hh"

#include "Forwarder.hh"
#include "CallbackFwdr.hh"

using namespace std;

CallbackImpl::CallbackImpl() {
  // nothing
}

CallbackImpl::~CallbackImpl() {
  // nothing
}

CORBA::Long
CallbackImpl::ping() {
  return 0;
}

CORBA::Long
CallbackImpl::notifyResults(const char *path,
                            const corba_profile_t &pb,
                            CORBA::Long reqID) {
  /*
     if (solve_res != GRPC_NO_ERROR)
     CallAsyncMgr::Instance()->setReqErrorCode(reqID, solve_res);
   */
  return 0;
}

CORBA::Long
CallbackImpl::solveResults(const char *path,
                           const corba_profile_t &pb,
                           CORBA::Long reqID,
                           CORBA::Long solve_res) {
  // notify solve result availability
  int err =
    CallAsyncMgr::Instance()->notifyRst(int(reqID),
                                        &(const_cast<corba_profile_t &>(pb)));
  if (solve_res != GRPC_NO_ERROR) {
    CallAsyncMgr::Instance()->setReqErrorCode(reqID, solve_res);
  }
  return err;
}

CallbackFwdrImpl::CallbackFwdrImpl(Forwarder_ptr fwdr, const char *objName) {
  this->forwarder = Forwarder::_duplicate(fwdr);
  this->objName = CORBA::string_dup(objName);
}

CORBA::Long
CallbackFwdrImpl::ping() {
  return forwarder->ping(objName);
}

CORBA::Long
CallbackFwdrImpl::notifyResults(const char *path,
                                const corba_profile_t &pb,
                                CORBA::Long reqID) {
  return forwarder->notifyResults(path, pb, reqID, objName);
}

CORBA::Long
CallbackFwdrImpl::solveResults(const char *path,
                               const corba_profile_t &pb,
                               CORBA::Long reqID,
                               CORBA::Long solve_res) {
  return forwarder->solveResults(path, pb, reqID, solve_res, objName);
}
