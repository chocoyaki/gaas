/****************************************************************************/
/* The MA DAG CORBA object implementation                                   */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/

/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.42  2011/04/05 14:03:42  bdepardo
 * IOR is printed only when the tracelevel is at least TRACE_MAIN_STEPS
 *
 * Revision 1.41  2011/03/21 08:27:39  bdepardo
 * Correctly register the logcomponent into the ORB, and correclty detroy it.
 *
 * Revision 1.40  2011/03/20 18:48:18  bdepardo
 * Be more robust when logComponent initialization fails
 *
 * Revision 1.39  2011/03/18 16:58:13  hguemar
 * fixes several issues in src/agent/workflow: reduce some variables scope, use diet::usleep instead of Posix deprecated usleep
 *
 * Revision 1.38  2011/03/16 21:36:37  bdepardo
 * Stop scheduling thread an unbind this agent in destructor.
 * Catch an exception when the MA cannot be found.
 *
 * Revision 1.37  2011/02/24 16:57:02  bdepardo
 * Use new parser
 *
 * Revision 1.36  2011/02/09 11:27:53  bdepardo
 * Removed endl at the end of the call to the WARNING macro
 *
 * Revision 1.35  2011/02/04 15:20:48  hguemar
 * fixes to new configuration parser
 * some cleaning
 *
 * Revision 1.34  2010/12/17 09:48:00  kcoulomb
 * * Set diet to use the new log with forwarders
 * * Fix a CoRI problem
 * * Add library version remove DTM flag from ccmake because deprecated
 *
 * Revision 1.33  2010/07/27 15:05:26  glemahec
 * Bug correction
 *
 * Revision 1.32  2010/07/20 08:59:36  bisnard
 * Added event generation
 *
 * Revision 1.31  2010/07/12 16:14:11  glemahec
 * DIET 2.5 beta 1 - Use the new ORB manager and allow the use of SSH-forwarders for all DIET CORBA objects
 *
 * Revision 1.30  2009/10/23 13:59:18  bisnard
 * replaced \n by std::endl
 *
 * Revision 1.29  2009/09/25 12:42:09  bisnard
 * - use new DagNodeLauncher classes to manage threads
 * - added dag cancellation method
 *
 * Revision 1.28  2009/08/26 10:33:36  bisnard
 * use new parser for single dags
 *
 * Revision 1.27  2009/02/06 14:50:35  bisnard
 * setup exceptions
 *
 * Revision 1.26  2009/01/16 13:41:22  bisnard
 * added common base class DagScheduler to simplify dag events handling
 * improved exception management
 *
 * Revision 1.25  2008/12/09 12:09:00  bisnard
 * added parameters to dag submit method to handle inter-dependent dags
 *
 * Revision 1.24  2008/12/02 10:21:03  bisnard
 * use MetaDags to handle multi-dag submission and execution
 *
 * Revision 1.23  2008/10/14 13:23:01  bisnard
 * - use dagId instead of wfReqId as key for dags
 * - new mapping table dagId to wfReqId
 *
 * Revision 1.22  2008/09/04 15:22:25  bisnard
 * Changed name of multiwf heuristic HEFT to GHEFT
 *
 * Revision 1.21  2008/09/04 14:33:55  bisnard
 * - New option for MaDag to select platform type (servers
 * with same service list or not)
 * - Optimization of the multiwfscheduler to avoid requests to
 * MA for server availability
 *
 * Revision 1.20  2008/07/24 21:08:11  rbolze
 * New multi-wf heuristic FCFS (First Come First Serve)
 *
 * Revision 1.19  2008/07/17 13:33:09  bisnard
 * New multi-wf heuristic SRPT
 *
 * Revision 1.18  2008/07/17 10:49:14  rbolze
 * change fflush(stdout) by stat_flush()
 *
 * Revision 1.17  2008/07/17 10:14:36  rbolze
 * add some stat_info
 *
 * Revision 1.16  2008/07/12 00:22:28  rbolze
 * add function getInterRoundDelay()
 * use this function when the maDag start to display this value.
 * display the dag_id when compute the ageFactor in AgingHEFT
 * add some stats info :
 * 	queuedNodeCount
 * 	change MA DAG to MA_DAG
 *
 * Revision 1.15  2008/07/08 15:52:03  bisnard
 * Set interRoundDelay as parameter of workflow scheduler
 *
 * Revision 1.14  2008/07/08 09:47:36  rbolze
 * send madag scheduler type through dietLogComponent
 *
 * Revision 1.13  2008/06/19 10:18:54  bisnard
 * new heuristic AgingHEFT for multi-workflow scheduling
 *
 * Revision 1.12  2008/06/03 12:19:36  bisnard
 * Method to get MA ref
 *
 * Revision 1.11  2008/06/01 14:06:57  rbolze
 * replace most ot the cout by adapted function from debug.cc
 * there are some left ...
 *
 * Revision 1.10  2008/06/01 09:22:14  rbolze
 * add getter to retrieve the dietLogComponent
 *
 * Revision 1.9  2008/05/31 08:45:55  rbolze
 * add DietLogComponent to the maDagAgent
 *
 * Revision 1.8  2008/05/16 12:30:20  bisnard
 * MaDag returns dagID to client after dag submission
 * (used for node execution)
 *
 * Revision 1.7  2008/04/30 07:37:01  bisnard
 * use relative timestamps for estimated and real completion time
 * make MultiWfScheduler abstract and add HEFT MultiWf scheduler
 *
 * Revision 1.6  2008/04/29 07:25:01  rbolze
 * change stat_out messages
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
 * Revision 1.2  2008/04/14 09:10:37  bisnard
 *  - Workflow rescheduling (CltReoMan) no longer used with MaDag v2
 *  - AbstractWfSched and derived classes no longer used with MaDag v2
 *
 * Revision 1.1  2008/04/10 09:13:29  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 * Revision 1.8  2008/03/21 10:22:04  rbolze
 *  - add ping() function to the MaDag in order to be able test this component.
 * this is use by goDIET.
 *  - print IOR in stdout in the constructor of the MaDag.
 *
 * Revision 1.7b ??? aamar (added by bisnard)
 * Added functions submit_dag and submit_dag_in_multi
 *
 * Revision 1.7  2006/11/06 15:14:53  aamar
 * Workflow support: Correct some code about reqID
 *
 * Revision 1.6  2006/11/06 12:05:15  aamar
 * Workflow support: correct some bugs.
 *
 * Revision 1.5  2006/11/02 17:10:19  rbolze
 * add some debug info
 *
 * Revision 1.4  2006/10/24 00:06:40  aamar
 * Adding statistics to submit workflow method.
 * The dag id is provided only by the MasterAgent.
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
 * Revision 1.1  2006/04/14 13:43:59  aamar
 * source of the MA DAG CORBA object.
 *
 ****************************************************************************/

