/**
 * @file  CallbackImpl.hh
 *
 * @brief   POA callback implementation
 *
 * @author   Christophe PERA (christophe.pera@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef _CALLBACKIMPL_H_
#define _CALLBACKIMPL_H_

#include "Callback.hh"
#include "CallAsyncMgr.hh"

#include "Forwarder.hh"
#include "CallbackFwdr.hh"

class CallbackImpl : public POA_Callback,
public PortableServer::RefCountServantBase {
public:
  CallbackImpl();

  virtual ~CallbackImpl();

  virtual CORBA::Long
  ping();

  virtual CORBA::Long
  notifyResults(const char *path,
                const corba_profile_t &pb,
                CORBA::Long reqID);

  virtual CORBA::Long
  solveResults(const char *path,
               const corba_profile_t &pb,
               CORBA::Long reqID,
               CORBA::Long solve_res);
};

class CallbackFwdrImpl : public POA_CallbackFwdr,
public PortableServer::RefCountServantBase {
public:
  CallbackFwdrImpl(Forwarder_ptr fwdr, const char *objName);

  virtual CORBA::Long
  ping();

  virtual CORBA::Long
  notifyResults(const char *path,
                const corba_profile_t &pb,
                CORBA::Long reqID);

  virtual CORBA::Long
  solveResults(const char *path,
               const corba_profile_t &pb,
               CORBA::Long reqID,
               CORBA::Long solve_res);

private:
  Forwarder_ptr forwarder;
  char *objName;
};

#endif /* ifndef _CALLBACKIMPL_H_ */
