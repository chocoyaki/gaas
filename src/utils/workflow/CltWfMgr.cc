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

#include <string>

#include <stdlib.h>

#include "CltWfMgr.hh"
#include "debug.hh"
#include "ORBMgr.hh"
#include "DagWfParser.hh"

using namespace std;


// Initialisation of static member myInstance
CltWfMgr * CltWfMgr::myInstance = NULL;

/**
 * Executes a node (CORBA method)
 */
// void
// CltWfMgr::execute(const char * node_id, const char * dag_id,
//                   _objref_SeD* sed) {
//   cout << "Executing the node " << node_id << endl;
//   Dag * dag = this->getDag(dag_id);
//   if (dag != NULL) {
//     cout << "  ** Dag " << dag_id << " found!" << endl;
//     Node * node = dag->getNode(node_id);
//     if (node != NULL) {
//       SeD_var sed_var = SeD::_narrow(sed);
//       node->setSeD(sed_var, "");
//       node->start(-1, true);
//     }
//     else
//       cerr << "Node " << node_id << " not fount!!!!" << endl;
//   }
//   else
//     cout << "  ** Dag " << dag_id << " not found!" << endl;
//   sed->ping();
//   cout << "CltWfMgr: end of execution of node" << endl;   // SED-TEST (to remove)
// } // end execute


void
CltWfMgr::execNodeOnSed(const char * node_id, const char * dag_id,
                  _objref_SeD* sed) {
  Dag * dag = this->getDag(dag_id);
  if (dag != NULL) {
    Node * node = dag->getNode(node_id);
    if (node != NULL) {
      SeD_var sed_var = SeD::_narrow(sed);
      node->setSeD(sed_var, "");
      node->start(-1, true);
    }
    else
      cerr << "Node " << node_id << " not fount!!!!" << endl;
  }
  else
    cout << "  Dag " << dag_id << " not found!" << endl;
  sed->ping();
} // end execNodeOnSed

void
CltWfMgr::execNode(const char * node_id, const char * dag_id) {
  Dag * dag = this->getDag(dag_id);
  if (dag != NULL) {
    Node * node = dag->getNode(node_id);
    if (node != NULL) {
      node->start(-1, true);
    }
    else
      cerr << "Node " << node_id << " not fount!!!!" << endl;
  }
  else
    cout << "  Dag " << dag_id << " not found!" << endl;
} // end execNode


CltWfMgr::CltWfMgr() : mySem(0), cltWfReqId(0) {
  this->myMA = MasterAgent::_nil();;
  this->myMaDag = MaDag::_nil();;
  this->myWfLogSrv = WfLogSrv::_nil();;
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
 * Init the workflow processing
 */
// Dag *
// CltWfMgr::init_wf_call(diet_wf_desc_t * profile,
//                        corba_pb_desc_seq_t& pbs_seq,
//                        unsigned int& dagSize) {
//   corba_wf_desc_t  * corba_profile = new corba_wf_desc_t;
//   mrsh_wf_desc(corba_profile, profile);
//   TRACE_TEXT (TRACE_ALL_STEPS,
//               "Marshalling the workflow description done" << endl);
//
//   DagWfParser reader(cltWfReqId++,profile->abstract_wf, false);
//   if (! reader.setup())
//     return NULL;
//
//   // create the profile sequence
//   unsigned int len = reader.pbs_list.size();
//   pbs_seq.length(len);
//   for (unsigned int ix=0; ix< len; ix++) {
//     pbs_seq[ix] = reader.pbs_list[ix];
//   }
//
//   dagSize = reader.getDagSize();
//   return reader.getDag();
// } // end init_wf_call

/**
 * Execute a workflow using the MA DAG.
 *
 */
diet_error_t
CltWfMgr::wf_call_madag(diet_wf_desc_t * profile,
                        bool mapping) {
  diet_error_t res(0);
  corba_wf_desc_t  * corba_profile = new corba_wf_desc_t;

  TRACE_TEXT (TRACE_ALL_STEPS,"Calling the MA DAG "<< endl);

  DagWfParser reader(cltWfReqId++, profile->abstract_wf, false);
  if (! reader.setup())
    return XML_MALFORMED;

  Dag * dag = reader.getDag();

  mrsh_wf_desc(corba_profile, profile);
  TRACE_TEXT (TRACE_ALL_STEPS,
	      "Marshalling the workflow description done" << endl);

  // call the MA DAG
  TRACE_TEXT (TRACE_ALL_STEPS,
	      "Try to send the workflow description to the MA_DAG ...");
  if (this->myMaDag != MaDag::_nil()) {
    CORBA::Long dagId = this->myMaDag->getWfReqId();
    string dag_id = itoa(dagId);
    dag->setId(dag_id);
    if (this->myMaDag->processDagWf(*corba_profile, myIOR(), dagId)) {
	TRACE_TEXT (TRACE_ALL_STEPS, " done" << endl);
	// Build the dag connexions to allow retrieval of input data
  	dag->linkAllPorts();
  	this->myProfiles[profile] = dag;
  	cout << "Dag ID " << dag->getId() << endl;
  	this->mySem.wait();
    } else {
	TRACE_TEXT (TRACE_ALL_STEPS, "MA DAG cancelled the request ...");
	res = 1;
    }
  }

  /*
  // Call the Workflow Log Service
  if (this->myWfLogSrv != WfLogSrv::_nil()) {
    this->myWfLogSrv->setWf(profile->abstract_wf);
  } // end if (this->myWfLogSrv)*/

  return res;

} // end wf_call_madag

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
//   else {
//     return this->wf_call_ma(profile);
//   }
  return res;
} // end wf_call

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
  if (this->myProfiles.find(profile) != this->myProfiles.end()) {
    Dag * dag = this->myProfiles.find(profile)->second;
    if (dag != NULL) {
      delete dag;
    }
    this->myProfiles.erase(profile);
    delete profile->abstract_wf;
    delete profile;
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
  cout << "ping!!!!!!!!!!!!!!" << endl;
} // end ping


/**
 * Release the waiting semaphore
 */
void
CltWfMgr::release(const char * dag_id) {
    this->mySem.post();
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
