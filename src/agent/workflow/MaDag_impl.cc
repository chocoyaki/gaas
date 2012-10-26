/**
 * @file MaDag_impl.cc
 *
 * @brief  The MA DAG CORBA object implementation
 *
 * @author  Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#include "MaDag_impl.hh"

#include <iostream>
#include "ORBMgr.hh"
#include "debug.hh"
#include "configuration.hh"
#include "statistics.hh"

#include "HEFTScheduler.hh"
#include "MultiWfBasicScheduler.hh"
#include "MultiWfFOFT.hh"
#include "MultiWfHEFT.hh"
#include "MultiWfSRPT.hh"
#include "MultiWfFCFS.hh"
#include "EventManager.hh"
#include "MaDagLogCentralDispatcher.hh"
#include "debug.hh"


using events::EventManager;
using events::sendEventFrom;
using madag::MultiWfScheduler;
using madag::MultiWfBasicScheduler;
using madag::MultiWfHEFT;
using madag::MultiWfAgingHEFT;
using madag::MultiWfSRPT;
using madag::MultiWfFCFS;
using madag::MultiWfFOFT;

MaDag_impl::MaDag_impl(const char *name, const MaDagSchedType schedType,
                       const int interRoundDelay)
  : myName(name), myMultiWfSched(0), wfReqIdCounter(0), dagIdCounter(0) {
  std::string MAName;

  // check if the parent is NULL
  if (!CONFIG_STRING(diet::PARENTNAME, MAName)) {
    ERROR_EXIT("MA name not provided");
  }

  try {
    CORBA::Object_var obj =
      ORBMgr::getMgr()->resolveObject(AGENTCTXT, MAName);
    this->myMA = MasterAgent::_duplicate(MasterAgent::_narrow(obj));
    if (CORBA::is_nil(this->myMA)) {
      ERROR_EXIT("Cannot locate the master agent " << MAName);
    }
  } catch (...) {
    ERROR_EXIT("Cannot locate the master agent " << MAName);
  }

  /* Bind the MA DAG to its name in the CORBA Naming Service */
  try {
    ORBMgr::getMgr()->bind(MADAGCTXT, this->myName, _this(), true);
    ORBMgr::getMgr()->fwdsBind(MADAGCTXT, this->myName,
                               ORBMgr::getMgr()->getIOR(_this()));
  } catch (...) {
    ERROR_EXIT("could not declare myself as " << this->myName);
  }

  TRACE_TEXT(TRACE_MAIN_STEPS, "\n"
             << "MA DAG " << this->myName << " created.\n");
  TRACE_TEXT(TRACE_MAIN_STEPS, "## MADAG_IOR "
             << ORBMgr::getMgr()->getIOR(this->_this()) << "\n");
#ifdef USE_LOG_SERVICE
  this->setupDietLogComponent();
  if (dietLogComponent) {
    EventManager::getEventMgr()->addObserver(
      new MaDagLogCentralDispatcher(dietLogComponent));
  }
#endif
  // starting the multiwfscheduler
  switch (schedType) {
  case BASIC:
    this->myMultiWfSched = new MultiWfBasicScheduler(this);
    sendEventFrom<MultiWfScheduler,
                  MultiWfScheduler::CONSTR>(myMultiWfSched, "Created",
                                            "BASIC", EventBase::INFO);
    break;
  case GHEFT:
    this->myMultiWfSched = new MultiWfHEFT(this);
    sendEventFrom<MultiWfScheduler,
                  MultiWfScheduler::CONSTR>(myMultiWfSched, "Created",
                                            "GLOBAL_HEFT", EventBase::INFO);
    break;
  case FOFT:
    this->myMultiWfSched = new MultiWfFOFT(this);
    sendEventFrom<MultiWfScheduler,
                  MultiWfScheduler::CONSTR>(myMultiWfSched, "Created",
                                            "FOFT", EventBase::INFO);
    break;
  case GAHEFT:
    this->myMultiWfSched = new MultiWfAgingHEFT(this);
    sendEventFrom<MultiWfScheduler,
                  MultiWfScheduler::CONSTR>(myMultiWfSched, "Created",
                                            "GLOBAL_AGING_HEFT",
                                            EventBase::INFO);
    break;
  case SRPT:
    this->myMultiWfSched = new MultiWfSRPT(this);
    sendEventFrom<MultiWfScheduler,
                  MultiWfScheduler::CONSTR>(myMultiWfSched, "Created",
                                            "SRPT", EventBase::INFO);
    break;
  case FCFS:
    this->myMultiWfSched = new MultiWfFCFS(this);
    sendEventFrom<MultiWfScheduler,
                  MultiWfScheduler::CONSTR>(myMultiWfSched, "Created",
                                            "FCFS", EventBase::INFO);
    break;
  } // switch

  if (interRoundDelay >= 0) {
    this->myMultiWfSched->setInterRoundDelay(interRoundDelay);
  }

  this->myMultiWfSched->start();
  TRACE_TEXT(TRACE_ALL_STEPS, "InterRoundDelay= " <<
             this->myMultiWfSched->getInterRoundDelay() << "\n");
  // init the statistics module
  stat_init();
}

