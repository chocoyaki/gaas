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
 * Revision 1.1  2003/04/10 12:59:34  pcombes
 * Replace agent_impl.hh. Apply CS. Use ChildID and NodeDescription.
 *
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


class AgentImpl : public POA_Agent,
		  public PortableServer::RefCountServantBase
{
public:

  /**************************************************************************/
  /* Public fields                                                          */
  /**************************************************************************/


  
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
  run(char* configFileName, char* parentName = NULL);

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

  //virtual void
  //createData(CORBA::Long dataId, CORBA::Long sonId);

  /** Used to test if this agent is alive. */
  virtual CORBA::Long
  ping();

  
protected:

  /**************************************************************************/
  /* Private fields                                                         */
  /**************************************************************************/

  /** Local host name */
  char localHostName[257];

  /** Identity in the CORBA Naming Service */
  char myName[257];

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
 
#if HAVE_FAST
  /** Mutex for FAST calls (FAST is not reentrant yet) */
  omni_mutex fastMutex;
#endif // HAVE_FAST
  
  /* LDAP and NWS parameters for FAST
     They are not conditioned by HAVE_FAST, because they can be initialized
     in the configuration file. */
  int  ldapUse;
  char ldapHost[257];
  int  ldapPort;
  char ldapMask[257];
  int  nwsUse;
  char nwsNSHost[257];
  int  nwsNSPort;
  char nwsForecasterHost[257];
  int  nwsForecasterPort;


  /**************************************************************************/
  /* Private methods                                                        */
  /**************************************************************************/

  /**
   * Parse the configration file \c configFileName to fill in this agent fields.
   * This method is to be implemented by Master and Local Agent classes.
   */
  virtual int
  parseConfigFile(char* configFileName, char* parentName = NULL) = 0;
  
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

