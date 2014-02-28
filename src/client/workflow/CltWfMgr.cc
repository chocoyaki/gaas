/**
 * @file  CltWfMgr.cc
 *
 * @brief   Client workflow manager class
 *
 * @author   Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "CltWfMgr.hh"
/* DIET */
#include "debug.hh"
#include "statistics.hh"
#include "ORBMgr.hh"
#include "DIET_uuid.hh"
/* DAGDA */
extern "C" {
#include "DIET_Dagda.h"
}
/* WORKFLOW UTILS */
#include "DagWfParser.hh"
#include "Dag.hh"
#include "MetaDag.hh"
#include "FWorkflow.hh"
#include "CltDagNodeLauncher.hh"
#include "events/EventManager.hh"
#include "events/EventTypes.hh"
#include "events/EventLogger.hh"
#include "WfLogDispatcher.hh"
#include "DIET_compat.hh"

#ifndef LOCK
#define LOCK {this->myLock.lock(); }
#endif

#ifndef UNLOCK
#define UNLOCK {this->myLock.unlock(); }
#endif

using namespace std;
using namespace events;


// Initialisation of static members
CltWfMgr *CltWfMgr::myInstance = NULL;
string CltWfMgr::defaultDataFileName("data.xml");

CORBA::Long
CltWfMgr::execNodeOnSed(const char *node_id,
                        const char *dag_id,
                        const char *sedName,
                        const CORBA::ULong reqID,
                        corba_estimation_t &ev) {
  return execNodeCommon(node_id, dag_id, sedName, reqID, ev);
}

CORBA::Long
CltWfMgr::execNode(const char *node_id, const char *dag_id) {
  corba_estimation_t ev;
  return execNodeCommon(node_id, dag_id, NULL, 0, ev);
}

CORBA::Long
CltWfMgr::execNodeCommon(const char *node_id,
                         const char *dag_id,
                         const char *sedName,
                         const CORBA::ULong reqID,
                         corba_estimation_t &ev) {
  SeD_var sed;
  if (sedName == NULL) {
    sed = SeD::_nil();
  } else {
    sed = ORBMgr::getMgr()->resolve<SeD, SeD_var>(SEDCTXT, sedName);
  }

  bool isSedDefined = !(sed == SeD::_nil());
  string failureReason;
  FWorkflow *wf;
  TRACE_TEXT(TRACE_MAIN_STEPS, "CltWfMgr: execute node " << node_id <<
             " of dag " << dag_id << " (SED " << (isSedDefined ? "" : "NOT")
                                                         << "DEFINED)" << endl);
  Dag *dag = this->getDag(dag_id);   // includes lock for critical section
  if (dag != NULL) {
    // DAG-LEVEL TRY BLOCK
    try {
      DagNode *node = dag->getDagNode(node_id);
      DagNodeLauncher *launcher = new CltDagNodeLauncher(node);

      if (isSedDefined) {
        launcher->setSeD(sedName, (unsigned long) reqID, ev);
      }

      // NODE-LEVEL TRY BLOCK
      try {
        node->initProfileExec();
        node->start(launcher);
        node->terminate();  // blocking

        if (!node->hasFailed()) {
          LOCK    /** LOCK (conflict with main instanciation thread) */
          // notify the workflow
          if ((wf = node->getWorkflow())) {
            TRACE_TEXT(TRACE_ALL_STEPS, "Checking pending nodes list");
            wf->handlerDagNodeDone(node);
            // if instanciator is in blocked state then unblock it
            if (wf->instanciationReady() && this->instanciationPending) {
              TRACE_TEXT(
                TRACE_MAIN_STEPS,
                "INSTANCIATION ready & instanciator blocked ==> POST" << endl);
              mySem.post();
            }
          }
          UNLOCK
          return 0;
        } else {
          failureReason = "Node failure";
        }
      } catch (WfDataException &e) {
        failureReason = "Data error: " + e.ErrorMsg();
      } // end of NODE-LEVEL TRY BLOCK
    } catch (WfStructException &e) {
      failureReason = e.ErrorMsg();
    } catch (CORBA::SystemException &e) {
      failureReason = string("CORBA ") + e._name() + string(" : ")
                      + e.NP_minorString();
    } catch (...) {
      failureReason = "Unknown exception";
    } // end of DAG-LEVEL TRY BLOCK
  } else {failureReason = "Dag not found";
  }
  cerr << "Execution of node " << dag_id << "-" << node_id << " failed! "
       << "(" << failureReason << ")" << endl;
  return 1;
} // execNodeCommon

CltWfMgr::CltWfMgr(const string &name):
  name(name), cltWfReqId(0), dagSentCount(0), mySem(0) {
  this->myMA = MasterAgent::_nil();
  this->myMaDag = MaDag::_nil();
#ifdef USE_LOG_SERVICE
  this->myLC = NULL;
#endif
  this->myLS = WfLogService::_nil();
  gettimeofday(&this->refTime, NULL);  // init reference time

  /** EVENT MGMT **/
  fstream *logfilehdl = new fstream("cltwfmgr.out", fstream::out);
  EventLogger *myLog = new EventLogger(*logfilehdl, EventBase::INFO);
  EventManager::getEventMgr()->addObserver(myLog);
}

