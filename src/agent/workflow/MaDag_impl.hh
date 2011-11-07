/**
 * @file MaDag_impl.hh
 *
 * @brief  The MA DAG CORBA object implementation header
 *
 * @author  Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */



#ifndef _MADAG_IMPL_HH_
#define _MADAG_IMPL_HH_

#include <map>
#include <string>

#include "MaDag.hh"
#include "MasterAgent.hh"
#include "WfScheduler.hh"
#include "MultiWfScheduler.hh"
#include "CltMan.hh"
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
  enum MaDagSchedType {BASIC, GHEFT, GAHEFT, FOFT, SRPT, FCFS};

  MaDag_impl(const char *name,
             const MaDagSchedType schedType = BASIC,
             const int interRoundDelay = -1);  // use default

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
  processDagWf(const corba_wf_desc_t &dag_desc, const char *cltMgrRef,
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
  processMultiDagWf(const corba_wf_desc_t &dag_desc, const char *cltMgrRef,
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
  getCltMan(const std::string &dagId);

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
  DietLogComponent *dietLogComponent;

  /**
   *  Get the DietLogComponent
   */
  DietLogComponent *
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
  processDagWfCommon(const corba_wf_desc_t &dag_desc,
                     const char *cltMgrRef,
                     CORBA::Long wfReqId,
                     MetaDag *mDag = NULL);

  /**
   * Parse a new dag provided in xml text and create a dag object
   * @param wf_desc   workflow string description
   * @param dagId     the dag identifier
   * @param mDag      ref to a metadag (optional) in case dag is linked to other dags
   * @return pointer to dag structure (to be destroyed by the caller)
   */
  Dag *
  parseNewDag(const corba_wf_desc_t &wf_desc,
              const std::string &dagId,
              MetaDag *mDag = NULL)
  throw(MaDag::InvalidDag);

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
  std::map<std::string, CORBA::Long> wfReqs;

  /**
   * The mapping table wfReqId => cltManager
   */
  std::map<CORBA::Long, CltMan_ptr> cltMans;

  /**
   * The mapping table wfReqId => MetaDag
   */
  std::map<CORBA::Long, MetaDag *> myMetaDags;

  /**
   * The meta-scheduler (used for multiworkflow support)
   */
  madag::MultiWfScheduler *myMultiWfSched;

  /**
   * Lock to prevent concurrent access
   */
  omni_mutex myMutex;

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
public:
  MaDagFwdrImpl(Forwarder_ptr fwdr, const char *objName);

  virtual CORBA::Long
  processDagWf(const corba_wf_desc_t &dag_desc, const char *cltMgrRef,
               CORBA::Long wfReqId);

  virtual CORBA::Long
  processMultiDagWf(const corba_wf_desc_t &dag_desc, const char *cltMgrRef,
                    CORBA::Long wfReqId, CORBA::Boolean release);

  virtual CORBA::Long
  getWfReqId();

  virtual void
  releaseMultiDag(CORBA::Long wfReqId);

  virtual void
  cancelDag(CORBA::Long dagId);

  virtual void
  setPlatformType(MaDag::pfmType_t pfmType);

  virtual CORBA::Long
  ping();

protected:
  Forwarder_ptr forwarder;
  char *objName;
};

#endif   /* not defined _MADAG_IMPL_HH */