#include <iostream>

#include "MaDag_impl.hh"

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

// FIXME: never ever import namespace globally
using namespace madag;

MaDag_impl::MaDag_impl(const char * name,
                       const MaDagSchedType schedType,
                       const int interRoundDelay) :
  myName(name), myMultiWfSched(0), wfReqIdCounter(0), dagIdCounter(0) {

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

  TRACE_TEXT(TRACE_MAIN_STEPS, std::endl
             <<  "MA DAG " << this->myName << " created." << std::endl);
  TRACE_TEXT(TRACE_MAIN_STEPS, "## MADAG_IOR "
             << ORBMgr::getMgr()->getIOR(this->_this()) << std::endl);
#ifdef USE_LOG_SERVICE
  this->setupDietLogComponent();
  if (dietLogComponent) {
    EventManager::getEventMgr()->addObserver(new MaDagLogCentralDispatcher(dietLogComponent));
  }
#endif
  // starting the multiwfscheduler
  switch (schedType) {
  case BASIC:
    this->myMultiWfSched = new MultiWfBasicScheduler(this);
    sendEventFrom<MultiWfScheduler, MultiWfScheduler::CONSTR>(myMultiWfSched, "Created", "BASIC", EventBase::INFO);
    break;
  case GHEFT:
    this->myMultiWfSched = new MultiWfHEFT(this);
    sendEventFrom<MultiWfScheduler, MultiWfScheduler::CONSTR>(myMultiWfSched, "Created", "GLOBAL_HEFT", EventBase::INFO);
    break;
  case FOFT:
    this->myMultiWfSched = new MultiWfFOFT(this);
    sendEventFrom<MultiWfScheduler, MultiWfScheduler::CONSTR>(myMultiWfSched, "Created", "FOFT", EventBase::INFO);
    break;
  case GAHEFT:
    this->myMultiWfSched = new MultiWfAgingHEFT(this);
    sendEventFrom<MultiWfScheduler, MultiWfScheduler::CONSTR>(myMultiWfSched, "Created", "GLOBAL_AGING_HEFT", EventBase::INFO);
    break;
  case SRPT:
    this->myMultiWfSched = new MultiWfSRPT(this);
    sendEventFrom<MultiWfScheduler, MultiWfScheduler::CONSTR>(myMultiWfSched, "Created", "SRPT", EventBase::INFO);
    break;
  case FCFS:
    this->myMultiWfSched = new MultiWfFCFS(this);
    sendEventFrom<MultiWfScheduler, MultiWfScheduler::CONSTR>(myMultiWfSched, "Created", "FCFS", EventBase::INFO);
    break;
  }

  if (interRoundDelay >= 0) {
    this->myMultiWfSched->setInterRoundDelay(interRoundDelay);
  }

  this->myMultiWfSched->start();
  TRACE_TEXT(TRACE_ALL_STEPS, "InterRoundDelay= " <<
             this->myMultiWfSched->getInterRoundDelay() << std::endl);
  // init the statistics module
  stat_init();
} // end MA DAG constructor

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
} // end MA DAG destructor