CltWfMgr *
CltWfMgr::instance() {
  if (myInstance == NULL) {
    ostringstream os;
    char host[256];

    gethostname(host, 256);
    host[255] = '\0';
    boost::uuids::uuid uuid = diet_generate_uuid();
    os << "CltWfMgr-" << host << "-" << uuid;
    myInstance = new CltWfMgr(os.str());
    ORBMgr::getMgr()->activate(myInstance);
    ORBMgr::getMgr()->bind(WFMGRCTXT, os.str(), myInstance->_this());
    ORBMgr::getMgr()->fwdsBind(WFMGRCTXT, os.str(),
                               ORBMgr::getMgr()->getIOR(myInstance->_this()));
  }
  return myInstance;
} // instance

void
CltWfMgr::terminate() {
  if (myInstance != NULL) {
    std::string instanceName = myInstance->name;
    // ORBMgr::getMgr()->deactivate(myInstance);  // Should we deactivate the object?
    ORBMgr::getMgr()->unbind(WFMGRCTXT, instanceName);
    ORBMgr::getMgr()->fwdsUnbind(WFMGRCTXT, instanceName);
    myInstance = NULL;
  }
}

/**
 * Set the MaDag reference
 */
void
CltWfMgr::setMaDag(MaDag_var maDagRef) {
  this->myMaDag = maDagRef;
}

/**
 * Set the Master Agent reference
 */
void
CltWfMgr::setMA(MasterAgent_var ma) {
  this->myMA = ma;
}

#ifdef USE_LOG_SERVICE
/**
 * Set the log service reference
 */
void
CltWfMgr::setLogComponent(DietLogComponent *logComponent) {
  this->myLC = logComponent;
}
#endif

void
CltWfMgr::setWfLogService(WfLogService_var logService) {
  TRACE_TEXT(TRACE_MAIN_STEPS, "CltWfMgr:: log service initialized" << endl);
  this->myLS = logService;

  // INITIALIZE EVENT DISPATCHER FOR THE WFLOGSERVICE
  fstream *dispatchFilehdl = new fstream("cltwfmgr_dispatch.out", fstream::out);
  WfLogDispatcher *myObs = new WfLogDispatcher(*dispatchFilehdl);
  if (myLS != WfLogService::_nil()) {
    myObs->setWfLogService(myLS);
  }
  EventManager::getEventMgr()->addObserver(myObs);
} // setWfLogService

/**
 * Get current time relative to construction time (in milliseconds)
 */
double
CltWfMgr::getCurrTime() {
  struct timeval current_time;
  gettimeofday(&current_time, NULL);
  return (double) ((current_time.tv_sec - refTime.tv_sec) * 1000
                   + (current_time.tv_usec - refTime.tv_usec) / 1000);
}

/**
 * Get a workflow request ID (to use for multi-dag submission)
 */
CORBA::Long
CltWfMgr::getNewWfReqID() {
  CORBA::Long wfReqId = 0;
  if (this->myMaDag != MaDag::_nil()) {
    wfReqId = this->myMaDag->getWfReqId();
  }
  return wfReqId;
}

/**
 * Initialize the status for a given functional workflow
 */
void
CltWfMgr::initDagStatus(FWorkflow *wf) {
  this->allDagsSent.insert(make_pair(wf, false));
}

/**
 * Set the dag status regarding dags sent for a given functional workflow
 */
void
CltWfMgr::setWfSubmissionComplete(FWorkflow *wf) {
  this->allDagsSent[wf] = true;
}

/**
 * Get the status regarding dags sent for a given functional workflow
 */
bool
CltWfMgr::isWfSubmissionComplete(FWorkflow *wf) {
  map<FWorkflow *, bool>::iterator wfIter = this->allDagsSent.find(wf);
  if (wfIter != this->allDagsSent.end()) {
    return (bool) wfIter->second;
  } else {
    return false;
  }
}

/**
 * Execute a single dag
 */

diet_error_t
CltWfMgr::wfDagCall(diet_wf_desc_t *profile) {
  diet_error_t res(0);
  Dag *dag = new Dag(profile->name, myMA);
  if (this->myMaDag != MaDag::_nil()) {
    // Call the common dag submission with parsing = ON and release = FALSE
    if (!wfDagCallCommon(profile, dag, true, false)) {
      TRACE_TEXT(TRACE_ALL_STEPS, "Waiting for release ==> WAIT" << endl);
      this->mySem.wait();
      // WAIT UNTIL RELEASE
      if (dag->isCancelled()) {
        res = 1;
      }
      // to let the release() call terminate before end of process
      diet::usleep(1000);
    } else {
      cerr << "DAG request cancelled!" << endl;
      res = 1;
    }
  } else {
    cerr << "FATAL ERROR: MA DAG not initialized!" << endl;
    res = 1;
  }
  return res;
} // wfDagCall

