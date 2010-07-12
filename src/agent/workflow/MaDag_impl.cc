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
#include "Parsers.hh"
#include "statistics.hh"

#include "HEFTScheduler.hh"
#include "MultiWfBasicScheduler.hh"
#include "MultiWfFOFT.hh"
#include "MultiWfHEFT.hh"
#include "MultiWfSRPT.hh"
#include "MultiWfFCFS.hh"
#include "debug.hh"

using namespace std;
using namespace madag;

MaDag_impl::MaDag_impl(const char * name,
                       const MaDagSchedType schedType,
                       const int interRoundDelay) :
myName(name), myMultiWfSched(NULL), wfReqIdCounter(0), dagIdCounter(0) {
  char* MAName = (char*)
	Parsers::Results::getParamValue(Parsers::Results::PARENTNAME);
	
  // check if the parent is NULL
  if (MAName == NULL) {
    ERROR_EXIT("MA name not provided");
  }
  CORBA::Object_var obj = ORBMgr::getMgr()->resolveObject(AGENTCTXT, MAName);
  this->myMA = MasterAgent::_duplicate(MasterAgent::_narrow(obj));
  if (CORBA::is_nil(this->myMA)) {
    ERROR_EXIT("Cannot locate the master agent " << MAName);
  }
	
  /* Bind the MA DAG to its name in the CORBA Naming Service */
	try {
		ORBMgr::getMgr()->bind(MADAGCTXT, this->myName, _this());
		ORBMgr::getMgr()->fwdsBind(MADAGCTXT, this->myName,
															 ORBMgr::getMgr()->getIOR(_this()));
	} catch (...) {
		ERROR_EXIT("could not declare myself as " << this->myName);
	}
	
  TRACE_TEXT(TRACE_MAIN_STEPS,
						 endl <<  "MA DAG " << this->myName << " created." << endl);
	
  TRACE_TEXT(NO_TRACE,
						 "## MADAG_IOR " << ORBMgr::getMgr()->getIOR(this->_this()) << endl);
	
	this->setupDietLogComponent();
	char* scheduler_type;
  // starting the multiwfscheduler
  switch (schedType) {
    case BASIC:
      this->myMultiWfSched = new MultiWfBasicScheduler(this);
      if (this->dietLogComponent != NULL) {
      	scheduler_type=strdup("BASIC");
      	dietLogComponent->maDagSchedulerType(scheduler_type);
      	free(scheduler_type);
      }
      break;
    case GHEFT:
      this->myMultiWfSched = new MultiWfHEFT(this);
      if (this->dietLogComponent != NULL) {
      	scheduler_type=strdup("GLOBAL_HEFT");
      	dietLogComponent->maDagSchedulerType(scheduler_type);
      	free(scheduler_type);
      }
      break;
    case FOFT:
      this->myMultiWfSched = new MultiWfFOFT(this);
      if (this->dietLogComponent != NULL) {
      	scheduler_type=strdup("FOFT");
      	dietLogComponent->maDagSchedulerType(scheduler_type);
      	free(scheduler_type);
      }
      break;
    case GAHEFT:
      this->myMultiWfSched = new MultiWfAgingHEFT(this);
      if (this->dietLogComponent != NULL) {
      	scheduler_type=strdup("GLOBAL_AGING_HEFT");
      	dietLogComponent->maDagSchedulerType(scheduler_type);
      	free(scheduler_type);
      }
      break;
    case SRPT:
      this->myMultiWfSched = new MultiWfSRPT(this);
      if (this->dietLogComponent != NULL) {
      	scheduler_type=strdup("SRPT");
      	dietLogComponent->maDagSchedulerType(scheduler_type);
      	free(scheduler_type);
      }
      break;
    case FCFS:
      this->myMultiWfSched = new MultiWfFCFS(this);
      if (this->dietLogComponent != NULL) {
      	scheduler_type=strdup("FCFS");
      	dietLogComponent->maDagSchedulerType(scheduler_type);
      	free(scheduler_type);
      }
      break;
  }
	
  if (interRoundDelay >= 0)
    this->myMultiWfSched->setInterRoundDelay(interRoundDelay);
	
  this->myMultiWfSched->start();
  TRACE_TEXT(TRACE_ALL_STEPS, "InterRoundDelay= " <<
						 this->myMultiWfSched->getInterRoundDelay() << endl);
  // init the statistics module
  stat_init();
} // end MA DAG constructor