/**
 * DAG Workflow processing
 */
CORBA::Long
MaDag_impl::processDagWf(const corba_wf_desc_t& dag_desc,
                         const char* cltMgrRef,
                         CORBA::Long wfReqId) {
    TRACE_TEXT(TRACE_ALL_STEPS,
	       "%%%%% MADAG receives a SINGLE DAG request (wfReqId = "
	       << wfReqId << ")" << std::endl);
    return processDagWfCommon(dag_desc, cltMgrRef, wfReqId);
}

/**
 * Multi DAG Workflow processing
 */
CORBA::Long
MaDag_impl::processMultiDagWf(const corba_wf_desc_t& dag_desc,
			      const char* cltMgrRef,
                              CORBA::Long wfReqId, CORBA::Boolean release)
{
    TRACE_TEXT(TRACE_ALL_STEPS,
	       "%%%%% MADAG receives a MULTIPLE DAG request (wfReqId = "
	       << wfReqId << " / release=" << release << ")" << std::endl);
    // Check if a MetaDag already exists for this wf request (or create one)
    MetaDag* mDag = 0;
    map<CORBA::Long, MetaDag*>::iterator mDagIter = myMetaDags.find(wfReqId);
    if (mDagIter != myMetaDags.end()) {
	mDag = (MetaDag*) mDagIter->second;
    } else {
	mDag = new MetaDag(itoa(wfReqId));
	myMetaDags[wfReqId] = mDag;
    }
    // Set the release flag
    mDag->setReleaseFlag(release);
    // Process the dag
    return processDagWfCommon(dag_desc, cltMgrRef, wfReqId, mDag);
}

/**
 * Multi DAG Workflow Release
 */
void
MaDag_impl::releaseMultiDag(CORBA::Long wfReqId)
{
    TRACE_TEXT(TRACE_ALL_STEPS,
	       "%%%%% MADAG receives a RELEASE request (wfReqId = "
	       << wfReqId << ")" << std::endl);
    MetaDag* mDag = 0;
    map<CORBA::Long, MetaDag*>::iterator mDagIter = myMetaDags.find(wfReqId);
    if (mDagIter != myMetaDags.end()) {
	mDag = (MetaDag*) mDagIter->second;
    } else {
	string errorMsg = "Request ID '" + itoa(wfReqId) + "' not found";
	throw (MaDag::InvalidRequest(errorMsg.c_str()));
    }
    // Set the release flag
    bool isDone = false;
    mDag->setReleaseFlag(true, isDone);
    if (isDone)
	delete mDag;
}