/**
 * (private)
 * The common part of dag submission
 */
diet_error_t
CltWfMgr::wfDagCallCommon(diet_wf_desc_t *dagProfile, Dag *dag, bool parse,
                          bool release) {
  diet_error_t res(0);

  if (parse) {
    try {
      SingleDagParser parser(*dag, dagProfile->abstract_wf);
      parser.parseXml();
    } catch (XMLParsingException &e) {
      cerr << "DAG PARSING FATAL ERROR:" << endl;
      cerr << e.ErrorMsg() << endl;
      return 1;
    }
  } // end if (parse)

  // check the dag structure (ie each node is connected to identified nodes)
  NodeSet *nodeSet = NULL;  // set of nodes used to check precedence
  MetaDag *currMetaDag = NULL;
  CORBA::Long wfReqID = dagProfile->wfReqID;
  if (wfReqID == -1) {
    nodeSet = dag;
  } else {
    TRACE_TEXT(TRACE_ALL_STEPS, "Get Metadag for request " << wfReqID << endl);
    // MetaDag initialization
    map<CORBA::Long, MetaDag *>::iterator mDagIter = myMetaDags.find(wfReqID);
    if (mDagIter != myMetaDags.end()) {
      currMetaDag = (MetaDag *) mDagIter->second;
    } else {
      currMetaDag = new MetaDag(itoa(wfReqID));
      myMetaDags[wfReqID] = currMetaDag;
    }
    currMetaDag->addDag(dag);
    currMetaDag->setReleaseFlag(release);
    currMetaDag->setCurrentDag(dag);
    nodeSet = currMetaDag;
  }
  try {
    dag->checkPrec(nodeSet);
  } catch (WfStructException &e) {
    cerr << "FATAL ERROR: WRONG DAG STRUCTURE:" << endl;
    cerr << e.ErrorMsg() << endl;
    if (currMetaDag) {
      currMetaDag->setCurrentDag(NULL);
    }
    return 1;
  }

  if (currMetaDag) {
    currMetaDag->setCurrentDag(NULL);
  }

  // Build the dag connexions to allow retrieval of input data
  try {
    dag->linkAllPorts();
  } catch (WfStructException &e) {
    cerr << "FATAL ERROR: INVALID DAG DATA CONNECTIONS:" << endl;
    cerr << e.ErrorMsg() << endl;
    return 1;
  }

  TRACE_TEXT(TRACE_ALL_STEPS, "Marshalling the workflow description" << endl);
  corba_wf_desc_t *corba_profile = new corba_wf_desc_t;
  mrsh_wf_desc(corba_profile, dagProfile);

  TRACE_TEXT(TRACE_ALL_STEPS,
             "Try to send the workflow description to the MA_DAG ..." << endl);
  CORBA::Long dagID;

  dag->setStartTime(this->getCurrTime());

  LOCK;    /** LOCK required to include MaDag call postprocessing */
  try {
    // CORBA CALL TO MADAG
    if (wfReqID < 0) {
      wfReqID = this->myMaDag->getWfReqId();
      dagProfile->wfReqID = wfReqID;
      dagID = this->myMaDag->processDagWf(*corba_profile,
                                          myName().c_str(), wfReqID);
    } else {
      dagID = this->myMaDag->processMultiDagWf(*corba_profile,
                                               myName().c_str(), wfReqID,
                                               release);
    }
  } catch (CORBA::SystemException &e) {
    cerr << "Caught a CORBA " << e._name() << " exception ("
         << e.NP_minorString() << ")" << endl;
    dagID = -1;
  } catch (MaDag::ServiceNotFound &e) {
    cerr << "Error on MaDag missing service: " << e.pbName << "(" << e.pbParams
         << ") for node: " << e.nodeId << endl;
    dagID = -1;
  } catch (MaDag::CommProblem &e) {
    cerr << "Error on MaDag communication pb : " << e.info << endl;
    dagID = -1;
  } catch (MaDag::InvalidDag &e) {
    cerr << "Error on MaDag invalid dag : " << e.info << endl;
    dagID = -1;
  }

  // MADAG CALL POSTPROCESSING (must be done before any MaDag callback)
  if (dagID != -1) {
    TRACE_TEXT(TRACE_ALL_STEPS, " done" << endl);
    // changing Dag ID (if part of metadag then remove and re-insert)
    string oldId = dag->getId();
    dag->setId(itoa((long) dagID));
    if (currMetaDag) {
      currMetaDag->removeDag(oldId);
      currMetaDag->addDag(dag);
    }
    // update local dag counter
    dagSentCount++;
    TRACE_TEXT(TRACE_MAIN_STEPS, "DAG SENT COUNT = " << dagSentCount << endl);
    // store dag ref in profiles map (used to retrieve results)
    this->myProfiles[dagProfile] = dag;
    TRACE_TEXT(TRACE_MAIN_STEPS,
               "Dag ID " << dag->getId() << " ready to execute" << endl);
  } else {
    cerr << "FATAL ERROR: MA DAG cancelled the request" << endl;
    res = 1;
  }
  UNLOCK;  /** UNLOCK (End of critical section for MaDAG call) */

  free(dagProfile->abstract_wf);
  delete corba_profile;

  return res;
} // wfDagCallCommon

