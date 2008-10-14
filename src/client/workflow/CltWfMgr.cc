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

#include "CltWfMgr.hh"
#include "debug.hh"
#include "ORBMgr.hh"
#include "workflow/DagWfParser.hh"
#include "statistics.hh"

using namespace std;


// Initialisation of static member myInstance
CltWfMgr * CltWfMgr::myInstance = NULL;

CORBA::Long
CltWfMgr::execNodeOnSed(const char * node_id,
                        const char * dag_id,
                        _objref_SeD* sed,
                        const CORBA::ULong reqID,
                        corba_estimation_t& ev) {
  this->myLock.lock();    /** LOCK */
  Dag * dag = this->getDag(dag_id);
  this->myLock.unlock();  /** UNLOCK */
  if (dag != NULL) {
    DagNode * node = dynamic_cast<DagNode*>(dag->getNode(node_id));
    if (node != NULL) {
      SeD_var sed_var = SeD::_narrow(sed);
      node->setSeD(sed_var, (unsigned long) reqID, ev);
      TRACE_TEXT (TRACE_MAIN_STEPS,"CltWfMgr: execute node " << node_id <<
          " of dag " << dag_id << " (SED DEFINED)" << endl);
      node->start(true);
      if (!node->hasFailed()) return 0;
    }
    else
      cerr << "Execution of node " << dag_id << "-" << node_id << " failed! "
           << " (Node not found)" << endl;
  }
  else
     cerr << "Execution of node " << dag_id << "-" << node_id << " failed! "
          << " (Dag not found)" << endl;
  return 1;
} // end execNodeOnSed

