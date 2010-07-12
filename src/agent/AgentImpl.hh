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
 * Revision 1.27  2010/07/12 16:14:11  glemahec
 * DIET 2.5 beta 1 - Use the new ORB manager and allow the use of SSH-forwarders for all DIET CORBA objects
 *
 * Revision 1.26  2009/11/30 17:57:47  bdepardo
 * New methods to remove the agent in a cleaner way when killing it.
 *
 * Revision 1.25  2009/10/26 09:18:57  bdepardo
 * Added method for dynamic hierarchy management:
 * - childUnsubscribe(...)
 * - removeElement(bool recursive)
 * Renamed serverRemoveService(...) into childRemoveService(...)
 *
 * Revision 1.24  2008/11/18 10:15:22  bdepardo
 * - Added the possibility to dynamically create and destroy a service
 *   (even if the SeD is already started). An example is available.
 *   This feature only works with DAGDA.
 * - Added policy commands for CMake 2.6
 * - Added the name of the service in the profile. It was only present in
 *   the profile description, but not in the profile. Currently, the name is
 *   copied in each solve function, but this should certainly be moved
 *   somewhere else.
 *
 * Revision 1.23  2008/07/16 00:45:09  ecaron
 * Remove HAVE_ALTPREDICT (deprecated code)
 *
 * Revision 1.22  2008/04/28 07:08:30  glemahec
 * The DAGDA API.
 *
 * Revision 1.21  2008/04/19 09:16:45  ycaniou
 * Check that pathToTmp and pathToNFS exist
 * Check and eventually correct if pathToTmp or pathToNFS finish or not by '/'
 * Rewrite of the propagation of the request concerning job parallel_flag
 * Rewrite (and addition) of the propagation of the response concerning:
 *   job parallel_flag and serverType (batch or serial for the moment)
 * Complete debug info with batch stuff
 *
 * Revision 1.20  2008/04/07 15:33:43  ycaniou
 * This should remove all HAVE_BATCH occurences (still appears in the doc, which
 *   must be updated.. soon :)
 * Add the definition of DIET_BATCH_JOBID wariable in batch scripts
 *
 * Revision 1.19  2008/01/14 09:35:48  glemahec
 * AgentImpl.cc/hh modifications to allow the use of DAGDA.
 *
 * Revision 1.18  2007/04/16 22:43:43  ycaniou
 * Make all necessary changes to have the new option HAVE_ALT_BATCH operational.
 * This is indented to replace HAVE_BATCH.
 *
 * First draw to manage batch systems with a new Cori plug-in.
 *
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
#if ! HAVE_DAGDA
#include "LocMgrImpl.hh"
#else
#include "DagdaImpl.hh"
#endif
#include "NodeDescription.hh"
#include "RequestID.hh"
#include "Request.hh"
#include "ServiceTable.hh"
#include "ts_container/ts_vector.hh"
#include "ts_container/ts_map.hh"
#include "DietLogComponent.hh"

// Forwarder part
#include "Forwarder.hh"
#include "AgentFwdr.hh"

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
	
  // Dagda doesn't use the DTM Location Manager.
#if ! HAVE_DAGDA
  /** Set this->locMgr */
  int
  linkToLocMgr(LocMgrImpl* locMgr);
#else
  void
  setDataManager(Dagda_ptr dataManager);
	
  /*DagdaImpl* */
  char* getDataManager();
#endif // ! HAVE_DAGDA
	
  /**
   * Set the DietLogManager. If the dietLogManager is NULL, no
   * monitoring information will be sent.
   */
  void
  setDietLogComponent(DietLogComponent* dietLogComponent);
	
  /** Subscribe an agent as a LA child. Remotely called by an LA. */
  virtual CORBA::Long
  agentSubscribe(const char* me, const char* hostName,
								 const SeqCorbaProfileDesc_t& services);
	
  /** Subscribe a server as a SeD child. Remotely called by an SeD. */
  virtual CORBA::Long
  serverSubscribe(const char* me, const char* hostName,
#if HAVE_JXTA
									const char* uuid,
#endif // HAVE_JXTA
									const SeqCorbaProfileDesc_t& services);
	
#ifdef HAVE_DYNAMICS
  /** Unsubscribe a child. Remotely called by an SeD. */
  virtual CORBA::Long
  childUnsubscribe(CORBA::ULong childID,
									 const SeqCorbaProfileDesc_t& services);
	
  /** Sends a request to the agent to commit suicide.
   * If "recursive" is true, then the agent forwards the request
   * to its children.
   */
  virtual CORBA::Long
  removeElement(bool recursive);
  void removeElementChildren(bool recursive);
  void removeElementClean(bool recursive);
#endif // HAVE_DYNAMICS
	
	
	
  /** Add services into the service table, and attach them to child me.*/
  virtual CORBA::Long
  addServices(CORBA::ULong myID, const SeqCorbaProfileDesc_t& services);
	
//#ifdef HAVE_DAGDA
  /** Remove services into the service table for a given child */
  virtual CORBA::Long
  childRemoveService(CORBA::ULong childID, const corba_profile_desc_t& profile);
//#endif
	
	
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
  // Dagda doesn't require this.
#if ! HAVE_DAGDA
  LocMgrImpl* locMgr;
#else
  Dagda_ptr dataManager;
#endif // ! HAVE_DAGDA
	
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
	
#if not defined HAVE_ALT_BATCH
  /** Send the request structure \c req to the child whose ID is \c childID. */
  void
  sendRequest(CORBA::ULong childID, const corba_request_t* req);
#else
  /**
   * Send the request structure \c req to the child whose ID is \c childID.
   * Decremente \c nb_children_contacted contacted when error.
   * Use \c serviceRef to request the correct // or seq profile flag.
   */
  void
  sendRequest(CORBA::ULong * childID, 
							size_t numero_child,
							const corba_request_t * req,
							int * nb_children_contacted,
							CORBA::ULong frontier) ;
#endif
	
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
	
  /**
   * Ptr to the DietLogComponent. This ptr can be NULL, so it has to
   * be checked every time it is used. If it is NULL, no monitoring 
   * messages have to be sent.
   */
  DietLogComponent* dietLogComponent;
};

class AgentFwdrImpl : public POA_AgentFwdr,
  public PortableServer::RefCountServantBase
{
protected:
	Forwarder_ptr forwarder;
	char* objName;
public:
	AgentFwdrImpl(Forwarder_ptr fwdr, const char* objName);
	
	virtual CORBA::Long
  agentSubscribe(const char* me, const char* hostName,
								 const SeqCorbaProfileDesc_t& services);
	virtual CORBA::Long
  serverSubscribe(const char* me, const char* hostName,
#if HAVE_JXTA
								  const char* uuid,
#endif // HAVE_JXTA
									const SeqCorbaProfileDesc_t& services);
#ifdef HAVE_DYNAMICS
  virtual CORBA::Long
  childUnsubscribe(CORBA::ULong childID,
									 const SeqCorbaProfileDesc_t& services);
	
  virtual CORBA::Long removeElement(bool recursive);
#endif // HAVE_DYNAMICS
  virtual CORBA::Long
  childRemoveService(CORBA::ULong childID, const corba_profile_desc_t& profile);
#ifdef HAVE_DAGDA
	virtual char* getDataManager();
#endif
	virtual CORBA::Long
  addServices(CORBA::ULong myID, const SeqCorbaProfileDesc_t& services);
	
  virtual void getResponse(const corba_response_t& resp);
	virtual CORBA::Long ping();
	
  virtual char* getHostname();
	
};
#endif // _AGENTIMPL_HH_