/**
 * Execute a functional workflow
 */
diet_error_t
CltWfMgr::wfFunctionalCall(diet_wf_desc_t *profile) {
  diet_error_t res(0);
  string wfName(profile->name);
  string wfId;

  // get a unique request id from MADAG
  TRACE_TEXT(TRACE_MAIN_STEPS, "*** Get wf request ID from MADAG ****" << endl);
  CORBA::Long wfReqId = 0;
  if (this->myMaDag != MaDag::_nil()) {
    wfReqId = this->myMaDag->getWfReqId();
    stringstream wfIdStr;
    wfIdStr << wfReqId;
    wfId = wfIdStr.str();
  } else {
    cerr << "FATAL ERROR: MA DAG not initialized" << endl;
    return 1;
  }

  FWorkflow *wf = new FWorkflow(wfId, wfName);

  EventManager::getEventMgr()->sendEvent(new EventCreateObject<FWorkflow,
                                                               FWorkflow>(wf,
                                                                          NULL));

  // store wf profile to allow results retrieval
  myProfiles[profile] = wf;

  // Initialize data file
  string dataFileName = defaultDataFileName;
  if (profile->dataFile) {
    dataFileName = profile->dataFile;
  }

  wf->setDataSrcXmlFile(dataFileName);

  // Read transcript file (if available)
  if (profile->transcriptFile) {
    readWorkflowExecutionTranscript(profile);
  }

  // Parse workflow description
  FWfParser reader(*wf, profile->abstract_wf);
  try {
    TRACE_TEXT(TRACE_MAIN_STEPS, "Parsing WORKFLOW XML" << endl);
    reader.parseXml(true);
    TRACE_TEXT(TRACE_MAIN_STEPS, "*** Checking WORKFLOW XML structure" << endl);
    wf->checkPrec(wf);
  } catch (XMLParsingException &e) {
    cerr << "FATAL ERROR: INVALID FUNCTIONAL WORKFLOW XML:" << endl;
    cerr << e.ErrorMsg() << endl;
    return XML_MALFORMED;
  } catch (WfStructException &e) {
    cerr << "FATAL ERROR: FUNCTIONAL WORKFLOW SYNTAX OR STRUCTURE ERROR:" <<
    endl;
    cerr << e.ErrorMsg() << endl;
    return WFL_BADSTRUCT;
  }

  TRACE_TEXT(TRACE_MAIN_STEPS, "*** Initialize functional wf ****" << endl);
  try {
    wf->initialize();
  } catch (WfStructException &e) {
    cerr << "FATAL ERROR: BAD FUNCTIONAL WORKFLOW STRUCTURE: " << endl;
    cerr << e.ErrorMsg() << endl;
    return WFL_BADSTRUCT;
  }

  initDagStatus(wf);

  TRACE_TEXT(TRACE_MAIN_STEPS, "*** INSTANCIATION ****" << endl);
  sendEventFrom<FWorkflow, FWorkflow::INST>(wf, "Start instanciation", "",
                                            EventBase::INFO);
  int dagCounter = 0;
  while (!res && !wf->instanciationCompleted() &&
         !wf->instanciationStopped()) {
    // GENERATE NEW DAG
    // set a temporary ID for the DAG
    // this ID will be changed when the DAG is submitted to the MaDag
    // the temp ID must not match any real ID (which are numbers)
    string dagId = "WF_" + wf->getId() + "_DAG_" + itoa(dagCounter++);
    Dag *currDag = new Dag(dagId, myMA);
    currDag->setWorkflow(wf);
    EventManager::getEventMgr()->sendEvent(new EventCreateObject<Dag,
                                                                 FWorkflow>(
                                             currDag, wf));

    LOCK;    /** LOCK */
    try {
      wf->instanciate(currDag);
    } catch (XMLParsingException &e) {
      cerr << "FATAL ERROR: DATA FILE Parsing exception: " << endl;
      cerr << e.ErrorMsg() << endl;
      wf->stopInstanciation();
    }
    UNLOCK;  /** UNLOCK */

    if (currDag->size() == 0) {
      TRACE_TEXT(TRACE_MAIN_STEPS, "*** GENERATED DAG IS EMPTY ***" << endl);
      sendEventFrom<Dag, Dag::EMPTY>(currDag, "Generated empty dag", "",
                                     EventBase::INFO);
      if (wf->instanciationCompleted()) {
        TRACE_TEXT(TRACE_MAIN_STEPS, "*** RELEASE request on MADAG ***" << endl);
        try {
          myMaDag->releaseMultiDag(wfReqId);
        } catch (...) {
          WARNING("Multi-dag release FAILURE");
        }
      }
    } else {
      string dagFileName = "dag_" + currDag->getId() + ".xml";

      // STORE DAG IN FILE
      TRACE_TEXT(TRACE_MAIN_STEPS,
                 "*** WRITE DAG IN FILE: " << dagFileName << endl);
      fstream filestr;
      filestr.open(dagFileName.c_str(), fstream::out);
      currDag->toXML(filestr);
      filestr.close();

      // DISPLAY DAG
      TRACE_TEXT(TRACE_ALL_STEPS,
                 "*** DISPLAY DAG " << currDag->getId()
                                    << "  ****" << endl <<
                 endl);
      if (TRACE_LEVEL >= TRACE_ALL_STEPS) {
        currDag->toXML(cout);
      }

      TRACE_TEXT(TRACE_MAIN_STEPS,
                 "*** SUBMIT DAG " << currDag->getId()
                                   << " TO MADAG ****" <<
                 endl << endl);
      diet_wf_desc_t *dagProfile = diet_wf_profile_alloc(
        dagFileName.c_str(), "testdag", DIET_WF_DAG);
      dagProfile->wfReqID = wfReqId;

      try {
        // Call the common DAG submission method
        // with PARSING option deactivated (the dag is already in memory)
        res = wfDagCallCommon(dagProfile, currDag, false,
                              wf->instanciationCompleted());
      } catch (...) {
        cerr << "Exception not caught in wfDagCallCommon!" << endl;
        res = 1;
      }

      string newDagFileName = "dag_" + currDag->getId() + ".xml";
      if (rename(dagFileName.c_str(), newDagFileName.c_str())) {
        WARNING("Error while renaming dag file");
      }
    } // end if (currDag->size() == 0)

    if (!res) {
      // If this is the last dag of the workflow instanciation, set the flag
      if (wf->instanciationCompleted()) {
        setWfSubmissionComplete(wf);
      }
      // CHECK IF INSTANCIATION IS PENDING (waiting for node execution)
      if (wf->instanciationPending()) {
        TRACE_TEXT(TRACE_MAIN_STEPS,
                   "INSTANCIATION PENDING ON NODE EXECUTION ==> WAIT" << endl);
        this->instanciationPending = true;
        this->mySem.wait();
        this->instanciationPending = false;

        // WAIT UNTIL A NODE HAS TERMINATED and UPDATED THE INSTANCIATOR

        if (wf->instanciationPending()) {
          TRACE_TEXT(TRACE_MAIN_STEPS, "INSTANCIATION RESTARTS" << endl);
        }
      }
    } else {
      cerr << "DAG SUBMISSION FAILED ==> INSTANCIATION STOPPED" << endl;
      wf->instanciationStopped();
    }
  } // while

  // Instanciation is finished
  if (wf->instanciationCompleted()) {
    sendEventFrom<FWorkflow, FWorkflow::INSTDONE>(wf, "Instanciation completed",
                                                  "",
                                                  EventBase::INFO);
  } else {
    sendEventFrom<FWorkflow, FWorkflow::INSTERROR>(wf, "Instanciation error",
                                                   "",
                                                   EventBase::NOTICE);
  }

  if (dagSentCount > 0) {
    TRACE_TEXT(TRACE_MAIN_STEPS, "NO MORE DAGS TO INSTANCIATE ==> WAIT" << endl);
    this->mySem.wait();
    // to avoid stopping process before end of release call
    diet::usleep(1000);
  }
  if (!wf->instanciationCompleted()) {
    cerr << "FUNCTIONAL WORKFLOW INSTANCIATION or EXECUTION FAILED!" << endl;
    res = 1;
    // TRACE_TEXT(TRACE_MAIN_STEPS, "*** RELEASE request on MADAG ***" << endl);
    // try {
    // myMaDag->releaseMultiDag(wfReqId);
    // } catch (...) {
    // WARNING("Multi-dag release FAILURE");
    // }
    sendEventFrom<FWorkflow, FWorkflow::CANCELLED>(wf, "Workflow cancelled", "",
                                                   EventBase::NOTICE);
  } else {
    sendEventFrom<FWorkflow, FWorkflow::COMPLETED>(wf, "Workflow completed", "",
                                                   EventBase::INFO);
  }
  wf->displayDagSummary(cout);
  // myMA->getDataManager()->checkpointState();
  return res;
} // wfFunctionalCall

