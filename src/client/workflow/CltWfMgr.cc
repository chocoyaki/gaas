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
#include "debug.hh"
#include "ORBMgr.hh"
#include "workflow/DagWfParser.hh"
#include "statistics.hh"
#include "workflow/FWorkflow.hh"

using namespace std;


// Initialisation of static member myInstance
CltWfMgr * CltWfMgr::myInstance = NULL;

CORBA::Long
CltWfMgr::execNodeOnSed(const char * node_id,
                        const char * dag_id,
                        _objref_SeD* sed,
                        const CORBA::ULong reqID,
                        corba_estimation_t& ev) {
  string failureReason;
  FWorkflow* wf;
  TRACE_TEXT (TRACE_MAIN_STEPS,"CltWfMgr: execute node " << node_id <<
          " of dag " << dag_id << " (SED DEFINED)" << endl);
  this->myLock.lock();    /** LOCK */
  Dag * dag = this->getDag(dag_id);
  this->myLock.unlock();  /** UNLOCK */
  if (dag != NULL) {
    DagNode * node = dag->getDagNode(node_id);
    if (node != NULL) {
      SeD_var sed_var = SeD::_narrow(sed);
      node->setSeD(sed_var, (unsigned long) reqID, ev);
      node->start(true);
      if (!node->hasFailed()) {
        // notify the workflow (if applicable and if wf is pending)
        if ((wf = dag->getWorkflow()) && (wf->instanciationPending())) {
          wf->handlerDagNodeDone(node);
          if (wf->instanciationReady()) {
            mySem.post();
          }
        }
        return 0;
      } else failureReason = "Node failure";
    }
    else failureReason = "Node not found";
  }
  else failureReason = "Dag not found";
  cout << "Execution of node " << dag_id << "-" << node_id << " failed! "
       << "(" << failureReason << ")" << endl;
  return 1;
} // end execNodeOnSed

CORBA::Long
CltWfMgr::execNode(const char * node_id, const char * dag_id) {
  string failureReason;
  FWorkflow* wf;
  TRACE_TEXT (TRACE_MAIN_STEPS,"CltWfMgr: execute node " << node_id <<
          " of dag " << dag_id << " (SED NOT DEFINED)" << endl);
  this->myLock.lock();    /** LOCK */
  Dag * dag = this->getDag(dag_id);
  this->myLock.unlock();  /** UNLOCK */
  if (dag != NULL) {
    DagNode * node = dag->getDagNode(node_id);
    if (node != NULL) {
      node->start(true);
      if (!node->hasFailed()) {
        // notify the workflow (if applicable and if wf is pending)
        if ((wf = dag->getWorkflow()) && (wf->instanciationPending())) {
          wf->handlerDagNodeDone(node);
          if (wf->instanciationReady()) {
            mySem.post();
          }
        }
        return 0;
      } else failureReason = "Node failure";
    }
    else failureReason = "Node not found";
  }
  else failureReason = "Dag not found";
  cout << "Execution of node " << dag_id << "-" << node_id << " failed! "
       << "(" << failureReason << ")" << endl;
  return 1;
} // end execNode


CltWfMgr::CltWfMgr() : mySem(0), cltWfReqId(0) {
  this->myMA = MasterAgent::_nil();
  this->myMaDag = MaDag::_nil();
  this->myWfLogSrv = WfLogSrv::_nil();
  gettimeofday(&this->refTime, NULL); // init reference time
} // end CltWfMgr constructor

CltWfMgr *
CltWfMgr::instance() {
  if (myInstance == NULL) {
    myInstance = new CltWfMgr();
    ORBMgr::activate(myInstance);
  }
  return myInstance;
} // end instance

/**
 * Set the MaDag reference
 */
void
CltWfMgr::setMaDag(MaDag_var maDagRef) {
  this->myMaDag = maDagRef;
} // end setMaDag

/**
 * Set the Master Agent reference
 */
void
CltWfMgr::setMA(MasterAgent_var ma) {
  this->myMA = ma;
} // end setMA

/**
 * Set the workflow log service reference
 */
void
CltWfMgr::setWfLogSrv(WfLogSrv_var logSrv) {
  this->myWfLogSrv = logSrv;
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
  cout << "*** GET WORKFLOW REQUEST ID FROM MADAG ****" << endl;
  CORBA::Long wfReqId = 0;
  if (this->myMaDag != MaDag::_nil()) {
    wfReqId = this->myMaDag->getWfReqId();
  }
  return wfReqId;
}

