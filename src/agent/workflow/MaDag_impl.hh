/****************************************************************************/
/* The MA DAG CORBA object implementation header                            */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/* - Benjamin Isnard (Benjamin.Isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/

/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.25  2010/12/17 09:48:00  kcoulomb
 * * Set diet to use the new log with forwarders
 * * Fix a CoRI problem
 * * Add library version remove DTM flag from ccmake because deprecated
 *
 * Revision 1.24  2010/07/12 16:14:11  glemahec
 * DIET 2.5 beta 1 - Use the new ORB manager and allow the use of SSH-forwarders for all DIET CORBA objects
 *
 * Revision 1.23  2009/09/25 12:42:09  bisnard
 * - use new DagNodeLauncher classes to manage threads
 * - added dag cancellation method
 *
 * Revision 1.22  2009/02/06 14:50:35  bisnard
 * setup exceptions
 *
 * Revision 1.21  2009/01/16 13:41:22  bisnard
 * added common base class DagScheduler to simplify dag events handling
 * improved exception management
 *
 * Revision 1.20  2008/12/09 12:09:00  bisnard
 * added parameters to dag submit method to handle inter-dependent dags
 *
 * Revision 1.19  2008/12/02 10:21:03  bisnard
 * use MetaDags to handle multi-dag submission and execution
 *
 * Revision 1.18  2008/10/14 13:23:01  bisnard
 * - use dagId instead of wfReqId as key for dags
 * - new mapping table dagId to wfReqId
 *
 * Revision 1.17  2008/09/04 15:22:25  bisnard
 * Changed name of multiwf heuristic HEFT to GHEFT
 *
 * Revision 1.16  2008/09/04 14:33:55  bisnard
 * - New option for MaDag to select platform type (servers
 * with same service list or not)
 * - Optimization of the multiwfscheduler to avoid requests to
 * MA for server availability
 *
 * Revision 1.15  2008/07/24 21:08:11  rbolze
 * New multi-wf heuristic FCFS (First Come First Serve)
 *
 * Revision 1.14  2008/07/17 13:33:09  bisnard
 * New multi-wf heuristic SRPT
 *
 * Revision 1.13  2008/07/08 15:52:03  bisnard
 * Set interRoundDelay as parameter of workflow scheduler
 *
 * Revision 1.12  2008/06/19 10:18:54  bisnard
 * new heuristic AgingHEFT for multi-workflow scheduling
 *
 * Revision 1.11  2008/06/03 12:19:36  bisnard
 * Method to get MA ref
 *
 * Revision 1.10  2008/06/01 09:22:14  rbolze
 * add getter to retrieve the dietLogComponent
 *
 * Revision 1.9  2008/05/31 08:45:55  rbolze
 * add DietLogComponent to the maDagAgent
 *
 * Revision 1.8  2008/05/30 14:16:25  bisnard
 * obsolete MultiDag (not used anymore for multi-wf)
 *
 * Revision 1.7  2008/05/16 12:30:20  bisnard
 * MaDag returns dagID to client after dag submission
 * (used for node execution)
 *
 * Revision 1.6  2008/04/30 07:37:01  bisnard
 * use relative timestamps for estimated and real completion time
 * make MultiWfScheduler abstract and add HEFT MultiWf scheduler
 *
 * Revision 1.5  2008/04/28 11:56:51  bisnard
 * choose wf scheduler type when creating madag
 *
 * Revision 1.4  2008/04/21 14:31:45  bisnard
 * moved common multiwf routines from derived classes to MultiWfScheduler
 * use wf request identifer instead of dagid to reference client
 * use nodeQueue to manage multiwf scheduling
 * renamed WfParser as DagWfParser
 *
 * Revision 1.3  2008/04/14 13:45:10  bisnard
 * - Removed wf mono-mode submit
 * - Renamed submit_wf in processDagWf
 *
 * Revision 1.2  2008/04/14 09:14:29  bisnard
 *  - Workflow rescheduling (CltReoMan) no longer used with MaDag v2
 *  - AbstractWfSched and derived classes no longer used with MaDag v2
 *
 * Revision 1.1  2008/04/10 09:13:29  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 * Revision 1.6  2008/03/21 10:22:04  rbolze
 *  - add ping() function to the MaDag in order to be able test this component.
 * this is use by goDIET.
 *  - print IOR in stdout in the constructor of the MaDag.
 *
 * Revision 1.5b ??? aamar (added by bisnard)
 * Added functions submit_dag and submit_dag_in_multi
 *
 * Revision 1.5  2006/11/27 09:53:00  aamar
 * Correct headers of source files used in workflow support.
 *
 * Revision 1.4  2006/11/06 15:14:53  aamar
 * Workflow support: Correct some code about reqID
 *
 * Revision 1.3  2006/10/20 09:13:55  aamar
 * Changing the submit_wf prototype (the return type)
 * Add the following function to the MA DAG interface
 *   - void setAsDone, setDagAsDone, registerClt
 * Some changes.
 *
 * Revision 1.2  2006/07/10 10:00:00  aamar
 * - Adding the function remainingDag to the MA DAG interface
 * - Round Robbin and HEFT scheduling
 *
 * Revision 1.1  2006/04/14 13:43:33  aamar
 * header of the MA DAG CORBA object.
 *
 ****************************************************************************/