/**
 * Release the waiting semaphore
 */
char *
CltWfMgr::release(const char *dag_id, bool successful) {
  Dag *dag = getDag(dag_id);
  FWorkflow *wf = NULL;
  try {
    wf = dag->getWorkflow();
  } catch (WfStructException &e) {
  }
  string traceHeader = "Release [" + string(dag_id) + "]: ";

  if (dag == NULL) {
    throw CltMan::DagNotFound(dag_id);
  }
  // SET DAG STATUS
  dag->setFinishTime(this->getCurrTime());
  if (!successful) {
    cerr << traceHeader << "cancelled by MaDag!" << endl;
    dag->setAsCancelled();
    if (wf) {
      LOCK;    /** LOCK */
      wf->stopInstanciation();  // stop instanciation
      UNLOCK;  /** UNLOCK */
    }
  }

  // RETURN MESSAGE CONTAINING REQUEST IDs (FOR VIZDIET)
  vector<diet_reqID_t> diet_request_ids = dag->getAllDietReqID();
  TRACE_TEXT(TRACE_ALL_STEPS, traceHeader << "got request ids (size = "
                                          << diet_request_ids.size() << ")" <<
             endl);
  stringstream message;
  message << dag_id;
  // Add request IDs to message
  for (unsigned int ix = 0; ix < diet_request_ids.size(); ix++) {
    message << ";" << diet_request_ids[ix];
  }
  // Add makespan to message
  message << "#" << dag->getMakespan();

  TRACE_TEXT(TRACE_ALL_STEPS, traceHeader << "message: |" << message.str()
                                          << "| " << message.str().size() <<
             endl);
  char *ret = new char[message.str().size()+1];//(char *) malloc(message.str().size() * sizeof(char) + 1);
  sprintf(ret, "%s", message.str().c_str());

  // UPDATE DAG COUNTER
  LOCK;    /** LOCK */
  dagSentCount--;
  TRACE_TEXT(TRACE_MAIN_STEPS,
             traceHeader << "DAG SENT COUNT = " << dagSentCount << endl);

  if (dagSentCount == 0) {
    if (!wf || isWfSubmissionComplete(wf) || wf->instanciationStopped()) {
      TRACE_TEXT(TRACE_MAIN_STEPS,
                 traceHeader << "No more dags running ==> POST" << endl);
      UNLOCK;  /** UNLOCK */
      this->mySem.post();
    } else {
      TRACE_TEXT(TRACE_MAIN_STEPS,
                 traceHeader << "Still some dags to send ==> CONTINUE" << endl);
      UNLOCK;  /** UNLOCK */
    }
  } else {
    TRACE_TEXT(TRACE_MAIN_STEPS,
               traceHeader << "Still some dags running ==> CONTINUE" << endl);
    UNLOCK;  /** UNLOCK */
  }
  return ret;
} // release

