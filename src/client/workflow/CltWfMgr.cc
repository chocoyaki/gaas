/****************************************************************************/
/* Client workflow manager class                                            */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.30  2009/07/07 08:58:32  bisnard
 * changed execNode to handle sub-workflows
 * modified FWorkflow instanciate method signature (create Dag before)
 *
 * Revision 1.29  2009/06/23 09:19:34  bisnard
 * use new classname for WfLogService
 * added missing exception catch in execNodeCommon
 *
 * Revision 1.28  2009/05/15 10:57:45  bisnard
 * minor changes for instanciation of conditionals (if)
 *
 * Revision 1.27  2009/04/17 08:49:32  bisnard
 * updated exception handling
 *
 * Revision 1.26  2009/02/11 07:50:42  bisnard
 * remove debug messages
 *
 * Revision 1.25  2009/02/06 14:51:27  bisnard
 * - setup exceptions
 * - refactoring dag and functional wf submission
 *
 * Revision 1.24  2009/01/22 09:01:08  bisnard
 * added client method to retrieve workflow container output
 *
 * Revision 1.23  2009/01/16 16:32:26  bisnard
 * added option to specify data source file name
 *
 * Revision 1.22  2009/01/16 13:51:10  bisnard
 * corrected synchronization pbs between dag instanciation and execution
 *
 * Revision 1.21  2008/12/09 12:09:03  bisnard
 * added parameters to dag submit method to handle inter-dependent dags
 *
 * Revision 1.20  2008/12/02 14:17:48  bisnard
 * manage multi-dag cancellation when one dag fails
 *
 * Revision 1.19  2008/12/02 10:19:14  bisnard
 * functional workflow submission to MaDag
 *
 * Revision 1.18  2008/11/08 19:12:38  bdepardo
 * A few warnings removal
 *
 * Revision 1.17  2008/10/20 07:56:48  bisnard
 * new classes XML parser (Dagparser,FWfParser)
 *
 * Revision 1.16  2008/10/14 13:24:53  bisnard
 * use new class structure for dags (DagNode,DagNodePort)
 *
 * Revision 1.15  2008/09/10 09:10:18  bisnard
 * removed alloc parameter for DagWfParser
 *
 * Revision 1.14  2008/09/02 07:37:08  rbolze
 * add some stat infos
 * correct the dagId output in the stats file
 *
 * Revision 1.13  2008/07/12 00:24:49  rbolze
 * add stat info when the persistent data are released
 * add stat info about the workflow execution.
 *
 * Revision 1.12  2008/07/11 08:35:47  bisnard
 * Added exclusion blocks to avoid dag not found error
 *
 * Revision 1.11  2008/07/08 11:14:21  bisnard
 * Add dag makespan value in release message
 *
 * Revision 1.10  2008/07/07 09:41:30  bisnard
 * wf_call returns error when dag was cancelled due to node exec failure
 *
 * Revision 1.9  2008/06/26 15:00:18  bisnard
 * corba type mismatch
 *
 * Revision 1.8  2008/06/25 09:59:39  bisnard
 * new params in execNodeOnSeD to provide ReqId and estimation vector
 * to client for solve request
 *
 * Revision 1.7  2008/06/04 07:52:36  bisnard
 * SeD mapping done by MaDag just before node execution
 *
 * Revision 1.6  2008/06/02 08:34:20  bisnard
 * Execute method (for wf node) now returns an error code in case of
 * communication failure with the SeD
 *
 * Revision 1.5  2008/06/01 15:50:59  rbolze
 * less verbose
 *
 * Revision 1.4  2008/06/01 14:06:58  rbolze
 * replace most ot the cout by adapted function from debug.cc
 * there are some left ...
 *
 * Revision 1.3  2008/06/01 09:20:37  rbolze
 * the remote method release now return a string which contains
 * feedback on the dag execution
 *
 * Revision 1.2  2008/05/16 12:32:10  bisnard
 * API function to retrieve all workflow results
 *
 * Revision 1.1  2008/04/30 07:26:10  bisnard
 * moved CltWfMgr into agent/workflow directory
 *
 * Revision 1.6  2008/04/28 12:08:16  bisnard
 * obsolete init_wf_call
 * changed constructor for Node (new param wfReqId)
 *
 * Revision 1.5  2008/04/21 14:31:45  bisnard
 * moved common multiwf routines from derived classes to MultiWfScheduler
 * use wf request identifer instead of dagid to reference client
 * use nodeQueue to manage multiwf scheduling
 * renamed WfParser as DagWfParser
 *
 * Revision 1.4  2008/04/15 14:20:20  bisnard
 * - Postpone sed mapping until wf node is executed
 *
 * Revision 1.3  2008/04/14 13:45:11  bisnard
 * - Removed wf mono-mode submit
 * - Renamed submit_wf in processDagWf
 *
 * Revision 1.2  2008/04/14 09:10:40  bisnard
 *  - Workflow rescheduling (CltReoMan) no longer used with MaDag v2
 *  - AbstractWfSched and derived classes no longer used with MaDag v2
 *
 * Revision 1.1  2008/04/10 08:38:50  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 ****************************************************************************/

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "CltWfMgr.hh"
/* DIET */
#include "debug.hh"
#include "statistics.hh"
#include "ORBMgr.hh"
/* WORKFLOW UTILS */
#include "DagWfParser.hh"
#include "FWorkflow.hh"

