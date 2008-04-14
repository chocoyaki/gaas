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

#include "RRScheduler.hh"
#include "HEFTScheduler.hh"
#include "MultiWfBasicScheduler.hh"
#include "MultiWfFOFT.hh"
#include "debug.hh"

using namespace std;
using namespace madag;

MaDag_impl::MaDag_impl(const char * name) :
  myName(name), mySched(NULL), myMultiWfSched(NULL), dagId(0) {

  char* parentName = (char*)
    Parsers::Results::getParamValue(Parsers::Results::PARENTNAME);

  // check if the parent is NULL
  if (parentName == NULL) {
    ERROR_EXIT("Parent name not provided");
  }
  CORBA::Object_var obj = ORBMgr::getObjReference(ORBMgr::AGENT, parentName);
  this->parent = MasterAgent::_duplicate(MasterAgent::_narrow(obj));
  if (CORBA::is_nil(this->parent)) {
    ERROR_EXIT("Cannot locate the master agent " << parentName);
  }

  /* Bind the MA DAG to its name in the CORBA Naming Service */
  if (ORBMgr::bindObjToName(_this(), ORBMgr::MA_DAG, this->myName.c_str())) {
    ERROR_EXIT("could not declare myself as " << this->myName);
  }

  TRACE_TEXT(TRACE_MAIN_STEPS,
	     endl <<  "MA DAG " << this->myName << " created." << endl);

  TRACE_TEXT(NO_TRACE, 
	       "## MADAG_IOR " << ORBMgr::getIORString(this->_this()) << endl);
  // init the statistics module
  stat_init();
} // end MA DAG constructor

MaDag_impl::~MaDag_impl() {
  if (this->mySched != NULL) {
    delete (this->mySched);
  }
  if (this->myMultiWfSched != NULL) {
    delete (this->myMultiWfSched);
  }
} // end MA DAG destructor

/**
 * Workflow submission function
 */
dag_response_t* 
MaDag_impl::submit_dag(const corba_wf_desc_t& dag_desc, CORBA::Long& firstReqId) {
    dag_response_t * dag_response = new dag_response_t;
    
    return dag_response;
}

/**
 * Workflow submission in multi-workflow mode
 */
CORBA::Boolean
MaDag_impl::submit_dag_in_multi(const corba_wf_desc_t& dag_desc, 
                                const char* cltMgrRef, 
                                CORBA::Long dag_id) {
    return true;
}

/** 
 * Workflow submission function. 
 */
wf_sched_response_t * 
MaDag_impl::submit_wf (const corba_wf_desc_t& wf_desc,
		       CORBA::Boolean used,
                       CORBA::Boolean multi_mode,
                       const char * cltMgrIOR,
                       CORBA::Long dag_id) {

  stat_in("MA DAG","Start workflow request");
  wf_sched_response_t * result = NULL;

  this->myMutex.lock();
  if (multi_mode) {
    // Get the client referencex
    CORBA::Object_ptr obj = ORBMgr::stringToObject(cltMgrIOR);
    CltMan_ptr cltMan = CltMan::_narrow(obj);
    if (this->multiMode_wf_sub(wf_desc, used, cltMan, dag_id)) {
    	result = new wf_sched_response_t;
    	result->dag_id = dag_id;
	result->complete = true;
    }
    else {
        stat_out("MA DAG","Workflow request aborted");
        result = new wf_sched_response_t;
	result->complete = false;
    }
  }
  else {
    result = this->monoMode_wf_sub(wf_desc, used, dag_id);
  } 
  this->myMutex.unlock();
  stat_out("MA DAG","End workflow request");
  return result;
} // end submit_wf

/**
 * set the scheduler for the MA DAG
 */
void
MaDag_impl::setScheduler(const madag_sched_t madag_sched) {
  switch (madag_sched) {
  case madag_rr:
    if (this->mySched != NULL) {
      delete (this->mySched);
    }
    this->mySched = new RRScheduler();
    break;

  case madag_heft:
    if (this->mySched != NULL) {
      delete (this->mySched);
    }
    this->mySched = new HEFTScheduler();
    break;

  default:
    break;
  } // end switch 
} // end setScheduler

/**
 * set the scheduler for multi workflow mode
 */
void
MaDag_impl::setMultiWfScheduler(const madag_mwf_sched_t madag_multi_sched) {
  switch (madag_multi_sched) {
  case MWF_DEFAULT:
    if (this->myMultiWfSched != NULL) {
      delete (this->myMultiWfSched);
    }
    this->myMultiWfSched = new MultiWfBasicScheduler();
    break;

  case MWF_FAIRNESS:
    if (this->myMultiWfSched != NULL) {
      delete (this->myMultiWfSched);
    }
    this->myMultiWfSched = new MultiWfFOFT();
    break;

  default:
    break;
  } // end switch 
} // end setScheduler



/**
 * Schedule workflow in a normal mode
 * @deprecated
 * TODO Remove wf submission in Mono mode and change return type of submit_wf
 */
wf_sched_response_t *
MaDag_impl::monoMode_wf_sub(const corba_wf_desc_t& wf_desc,
                            const bool used,
                            const long dag_id) {
  wf_sched_response_t * sched_resp = new wf_sched_response_t;
  sched_resp->dag_id = dag_id;
  sched_resp->wf_node_sched_seq.length(0);
  WfParser reader(wf_desc.abstract_wf);
  reader.setup();
  // check the services
  unsigned int len = reader.pbs_list.size();
  corba_pb_desc_seq_t pbs_seq;
  pbs_seq.length(len);
  for (unsigned int ix=0; ix< len; ix++) {
    pbs_seq[ix] = reader.pbs_list[ix];
  }

  wf_response_t * wf_response = 
    this->parent->submit_pb_set(pbs_seq, reader.getDagSize(), used);

  sched_resp->firstReqID = wf_response->firstReqID;
  sched_resp->lastReqID = wf_response->lastReqID;
  sched_resp->ma_response = *wf_response;

  if ( ! sched_resp->ma_response.complete) {
    cerr << "The response is incomplete" << endl;
    return sched_resp;
  }

  // By default use the Round Robbin scheduler
  if (this->mySched == NULL) {
    this->mySched = new RRScheduler();
  }

  sched_resp->wf_node_sched_seq = mySched->schedule(&sched_resp->ma_response, 
                                                    reader, dag_id);

  return sched_resp;
} // end monoMode_wf_sub

/**
 * Schedule workflow in multi-workflow mode
 */
bool
MaDag_impl::multiMode_wf_sub(const corba_wf_desc_t& wf_desc,  
                             const bool used,
                             CltMan_ptr cltMan,
                             const long dag_id) {
  if (this->myMultiWfSched == NULL)
    this->myMultiWfSched = new MultiWfBasicScheduler();
  return this->myMultiWfSched->submit_wf(wf_desc, dag_id, 
                                  this->parent, used,
                                  cltMan);
} // end multiMode_wf_sub

/**
 * Get a new DAG identifier
 */ 
CORBA::Long
MaDag_impl::getDagId() {
  this->myMutex.lock();
  CORBA::Long res = this->dagId++;
  this->myMutex.unlock();
  return res;
}

/**
 * Used to test if it is alive.
 */
CORBA::Long
MaDag_impl::ping()
{
  TRACE_TEXT(TRACE_ALL_STEPS, "ping()\n");
  fflush(stdout); 
  return getpid();
} // ping()