/**
 * Cancel a dag
 */
diet_error_t
CltWfMgr::cancelDag(const char *dagId) {
  try {
    myMaDag->cancelDag(atoi(dagId));
  } catch (MaDag::InvalidDag &e) {
    cerr << "Error on MaDag invalid dag : " << e.info << endl;
    return 1;
  }
  return 0;
}

/**
 * Print all results of a dag
 */
diet_error_t
CltWfMgr::printAllDagResults(diet_wf_desc_t *profile) {
  map<diet_wf_desc_t *, NodeSet *>::iterator nsp = myProfiles.find(profile);
  if (nsp != myProfiles.end()) {
    Dag *dag = dynamic_cast<Dag *>(nsp->second);
    if (dag != NULL) {
      if (!dag->isCancelled()) {
        try {
          dag->displayAllResults(cout);
          return 0;
        } catch (WfDataException &e) {
          cerr << "Data error: " << e.ErrorMsg() << endl;
        }
      } else {
        cerr << "** DAG " << dag->getId()
             << " was cancelled => no results **" << endl;
      }
    }
  }
  return 1;
} // printAllDagResults

/**
 * Print all results of a functional wf
 */
diet_error_t
CltWfMgr::printAllFunctionalWfResults(diet_wf_desc_t *profile) {
  map<diet_wf_desc_t *, NodeSet *>::iterator nsp = myProfiles.find(profile);
  if (nsp != myProfiles.end()) {
    FWorkflow *wf = dynamic_cast<FWorkflow *>(nsp->second);
    if (wf != NULL) {
      try {
        wf->downloadSinkData();
        wf->displayAllResults(cout);
        return 0;
      } catch (WfDataException &e) {
        cerr << "Data error: " << e.ErrorMsg() << endl;
      }
    }
  }
  return 1;
} // printAllFunctionalWfResults

diet_error_t
CltWfMgr::readWorkflowExecutionTranscript(diet_wf_desc_t *profile) {
  // check if file exists
  ifstream inFile;
  inFile.exceptions(ifstream::failbit | ifstream::badbit);
  try {
    inFile.open(profile->transcriptFile);
    inFile.close();
  } catch (...) {
    TRACE_TEXT(TRACE_MAIN_STEPS,
               "Transcript file: file not found => use for output only" << endl);
    return 0;
  }
  // find workflow & parse file
  map<diet_wf_desc_t *, NodeSet *>::iterator nsp = myProfiles.find(profile);
  if (nsp != myProfiles.end()) {
    FWorkflow *wf = dynamic_cast<FWorkflow *>(nsp->second);
    if (wf != NULL) {
      try {
        MultiDagParser dagsParser(string(profile->transcriptFile));
        dagsParser.setWorkflow(wf);
        dagsParser.parseXml(false);
        wf->readDagsState(dagsParser.getDags());
      } catch (XMLParsingException &e) {
        cerr << __FUNCTION__ << " : file read error :" << e.ErrorMsg() << endl;
        return 1;
      }
      return 0;
    }
  }
  return 1;
} // readWorkflowExecutionTranscript