MaDag_impl::~MaDag_impl() {
  ORBMgr::getMgr()->unbind(MADAGCTXT, this->myName);
  ORBMgr::getMgr()->fwdsUnbind(MADAGCTXT, this->myName);

  if (this->myMultiWfSched != NULL) {
    /* Stop the scheduling thread
     * This is mandatory, otherwise we get an omni_thread_fatal error
     */
    this->myMultiWfSched->stop();
    delete (this->myMultiWfSched);
  }
}


/**
 * DAG Workflow processing
 */
CORBA::Long
MaDag_impl::processDagWf(const corba_wf_desc_t &dag_desc,
                         const char *cltMgrRef,
                         CORBA::Long wfReqId) {
  TRACE_TEXT(TRACE_ALL_STEPS,
             "%%%%% MADAG receives a SINGLE DAG request (wfReqId = "
             << wfReqId << ")\n");
  return processDagWfCommon(dag_desc, cltMgrRef, wfReqId);
}

/**
 * Multi DAG Workflow processing
 */
CORBA::Long
MaDag_impl::processMultiDagWf(const corba_wf_desc_t &dag_desc,
                              const char *cltMgrRef,
                              CORBA::Long wfReqId, CORBA::Boolean release) {
  TRACE_TEXT(TRACE_ALL_STEPS,
             "%%%%% MADAG receives a MULTIPLE DAG request (wfReqId = "
             << wfReqId << " / release=" << release << ")\n");
  // Check if a MetaDag already exists for this wf request (or create one)
  MetaDag *mDag = 0;
  std::map<CORBA::Long, MetaDag *>::iterator mDagIter = myMetaDags.find(wfReqId);
  if (mDagIter != myMetaDags.end()) {
    mDag = (MetaDag *) mDagIter->second;
  } else {
    mDag = new MetaDag(itoa(wfReqId));
    myMetaDags[wfReqId] = mDag;
  }
  // Set the release flag
  mDag->setReleaseFlag(release);
  // Process the dag
  return processDagWfCommon(dag_desc, cltMgrRef, wfReqId, mDag);
} // processMultiDagWf

/**
 * Multi DAG Workflow Release
 */
void
MaDag_impl::releaseMultiDag(CORBA::Long wfReqId) {
  TRACE_TEXT(TRACE_ALL_STEPS,
             "%%%%% MADAG receives a RELEASE request (wfReqId = "
             << wfReqId << ")\n");
  MetaDag *mDag = 0;
  std::map<CORBA::Long, MetaDag *>::iterator mDagIter =
    myMetaDags.find(wfReqId);
  if (mDagIter != myMetaDags.end()) {
    mDag = (MetaDag *) mDagIter->second;
  } else {
    std::string errorMsg = "Request ID '" + itoa(wfReqId) + "' not found";
    throw(MaDag::InvalidRequest(errorMsg.c_str()));
  }
  // Set the release flag
  bool isDone = false;
  mDag->setReleaseFlag(true, isDone);
  if (isDone) {
    delete mDag;
  }
} // releaseMultiDag

/**
 * Common part
 */
