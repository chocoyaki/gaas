/****************************************************************************/
/* DIET master agent implementation header                                  */
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
 * Revision 1.24  2011/02/04 15:20:48  hguemar
 * fixes to new configuration parser
 * some cleaning
 *
 * Revision 1.23  2010/11/24 15:18:08  bdepardo
 * searchData is now available on all agents. SeDs are now able to retrieve
 * a DAGDA data from an alias specified by a client.
 * Currently a SeD cannot declare an alias.
 *
 * Revision 1.22  2010/07/12 20:14:32  glemahec
 * DIET 2.5 beta 1 - Forwarders with Multi-MAs bug correction
 *
 * Revision 1.21  2010/07/12 16:14:11  glemahec
 * DIET 2.5 beta 1 - Use the new ORB manager and allow the use of SSH-forwarders for all DIET CORBA objects
 *
 * Revision 1.20  2008/06/25 09:55:56  bisnard
 * removed unused parameter in submit_pb_set & corrected bug with ReqIDs
 *
 * Revision 1.19  2008/05/23 11:55:03  glemahec
 * 64 bits CORBA mapping bug correction.
 *
 * Revision 1.18  2008/04/29 22:22:02  glemahec
 * DAGDA improvements :
 *   - Asynchronous API.
 *   - Data ID alias managing.
 *   - Data manager state backup and restore.
 *
 * Revision 1.17  2008/04/14 13:44:29  bisnard
 * - Parameter 'used' obsoleted in MultiWfScheduler::submit_wf & submit_pb_set
 *
 * Revision 1.16  2008/04/10 09:17:09  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 * Revision 1.15  2007/07/13 10:00:25  ecaron
 * Remove deprecated code (ALTPREDICT part)
 *
 * Revision 1.14  2006/11/16 09:55:54  eboix
 *   DIET_config.h is no longer used. --- Injay2461
 *
 * Revision 1.13  2006/11/06 15:14:53  aamar
 * Workflow support: Correct some code about reqID
 *
 * Revision 1.12  2006/10/20 08:48:59  aamar
 * Remove the submit_wf function.
 * Handle the request ID in workflow submission.
 *
 * Revision 1.11  2006/04/14 14:17:38  aamar
 * Implementing the two methods for workflow support:
 *   - submit_wf (TO REMOVE)
 *   - submit_pb_set.
 *
 * Revision 1.10  2005/09/05 16:06:56  hdail
 * Addition of client hostname and location information to submit call.
 *
 * Revision 1.9  2004/12/16 11:16:44  sdahan
 * adds multi-mas informations into the logService
 *
 * Revision 1.8  2004/12/15 15:57:08  sdahan
 * rewrite the FloodRequestsList to use a simplest implementation. The previous mutex bugs does not exist anymore.
 *
 * Revision 1.7  2004/10/06 16:40:24  rbolze
 * implement function to return the Profiles avialable on platform when a client ask it
 *
 * Revision 1.6  2004/10/05 07:45:34  hdail
 * Grouped private and public members together.
 *
 * Revision 1.5  2004/10/04 09:40:43  sdahan
 * warning fix :
 *  - debug.cc : change the printf format from %ul to %lu and from %l to %ld
 *  - ReferenceUpdateThread and BindService : The omniORB documentation said that
 *    it's better to create private destructor for the thread subclasses. But
 *    private destructors generate warning, so I set the destructors public.
 *  - CORBA.h and DIET_config.h define the same macros. So I include the CORBA.h
 *    before the DIET_config.h to avoid to define two times the same macros.
 *  - remove the deprecated warning when including iostream.h and set.h
 *
 * Revision 1.4  2004/09/29 13:35:31  sdahan
 * Add the Multi-MAs feature.
 *
 * Revision 1.3  2004/02/27 10:25:11  bdelfabr
 * methods for data id creation and  methods to retrieve data descriptor are added
 *
 * Revision 1.2  2003/05/10 08:53:34  pcombes
 * New format for configuration files, new Parsers.
 *
 * Revision 1.1  2003/04/10 13:00:55  pcombes
 * Replace MasterAgent_impl.hh. Apply CS. Update submit profile.
 ****************************************************************************/

#ifndef _MASTERAGENTIMPL_HH_
#define _MASTERAGENTIMPL_HH_

#include <boost/uuid/uuid_generators.hpp>
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
  };

  /** Launch this agent (initialization + registration in the hierarchy). */
  int
  run();

  /** Problem submission : remotely called by client. */
  corba_response_t*
  submit(const corba_pb_desc_t& pb_profile, CORBA::ULong maxServers);

  /** Problem submission. Looking for SeDs that can resolve the
      problem in the local domain. */
  corba_response_t*
  submit_local(const corba_request_t& creq);

  /** get session id */
  virtual  CORBA::Long
  get_session_num();

  /** returns id of a data */
  virtual  char *
  get_data_id();

  /** returns the List of Profile available */
  /*  virtual  SeqCorbaProfileDesc_t*
      getProfiles();*/

  virtual  SeqCorbaProfileDesc_t*
  getProfiles(CORBA::Long& length);

  /** returns true if data present, FALSE elsewhere */
  virtual CORBA::ULong
  dataLookUp(const char* argID);

  /** return the descriptor the the data */
  corba_data_desc_t*
  get_data_arg(const char* argID);

  /** removes data from the list */
  virtual CORBA::Long
  diet_free_pdata(const char *argID);