#ifndef _MADAG_IMPL_HH_
#define _MADAG_IMPL_HH_

#include <string>

#include "MaDag.hh"
#include "MasterAgent.hh"
#include "WfScheduler.hh"
#include "MultiWfScheduler.hh"
#include "CltMan.hh"
#include "MaDag.hh"
#ifdef USE_LOG_SERVICE
#include "DietLogComponent.hh"
#endif
#include "DagWfParser.hh"
#include "MetaDag.hh"

#include "Forwarder.hh"
#include "MaDagFwdr.hh"

class MaDag_impl : public POA_MaDag,
                   public PortableServer::RefCountServantBase {
public:
  enum MaDagSchedType { BASIC, GHEFT, GAHEFT, FOFT, SRPT , FCFS };

  MaDag_impl(const char * name,
             const MaDagSchedType schedType = BASIC,
             const int interRoundDelay = -1); // use default

  virtual ~MaDag_impl();

  /**
   * DAG Workflow processing
   * Manages the scheduling and orchestration of the submitted DAG
   * Delegates the execution of each individual task to the client
   * workflow mgr
   *
   * @param  dag_desc   workflow textual description
   * @param  cltMgrRef  client workflow manager reference
   * @param  wfReqId    submitted workflow identifier (obtained by
   * getWfReqId function)
   * @return the dag Id if workflow submission succeed, -1 if not
   */

  virtual CORBA::Long
  processDagWf(const corba_wf_desc_t& dag_desc, const char* cltMgrRef,
	       CORBA::Long wfReqId);
  /**
   * Multi DAG Workflow processing
   * Manages the scheduling and orchestration of the submitted DAG
   * when this DAG is linked to other DAGs from the same wf request.
   * Delegates the execution of each individual task to the client
   *
   * @param  dag_desc     workflow textual description
   * @param  cltMgrRef    client workflow manager reference
   * @param  wfReqId      submitted workflow identifier (obtained by
   * getWfReqId function)
   * @param  release      if false, does not destroy metadag when this request ends
   * @return the dag Id if workflow submission succeed, -1 if not
   */

  virtual CORBA::Long
  processMultiDagWf(const corba_wf_desc_t& dag_desc, const char* cltMgrRef,
		    CORBA::Long wfReqId, CORBA::Boolean release);

  /**
   * Multi DAG release
   * Free all ressources after the multi-dag is completed
   */
  virtual void
  releaseMultiDag(CORBA::Long wfReqId);

  /**
   * DAG cancellation method (non-blocking)
   * Will stop execution of the dag as soon as all running dag nodes are completed.
   * (running nodes cannot be stopped asynchronously)
   * This method returns immediately. The MaDag will call release() on the CltWfMgr
   * when the last running node is completed.
   * @param  dagId  id of the dag (provided by MaDag at submission)
   */
  virtual void
  cancelDag(CORBA::Long dagId);

  /**
   * Get a new workflow request identifier
   * @return a new identifier to be used for a wf request
   */
  virtual CORBA::Long
  getWfReqId();

  /**
   * Get the client mgr for a given dag
   * @param dagId the dag identifier
   * @return client manager pointer (CORBA)
   */
  virtual CltMan_ptr
  getCltMan(const string& dagId);

  /** Used to test if it is alive. */
  virtual CORBA::Long
  ping();

  /**
   * Set the platform type
   */
  virtual void
  setPlatformType(MaDag::pfmType_t pfmType);

#ifdef USE_LOG_SERVICE
  /**
   * Ptr to the DietLogComponent. This ptr can be NULL, so it has to
   * be checked every time it is used. If it is NULL, no monitoring
   * messages have to be sent.
   */
  DietLogComponent* dietLogComponent;

  /**
   *  Get the DietLogComponent
   */
  DietLogComponent*
  getDietLogComponent();
#endif
  /**
   * Get the MA
   */
  MasterAgent_var
  getMA() const;

protected:
  /**
   * set the client manager for a wf request
   */
  void
  setCltMan(CORBA::Long wfReqId, CltMan_ptr cltMan);

  /**
   * set the wf request id for a given dag
   */
  void
  setWfReq(CORBA::Long dagId, CORBA::Long wfReqId);

  /**
   * Common part of dag processing call methods
   */
  CORBA::Long
  processDagWfCommon(const corba_wf_desc_t& dag_desc,
                     const char* cltMgrRef,
                     CORBA::Long wfReqId,
                     MetaDag* mDag = NULL);

  /**
   * Parse a new dag provided in xml text and create a dag object
   * @param wf_desc   workflow string description
   * @param dagId     the dag identifier
   * @param mDag      ref to a metadag (optional) in case dag is linked to other dags
   * @return pointer to dag structure (to be destroyed by the caller)
   */
  Dag *
  parseNewDag(const corba_wf_desc_t& wf_desc,
              const string& dagId,
              MetaDag * mDag = NULL)
  throw (MaDag::InvalidDag);

#ifdef USE_LOG_SERVICE
  /**
   * setup the DietLogComponent
   */
  void
  setupDietLogComponent();
#endif

private:
  /**
   * The Ma Dag name (used for CORBA naming service binding)
   */
  std::string myName;

  /**
   * The master agent reference
   */
  MasterAgent_var myMA;

  /**
   * The mapping table dagId => wfReqId
   */
  map<string, CORBA::Long> wfReqs;

  /**
   * The mapping table wfReqId => cltManager
   */
  map<CORBA::Long, CltMan_ptr> cltMans;

  /**
   * The mapping table wfReqId => MetaDag
   */
  map<CORBA::Long, MetaDag*> myMetaDags;

  /**
   * The meta-scheduler (used for multiworkflow support)
   */
  madag::MultiWfScheduler * myMultiWfSched;

  /**
   * Lock to prevent concurrent access
   */
  omni_mutex myMutex ;

  /**
   * Dag identifier counter
   */
  CORBA::Long wfReqIdCounter;

  /**
   * Dag counter
   */
  CORBA::Long dagIdCounter;

};

class MaDagFwdrImpl : public POA_MaDag,
		      public PortableServer::RefCountServantBase {
protected:
  Forwarder_ptr forwarder;
  char* objName;
public:
  MaDagFwdrImpl(Forwarder_ptr fwdr, const char* objName);
  virtual CORBA::Long
  processDagWf(const corba_wf_desc_t& dag_desc, const char* cltMgrRef,
	       CORBA::Long wfReqId);
  virtual CORBA::Long
  processMultiDagWf(const corba_wf_desc_t& dag_desc, const char* cltMgrRef,
		    CORBA::Long wfReqId, CORBA::Boolean release);
  virtual CORBA::Long getWfReqId();
  virtual void releaseMultiDag(CORBA::Long wfReqId);
  virtual void cancelDag(CORBA::Long dagId);
  virtual void setPlatformType(MaDag::pfmType_t pfmType);
  virtual CORBA::Long     ping();
};

#endif   /* not defined _MADAG_IMPL_HH */



