/****************************************************************************/
/* DIET agent implementation header                                         */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*    - Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)                   */
/*    - Frederic LOMBARD (Frederic.Lombard@lifc.univ-fcomte.fr)             */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.4  2003/09/18 09:47:19  bdelfabr
 * adding data persistence
 *
 * Revision 1.3  2003/08/01 19:33:11  pcombes
 * Use FASTMgr.
 *
 * Revision 1.2  2003/05/10 08:53:34  pcombes
 * New format for configuration files, new Parsers.
 *
 * Revision 1.1  2003/04/10 12:59:34  pcombes
 * Replace agent_impl.hh. Apply CS. Use ChildID and NodeDescription.
 ****************************************************************************/


#ifndef _AGENTIMPL_HH_
#define _AGENTIMPL_HH_

#include "DIET_data.h"
#include "Agent.hh"
#include "LocalAgent.hh"

#include "Counter.hh"
#include "ChildID.hh"
#include "dietTypes.hh"
#include "NodeDescription.hh"
#include "RequestID.hh"
#include "Request.hh"
#include "ServiceTable.hh"
#include "ts_container/ts_vector.hh"
#include "ts_container/ts_map.hh"
#include "locMgr.hh"
#include "common_types.hh"

class locMgrImpl;

class AgentImpl : public POA_Agent,
		  public PortableServer::RefCountServantBase
{
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

  /** Subscribe an agent as a LA child. Remotely called by an LA. */
  virtual CORBA::ULong
  agentSubscribe(Agent_ptr me, const char* hostName,
		 const SeqCorbaProfileDesc_t& services);
  /** Subscribe a server as a SeD child. Remotely called by an SeD. */
  virtual CORBA::ULong
  serverSubscribe(SeD_ptr me, const char* hostName,
		  const SeqCorbaProfileDesc_t& services);
  /** Add \c services into the service table, and attach them to child \c me.*/
  virtual void
  addServices(CORBA::ULong myID, const SeqCorbaProfileDesc_t& services);

  /** Get the response of a child */
  virtual void
  getResponse(const corba_response_t& resp);


  /** Used to test if this agent is alive. */
  virtual CORBA::Long
  ping();

  /* loc management */
  virtual char *
  getMyName();
  
  virtual char *
  getMyFatherName();
  void 
  setMyName(locMgrImpl *_loc);

  /** Get pointer to the read-only name of this agent. */
  inline const char*
  getName() {return (const char*)this->myName;};
  
protected:

  /**************************************************************************/
  /* Private fields                                                         */
  /**************************************************************************/

  locMgrImpl *myLoc;
  /** Local host name */
  char localHostName[257];

  /** Identity in the CORBA Naming Service */
  char myName[257];
  /* loc management */
  char fatherName[257];
  /** ID of this agent amongst the children of its parent */
  ChildID childID;
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
  ServiceTable* SrvT;
  /** Mutex for accesses to SrvT */
  omni_mutex srvTMutex;

  /** All requests beeing processed */ 
  ts_map<RequestID, Request*> reqList;
 

  /**************************************************************************/
  /* Private methods                                                        */
  /**************************************************************************/
  
  /**
   * Forward a request, schedule and merge the responses associated.
   * @param req     the request.
   * @param max_srv the maximum number of servers to sort.
   */
  corba_response_t*
  findServer(Request* req, size_t max_srv);
 
  /** Send the request structure \c req to the child whose ID is \c childID. */
  void
  sendRequest(CORBA::ULong childID, const corba_request_t* req);
   
  /**
   * Get communication time between this agent and the child \c childID for a data
   * amount of size \c size. The way of the data transfer can be specified with
   * \c to : if (to), from this agent to the child, else from the child to this
   * agent.
   */
  inline double
  getCommTime(CORBA::Long childID, unsigned long size, bool to = true);

  /**
   * Return a pointer to a unique aggregated response from various responses.
   * @param request contains pointers to the scheduler and the responses.
   * @param max_srv the maximum number of servers to aggregate (all if 0).
   */
  corba_response_t*
  aggregate(Request* request, size_t max_srv);

  /** Get host name of a child (returned string is ms_stralloc'd). */
  char*
  getChildHostName(CORBA::Long childID);
};

#endif // _AGENTIMPL_HH_