#ifdef HAVE_MULTI_MA
  /** Ask the authorization to create a link with this Master Agent */
  virtual CORBA::Boolean
  handShake(const char* name, const char* myName);
  /** returns the address of the bind service of the Master Agent */
  virtual char*
  getBindName();
  /** Updates all the references to the other Master Agent. It also
      connect to some new Master Agent if there is not enough links. */
  void
  updateRefs();
  virtual void
  searchService(const char* predecessor,
                const char* predecessorId,
                const corba_request_t& request);

  virtual void
  stopFlooding(CORBA::Long reqId, const char* senderId);

  virtual void
  serviceNotFound(CORBA::Long reqId, const char* senderId);

  virtual void
  newFlood(CORBA::Long reqId, const char* senderId);

  virtual void
  floodedArea(CORBA::Long reqId, const char* senderId);

  virtual void
  alreadyContacted(CORBA::Long reqId, const char* senderId);

  virtual void
  serviceFound(CORBA::Long reqId, const corba_response_t& decision);

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
  submit_pb_set(const corba_pb_desc_seq_t& seq_pb);

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
  virtual response_seq_t*
  submit_pb_seq(const corba_pb_desc_seq_t& pb_seq,
                CORBA::Long reqCount,
                CORBA::Boolean& complete,
                CORBA::Long& firstReqId,
                CORBA::Long& seqReqId);

#endif  // HAVE_WORKFLOW
  virtual SeqString*
  searchData(const char* request);

  virtual CORBA::Long
  insertData(const char* key, const SeqString& values);

private:
  DagdaCatalog* catalog;

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
  BindService* bindSrv;
  CORBA::String_var bindName;
  FloodRequestsList* floodRequestsList;
  typedef ts_set<CORBA::Long> ReqIdList;
  ReqIdList reqIdList;
  void
  logNeighbors();
#endif  // HAVE_MULTI_MA
  boost::uuids::random_generator uuid_rg;

  void
  cp_arg_to_pb(corba_data_desc_t& pb, corba_data_desc_t arg_desc);
};  // MasterAgentImpl

class MasterAgentFwdrImpl : public POA_MasterAgentFwdr,
                            public PortableServer::RefCountServantBase {
public:
  MasterAgentFwdrImpl(Forwarder_ptr fwdr, const char* objName);

  virtual CORBA::Long
  agentSubscribe(const char* me, const char* hostName,
                 const SeqCorbaProfileDesc_t& services);
  virtual CORBA::Long
  serverSubscribe(const char* me, const char* hostName,
                  const SeqCorbaProfileDesc_t& services);

  virtual CORBA::Long
  childUnsubscribe(CORBA::ULong childID,
                   const SeqCorbaProfileDesc_t& services);

  virtual CORBA::Long
  removeElement(bool recursive);

  CORBA::Long
  bindParent(const char * parentName);

  CORBA::Long
  disconnect();

  virtual void
  getResponse(const corba_response_t& resp);
  virtual CORBA::Long
  ping();

  virtual char*
  getHostname();

  virtual CORBA::Long
  addServices(CORBA::ULong myID, const SeqCorbaProfileDesc_t& services);

  corba_response_t*
  submit(const corba_pb_desc_t& pb_profile, CORBA::ULong maxServers);

  virtual CORBA::Long
  get_session_num();

  virtual char *
  get_data_id();

  virtual CORBA::ULong
  dataLookUp(const char* argID);

  virtual corba_data_desc_t*
  get_data_arg(const char* argID);

  virtual CORBA::Long
  diet_free_pdata(const char *argID);

  virtual SeqCorbaProfileDesc_t*
  getProfiles(CORBA::Long& length);

#ifdef HAVE_MULTI_MA
  virtual CORBA::Boolean
  handShake(const char* name, const char* myName);

  virtual char*
  getBindName();

  virtual void
  searchService(const char* predecessor,
                const char* predecessorId,
                const corba_request_t& request);

  virtual void
  stopFlooding(CORBA::Long reqId, const char* senderId);

  virtual void
  serviceNotFound(CORBA::Long reqId, const char* senderId);

  virtual void
  newFlood(CORBA::Long reqId, const char* senderId);

  virtual void
  floodedArea(CORBA::Long reqId, const char* senderId);

  virtual void
  alreadyContacted(CORBA::Long reqId, const char* senderId);

  virtual void
  serviceFound(CORBA::Long reqId, const corba_response_t& decision);
#endif
#ifdef HAVE_WORKFLOW
  virtual wf_response_t *
  submit_pb_set(const corba_pb_desc_seq_t& seq_pb);

  virtual response_seq_t*
  submit_pb_seq(const corba_pb_desc_seq_t& pb_seq,
                CORBA::Long reqCount,
                CORBA::Boolean& complete,
                CORBA::Long& firstReqId,
                CORBA::Long& seqReqId);

#endif
  virtual CORBA::Long
  childRemoveService(CORBA::ULong childID, const corba_profile_desc_t& profile);

  virtual SeqString*
  searchData(const char* request);

  virtual CORBA::Long
  insertData(const char* key, const SeqString& values);

  virtual char*
  getDataManager();

protected:
  Forwarder_ptr forwarder;
  char* objName;
};
#endif  // _MASTERAGENTIMPL_HH_
