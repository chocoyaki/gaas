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
 * Revision 1.17  2006/09/11 11:09:12  ycaniou
 * Call ServiceTable::getChildren(corba_pb_desc) in findServer, in order to
 *   call both parallel and sequential server for a default request that can
 *   possibly be executed in both modes.
 *
 * Revision 1.16  2005/09/07 07:41:02  hdail
 * Cleanup of alternative prediction handling
 *
 * Revision 1.15  2005/09/05 16:06:09  hdail
 * - Addition of method for aggregating parameter location data.
 * - Handling of alternative performance prediction for parameter transfer
 *   times by agents. (experimental and protected by HAVE_ALTPREDICT)
 *
 * Revision 1.14  2005/05/18 14:18:10  mjan
 * Initial adding of JuxMem support inside DIET. dmat_manips examples tested without JuxMem and with JuxMem
 *
 * Revision 1.13  2005/05/15 15:49:40  alsu
 * to indicate sucess/failure, addServices not returns a value
 *
 * Revision 1.12  2005/04/13 08:49:11  hdail
 * Beginning of adoption of new persistency model: DTM is enabled by default and
 * JuxMem will be supported via configure flags.  DIET will always provide at
 * least one type of persistency.  As a first step, persistency across DTM and
 * JuxMem is not supported so all persistency handling should be surrounded by
 *     #if HAVE_JUXMEM
 *         // JuxMem code
 *     #else
 *         // DTM code
 *     #endif
 * This check-in prepares for the JuxMem check-in by cleaning up old
 * DEVELOPPING_DATA_PERSISTENCY flags and surrounding DTM code with
 * #if ! HAVE_JUXMEM / #endif flags to be replaced by above format by Mathieu's
 * check-in.  Currently the HAVE_JUXMEM flag is set in AgentImpl.hh - to be
 * replaced by Mathieu's check-in of a configure system for JuxMem.
 *
 * Revision 1.11  2005/04/08 13:02:43  hdail
 * The code for LogCentral has proven itself stable and it seems bug free.
 * Since no external libraries are required to compile in LogCentral, its now
 * going to be compiled in by default always ... its usage is easily controlled by
 * configuration file.
 *
 * Revision 1.10  2004/09/29 13:35:31  sdahan
 * Add the Multi-MAs feature.
 *
 * Revision 1.9  2004/09/13 14:11:06  hdail
 * Cleaned up memory management for local variables localHostName and myName.
 *
 * Revision 1.8  2004/06/11 15:45:39  ctedesch
 * add DIET/JXTA
 *
 * Revision 1.7  2004/03/01 18:43:57  rbolze
 * add logservice
 ****************************************************************************/


#ifndef _AGENTIMPL_HH_
#define _AGENTIMPL_HH_

#include "DIET_data.h"
#include "Agent.hh"
#include "LocalAgent.hh"

#include "Counter.hh"
#include "ChildID.hh"
#include "DIET_data_internal.hh"
#include "LocMgrImpl.hh"
#include "NodeDescription.hh"
#include "RequestID.hh"
#include "Request.hh"
#include "ServiceTable.hh"
#include "ts_container/ts_vector.hh"
#include "ts_container/ts_map.hh"
#include "DietLogComponent.hh"

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

  /** Set this->locMgr */
  int
  linkToLocMgr(LocMgrImpl* locMgr);

  /**
   * Set the DietLogManager. If the dietLogManager is NULL, no
   * monitoring information will be sent.
   */
  void
  setDietLogComponent(DietLogComponent* dietLogComponent);

  /** Subscribe an agent as a LA child. Remotely called by an LA. */
  virtual CORBA::Long
  agentSubscribe(Agent_ptr me, const char* hostName,
		 const SeqCorbaProfileDesc_t& services);

  /** Subscribe a server as a SeD child. Remotely called by an SeD. */
  virtual CORBA::Long
  serverSubscribe(SeD_ptr me, const char* hostName,
#if HAVE_JXTA
		  const char* uuid,
#endif // HAVE_JXTA
		  const SeqCorbaProfileDesc_t& services);

  /** Add services into the service table, and attach them to child me.*/
  virtual CORBA::Long
  addServices(CORBA::ULong myID, const SeqCorbaProfileDesc_t& services);

  /** Get the response of a child */
  virtual void
  getResponse(const corba_response_t& resp);


  /** Used to test if this agent is alive. */
  virtual CORBA::Long
  ping();

  /** returns the hostname of the agent */
  virtual char*
  getHostname() ;


protected:

  /**************************************************************************/
  /* Private fields                                                         */
  /**************************************************************************/

  /** Local host name */
  char* localHostName;

  /** Identity in the CORBA Naming Service */
  char* myName;
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
 
  /** Data Location Manager associated to this agent */
  LocMgrImpl* locMgr;


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
 
#ifndef HAVE_BATCH
  /** Send the request structure \c req to the child whose ID is \c childID. */
  void
  sendRequest(CORBA::ULong childID, const corba_request_t* req);
#else
  /**
   * Send the request structure \c req to the child whose ID is \c childID.
   * Decremente \c nb_children_contacted contacted when error.
   */
  void
  sendRequest(CORBA::ULong childID, const corba_request_t* req,
	      int * nb_children_contacted) ;
#endif


#if ! HAVE_ALTPREDICT
  /**
   * Get communication time between this agent and the child \c childID for a data
   * amount of size \c size. The way of the data transfer can be specified with
   * \c to : if (to), from this agent to the child, else from the child to this
   * agent.
   */
  inline double
  getCommTime(CORBA::Long childID, unsigned long size, bool to = true);
#endif // ! HAVE_ALTPREDICT

  /**
   * Return a pointer to a unique aggregated response from various responses.
   * @param request contains pointers to the scheduler and the responses.
   * @param max_srv the maximum number of servers to aggregate (all if 0).
   */
  corba_response_t*
  aggregate(Request* request, size_t max_srv);

#if HAVE_ALTPREDICT
  /**
   * Return a pointer to a new response containing updated information on
   * data locality.  Changes the request by changing performance predictions
   * for data transfer costs.
   * @param request contains pointers to the scheduler and the responses.
   * @param max_srv the maximum number of servers to aggregate (all if 0).
   */
  corba_response_t*
  aggregateLocationInfo(Request* request);
#endif // HAVE_ALTPREDICT

  /** Get host name of a child (returned string is ms_stralloc'd). */
  char*
  getChildHostName(CORBA::Long childID);

  /**
   * Ptr to the DietLogComponent. This ptr can be NULL, so it has to
   * be checked every time it is used. If it is NULL, no monitoring 
   * messages have to be sent.
   */
  DietLogComponent* dietLogComponent;
};

#endif // _AGENTIMPL_HH_
