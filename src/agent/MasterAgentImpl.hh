/**
 * @file  MasterAgentImpl.hh
 *
 * @brief  DIET master agent implementation header
 *
 * @author  Philippe COMBES (Philippe.Combes@ens-lyon.fr)
 *          Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)
 *          Frederic LOMBARD (Frederic.Lombard@lifc.univ-fcomte.fr)
 *
 * @section Licence
 *   |LICENSE|
 */

#ifndef _MASTERAGENTIMPL_HH_
#define _MASTERAGENTIMPL_HH_

#include "MasterAgent.hh"
#include "AgentImpl.hh"
#include "LinkedList.hh"

#if HAVE_MULTI_MA
#include "BindService.hh"
#include "KeyString.hh"
#include "ts_container/ts_set.hh"
class FloodRequestsList;
#endif

#include "DagdaCatalog.hh"

#include "Forwarder.hh"
#include "MasterAgentFwdr.hh"

class MasterAgentImpl : public POA_MasterAgent, public AgentImpl {
public:
  MasterAgentImpl();
  virtual
  ~MasterAgentImpl();

  /** Force call for POA_MasterAgent::_this. */
  inline MasterAgent_ptr
  _this() {
    return this->POA_MasterAgent::_this();
  }

  /** Launch this agent (initialization + registration in the hierarchy). */
  int
  run();

  /** Problem submission : remotely called by client. */
  corba_response_t *
  submit(const corba_pb_desc_t &pb_profile, CORBA::ULong maxServers);

  /** Problem submission. Looking for SeDs that can resolve the
      problem in the local domain. */
  corba_response_t *
  submit_local(const corba_request_t &creq);

  /** get session id */
  virtual CORBA::Long
  get_session_num();

  /** returns id of a data */
  virtual char *
  get_data_id();

  /** returns the List of Profile available */
  /*  virtual  SeqCorbaProfileDesc_t*
      getProfiles();*/

  virtual SeqCorbaProfileDesc_t *
  getProfiles(CORBA::Long &length);

  /** returns true if data present, FALSE elsewhere */
  virtual CORBA::ULong
  dataLookUp(const char *argID);

  /** return the descriptor the the data */
  corba_data_desc_t *
  get_data_arg(const char *argID);

  /** removes data from the list */
  virtual CORBA::Long
  diet_free_pdata(const char *argID);

#ifdef HAVE_MULTI_MA
  /** Ask the authorization to create a link with this Master Agent */
  virtual CORBA::Boolean
  handShake(const char *name, const char *myName);
  /** returns the address of the bind service of the Master Agent */
  virtual char *
  getBindName();
  /** Updates all the references to the other Master Agent. It also
      connect to some new Master Agent if there is not enough links. */
  void
  updateRefs();
  virtual void
  searchService(const char *predecessor,
                const char *predecessorId,
                const corba_request_t &request);

  virtual void
  stopFlooding(CORBA::Long reqId, const char *senderId);

  virtual void
  serviceNotFound(CORBA::Long reqId, const char *senderId);

  virtual void
  newFlood(CORBA::Long reqId, const char *senderId);

  virtual void
  floodedArea(CORBA::Long reqId, const char *senderId);

  virtual void
  alreadyContacted(CORBA::Long reqId, const char *senderId);

  virtual void
  serviceFound(CORBA::Long reqId, const corba_response_t &decision);

  typedef NodeDescription<MasterAgent, MasterAgent_ptr> MADescription;

  typedef ts_map<KeyString, MADescription> MAList;
#endif  // HAVE_MULTI_MA

#ifdef HAVE_WORKFLOW
  /**
   * Workflow submission function. *
   * called by the MA_DAG or a client to submit a set of problems
   * @param  seq_pb     list of pb descriptions
   */
  virtual wf_response_t *
  submit_pb_set(const corba_pb_desc_seq_t &seq_pb);