using namespace std;


// Initialisation of static members
CltWfMgr * CltWfMgr::myInstance = NULL;
string CltWfMgr::defaultDataFileName("data.xml");

CORBA::Long
CltWfMgr::execNodeOnSed(const char * node_id,
                        const char * dag_id,
                        _objref_SeD* sed,
                        const CORBA::ULong reqID,
                        corba_estimation_t& ev) {
  return execNodeCommon(node_id, dag_id, sed, reqID, ev);
}

CORBA::Long
CltWfMgr::execNode(const char * node_id, const char * dag_id) {
  SeD_var sed = SeD::_nil();
  corba_estimation_t ev;
  return execNodeCommon(node_id, dag_id, sed, 0, ev);
}

CORBA::Long
CltWfMgr::execNodeCommon(const char * node_id,
                        const char * dag_id,
                        _objref_SeD* sed,
                        const CORBA::ULong reqID,
                        corba_estimation_t& ev) {
  bool isSedDefined = !(sed == SeD::_nil());
  string failureReason;
  FWorkflow* wf;
  TRACE_TEXT (TRACE_MAIN_STEPS,"CltWfMgr: execute node " << node_id <<
              " of dag " << dag_id << " (SED " << (isSedDefined ? "" : "NOT")
              << "DEFINED)" << endl);
  Dag * dag = this->getDag(dag_id); // includes lock for critical section
  if (dag != NULL) {
    // DAG-LEVEL TRY BLOCK
    try {

      DagNode *node = dag->getDagNode(node_id);

      string SeDHostName = "";
      if (isSedDefined) {
        SeD_var sed_var = SeD::_narrow(sed);
//         SeDHostName = sed_var->getHostname();
        node->setSeD(sed_var, (unsigned long) reqID, ev);
      }

//       if (this->myWfLogService != WfLogService::_nil()) {
//         myWfLogService->nodeIsRunning(node_id, SeDHostName.c_str());
//       }

      // NODE-LEVEL TRY BLOCK
      try {
        node->initProfileExec();
        node->start(true);

        if (!node->hasFailed()) {
          this->myLock.lock();    /** LOCK (conflict with main instanciation thread) */
          // notify the workflow (if applicable and if wf is pending)
          if ((wf = node->getWorkflow()) && (wf->instanciationPending())) {
            wf->handlerDagNodeDone(node);
            if (wf->instanciationReady() && this->instanciationPending) {
              TRACE_TEXT (TRACE_MAIN_STEPS,"INSTANCIATION pending & ready ==> POST" << endl);
              mySem.post();
            }
          }
          this->myLock.unlock();  /** UNLOCK */
          return 0;
        } else failureReason = "Node failure";

      } catch (WfDataException& e) {
        failureReason = "Data error: " + e.ErrorMsg();
      } // end of NODE-LEVEL TRY BLOCK

//       if (this->myWfLogService != WfLogService::_nil()) {
//         myWfLogService->nodeIsDone(node_id);
//       }

    } catch (WfStructException& e) {
      failureReason = e.ErrorMsg();
    } catch (CORBA::SystemException& e) {
      failureReason = string("CORBA ") + e._name() + string(" : ")
                        + e.NP_minorString();
    } catch (...) {
      failureReason = "Unknown exception";
    } // end of DAG-LEVEL TRY BLOCK

  } else failureReason = "Dag not found";
  cerr << "Execution of node " << dag_id << "-" << node_id << " failed! "
       << "(" << failureReason << ")" << endl;
  return 1;
}