CORBA::Long
CltWfMgr::execNode(const char * node_id, const char * dag_id) {
  this->myLock.lock();    /** LOCK */
  Dag * dag = this->getDag(dag_id);
  this->myLock.unlock();  /** UNLOCK */
  if (dag != NULL) {
    DagNode * node = dynamic_cast<DagNode*>(dag->getNode(node_id));
    if (node != NULL) {
      TRACE_TEXT (TRACE_MAIN_STEPS,"CltWfMgr: execute node " << node_id <<
          " of dag " << dag_id << " (SED NOT DEFINED)" << endl);
      node->start(true);
      if (!node->hasFailed()) return 0;
    }
    else
      cerr << "Execution of node " << dag_id << "-" << node_id << " failed! "
           << " (Node not found)" << endl;
  }
  else
     cerr << "Execution of node " << dag_id << "-" << node_id << " failed! "
          << " (Dag not found)" << endl;
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
 * Execute a workflow using the MA DAG.
 *
 */
diet_error_t
CltWfMgr::wf_call_madag(diet_wf_desc_t * profile,
                        bool mapping) {
  diet_error_t res(0);
  char statMsg[128];
  corba_wf_desc_t  * corba_profile = new corba_wf_desc_t;

  TRACE_TEXT (TRACE_ALL_STEPS,"Calling the MA DAG "<< endl);

  sprintf(statMsg,"xml_reader",__FUNCTION__);
  stat_in("cltwfmgr",statMsg);

  Dag *dag = new Dag();
  DagWfParser reader(*dag, profile->abstract_wf);
  if (! reader.parseAndCheck())
    return XML_MALFORMED;

  stat_out("cltwfmgr",statMsg);
  // At this stage only the XML validity is checked but not the coherence
  // of the dag links between inputs and outputs

  dag->setStartTime(this->getCurrTime());

  mrsh_wf_desc(corba_profile, profile);
  TRACE_TEXT (TRACE_ALL_STEPS,
	      "Marshalling the workflow description done" << endl);

  TRACE_TEXT (TRACE_ALL_STEPS,
	      "Try to send the workflow description to the MA_DAG ..." << endl);
  if (this->myMaDag != MaDag::_nil()) {

    this->myLock.lock();  /** LOCK */
    // CALL MADAG
    CORBA::Long wfReqId = this->myMaDag->getWfReqId();
    sprintf(statMsg,"%s",__FUNCTION__);
    stat_in("cltwfmgr",statMsg);
    CORBA::Long dagId   = this->myMaDag->processDagWf(*corba_profile, myIOR(), wfReqId);

    // DO LOCAL STUFF and SLEEP until released by MaDag
    if (dagId != -1) {
	TRACE_TEXT (TRACE_ALL_STEPS, " done" << endl);
        dag->setId(itoa((long) dagId));
	// Build the dag connexions to allow retrieval of input data
        TRACE_TEXT (TRACE_ALL_STEPS,
                    "Linking the dag ports..." << endl);
  	dag->linkAllPorts();
        TRACE_TEXT (TRACE_ALL_STEPS, " done" << endl);
  	this->myProfiles[profile] = dag;
	TRACE_TEXT (TRACE_MAIN_STEPS,"Dag ID " << dag->getId() << " ready to execute" << endl);
        this->myLock.unlock();  /** UNLOCK */

  	this->mySem.wait();
        // WAIT UNTIL Workflow IS COMPLETED OR CANCELLED
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
 * Execute the workflow
 * The param scheduler is set to basic by default
 */
diet_error_t
CltWfMgr::wf_call(diet_wf_desc_t* profile) {
  diet_error_t res(0);
  if (this->myMaDag != MaDag::_nil()) {
    return this->wf_call_madag(profile, true);
  }
  return res;
}

/**
 * Get all results of the workflow
 */
diet_error_t
CltWfMgr::getAllWfResults(diet_wf_desc_t* profile) {
  if (this->myProfiles.find(profile) != this->myProfiles.end()) {
    Dag * dag = this->myProfiles.find(profile)->second;
    if (dag != NULL) {
      return dag->get_all_results();
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
  if (this->myProfiles.find(profile) != this->myProfiles.end()) {
    Dag * dag = this->myProfiles.find(profile)->second;
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
  if (this->myProfiles.find(profile) != this->myProfiles.end()) {
    Dag * dag = this->myProfiles.find(profile)->second;
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
  if (this->myProfiles.find(profile) != this->myProfiles.end()) {
    Dag * dag = this->myProfiles.find(profile)->second;
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
  if (this->myProfiles.find(profile) != this->myProfiles.end()) {
    Dag * dag = this->myProfiles.find(profile)->second;
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

  if (this->myProfiles.find(profile) != this->myProfiles.end()) {
    Dag * dag = this->myProfiles.find(profile)->second;
    if (dag != NULL) {
      sprintf(statMsg,"%s %s",__FUNCTION__,dag->getId().c_str());
      stat_in("cltwfmgr",statMsg);
      dag->deleteAllResults();
      delete dag;
    }
    this->myProfiles.erase(profile);
    delete profile->abstract_wf;
    delete profile;
    stat_out("cltwfmgr",statMsg);
  }
} // end wf_free

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
CltWfMgr::release(const char * dag_id) {
   Dag * dag = getDag(dag_id);
   vector<diet_reqID_t> diet_request_ids = dag->getAllDietReqID();
   std::string message = string(dag_id);
   // Add request IDs to message
   for (unsigned int ix=0; ix<diet_request_ids.size(); ix++){
	    char str[64];
	    sprintf(str, "%ld", diet_request_ids[ix]);
	    message +=";"+string(str);
   }
   // Add makespan to message
   char makespan[64];
   sprintf(makespan, "%f", this->getCurrTime() - dag->getStartTime());
   message +="#"+string(makespan);

   TRACE_TEXT(TRACE_ALL_STEPS,"release |"<< message << "| " << message.size()<< endl);
   char * ret = (char*)malloc(message.size()*sizeof(char)+1);
   sprintf(ret,"%s",message.c_str());
   this->mySem.post();
   return ret;
}

/**
 * Return the DAG with a given identifier
 */
Dag *
CltWfMgr::getDag(string dag_id) {
  Dag * dag = NULL;
  for (map<diet_wf_desc_t *, Dag *>::iterator p = this->myProfiles.begin();
       p != this->myProfiles.end();
       ++p) {
    if ( (p->second != NULL) &&
         (p->second->getId() == dag_id) ) {
      dag = p->second;
      break;
    } // end if
  } // end for
  return dag;
} // end getDag

