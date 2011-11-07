/**
 * @file  LocalAgentImpl.hh
 *
 * @brief  DIET local agent implementation header
 *
 * @author  Philippe COMBES (Philippe.Combes@ens-lyon.fr)
 *          Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)
 *          Frederic LOMBARD (Frederic.Lombard@lifc.univ-fcomte.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#ifndef _LOCALAGENTIMPL_HH_
#define _LOCALAGENTIMPL_HH_

#include "Agent.hh"
#include "AgentImpl.hh"

#include "Forwarder.hh"
#include "LocalAgentFwdr.hh"

class LocalAgentImpl : public POA_LocalAgent, public AgentImpl {
public:
  LocalAgentImpl();

  ~LocalAgentImpl() {
  }

  /** Force call for POA_LocalAgent::_this. */
  inline LocalAgent_ptr
  _this() {
    return this->POA_LocalAgent::_this();
  }

  /** Change the parent */
  CORBA::Long
  bindParent(const char *parentName);

  /** Disconnect from the parent */
  CORBA::Long
  disconnect();

  /** Sends a request to the agent to commit suicide.
   * If "recursive" is true, then the agent forwards the request
   * to its children.
   */
  virtual CORBA::Long
  removeElement(bool recursive);

  virtual void
  removeElementClean(bool recursive);

  /** Launch this agent (initialization + registration in the hierarchy). */
  int
  run();

  /** Get a request from the parent */
  virtual void
  getRequest(const corba_request_t &req);

  /**
   * Add \c services into the service table, and attach them to child \c me.
   * Then, propagate information to the parent
   */
  virtual CORBA::Long
  addServices(CORBA::ULong myID, const SeqCorbaProfileDesc_t &services);

  virtual CORBA::Long
  childUnsubscribe(CORBA::ULong childID,
                   const SeqCorbaProfileDesc_t &services);

  /** Remove services into the service table for a given child */
  virtual CORBA::Long
  childRemoveService(CORBA::ULong childID, const corba_profile_desc_t &profile);
  virtual SeqString *
  searchData(const char *request);

private:

  /** Reference of the parent */
  Agent_var parent;
  /** ID of this agent amongst the children of its parent */
  ChildID childID;
};  // LocalAgentImpl

class LocalAgentFwdrImpl : public POA_LocalAgentFwdr,
public PortableServer::RefCountServantBase {
public:
  LocalAgentFwdrImpl(Forwarder_ptr fwdr, const char *objName);

  virtual CORBA::Long
  agentSubscribe(const char *me, const char *hostName,
                 const SeqCorbaProfileDesc_t &services);
  virtual CORBA::Long
  serverSubscribe(const char *me, const char *hostName,
                  const SeqCorbaProfileDesc_t &services);

  virtual CORBA::Long
  childUnsubscribe(CORBA::ULong childID,
                   const SeqCorbaProfileDesc_t &services);

  virtual CORBA::Long
  removeElement(bool recursive);

  CORBA::Long
  bindParent(const char *parentName);

  CORBA::Long
  disconnect();

  virtual void
  getRequest(const corba_request_t &req);

  virtual void
  getResponse(const corba_response_t &resp);

  virtual CORBA::Long
  ping();

  virtual char *
  getHostname();

  virtual CORBA::Long
  addServices(CORBA::ULong myID, const SeqCorbaProfileDesc_t &services);

  virtual CORBA::Long
  childRemoveService(CORBA::ULong childID,
                     const corba_profile_desc_t &profile);

  virtual char *
  getDataManager();

  virtual SeqString *
  searchData(const char *request);

private:
  Forwarder_ptr forwarder;
  char *objName;
};

#endif  // _LOCALAGENTIMPL_HH_