diet_error_t
CltWfMgr::saveWorkflowExecutionTranscript(diet_wf_desc_t *profile,
                                          const char *transcriptFileName) {
  map<diet_wf_desc_t *, NodeSet *>::iterator nsp = myProfiles.find(profile);
  if (nsp != myProfiles.end()) {
    FWorkflow *wf = dynamic_cast<FWorkflow *>(nsp->second);
    if (wf != NULL) {
      // open the file
      ofstream outFile;
      outFile.exceptions(ofstream::failbit | ofstream::badbit);
      try {
        outFile.open(transcriptFileName, ofstream::trunc);
      } catch (...) {
        cerr << __FUNCTION__ << " : file open failed" << endl;
        return 1;
      }
      // write the workflow transcript
      wf->writeAllDagsState(outFile);
      // close the file
      try {
        outFile.close();
      } catch (...) {
        cerr << __FUNCTION__ << " : file close failed" << endl;
        return 1;
      }
      return 0;
    }
  }
  return 1;
} // saveWorkflowExecutionTranscript

diet_error_t
CltWfMgr::saveWorkflowDataFile(diet_wf_desc_t *profile,
                               const char *dataFileName) {
  map<diet_wf_desc_t *, NodeSet *>::iterator nsp = myProfiles.find(profile);
  if (nsp != myProfiles.end()) {
    FWorkflow *wf = dynamic_cast<FWorkflow *>(nsp->second);
    if (wf != NULL) {
      // open the file
      ofstream outFile;
      outFile.exceptions(ofstream::failbit | ofstream::badbit);
      try {
        outFile.open(dataFileName, ofstream::trunc);
      } catch (...) {
        cerr << __FUNCTION__ << " : file open failed" << endl;
        return 1;
      }
      try {
        // update data IDs and values
        wf->downloadSinkData();
        // write the workflow transcript
        wf->writeAllSourcesAndSinksData(outFile);
        // close the file
      } catch (WfDataException &e) {
        // Do something?
      }
      try {
        outFile.close();
      } catch (...) {
        cerr << __FUNCTION__ << " : file close failed" << endl;
        return 1;
      }
      return 0;
    }
  }
  return 1;
} // saveWorkflowDataFile

/**
 * Get a result from the workflow
 * TODO catch WfStructException
 */
int
CltWfMgr::getWfOutputScalar(diet_wf_desc_t *profile,
                            const char *id,
                            void **value) {
  map<diet_wf_desc_t *, NodeSet *>::iterator nsp = myProfiles.find(profile);
  if (nsp != myProfiles.end()) {
    Dag *dag = dynamic_cast<Dag *>(nsp->second);
    if (dag != NULL) {
      return dag->get_scalar_output(id, value);
    } else {
      return 1;
    }
  } else {
    return 1;
  }
} // end getWfOutputScalar

int
CltWfMgr::getWfOutputString(diet_wf_desc_t *profile,
                            const char *id,
                            char **value) {
  map<diet_wf_desc_t *, NodeSet *>::iterator nsp = myProfiles.find(profile);
  if (nsp != myProfiles.end()) {
    Dag *dag = dynamic_cast<Dag *>(nsp->second);
    if (dag != NULL) {
      return dag->get_string_output(id, value);
    } else {
      return 1;
    }
  } else {
    return 1;
  }
} // end getWfOutputString

int
CltWfMgr::getWfOutputFile(diet_wf_desc_t *profile,
                          const char *id,
                          size_t *size, char **path) {
  map<diet_wf_desc_t *, NodeSet *>::iterator nsp = myProfiles.find(profile);
  if (nsp != myProfiles.end()) {
    Dag *dag = dynamic_cast<Dag *>(nsp->second);
    if (dag != NULL) {
      return dag->get_file_output(id, size, path);
    } else {
      return 1;
    }
  } else {
    return 1;
  }
} // getWfOutputFile

int
CltWfMgr::getWfOutputMatrix(diet_wf_desc_t *profile,
                            const char *id,
                            void **value,
                            size_t *nb_rows,
                            size_t *nb_cols,
                            diet_matrix_order_t *order) {
  map<diet_wf_desc_t *, NodeSet *>::iterator nsp = myProfiles.find(profile);
  if (nsp != myProfiles.end()) {
    Dag *dag = dynamic_cast<Dag *>(nsp->second);
    if (dag != NULL) {
      return dag->get_matrix_output(id, value, nb_rows, nb_cols, order);
    } else {
      return 1;
    }
  } else {
    return 1;
  }
} // getWfOutputMatrix

int
CltWfMgr::getWfOutputContainer(diet_wf_desc_t *profile,
                               const char *id,
                               char **dataID) {
  map<diet_wf_desc_t *, NodeSet *>::iterator nsp = myProfiles.find(profile);
  if (nsp != myProfiles.end()) {
    Dag *dag = dynamic_cast<Dag *>(nsp->second);
    if (dag != NULL) {
      return dag->get_container_output(id, dataID);
    } else {
      return 1;
    }
  } else {
    return 1;
  }
} // getWfOutputContainer