/**
 * Execute a dag
 */
diet_error_t
CltWfMgr::wfDagCall(diet_wf_desc_t * profile) {
  diet_error_t res(0);
  char statMsg[128];
  corba_wf_desc_t  * corba_profile = new corba_wf_desc_t;
  Dag * dag = new Dag();
  CORBA::Long wfReqID = profile->wfReqID;
  CORBA::Long dagID;
  MetaDag* currMetaDag = NULL;

  sprintf(statMsg,"xml_reader",__FUNCTION__);
  stat_in("cltwfmgr",statMsg);

  try {
    TRACE_TEXT (TRACE_ALL_STEPS,"Parsing DAG XML" << endl);
    DagParser parser(*dag, profile->abstract_wf);
    parser.parseXml();

    TRACE_TEXT (TRACE_ALL_STEPS,"Checking DAG structure" << endl);
    if (wfReqID == 0) {
      dag->checkPrec(dag);
    } else {
      // MetaDag initialization
      map<CORBA::Long, MetaDag*>::iterator mDagIter = myMetaDags.find(wfReqID);
      if (mDagIter != myMetaDags.end()) {
        currMetaDag = (MetaDag*) mDagIter->second;
      } else {
        currMetaDag = new MetaDag(itoa(wfReqID));
        myMetaDags[wfReqID] = currMetaDag;
      }
      currMetaDag->addDag(dag);
      currMetaDag->setReleaseFlag(false);
      currMetaDag->setCurrentDag(dag);
      // Dag precedence check
      dag->checkPrec(currMetaDag);
      currMetaDag->setCurrentDag(NULL);
    }
  } catch (XMLParsingException &e) {
    return XML_MALFORMED;
  } catch (WfStructException &e) {
    return DAG_BADSTRUCT;
  }

  stat_out("cltwfmgr",statMsg);

  dag->setStartTime(this->getCurrTime());

  TRACE_TEXT (TRACE_ALL_STEPS,
	      "Marshalling the workflow description" << endl);
  mrsh_wf_desc(corba_profile, profile);

  TRACE_TEXT (TRACE_ALL_STEPS,
	      "Try to send the workflow description to the MA_DAG ..." << endl);
  if (this->myMaDag != MaDag::_nil()) {

    this->myLock.lock();  /** LOCK */

    sprintf(statMsg,"%s",__FUNCTION__);
    stat_in("cltwfmgr",statMsg);

    // CALL MADAG
    if (wfReqID == 0) {
      wfReqID = this->myMaDag->getWfReqId();
      dagID   = this->myMaDag->processDagWf(*corba_profile, myIOR(), wfReqID);
    } else {
      dagID   = this->myMaDag->processMultiDagWf(*corba_profile, myIOR(), wfReqID, false);
    }
    // DO LOCAL STUFF and SLEEP until released by MaDag
    if (dagID != -1) {
	TRACE_TEXT (TRACE_ALL_STEPS, " done" << endl);
        // changing Dag ID (if part of metadag then remove and re-insert)
        if (currMetaDag) {
          currMetaDag->removeDag(dag);
        }
        dag->setId(itoa((long) dagID));
        if (currMetaDag) {
          currMetaDag->addDag(dag);
        }
	// Build the dag connexions to allow retrieval of input data
        TRACE_TEXT (TRACE_ALL_STEPS,
                    "Linking the dag ports..." << endl);
  	dag->linkAllPorts();
        TRACE_TEXT (TRACE_ALL_STEPS, " done" << endl);
        dagSentCount++;
        cout << "DAG SENT COUNT = " << dagSentCount << endl;
  	this->myProfiles[profile] = dag;
	TRACE_TEXT (TRACE_MAIN_STEPS,"Dag ID " << dag->getId() << " ready to execute" << endl);
        this->myLock.unlock();  /** UNLOCK */

  	this->mySem.wait();
        // WAIT UNTIL DAG IS COMPLETED OR CANCELLED
        if (dag->isCancelled()) {
          TRACE_TEXT (TRACE_MAIN_STEPS,"Dag ID " << dag->getId()
              << " WAS CANCELLED BY MADAG!" << endl);
          usleep(1000); // to let the release() call terminate before end of process
          res = 1;
        }
    } else {
	TRACE_TEXT (TRACE_ALL_STEPS, "MA DAG cancelled the request ...");
	res = 1;
        this->myLock.unlock();  /** UNLOCK */
    }
  }
  sprintf(statMsg,"%s %s",__FUNCTION__,dag->getId().c_str());
  stat_out("cltwfmgr",statMsg);
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

  TRACE_TEXT (TRACE_ALL_STEPS,"Parsing WORKFLOW XML" << endl);
  FWorkflow *wf = new FWorkflow(wfName);
  FWfParser reader(*wf, profile->abstract_wf);
  try {
    reader.parseXml();
  } catch (XMLParsingException& exception) {
      cout << "FUNCTIONAL WORKFLOW PARSING FATAL ERROR:" << endl;
      switch(exception.Type()) {
        case XMLParsingException::eBAD_STRUCT :
          cout << "BAD XML STRUCTURE (" << exception.Info() << ")" << endl;
          break;
        case XMLParsingException::eEMPTY_ATTR :
          cout << "MISSING DATA (" << exception.Info() << ")" << endl;
          break;
        case XMLParsingException::eUNKNOWN_TAG :
          cout << "UNKNOWN XML TAG (" << exception.Info() << ")" << endl;
          break;
        case XMLParsingException::eUNKNOWN_ATTR :
          cout << "UNKNOWN XML ATTRIBUTE (" << exception.Info() << ")" << endl;
          break;
        case XMLParsingException::eINVALID_REF :
          cout << "INVALID REFERENCE (" << exception.Info() << ")" << endl;
          break;
      }
      return XML_MALFORMED;
  }

  TRACE_TEXT (TRACE_ALL_STEPS,"Checking WORKFLOW XML structure" << endl);
  try {
    wf->checkPrec(wf);
  } catch (WfStructException& e) {
    cout << "BAD WORKFLOW STRUCTURE (" << e.Info() << ")" << endl;
    return WFL_BADSTRUCT;
  }

  cout << "*** INITIALIZE FUNCTIONAL WORKFLOW ****" << endl;
  wf->initialize();
  // store wf profile to allow results retrieval
  myProfiles[profile] = wf;

  cout << "*** GET WORKFLOW REQUEST ID FROM MADAG ****" << endl;
  CORBA::Long wfReqId = 0;
  if (this->myMaDag != MaDag::_nil()) {
    wfReqId = this->myMaDag->getWfReqId();
  }
  dagSentCount = 0;

  cout << "*** INSTANCIATION ****" << endl;
  while (!wf->instanciationCompleted()) {
    Dag * currDag = wf->instanciateDag();
    if (currDag == NULL) {
      cout << "!!! dag instanciation failed !!!" << endl;
      break;
    }
    string dagFileName = "dag_" + currDag->getId(); // FIXME the current dag id may be changed
    // later as it will be given a new id by the MaDag
    cout << "*** DISPLAY DAG " << currDag->getId() << "  ****" << endl << endl;
    fstream filestr;
    filestr.open(dagFileName.c_str(), fstream::out);
    filestr << currDag->toXML();
    filestr.close();
    cout << currDag->toXML() << endl;

    cout << "*** SUBMIT DAG " << currDag->getId() << " TO MADAG ****" << endl << endl;
    diet_wf_desc_t * dagProfile = diet_wf_profile_alloc(dagFileName.c_str(),"testdag",DIET_WF_DAG);
    corba_wf_desc_t  * corba_profile = new corba_wf_desc_t;
    mrsh_wf_desc(corba_profile, dagProfile);

    if (this->myMaDag != MaDag::_nil()) {

      this->myLock.lock();  /** LOCK */

      // CALL MADAG (schedule the dag)
      currDag->setStartTime(this->getCurrTime());
      CORBA::Long dagId = -1;
      bool release = wf->instanciationCompleted();
      try {
        dagId = this->myMaDag->processMultiDagWf(*corba_profile, myIOR(), wfReqId, release);
      } catch(CORBA::SystemException& e) {
        cout << "Caught a CORBA " << e._name() << " exception ("
        << e.NP_minorString() << ")" << endl ;
      }

      // DO LOCAL STUFF
      if (dagId != -1) {
	TRACE_TEXT (TRACE_ALL_STEPS, " done" << endl);
        currDag->setId(itoa((long) dagId));
	// Build the dag connexions to allow retrieval of input data
        TRACE_TEXT (TRACE_ALL_STEPS,
                    "Linking the dag ports..." << endl);
  	currDag->linkAllPorts();
        TRACE_TEXT (TRACE_ALL_STEPS, " done" << endl);
        dagSentCount++;
        cout << "DAG SENT COUNT = " << dagSentCount << endl;
        this->myProfiles[dagProfile] = currDag;
	TRACE_TEXT (TRACE_MAIN_STEPS,"Dag ID " << currDag->getId() << " ready to execute" << endl);
        this->myLock.unlock();  /** UNLOCK */
      } else {
	TRACE_TEXT (TRACE_ALL_STEPS, "MA DAG cancelled the request ..." << endl);
	res = 1;
        this->myLock.unlock();  /** UNLOCK */
      }
    }

    // CHECK IF INSTANCIATION IS PENDING (waiting for node execution)
    if (wf->instanciationPending()) {
      cout << "INSTANCIATION WAITING FOR NODE EXECUTION" << endl;
      this->mySem.wait();
      cout << "INSTANCIATION RESTARTS" << endl;
    }

  }
  // Instanciation is finished
  if (dagSentCount > 0) {
    cout << "NO MORE DAGS TO INSTANCIATE ==> WAITING DAG RELEASE" << endl;
    this->mySem.wait();
    usleep(1000); // to avoid stopping process before end of release call
  } else {
    cout << "NO DAGS COULD BE INSTANTIATED!!" << endl;
    res = 1;
  }
  return res;
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
        dag->displayAllResults();
      else
        cout << "** DAG " << dag->getId()
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
      list<Dag*>& dagList = wf->getDagList();
      list<Dag*>::iterator dagIter = dagList.begin();
      while (dagIter != dagList.end()) {
        Dag * currDag = (Dag*) *dagIter;
        if (!currDag->isCancelled())
          currDag->displayAllResults();
        else
          cout << "** DAG " << currDag->getId()
               << " was cancelled => no results **" << endl;
        ++dagIter;
      }
    }
    else return 1;
  }
  else return 1;
}