CORBA::Long
MaDag_impl::processDagWfCommon(const corba_wf_desc_t &dag_desc,
                               const char *cltMgrRef,
                               CORBA::Long wfReqId,
                               MetaDag *mDag) {
  char statMsg[128];
#ifdef __WIN32__
  _snprintf(statMsg, 128, "Start workflow request %ld",
           static_cast<long int>(wfReqId));
#else
  snprintf(statMsg, 128, "Start workflow request %ld",
           static_cast<long int>(wfReqId));
#endif
  
  stat_in("MA_DAG", statMsg);

  this->myMutex.lock();

  // Register the client workflow manager
  CORBA::Object_ptr obj = ORBMgr::getMgr()->resolveObject(WFMGRCTXT, cltMgrRef);
  CltMan_ptr cltMan = CltMan::_narrow(obj);
  setCltMan(wfReqId, cltMan);

  /* Process the request ie merge dag into the global workflow
     managed by the MaDag */
  CORBA::Long dagId = dagIdCounter++;
  setWfReq(dagId, wfReqId);
  try {
    Dag *newDag = this->parseNewDag(dag_desc, itoa(dagId), mDag);
    this->myMultiWfSched->scheduleNewDag(newDag, mDag);
  } catch (...) {
    sprintf(statMsg,
            "Dag request (%ld) aborted", static_cast<long int>(dagId));
    stat_out("MA_DAG", statMsg);
    TRACE_TEXT(TRACE_ALL_STEPS, "MADAG cancelled DAG request (wfReqId = "
               << wfReqId << ")\n");
    this->myMutex.unlock();
    throw;
  }

  this->myMutex.unlock();
  sprintf(statMsg, "End Dag request %ld", static_cast<long int>(dagId));
  stat_out("MA_DAG", statMsg);
  stat_flush();
  return dagId;
} // processDagWfCommon

/**
 * Parse dag xml description and create a dag object
 */
Dag *
MaDag_impl::parseNewDag(const corba_wf_desc_t &wf_desc,
                        const std::string &dagId,
                        MetaDag *mDag)
throw(MaDag::InvalidDag) {
  // CREATION & PARSING
  Dag *newDag = new Dag(dagId);
  SingleDagParser *reader = new SingleDagParser(*newDag, wf_desc.abstract_wf);

  try {
    reader->parseXml();
  } catch (XMLParsingException &e) {
    throw(MaDag::InvalidDag(e.ErrorMsg().c_str()));
  }

  delete reader;
  // CHECK STRUCTURE
  NodeSet *contextNodeSet;
  if (!mDag) {
    contextNodeSet = (NodeSet *) newDag;
  } else {
    mDag->addDag(newDag);
    contextNodeSet = (NodeSet *) mDag;
    mDag->setCurrentDag(newDag);
  }
  try {
    newDag->checkPrec(contextNodeSet);
  } catch (WfStructException &e) {
    if (mDag) {
      mDag->setCurrentDag(0);
    }
    throw(MaDag::InvalidDag(e.ErrorMsg().c_str()));
  }

  if (mDag) {
    // release current dag in metadag
    mDag->setCurrentDag(0);
  }

  return newDag;
} // parseNewDag

/**
 * Get a new workflow request identifier
 */
CORBA::Long
MaDag_impl::getWfReqId() {
  this->myMutex.lock();
  CORBA::Long res = this->wfReqIdCounter++;
  this->myMutex.unlock();
  return res;
}

/**
 * Used to test if it is alive.
 */
CORBA::Long
MaDag_impl::ping() {
  TRACE_TEXT(TRACE_ALL_STEPS, "ping()\n");
  stat_flush();
  return getpid();
}

/**
 * Cancel a dag
 */
void
MaDag_impl::cancelDag(CORBA::Long dagId) {
  this->myMultiWfSched->cancelDag(itoa(dagId));
}

/**
 * Get the MA
 */
MasterAgent_var
MaDag_impl::getMA() const {
  return this->myMA;
}

/**
 * Get the client manager for a given dag id
 */
CltMan_ptr
MaDag_impl::getCltMan(const std::string &dagId) {
  std::map<std::string, CORBA::Long>::iterator wfReqIter = wfReqs.find(dagId);
  if (wfReqIter != wfReqs.end()) {
    std::map<CORBA::Long, CltMan_ptr>::iterator cltManIter =
      cltMans.find(wfReqIter->second);
    if (cltManIter != cltMans.end()) {
      return cltManIter->second;
    }
  }
  return CltMan::_nil();
} // getCltMan

/**
 * Set the client manager for a wf request
 */
void
MaDag_impl::setCltMan(CORBA::Long wfReqId, CltMan_ptr cltMan) {
  this->cltMans[wfReqId] = cltMan;
}

/**
 * set the wf request id for a given dag
 */
void
MaDag_impl::setWfReq(CORBA::Long dagId, CORBA::Long wfReqId) {
  this->wfReqs[itoa(dagId)] = wfReqId;
}

/**
 * Set the platform type
 */