/**
 * Common part
 */
CORBA::Long
MaDag_impl::processDagWfCommon(const corba_wf_desc_t& dag_desc,
                               const char* cltMgrRef,
                               CORBA::Long wfReqId,
                               MetaDag* mDag) {
    char statMsg[128];
    sprintf(statMsg,"Start workflow request %ld", static_cast<long int>(wfReqId));
    stat_in("MA_DAG",statMsg);

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
	Dag * newDag = this->parseNewDag(dag_desc, itoa(dagId), mDag);
	this->myMultiWfSched->scheduleNewDag(newDag, mDag);
    } catch (...) {
	sprintf(statMsg,
		"Dag request (%ld) aborted", static_cast<long int>(dagId));
	stat_out("MA_DAG",statMsg);
	TRACE_TEXT(TRACE_ALL_STEPS, "MADAG cancelled DAG request (wfReqId = "
		   << wfReqId << ")" << std::endl);
	this->myMutex.unlock();
	throw;
    }

    this->myMutex.unlock();
    sprintf(statMsg,"End Dag request %ld", static_cast<long int>(dagId));
    stat_out("MA_DAG",statMsg);
    stat_flush();
    return dagId;
}

/**
 * Parse dag xml description and create a dag object
 */
Dag *
MaDag_impl::parseNewDag(const corba_wf_desc_t& wf_desc,
                        const string& dagId,
                        MetaDag * mDag)
    throw (MaDag::InvalidDag)
{
    // CREATION & PARSING
    Dag *newDag = new Dag(dagId);
    SingleDagParser* reader = new SingleDagParser(*newDag, wf_desc.abstract_wf);

    try {
	reader->parseXml();
    } catch (XMLParsingException& e) {
	throw (MaDag::InvalidDag(e.ErrorMsg().c_str()));
    }

    delete reader;
    // CHECK STRUCTURE
    NodeSet * contextNodeSet;
    if (!mDag) {
	contextNodeSet = (NodeSet*) newDag;
    } else {
	mDag->addDag(newDag);
	contextNodeSet = (NodeSet*) mDag;
	mDag->setCurrentDag(newDag);
    }
    try {
	newDag->checkPrec(contextNodeSet);
    } catch (WfStructException& e) {
	if (mDag)
	    mDag->setCurrentDag(0);
	throw (MaDag::InvalidDag(e.ErrorMsg().c_str()));
    }

    if (mDag) {
	// release current dag in metadag
	mDag->setCurrentDag(0);
    }

    return newDag;
}

/**
 * Get a new workflow request identifier
 */
CORBA::Long
MaDag_impl::getWfReqId()
{
    this->myMutex.lock();
    CORBA::Long res = this->wfReqIdCounter++;
    this->myMutex.unlock();
    return res;
} // end getWfReqId

/**
 * Used to test if it is alive.
 */
CORBA::Long
MaDag_impl::ping()
{
    TRACE_TEXT(TRACE_ALL_STEPS, "ping()" << std::endl);
    stat_flush();
    return getpid();
} // ping()

/**
 * Cancel a dag
 */
void
MaDag_impl::cancelDag(CORBA::Long dagId)
{
    this->myMultiWfSched->cancelDag(itoa(dagId));
}

/**
 * Get the MA
 */
MasterAgent_var
MaDag_impl::getMA() const
{
    return this->myMA;
}

/**
 * Get the client manager for a given dag id
 */
CltMan_ptr
MaDag_impl::getCltMan(const string& dagId)
{
    map<string,CORBA::Long>::iterator wfReqIter = wfReqs.find(dagId);
    if (wfReqIter != wfReqs.end()) {
	map<CORBA::Long,CltMan_ptr>::iterator cltManIter = cltMans.find(wfReqIter->second);
	if (cltManIter != cltMans.end())
	    return cltManIter->second;
    }
    return CltMan::_nil();
}

/**
 * Set the client manager for a wf request
 */
