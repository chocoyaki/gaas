/**
 * @file AgentImpl.hh
 *
 * @brief  DIET agent implementation source code
 *
 * @author
 *
 * @section Licence
 *   |LICENCE|
 */



#ifndef _AGENTIMPL_HH_
#define _AGENTIMPL_HH_

#include "DIET_data.h"
#include "Agent.hh"
#include "LocalAgent.hh"

#include "Counter.hh"
#include "ChildID.hh"
#include "DIET_data_internal.hh"
#include "DagdaImpl.hh"
#include "NodeDescription.hh"
#include "RequestID.hh"
#include "Request.hh"
#include "ServiceTable.hh"
#include "ts_container/ts_vector.hh"
#include "ts_container/ts_map.hh"

#ifdef USE_LOG_SERVICE
#include "DietLogComponent.hh"
#endif


// Forwarder part
#include "Forwarder.hh"
#include "AgentFwdr.hh"

class AgentImpl : public POA_Agent,
public PortableServer::RefCountServantBase {
public:

  /**************************************************************************/
  /* Public methods                                                         */
  /**************************************************************************/

  AgentImpl();
  virtual
  ~AgentImpl();

  /**
   * Launch this agent (initialization + registration in the hierarchy).
   */
  virtual int
  run();

  void
  setDataManager(Dagda_ptr dataManager);

  /*DagdaImpl* */
  char *
  getDataManager();

  virtual SeqString *
  searchData(const char *request) = 0;

  /**
   * Set the DietLogManager. If the dietLogManager is NULL, no
   * monitoring information will be sent.
   */
#ifdef USE_LOG_SERVICE
  void
  setDietLogComponent(DietLogComponent *dietLogComponent);
#endif

  /** Subscribe an agent as a LA child. Remotely called by an LA. */
  virtual CORBA::Long
  agentSubscribe(const char *me, const char *hostName,
                 const SeqCorbaProfileDesc_t &services);

  /** Subscribe a server as a SeD child. Remotely called by an SeD. */
  virtual CORBA::Long
  serverSubscribe(const char *me, const char *hostName,
                  const SeqCorbaProfileDesc_t &services);

  /** Unsubscribe a child. Remotely called by an SeD. */
  virtual CORBA::Long
  childUnsubscribe(CORBA::ULong childID,
                   const SeqCorbaProfileDesc_t &services);

  /** Sends a request to the agent to commit suicide.
   * If "recursive" is true, then the agent forwards the request
   * to its children.
   */
  virtual CORBA::Long
  removeElement(bool recursive);
  void
  removeElementChildren(bool recursive);
  virtual void
  removeElementClean(bool recursive);

  /** Add services into the service table, and attach them to child me.*/
  virtual CORBA::Long
  addServices(CORBA::ULong myID, const SeqCorbaProfileDesc_t &services);

  /** Remove services into the service table for a given child */
  virtual CORBA::Long
  childRemoveService(CORBA::ULong childID,
                     const corba_profile_desc_t &profile);

  /** Get the response of a child */
  virtual void
  getResponse(const corba_response_t &resp);

  /** Used to test if this agent is alive. */
  virtual CORBA::Long
  ping();

  /** returns the hostname of the agent */
  virtual char *
  getHostname();

protected:
  /** Local host name */
  char *localHostName;

  /** Identity in the CORBA Naming Service */
  char *myName;
  /** ID of next subscribing child */
  Counter childIDCounter;

  /** Number of children of this agent that are local agents */
  Counter nbLAChildren;
  typedef NodeDescription<LocalAgent, LocalAgent_ptr> LAChild;
  /** List of the LA children of this agent */
  ts_vector<LAChild> LAChildren;

  /** Number of children of this agent that are servers */
  Counter nbSeDChildren;
  typedef NodeDescription<SeD, SeD_ptr> SeDChild;
  /** List of the SeD children of this agent */
  ts_vector<SeDChild> SeDChildren;

  /** Table of all services that are offered in the sub-tree which has this
   *  agent for root */
  ServiceTable *SrvT;
  /** Mutex for accesses to SrvT */
  omni_mutex srvTMutex;

  /** All requests beeing processed */
  ts_map<RequestID, Request *> reqList;

  Dagda_ptr dataManager;

  /**************************************************************************/
  /* Private methods                                                        */
  /**************************************************************************/

  /**
   * Forward a request, schedule and merge the responses associated.
   * @param req     the request.
   * @param max_srv the maximum number of servers to sort.
   */
  corba_response_t *
  findServer(Request *req, size_t max_srv);

#if not defined HAVE_ALT_BATCH
  /** Send the request structure \c req to the child whose ID is \c childID. */
  void
  sendRequest(CORBA::ULong childID, const corba_request_t *req);
#else
  /**
   * Send the request structure \c req to the child whose ID is \c childID.
   * Decremente \c nb_children_contacted contacted when error.
   * Use \c serviceRef to request the correct // or seq profile flag.
   */
  void
  sendRequest(CORBA::ULong *childID,
              size_t numero_child,
              const corba_request_t *req,
              int *nb_children_contacted,
              CORBA::ULong frontier);
#endif /* if not defined HAVE_ALT_BATCH */

  /**
   * Get communication time between this agent and the child \c childID
   * for a data amount of size \c size. The way of the data transfer can be
   * specified with \c to : if (to), from this agent to the child, else from
   * the child to this agent.
   */
  inline double
  getCommTime(CORBA::Long childID, unsigned long size, bool to = true);

  /**
   * Return a pointer to a unique aggregated response from various responses.
   * @param request contains pointers to the scheduler and the responses.
   * @param max_srv the maximum number of servers to aggregate (all if 0).
   */
  corba_response_t *
  aggregate(Request *request, size_t max_srv);

  /** Get host name of a child (returned string is ms_stralloc'd). */
  char *
  getChildHostName(CORBA::Long childID);

#ifdef USE_LOG_SERVICE
  /**
   * Ptr to the DietLogComponent. This ptr can be NULL, so it has to
   * be checked every time it is used. If it is NULL, no monitoring
   * messages have to be sent.
   */
  DietLogComponent *dietLogComponent;
#endif
};

class AgentFwdrImpl : public POA_AgentFwdr,
public PortableServer::RefCountServantBase {
public:
  AgentFwdrImpl(Forwarder_ptr fwdr, const char *objName);

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

  virtual CORBA::Long
  childRemoveService(CORBA::ULong childID,
                     const corba_profile_desc_t &profile);

  virtual char *
  getDataManager();

  SeqString *
  searchData(const char *request);

  virtual CORBA::Long
  addServices(CORBA::ULong myID, const SeqCorbaProfileDesc_t &services);

  virtual void
  getResponse(const corba_response_t &resp);

  virtual CORBA::Long
  ping();

  virtual char *
  getHostname();

protected:
  Forwarder_ptr forwarder;
  char *objName;
};
#endif  // _AGENTIMPL_HH_