void
MaDag_impl::setPlatformType(MaDag::pfmType_t pfmType) {
  switch (pfmType) {
  case (MaDag::DISTINCT_SERVICES):
    this->myMultiWfSched->setPlatformType(MultiWfScheduler::PFM_ANY);
    break;
  case (MaDag::SAME_SERVICES):
    this->myMultiWfSched->setPlatformType(MultiWfScheduler::PFM_SAME_SERVICES);
    break;
  default:
    WARNING("Wrong platform type");
  }
} // setPlatformType

#ifdef USE_LOG_SERVICE
/**
 * Get the DietLogComponent
 */
DietLogComponent *
MaDag_impl::getDietLogComponent() {
  return this->dietLogComponent;
}

/**
 * Set the DietLogComponent
 */
void
MaDag_impl::setupDietLogComponent() {
  /* Create the DietLogComponent for use with LogService */
  bool useLS;

  // size_t --> unsigned int
  bool useLogService = false;
  CONFIG_BOOL(diet::USELOGSERVICE, useLogService);
  if (!useLogService) {
    WARNING("useLogService disabled");
  } else {
    useLS = true;
  }

  if (useLS) {
    int outBufferSize;
    int flushTime;

    if (!CONFIG_INT(diet::LSOUTBUFFERSIZE, outBufferSize)) {
      outBufferSize = 0;
      WARNING("lsOutbuffersize not configured, using default");
    }

    if (!CONFIG_INT(diet::LSFLUSHINTERVAL, flushTime)) {
      flushTime = 10000;
      WARNING("lsFlushinterval not configured, using default");
    }

    TRACE_TEXT(TRACE_ALL_STEPS, "LogService enabled\n");
    char *agtTypeName = NULL;
    char *agtParentName = NULL;
    char *agtName;
    std::string tmpString;
    if (CONFIG_STRING(diet::PARENTNAME, tmpString)) {
      agtParentName = strdup(tmpString.c_str());
    }

    if (CONFIG_STRING(diet::NAME, tmpString)) {
      agtName = strdup(tmpString.c_str());
    }

    // the agent names should be correct if we arrive here
    this->dietLogComponent = new DietLogComponent(agtName, outBufferSize,
                                                  0, 0);

    ORBMgr::getMgr()->activate(dietLogComponent);

    agtTypeName = strdup("MA_DAG");
    if (dietLogComponent->run(agtTypeName, agtParentName, flushTime) != 0) {
      WARNING("Could not initialize DietLogComponent");
      TRACE_TEXT(TRACE_ALL_STEPS, "* LogService: disabled\n");
      dietLogComponent = NULL;
    }
    free(agtTypeName);
  } else {
    TRACE_TEXT(TRACE_ALL_STEPS, "LogService disabled\n");
    this->dietLogComponent = NULL;
  }
} // setupDietLogComponent
#endif // ifdef USE_LOG_SERVICE

MaDagFwdrImpl::MaDagFwdrImpl(Forwarder_ptr fwdr, const char *objName) {
  this->forwarder = Forwarder::_duplicate(fwdr);
  this->objName = CORBA::string_dup(objName);
}

CORBA::Long
MaDagFwdrImpl::processDagWf(const corba_wf_desc_t &dag_desc,
                            const char *cltMgrRef,
                            CORBA::Long wfReqId) {
  return forwarder->processDagWf(dag_desc, cltMgrRef, wfReqId, objName);
}

CORBA::Long
MaDagFwdrImpl::processMultiDagWf(const corba_wf_desc_t &dag_desc,
                                 const char *cltMgrRef,
                                 CORBA::Long wfReqId,
                                 CORBA::Boolean release) {
  return forwarder->processMultiDagWf(dag_desc, cltMgrRef,
                                      wfReqId, release, objName);
}

CORBA::Long
MaDagFwdrImpl::getWfReqId() {
  return forwarder->getWfReqId(objName);
}

void
MaDagFwdrImpl::releaseMultiDag(CORBA::Long wfReqId) {
  forwarder->releaseMultiDag(wfReqId, objName);
}

void
MaDagFwdrImpl::cancelDag(CORBA::Long dagId) {
  forwarder->cancelDag(dagId, objName);
}

void
MaDagFwdrImpl::setPlatformType(MaDag::pfmType_t pfmType) {
  forwarder->setPlatformType(pfmType, objName);
}

CORBA::Long
MaDagFwdrImpl::ping() {
  return forwarder->ping(objName);
}