void
MaDag_impl::setCltMan(CORBA::Long wfReqId, CltMan_ptr cltMan)
{
    this->cltMans[wfReqId] = cltMan;
}

/**
 * set the wf request id for a given dag
 */
void
MaDag_impl::setWfReq(CORBA::Long dagId, CORBA::Long wfReqId)
{
    this->wfReqs[itoa(dagId)] = wfReqId;
}

/**
 * Set the platform type
 */
void
MaDag_impl::setPlatformType(MaDag::pfmType_t pfmType)
{
    switch(pfmType) {
    case (MaDag::DISTINCT_SERVICES):
	this->myMultiWfSched->setPlatformType(MultiWfScheduler::PFM_ANY);
	break;
    case (MaDag::SAME_SERVICES):
	this->myMultiWfSched->setPlatformType(MultiWfScheduler::PFM_SAME_SERVICES);
	break;
    default:
	WARNING("Wrong platform type");
  }
}

#ifdef USE_LOG_SERVICE
/**
 * Get the DietLogComponent
 */
DietLogComponent*
MaDag_impl::getDietLogComponent()
{
    return this->dietLogComponent;
}

/**
 * Set the DietLogComponent
 */
void
MaDag_impl::setupDietLogComponent()
{
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

    TRACE_TEXT(TRACE_ALL_STEPS, "LogService enabled" << std::endl);
    char* agtTypeName = NULL;
    char* agtParentName = NULL;
    char* agtName;
    std::string tmpString;
    if (CONFIG_STRING(diet::PARENTNAME, tmpString)) {
      agtParentName = strdup(tmpString.c_str());
    }

    if (CONFIG_STRING(diet::NAME, tmpString)) {
      agtName = strdup(tmpString.c_str());
    }

    // the agent names should be correct if we arrive here
    this->dietLogComponent = new DietLogComponent(agtName,
                                                  outBufferSize,
                                                  0, 0);
    
    ORBMgr::getMgr()->activate(dietLogComponent);

    agtTypeName = strdup("MA_DAG");
    if (dietLogComponent->run(agtTypeName, agtParentName, flushTime) != 0) {
      WARNING("Could not initialize DietLogComponent");
      TRACE_TEXT(TRACE_ALL_STEPS, "* LogService: disabled" << endl);
      dietLogComponent = NULL;
    }
    free(agtTypeName);
  } else {
    TRACE_TEXT(TRACE_ALL_STEPS, "LogService disabled" << endl);
    this->dietLogComponent = NULL;
  }
}
#endif

MaDagFwdrImpl::MaDagFwdrImpl(Forwarder_ptr fwdr, const char* objName) {
    this->forwarder = Forwarder::_duplicate(fwdr);
    this->objName = CORBA::string_dup(objName);
}

CORBA::Long
MaDagFwdrImpl::processDagWf(const corba_wf_desc_t& dag_desc,
			    const char* cltMgrRef,
			    CORBA::Long wfReqId)
{
    return forwarder->processDagWf(dag_desc, cltMgrRef, wfReqId, objName);
}

CORBA::Long
MaDagFwdrImpl::processMultiDagWf(const corba_wf_desc_t& dag_desc,const char* cltMgrRef,
				 CORBA::Long wfReqId,
				 CORBA::Boolean release)
{
    return forwarder->processMultiDagWf(dag_desc, cltMgrRef, wfReqId, release, objName);
}

CORBA::Long
MaDagFwdrImpl::getWfReqId()
{
    return forwarder->getWfReqId(objName);
}

void
MaDagFwdrImpl::releaseMultiDag(CORBA::Long wfReqId)
{
    forwarder->releaseMultiDag(wfReqId, objName);
}

void
MaDagFwdrImpl::cancelDag(CORBA::Long dagId)
{
    forwarder->cancelDag(dagId, objName);
}

void
MaDagFwdrImpl::setPlatformType(MaDag::pfmType_t pfmType)
{
    forwarder->setPlatformType(pfmType, objName);
}

CORBA::Long
MaDagFwdrImpl::ping()
{
    return forwarder->ping(objName);
}