CltWfMgr::CltWfMgr() : mySem(0), cltWfReqId(0), dagSentCount(0) {
  this->myMA = MasterAgent::_nil();
  this->myMaDag = MaDag::_nil();
  this->myWfLogService = WfLogService::_nil();
  gettimeofday(&this->refTime, NULL); // init reference time
}

CltWfMgr *
CltWfMgr::instance() {
  if (myInstance == NULL) {
    myInstance = new CltWfMgr();
    ORBMgr::activate(myInstance);
  }
  return myInstance;
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

/**
 * Set the workflow log service reference
 */
void
CltWfMgr::setWfLogSrv(WfLogService_var logSrv) {
  this->myWfLogService = logSrv;
}

/**
 * Get current time relative to construction time (in milliseconds)
 */
double
CltWfMgr::getCurrTime() {
  struct timeval current_time;
  gettimeofday(&current_time, NULL);
  return (double) ((current_time.tv_sec - refTime.tv_sec)*1000
      + (current_time.tv_usec - refTime.tv_usec)/1000);
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
CltWfMgr::initDagStatus(FWorkflow* wf) {
  this->allDagsSent.insert(make_pair(wf,false));
}

/**
 * Set the dag status regarding dags sent for a given functional workflow
 */
void
CltWfMgr::setWfSubmissionComplete(FWorkflow* wf) {
  this->allDagsSent[wf] = true;
}

/**
 * Get the status regarding dags sent for a given functional workflow
 */
bool
CltWfMgr::isWfSubmissionComplete(FWorkflow* wf) {
  map<FWorkflow*,bool>::iterator wfIter = this->allDagsSent.find(wf);
  if (wfIter != this->allDagsSent.end())
    return (bool) wfIter->second;
  else
    return false;
}

/**
 * Execute a single dag
 */

diet_error_t
CltWfMgr::wfDagCall(diet_wf_desc_t * profile) {
  diet_error_t res(0);
  Dag * dag = new Dag(myMA);
  if (this->myMaDag != MaDag::_nil()) {
    // Call the common dag submission with parsing=ON and release=FALSE
    if (!wfDagCallCommon(profile, dag, true, false)) {
      TRACE_TEXT (TRACE_ALL_STEPS,"Waiting for release ==> WAIT" << endl);
      this->mySem.wait();
      // WAIT UNTIL RELEASE
      if (dag->isCancelled()) res = 1;
      usleep(1000); // to let the release() call terminate before end of process
    } else {
      cerr << "DAG request cancelled!" << endl;
      res = 1;
    }
  } else {
    cerr << "FATAL ERROR: MA DAG not initialized!" << endl;
    res = 1;
  }
  return res;
}

/**
 * (private)
 * The common part of dag submission
 */
diet_error_t
CltWfMgr::wfDagCallCommon(diet_wf_desc_t *dagProfile, Dag *dag, bool parse, bool release) {
  diet_error_t res(0);

  if (parse) {
    try {
      DagParser parser(*dag, dagProfile->abstract_wf);
      parser.parseXml();
    } catch (XMLParsingException& e) {
      cerr << "DAG PARSING FATAL ERROR:" << endl;
      cerr << e.ErrorMsg() << endl;
      return 1;
    }
  } // end if (parse)

  // check the dag structure (ie each node is connected to identified nodes)
  NodeSet *nodeSet = NULL; // set of nodes used to check precedence
  MetaDag *currMetaDag = NULL;
  CORBA::Long wfReqID  = dagProfile->wfReqID;
  if (wfReqID == -1) {
    nodeSet = dag;
  } else {
    TRACE_TEXT (TRACE_ALL_STEPS,"Get Metadag for request " << wfReqID << endl);
    // MetaDag initialization
    map<CORBA::Long, MetaDag*>::iterator mDagIter = myMetaDags.find(wfReqID);
    if (mDagIter != myMetaDags.end()) {
      currMetaDag = (MetaDag*) mDagIter->second;
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
    if (currMetaDag)
      currMetaDag->setCurrentDag(NULL);
    return 1;
  }

  if (currMetaDag)
    currMetaDag->setCurrentDag(NULL);

  // Build the dag connexions to allow retrieval of input data
  try {
    dag->linkAllPorts();
  } catch (WfStructException &e) {
    cerr << "FATAL ERROR: INVALID DAG DATA CONNECTIONS:" << endl;
    cerr << e.ErrorMsg() << endl;
    return 1;
  }

  TRACE_TEXT (TRACE_ALL_STEPS,"Marshalling the workflow description" << endl);
  corba_wf_desc_t  *corba_profile = new corba_wf_desc_t;
  mrsh_wf_desc(corba_profile, dagProfile);

  TRACE_TEXT (TRACE_ALL_STEPS,
	      "Try to send the workflow description to the MA_DAG ..." << endl);
  CORBA::Long dagID;

  dag->setStartTime(this->getCurrTime());

  this->myLock.lock();    /** LOCK required to include MaDag call postprocessing */
  try {
    // CORBA CALL TO MADAG
    if (wfReqID < 0) {
      wfReqID = this->myMaDag->getWfReqId();
      dagProfile->wfReqID = wfReqID;
      dagID   = this->myMaDag->processDagWf(*corba_profile, myIOR(), wfReqID);
    } else {
      dagID   = this->myMaDag->processMultiDagWf(*corba_profile, myIOR(), wfReqID, release);
    }
  } catch (CORBA::SystemException& e) {
    cerr << "Caught a CORBA " << e._name() << " exception ("
         << e.NP_minorString() << ")" << endl ;
    dagID = -1;
  } catch (MaDag::ServiceNotFound& e) {
    cerr << "Error on MaDag missing service: " << e.pbName << "(" << e.pbParams
         << ") for node: " << e.nodeId << endl;
    dagID = -1;
  } catch (MaDag::CommProblem& e) {
    cerr << "Error on MaDag communication pb : " << e.info << endl;
    dagID = -1;
  } catch (MaDag::InvalidDag& e) {
    cerr << "Error on MaDag invalid dag : " << e.info << endl;
    dagID = -1;
  }

  // MADAG CALL POSTPROCESSING (must be done before any MaDag callback)
  if (dagID != -1) {
    TRACE_TEXT (TRACE_ALL_STEPS, " done" << endl);
    // changing Dag ID (if part of metadag then remove and re-insert)
    string oldId = dag->getId();
    dag->setId(itoa((long) dagID));
    if (currMetaDag) {
      currMetaDag->removeDag(oldId);
      currMetaDag->addDag(dag);
    }
    // update local dag counter
    dagSentCount++;
    TRACE_TEXT (TRACE_MAIN_STEPS,"DAG SENT COUNT = " << dagSentCount << endl);
    // store dag ref in profiles map (used to retrieve results)
    this->myProfiles[dagProfile] = dag;
    TRACE_TEXT (TRACE_MAIN_STEPS,"Dag ID " << dag->getId() << " ready to execute" << endl);
  } else {
    cerr << "FATAL ERROR: MA DAG cancelled the request" << endl;
    res = 1;
  }
  this->myLock.unlock();  /** UNLOCK (End of critical section for MaDAG call) */

  // Call the Workflow Log Service if used
  try {
    if (this->myWfLogService != WfLogService::_nil()) {
      myWfLogService->setWf(dagProfile->abstract_wf);
    }
  } catch (CORBA::SystemException& e) {
    cerr << "WfLogService : Caught a CORBA " << e._name() << " exception ("
         << e.NP_minorString() << ")" << endl ;
  } catch (...) {
    cerr << "Unknown exception with WfLogService (setWf)" << endl;
  }

  return res;
}

/**
 * Execute a functional workflow
 */
diet_error_t
CltWfMgr::wfFunctionalCall(diet_wf_desc_t * profile) {
  diet_error_t res(0);
  char statMsg[128];
  string wfName(profile->name);

  FWorkflow *wf = new FWorkflow(wfName);
  FWfParser reader(*wf, profile->abstract_wf);  //TODO use initFromXmlFile()
  try {

    TRACE_TEXT (TRACE_ALL_STEPS,"Parsing WORKFLOW XML" << endl);
    reader.parseXml();
    TRACE_TEXT (TRACE_ALL_STEPS,"*** Checking WORKFLOW XML structure" << endl);
    wf->checkPrec(wf);

  } catch (XMLParsingException& e) {
    cerr << "FATAL ERROR: INVALID FUNCTIONAL WORKFLOW XML:" << endl;
    cerr << e.ErrorMsg() << endl;
    return XML_MALFORMED;
  } catch (WfStructException& e) {
    cerr << "FATAL ERROR: FUNCTIONAL WORKFLOW SYNTAX OR STRUCTURE ERROR:" << endl;
    cerr << e.ErrorMsg() << endl;
    return WFL_BADSTRUCT;
  }

  TRACE_TEXT (TRACE_ALL_STEPS, "*** Initialize functional wf ****" << endl);
  string dataFileName = defaultDataFileName;
  if (profile->dataFile) {
    dataFileName = profile->dataFile;
  }
  cout << "XML DATA SOURCE FILE = " << dataFileName << endl;
  wf->setDataSrcXmlFile(dataFileName);
  try {
    wf->initialize();
  } catch (XMLParsingException& e) {
    cerr << "FATAL ERROR: DATA FILE Parsing exception: " << endl;
    cerr << e.ErrorMsg() << endl;
    return XML_MALFORMED;
  } catch (WfStructException& e) {
    cerr << "FATAL ERROR: BAD FUNCTIONAL WORKFLOW STRUCTURE: " << endl;
    cerr << e.ErrorMsg() << endl;
    return WFL_BADSTRUCT;
  }
  // store wf profile to allow results retrieval
  myProfiles[profile] = wf;

  TRACE_TEXT (TRACE_ALL_STEPS, "*** Get wf request ID from MADAG ****" << endl);
  CORBA::Long wfReqId = 0;
  if (this->myMaDag != MaDag::_nil()) {
    wfReqId = this->myMaDag->getWfReqId();
  } else {
    cerr << "FATAL ERROR: MA DAG not initialized" << endl;
    return 1;
  }

  initDagStatus(wf);

  TRACE_TEXT (TRACE_ALL_STEPS, "*** INSTANCIATION ****" << endl);
  int dagCounter = 0;
  while (!res && !wf->instanciationCompleted() && !wf->instanciationStopped()) {
    // synchronization with end-of-node-execution events
    this->instanciationPending = false;

    // GENERATE NEW DAG
    Dag * currDag = new Dag(myMA);
    // set a temporary ID for the DAG
    // this ID will be changed when the DAG is submitted to the MaDag
    // the temp ID must not match any real ID (which are numbers)
    currDag->setId("WF_" + wf->getId() + "_DAG_" + itoa(dagCounter++));
    currDag->setWorkflow(wf);
    this->myLock.lock();    /** LOCK */
    wf->instanciate(currDag);
    this->myLock.unlock();  /** UNLOCK */

    if (currDag->size() == 0) {
      cout << "*** GENERATED DAG IS EMPTY ***" << endl;
    } else {
      string dagFileName = "dag_" + currDag->getId(); // will be changed when dag submitted to MaDag

      if (TRACE_LEVEL >= TRACE_ALL_STEPS) {
        cout << "*** DISPLAY DAG " << currDag->getId()
                                  << "  ****" << endl << endl;
        fstream filestr;
        filestr.open(dagFileName.c_str(), fstream::out);
        currDag->toXML(filestr);
        filestr.close();
        currDag->toXML(cout);
      }

      TRACE_TEXT (TRACE_MAIN_STEPS, "*** SUBMIT DAG " << currDag->getId()
                                    << " TO MADAG ****" << endl << endl);
      diet_wf_desc_t * dagProfile = diet_wf_profile_alloc(dagFileName.c_str(),"testdag",DIET_WF_DAG);
      dagProfile->wfReqID = wfReqId;

      try {

        // Call the common DAG submission method
        // with PARSING option deactivated (the dag is already in memory)
        res = wfDagCallCommon(dagProfile, currDag, false, wf->instanciationCompleted());

      } catch (...) {
        cerr << "Exception not caught in wfDagCallCommon!" << endl;
        res = 1;
      }
    } // end if (currDag->size() == 0)

    if (!res) {
      // If this is the last dag of the workflow instanciation, set the flag
      if (wf->instanciationCompleted())
        setWfSubmissionComplete(wf);
      // CHECK IF INSTANCIATION IS PENDING (waiting for node execution)
      if (wf->instanciationPending()) {
        TRACE_TEXT (TRACE_ALL_STEPS,"INSTANCIATION PENDING ON NODE EXECUTION ==> WAIT" << endl);
        this->instanciationPending = true;
        this->mySem.wait();
        if (wf->instanciationPending())
          TRACE_TEXT (TRACE_ALL_STEPS,"INSTANCIATION RESTARTS" << endl);
      }
    } else {
      cerr << "DAG SUBMISSION FAILED ==> INSTANCIATION STOPPED" << endl;
      wf->instanciationStopped();
    }
  } // while

  // Instanciation is finished
  if (dagSentCount > 0) {
    TRACE_TEXT (TRACE_ALL_STEPS,"NO MORE DAGS TO INSTANCIATE ==> WAIT" << endl);
    this->mySem.wait();
    usleep(1000); // to avoid stopping process before end of release call
  }
  if (!wf->instanciationCompleted()) {
      cerr << "FUNCTIONAL WORKFLOW INSTANCIATION or EXECUTION FAILED!" << endl;
      res = 1;
  }
  wf->displayDagSummary(cout);
  return res;
}

/**
 * Release the waiting semaphore
 */
char *
CltWfMgr::release(const char * dag_id, bool successful) {
  Dag * dag = getDag(dag_id);
  FWorkflow * wf = dag->getWorkflow(); // may be NULL
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
      this->myLock.lock();    /** LOCK */
      wf->stopInstanciation(); // stop instanciation
      this->myLock.unlock();  /** UNLOCK */
    }
  }

  // RETURN MESSAGE CONTAINING REQUEST IDs (FOR VIZDIET)
  vector<diet_reqID_t> diet_request_ids = dag->getAllDietReqID();
  TRACE_TEXT(TRACE_ALL_STEPS, traceHeader << "got request ids (size = "
              << diet_request_ids.size() << ")" << endl);
  stringstream message;
  message << dag_id;
   // Add request IDs to message
  for (unsigned int ix=0; ix<diet_request_ids.size(); ix++) {
    message << ";" << diet_request_ids[ix];
  }
   // Add makespan to message
  message << "#" << dag->getMakespan();

  TRACE_TEXT(TRACE_ALL_STEPS, traceHeader << "message: |" << message.str()
              << "| " << message.str().size()<< endl);
  char * ret = (char*)malloc(message.str().size()*sizeof(char)+1);
  sprintf(ret,"%s",message.str().c_str());

   // UPDATE DAG COUNTER
  this->myLock.lock();    /** LOCK */
  dagSentCount--;
  TRACE_TEXT(TRACE_MAIN_STEPS,traceHeader << "DAG SENT COUNT = " << dagSentCount << endl);

  if (dagSentCount == 0) {

    if (!wf || isWfSubmissionComplete(wf) || wf->instanciationStopped()) {
      TRACE_TEXT(TRACE_ALL_STEPS,traceHeader << "No more dags running ==> POST" << endl);
      this->myLock.unlock();  /** UNLOCK */
      this->mySem.post();
    } else {
      TRACE_TEXT(TRACE_ALL_STEPS,traceHeader << "Still some dags to send ==> CONTINUE" << endl);
      this->myLock.unlock();  /** UNLOCK */
    }
  } else {
    TRACE_TEXT(TRACE_ALL_STEPS,traceHeader << "Still some dags running ==> CONTINUE" << endl);
    this->myLock.unlock();  /** UNLOCK */
  }
  return ret;
}

/**
 * Print all results of a dag
 */
diet_error_t
CltWfMgr::printAllDagResults(diet_wf_desc_t* profile) {
  map<diet_wf_desc_t*,NodeSet*>::iterator nsp = myProfiles.find(profile);
  if (nsp != myProfiles.end()) {
    Dag * dag = dynamic_cast<Dag*>(nsp->second);
    if (dag != NULL) {
      if (!dag->isCancelled())
        try {
          dag->displayAllResults(cout);
        } catch (WfDataException& e) {
          cerr << "Data error: " << e.ErrorMsg() << endl;
          return 1;
        }
      else
        cerr << "** DAG " << dag->getId()
             << " was cancelled => no results **" << endl;
    }
    else return 1;
  }
  else return 1;
}

/**
 * Print all results of a functional wf
 */
diet_error_t
CltWfMgr::printAllFunctionalWfResults(diet_wf_desc_t* profile) {
  diet_error_t res(0);
  map<diet_wf_desc_t*,NodeSet*>::iterator nsp = myProfiles.find(profile);
  if (nsp != myProfiles.end()) {
    FWorkflow * wf = dynamic_cast<FWorkflow*>(nsp->second);
    if (wf != NULL) {
      try {
        wf->displayAllResults(cout);
      } catch (WfDataException& e) {
        cerr << "Data error: " << e.ErrorMsg() << endl;
        return 1;
      }
    }
    else return 1;
  }
  else return 1;
}

/**
 * Get a result from the workflow
 * TODO catch WfStructException
 */
int
CltWfMgr::getWfOutputScalar(diet_wf_desc_t* profile,
                   const char * id,
		   void** value) {
  map<diet_wf_desc_t*,NodeSet*>::iterator nsp = myProfiles.find(profile);
  if (nsp != myProfiles.end()) {
    Dag * dag = dynamic_cast<Dag*>(nsp->second);
    if (dag != NULL) {
	return dag->get_scalar_output(id, value);
    }
    else return 1;
  }
  else return 1;
} // end getWfOutputScalar

int
CltWfMgr::getWfOutputString(diet_wf_desc_t* profile,
                   const char * id,
		   char** value) {
  map<diet_wf_desc_t*,NodeSet*>::iterator nsp = myProfiles.find(profile);
  if (nsp != myProfiles.end()) {
    Dag * dag = dynamic_cast<Dag*>(nsp->second);
    if (dag != NULL) {
	return dag->get_string_output(id, value);
    }
    else return 1;
  }
  else return 1;
} // end getWfOutputString

int
CltWfMgr::getWfOutputFile(diet_wf_desc_t* profile,
                const char * id,
		size_t* size, char** path) {
  map<diet_wf_desc_t*,NodeSet*>::iterator nsp = myProfiles.find(profile);
  if (nsp != myProfiles.end()) {
    Dag * dag = dynamic_cast<Dag*>(nsp->second);
    if (dag != NULL) {
	return dag->get_file_output(id, size, path);
    }
    else return 1;
  }
  else return 1;
}

int
CltWfMgr::getWfOutputMatrix(diet_wf_desc_t* profile,
                   const char * id,
		   void** value,
		   size_t* nb_rows,
		   size_t *nb_cols,
		   diet_matrix_order_t* order) {
  map<diet_wf_desc_t*,NodeSet*>::iterator nsp = myProfiles.find(profile);
  if (nsp != myProfiles.end()) {
    Dag * dag = dynamic_cast<Dag*>(nsp->second);
    if (dag != NULL) {
	return dag->get_matrix_output(id, value, nb_rows, nb_cols, order);
    }
    else return 1;
  }
  else return 1;
}

int
CltWfMgr::getWfOutputContainer(diet_wf_desc_t* profile,
                               const char * id,
                               char** dataID) {
  map<diet_wf_desc_t*,NodeSet*>::iterator nsp = myProfiles.find(profile);
  if (nsp != myProfiles.end()) {
    Dag * dag = dynamic_cast<Dag*>(nsp->second);
    if (dag != NULL) {
	return dag->get_container_output(id, dataID);
    }
    else return 1;
  }
  else return 1;
}

/**
 * terminate a workflow session and free the memory
 */
void
CltWfMgr::wf_free(diet_wf_desc_t * profile) {
  char statMsg[64];
  map<diet_wf_desc_t*,NodeSet*>::iterator nsp = myProfiles.find(profile);
  if (nsp != myProfiles.end()) {
    if (profile->level == DIET_WF_DAG) {
      Dag * dag = dynamic_cast<Dag*>(nsp->second);
      if (dag != NULL) {
        sprintf(statMsg,"%s DAG %s",__FUNCTION__,dag->getId().c_str());
        stat_in("cltwfmgr",statMsg);
        dag->deleteAllResults();
        if (profile->wfReqID) {
          map<CORBA::Long, MetaDag*>::iterator mDagIter = myMetaDags.find(profile->wfReqID);
          if (mDagIter != myMetaDags.end()) {
            MetaDag * metaDag = (MetaDag*) mDagIter->second;
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
      FWorkflow * wf = dynamic_cast<FWorkflow*>(nsp->second);
      if (wf != NULL) {
        sprintf(statMsg,"%s WF %s",__FUNCTION__,wf->getId().c_str());
        stat_in("cltwfmgr",statMsg);
        wf->deleteAllResults();
        wf->deleteAllDags();
        delete wf;
      }
    }
    this->myProfiles.erase(profile);
    delete profile->abstract_wf;
    delete profile;
    stat_out("cltwfmgr",statMsg);
  } else {
    WARNING("wf_free : profile not found\n");
  }
}

/**
 * Return the object IOR
 */
const char *
CltWfMgr::myIOR() {
  return ORBMgr::getIORString(this->_this());
} // end ping

/**
 * Debug function
 */
void
CltWfMgr::ping() {
  TRACE_TEXT (TRACE_ALL_STEPS, "ping!!!!!!!!!!!!!!"<< endl);
} // end ping

/**
 * Return the DAG with a given identifier
 * Protected against concurrent access due to simultaneous dag creation and
 * execution
 */
Dag *
CltWfMgr::getDag(string dag_id) {
  Dag * dag = NULL;
  this->myLock.lock();    /** LOCK */
  for (map<diet_wf_desc_t *, NodeSet *>::iterator p = this->myProfiles.begin();
       p != this->myProfiles.end();
       ++p) {
    dag = dynamic_cast<Dag*>(p->second); // myProfiles contains Dags AND FWorkflows
    if ( (dag != NULL) &&
         (dag->getId() == dag_id) ) {
      break;
    }
  }
  this->myLock.unlock();  /** UNLOCK */
  return dag;
}