MaDag_impl::~MaDag_impl() {
  if (this->myMultiWfSched != NULL) {
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
  TRACE_TEXT(TRACE_ALL_STEPS, "%%%%% MADAG receives a SINGLE DAG request (wfReqId = "
						 << wfReqId << ")" << endl);
  return processDagWfCommon(dag_desc, cltMgrRef, wfReqId);
}

/**
 * Multi DAG Workflow processing
 */
CORBA::Long
MaDag_impl::processMultiDagWf(const corba_wf_desc_t& dag_desc, const char* cltMgrRef,
                              CORBA::Long wfReqId, CORBA::Boolean release)
{
  TRACE_TEXT(TRACE_ALL_STEPS, "%%%%% MADAG receives a MULTIPLE DAG request (wfReqId = "
						 << wfReqId << " / release=" << release << ")" << endl);
  // Check if a MetaDag already exists for this wf request (or create one)
  MetaDag* mDag = NULL;
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
MaDag_impl::releaseMultiDag(CORBA::Long wfReqId) {
  TRACE_TEXT(TRACE_ALL_STEPS, "%%%%% MADAG receives a RELEASE request (wfReqId = "
						 << wfReqId << ")" << endl);
  MetaDag* mDag = NULL;
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
	
  // Process the request ie merge dag into the global workflow managed by the MaDag
  CORBA::Long dagId = dagIdCounter++;
  setWfReq(dagId, wfReqId);
  try {
    Dag * newDag = this->parseNewDag(dag_desc, itoa(dagId), mDag);
    this->myMultiWfSched->scheduleNewDag(newDag, mDag);
  }
  catch (...) {
    sprintf(statMsg,"Dag request (%ld) aborted", static_cast<long int>(dagId));
    stat_out("MA_DAG",statMsg);
    TRACE_TEXT(TRACE_ALL_STEPS, "MADAG cancelled DAG request (wfReqId = "
							 << wfReqId << ")" << endl);
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
throw (MaDag::InvalidDag) {
  // CREATION & PARSING
  Dag *        newDag = new Dag();
  SingleDagParser* reader = new SingleDagParser(*newDag, wf_desc.abstract_wf);
	
  try {
    reader->parseXml();
  } catch (XMLParsingException& e) {
    throw (MaDag::InvalidDag(e.ErrorMsg().c_str()));
  }
	
  delete reader;
  newDag->setId(dagId);
	
  // CHECK STRUCTURE
  NodeSet * contextNodeSet;
  if (mDag == NULL) {
    contextNodeSet = (NodeSet*) newDag;
  } else {
    mDag->addDag(newDag);
    contextNodeSet = (NodeSet*) mDag;
    mDag->setCurrentDag(newDag);
  }
  try {
    newDag->checkPrec(contextNodeSet);
  } catch (WfStructException& e) {
    if (mDag != NULL)
      mDag->setCurrentDag(NULL);
    throw (MaDag::InvalidDag(e.ErrorMsg().c_str()));
  }
	
  if (mDag != NULL) {
    // release current dag in metadag
    mDag->setCurrentDag(NULL);
  }
	
  return newDag;
}

/**
 * Get a new workflow request identifier
 */
CORBA::Long
MaDag_impl::getWfReqId() {
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
  TRACE_TEXT(TRACE_ALL_STEPS, "ping()" << endl);
  stat_flush();
  return getpid();
} // ping()

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
MaDag_impl::getCltMan(const string& dagId) {
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

/**
 * Get the DietLogComponent
 */
DietLogComponent*
MaDag_impl::getDietLogComponent(){
	return this->dietLogComponent;
}

/**
 * Set the DietLogComponent
 */
void
MaDag_impl::setupDietLogComponent(){
	
  /* Create the DietLogComponent for use with LogService */
  bool useLS;
	// size_t --> unsigned int
  unsigned int* ULSptr;
  int outBufferSize;
	// size_t --> unsigned int
  unsigned int* OBSptr;
  int flushTime;
	// size_t --> unsigned int
  unsigned int* FTptr;
	
	// size_t --> unsigned int
  ULSptr = (unsigned int*)Parsers::Results::getParamValue(
																													Parsers::Results::USELOGSERVICE);
  useLS = false;
  if (ULSptr == NULL) {
    WARNING(" useLogService not configured. Disabled by default" << endl);
  } else {
    if (*ULSptr) {
      useLS = true;
    }
  }
	
  if (useLS) {
    // size_t --> unsigned int
    OBSptr = (unsigned int*)Parsers::Results::getParamValue(
																														Parsers::Results::LSOUTBUFFERSIZE);
    if (OBSptr != NULL) {
      outBufferSize = (int)(*OBSptr);
    } else {
      outBufferSize = 0;
      WARNING("lsOutbuffersize not configured, using default");
    }
    // size_t --> unsigned int
    FTptr = (unsigned int*)Parsers::Results::getParamValue(
																													 Parsers::Results::LSFLUSHINTERVAL);
    if (FTptr != NULL) {
      flushTime = (int)(*FTptr);
    } else {
      flushTime = 10000;
      WARNING("lsFlushinterval not configured, using default");
    }
  }
	
  if (useLS) {
    TRACE_TEXT(TRACE_ALL_STEPS, "LogService enabled" << endl);
    char* agtTypeName;
    char* agtParentName;
    char* agtName;
    agtParentName = (char*)Parsers::Results::getParamValue
		(Parsers::Results::PARENTNAME);
    agtName =       (char*)Parsers::Results::getParamValue
		(Parsers::Results::NAME);
    // the agent names should be correct if we arrive here
		
    this->dietLogComponent = new DietLogComponent(agtName, outBufferSize);
    ORBMgr::getMgr()->activate(dietLogComponent);
		
    agtTypeName = strdup("MA_DAG");
    if (dietLogComponent->run(agtTypeName, agtParentName, flushTime) != 0) {
			
      WARNING("Could not initialize DietLogComponent");
      dietLogComponent = NULL; // this should not happen;
    }
		
    free(agtTypeName);
		
  } else {
    TRACE_TEXT(TRACE_ALL_STEPS, "LogService disabled" << endl);
    this->dietLogComponent = NULL;
  }
}

MaDagFwdrImpl::MaDagFwdrImpl(Forwarder_ptr fwdr, const char* objName) {
	this->forwarder = Forwarder::_duplicate(fwdr);
	this->objName = CORBA::string_dup(objName);
}

CORBA::Long MaDagFwdrImpl::processDagWf(const corba_wf_desc_t& dag_desc,
																				const char* cltMgrRef,
																				CORBA::Long wfReqId)
{
	return forwarder->processDagWf(dag_desc, cltMgrRef, wfReqId, objName);
}

CORBA::Long MaDagFwdrImpl::processMultiDagWf(const corba_wf_desc_t& dag_desc,
																						 const char* cltMgrRef,
																						 CORBA::Long wfReqId, CORBA::Boolean release)
{
	return forwarder->processMultiDagWf(dag_desc, cltMgrRef, wfReqId, release, objName);
}

CORBA::Long MaDagFwdrImpl::getWfReqId() {
	return forwarder->getWfReqId(objName);
}

void MaDagFwdrImpl::releaseMultiDag(CORBA::Long wfReqId) {
	forwarder->releaseMultiDag(wfReqId, objName);
}

void MaDagFwdrImpl::cancelDag(CORBA::Long dagId) {
	forwarder->cancelDag(dagId, objName);
}

void MaDagFwdrImpl::setPlatformType(MaDag::pfmType_t pfmType) {
	forwarder->setPlatformType(pfmType, objName);
}

CORBA::Long	MaDagFwdrImpl::ping() {
	return forwarder->ping(objName);
}