  /**
   * A submission function used to submit a set of problem to the MA (OBSOLETE)
   *
   * @param pb_seq     sequence of problems
   * @param reqCount   number of requests of the client. The request number is at least
   *                   equal to sequence problem length but it can be greater if a problem
   *                   has to be executed more than one time
   * @param complete   indicates if the response is complete. The function return at the first problem
   *                   that cannot be solved
   * @param firstReqId the first request identifier to be used by the client
   * @param seqReqId   an identifier to the submission (each sequence submission
   *                   has a unique identifier)
   */
  virtual response_seq_t *
  submit_pb_seq(const corba_pb_desc_seq_t &pb_seq,
                CORBA::Long reqCount,
                CORBA::Boolean &complete,
                CORBA::Long &firstReqId,
                CORBA::Long &seqReqId);

#endif  // HAVE_WORKFLOW
  virtual SeqString *
  searchData(const char *request);

  virtual CORBA::Long
  insertData(const char *key, const SeqString &values);

private:
  DagdaCatalog *catalog;

  /** ID of next incoming request. */
  Counter reqIDCounter;
  Counter num_session;
  Counter num_data;
#ifdef HAVE_MULTI_MA
  typedef ts_set<KeyString> StrList;
  StrList MAIds;
  MAList knownMAs;
  int minMAlinks;
  int maxMAlinks;
  BindService *bindSrv;
  CORBA::String_var bindName;
  FloodRequestsList *floodRequestsList;
  typedef ts_set<CORBA::Long> ReqIdList;
  ReqIdList reqIdList;
  void
  logNeighbors();
#endif  // HAVE_MULTI_MA

  void
  cp_arg_to_pb(corba_data_desc_t &pb, corba_data_desc_t arg_desc);
};  // MasterAgentImpl

class MasterAgentFwdrImpl : public POA_MasterAgentFwdr,
public PortableServer::RefCountServantBase {
public:
  MasterAgentFwdrImpl(Forwarder_ptr fwdr, const char *objName);

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
  getResponse(const corba_response_t &resp);
  virtual CORBA::Long
  ping();

  virtual char *
  getHostname();

  virtual CORBA::Long
  addServices(CORBA::ULong myID, const SeqCorbaProfileDesc_t &services);

  corba_response_t *
  submit(const corba_pb_desc_t &pb_profile, CORBA::ULong maxServers);

  virtual CORBA::Long
  get_session_num();

  virtual char *
  get_data_id();

  virtual CORBA::ULong
  dataLookUp(const char *argID);

  virtual corba_data_desc_t *
  get_data_arg(const char *argID);

  virtual CORBA::Long
  diet_free_pdata(const char *argID);

  virtual SeqCorbaProfileDesc_t *
  getProfiles(CORBA::Long &length);

#ifdef HAVE_MULTI_MA
  virtual CORBA::Boolean
  handShake(const char *name, const char *myName);

  virtual char *
  getBindName();

  virtual void
  searchService(const char *predecessor,
                const char *predecessorId,
                const corba_request_t &request);

  virtual void
  stopFlooding(CORBA::Long reqId, const char *senderId);

  virtual void
  serviceNotFound(CORBA::Long reqId, const char *senderId);

  virtual void
  newFlood(CORBA::Long reqId, const char *senderId);

  virtual void
  floodedArea(CORBA::Long reqId, const char *senderId);

  virtual void
  alreadyContacted(CORBA::Long reqId, const char *senderId);

  virtual void
  serviceFound(CORBA::Long reqId, const corba_response_t &decision);
#endif /* ifdef HAVE_MULTI_MA */
#ifdef HAVE_WORKFLOW
  virtual wf_response_t *
  submit_pb_set(const corba_pb_desc_seq_t &seq_pb);

  virtual response_seq_t *
  submit_pb_seq(const corba_pb_desc_seq_t &pb_seq,
                CORBA::Long reqCount,
                CORBA::Boolean &complete,
                CORBA::Long &firstReqId,
                CORBA::Long &seqReqId);

#endif /* ifdef HAVE_WORKFLOW */
  virtual CORBA::Long
  childRemoveService(CORBA::ULong childID, const corba_profile_desc_t &profile);

  virtual SeqString *
  searchData(const char *request);

  virtual CORBA::Long
  insertData(const char *key, const SeqString &values);

  virtual char *
  getDataManager();

protected:
  Forwarder_ptr forwarder;
  char *objName;
};
#endif  // _MASTERAGENTIMPL_HH_