int
CltWfMgr::getWfSinkContainer(diet_wf_desc_t *profile,
                             const char *id,
                             char **dataID) {
  map<diet_wf_desc_t *, NodeSet *>::iterator nsp = myProfiles.find(profile);
  if (nsp == myProfiles.end()) {
    return 1;
  }
  FWorkflow *wf = dynamic_cast<FWorkflow *>(nsp->second);
  if (wf == NULL) {
    return 1;
  }
  try {
    string dataIDStr;
    wf->getSinkContainer(id, dataIDStr);
    *dataID = strdup(dataIDStr.c_str());
  } catch (const WfStructException &e) {
    WARNING("Cannot get sink results: " << e.ErrorMsg() << endl);
    return 1;
  } catch (const WfDataException &e) {
    WARNING("Invalid sink result: " << e.ErrorMsg() << endl);
    return 1;
  }
  return 0;
} // getWfSinkContainer

/**
 * terminate a workflow session and free the memory
 */
void
CltWfMgr::wf_free(diet_wf_desc_t *profile) {
  char statMsg[64];
  map<diet_wf_desc_t *, NodeSet *>::iterator nsp = myProfiles.find(profile);
  if (nsp != myProfiles.end()) {
    if (profile->level == DIET_WF_DAG) {
      Dag *dag = dynamic_cast<Dag *>(nsp->second);
      if (dag != NULL) {
        sprintf(statMsg, "%s DAG %s", __FUNCTION__, dag->getId().c_str());
        stat_in("cltwfmgr", statMsg);
        dag->deleteAllResults();
        if (profile->wfReqID) {
          map<CORBA::Long, MetaDag *>::iterator mDagIter = myMetaDags.find(
            profile->wfReqID);
          if (mDagIter != myMetaDags.end()) {
            MetaDag *metaDag = (MetaDag *) mDagIter->second;
            metaDag->removeDag(dag->getId());
            if (metaDag->getDagNb() == 0) {
              delete metaDag;
            }
          }
        } else {
          delete dag;
        }
      }
    } else if (profile->level == DIET_WF_FUNCTIONAL) {
      FWorkflow *wf = dynamic_cast<FWorkflow *>(nsp->second);
      if (wf != NULL) {
        sprintf(statMsg, "%s WF %s", __FUNCTION__, wf->getId().c_str());
        stat_in("cltwfmgr", statMsg);
        wf->deleteAllResults();
        wf->deleteAllDags();
        wf->deleteAllInputData(myMA);
        wf->deleteAllIntermediateData(myMA);
        delete wf;
      }
    }
    this->myProfiles.erase(profile);
    // delete profile->abstract_wf;
    delete profile;
    stat_out("cltwfmgr", statMsg);
  } else {
    WARNING("wf_free : profile not found");
  }

  // dagda_save_platform();
} // wf_free

/**
 * Return the object IOR
 */
const string &
CltWfMgr::myName() const {
  return name;
}

/**
 * Debug function
 */
CORBA::Long
CltWfMgr::ping() {
  TRACE_TEXT(TRACE_ALL_STEPS, "ping!!!!!!!!!!!!!!" << endl);
  return 0;
} // end ping

/**
 * Return the DAG with a given identifier
 * Protected against concurrent access due to simultaneous dag creation and
 * execution
 */
Dag *
CltWfMgr::getDag(string dag_id) {
  Dag *dag = NULL;
  LOCK    /** LOCK */
  for (map<diet_wf_desc_t *, NodeSet *>::iterator p = this->myProfiles.begin();
       p != this->myProfiles.end();
       ++p) {
    dag = dynamic_cast<Dag *>(p->second);   // myProfiles contains Dags AND FWorkflows
    if ((dag != NULL) &&
        (dag->getId() == dag_id)) {
      break;
    }
  }
  UNLOCK  /** UNLOCK */
  return dag;
} // getDag

/* Forwarder part. */

/* Constructor takes the object name and the forwarder to use to
 * contact the object.
 */
CltWfMgrFwdr::CltWfMgrFwdr(Forwarder_ptr fwdr, const char *objName) {
  this->forwarder = Forwarder::_duplicate(fwdr);
  this->objName = CORBA::string_dup(objName);
}

/* Each method simply calls the forwarder correponding method. */
/* i.e: fun(a, b, ...) => forwarder->fun(a, b, ..., objName) */
CORBA::Long
CltWfMgrFwdr::execNodeOnSed(const char *node_id,
                            const char *dag_id,
                            const char *sed,
                            const CORBA::ULong reqID,
                            corba_estimation_t &ev) {
  return forwarder->execNodeOnSed(node_id, dag_id, sed, reqID, ev, objName);
}

CORBA::Long
CltWfMgrFwdr::execNode(const char *node_id, const char *dag_id) {
  return forwarder->execNode(node_id, dag_id, objName);
}

char *
CltWfMgrFwdr::release(const char *dag_id, bool successful) {
  return forwarder->release(dag_id, successful, objName);
}

CORBA::Long
CltWfMgrFwdr::ping() {
  return forwarder->ping(objName);
}