/**
 * Get a result from the workflow
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
            metaDag->removeDag(dag);
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
 * Release the waiting semaphore
 */
char *
CltWfMgr::release(const char * dag_id, bool successful) {
  Dag * dag = getDag(dag_id);
  if (dag == NULL) {
    throw CltMan::DagNotFound(dag_id);
  }
   // SET DAG STATUS
  if (!successful) {
    dag->setAsCancelled();
  }

   // RETURN MESSAGE CONTAINING REQUEST IDs (FOR VIZDIET)
  vector<diet_reqID_t> diet_request_ids = dag->getAllDietReqID();
  cout << " got request ids" << endl;
  stringstream message;
  message << dag_id;
   // Add request IDs to message
  for (unsigned int ix=0; ix<diet_request_ids.size(); ix++) {
    message << ";" << diet_request_ids[ix];
  }
   // Add makespan to message
  message << "#" << this->getCurrTime() - dag->getStartTime();

  TRACE_TEXT(TRACE_ALL_STEPS,"release |"<< message.str() << "| " << message.str().size()<< endl);
  char * ret = (char*)malloc(message.str().size()*sizeof(char)+1);
  sprintf(ret,"%s",message.str().c_str());

   // UPDATE DAG COUNTER
  this->myLock.lock();    /** LOCK */
  dagSentCount--;
  this->myLock.unlock();  /** UNLOCK */
  cout << "DAG SENT COUNT = " << dagSentCount << endl;

  if (dagSentCount == 0) {
    FWorkflow * wf = dag->getWorkflow();
    if (!wf || (wf && wf->instanciationCompleted())) {
      TRACE_TEXT(TRACE_ALL_STEPS,"No more dags running ==> POST" << endl);
      this->mySem.post();
    } else {
      TRACE_TEXT(TRACE_ALL_STEPS,"Instanciation not finished ==> CONTINUE" << endl);
    }
  } else {
    TRACE_TEXT(TRACE_ALL_STEPS,"Still some dags running ==> CONTINUE" << endl);
  }
  return ret;
}

/**
 * Return the DAG with a given identifier
 */
Dag *
CltWfMgr::getDag(string dag_id) {
  Dag * dag = NULL;
  for (map<diet_wf_desc_t *, NodeSet *>::iterator p = this->myProfiles.begin();
       p != this->myProfiles.end();
       ++p) {
    dag = dynamic_cast<Dag*>(p->second); // myProfiles contains Dags AND FWorkflows
    if ( (dag != NULL) &&
         (dag->getId() == dag_id) ) {
      break;
    }
  }
  return dag;
}

