/****************************************************************************/
/* The MA DAG CORBA object implementation                                   */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
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
//#include "WfReader.hh"
#include "ORBMgr.hh"
#include "debug.hh"
#include "Parsers.hh"
#include "statistics.hh"

#include "RoundRobbin_MaDag_Sched.hh"
#include "HEFT_MaDag_Sched.hh"
#include "MultiWfFairness_Sched.hh"

using namespace std;

MaDag_impl::MaDag_impl(const char * name) :
  myName(name),
  mySched(NULL) {
  char* parentName = (char*)
    Parsers::Results::getParamValue(Parsers::Results::PARENTNAME);

  // check if the parent is NULL

  this->parent =
    MasterAgent::_duplicate(MasterAgent::_narrow(ORBMgr::getObjReference(ORBMgr::AGENT,
							     parentName)));
  if (CORBA::is_nil(this->parent)) {
    cerr << "Cannot locate the master agent " << parentName << endl;
    exit(1);
  }

  /* Bind the MA DAG to its name in the CORBA Naming Service */
  if (ORBMgr::bindObjToName(_this(), ORBMgr::MA_DAG, this->myName.c_str())) {
    cerr << "could not declare myself as " << this->myName << endl;
    exit(1);
  }
  else {
    TRACE_TEXT(TRACE_MAIN_STEPS, 
	       "binding MA_DAG "<<this->myName<<" to naming service"<<endl);
  }

  TRACE_TEXT(TRACE_MAIN_STEPS,
	     "\nMA DAG " << this->myName << " created.\n");

  // set the parameters
  this->multi_wf = false;
  this->metaSched = NULL;

  // init the statistics module
  stat_init();
} // end MA DAG constructor

MaDag_impl::~MaDag_impl() {
  if (mySched != NULL)
    delete (mySched);
  if (metaSched != NULL) 
    delete (metaSched);
} // end MA DAG destructor

/** 
 * Workflow submission function. 
 */
wf_sched_response_t * 
MaDag_impl::submit_wf (const corba_wf_desc_t& wf_desc) {

  stat_in("MA DAG","Start workflow request");

  // MA DAG delivers odd dag id
  static int dag_id = -1;
  dag_id += 2;
  this->myMutex.lock();
  if (this->multi_wf) {
    wf_sched_response_t * tmp = 
      this->metaSched->submit_wf(wf_desc, dag_id, this->parent);
    this->myMutex.unlock();
    stat_out("MA DAG","End workflow request");
    return tmp;
  }
  wf_sched_response_t * sched_resp = new wf_sched_response_t;
  sched_resp->dag_id = dag_id;
  sched_resp->wf_node_sched_seq.length(0);
  wf_response_t * wf_response = NULL; // new wf_response_t;
  //  wf_response-> complete = true;
  cout << "The MaDag receives a workflow submission!" <<endl;

  // read the workflow description
  // transform the description to a data structure
  WfExtReader reader(wf_desc.abstract_wf);
  reader.setup();

  // check the services
  unsigned int len = reader.pbs_list.size();
  corba_pb_desc_seq_t pbs_seq;
  pbs_seq.length(len);
  for (unsigned int ix=0; ix< len; ix++) {
    pbs_seq[ix] = reader.pbs_list[ix];
  }

  cout << "send the problems sequence to the master agent ... " << endl;
  wf_response = this->parent->submit_pb_set(pbs_seq, reader.getDagSize());
  cout << "... done" << endl;

  

  // construct the response/scheduling

  if ( ! wf_response->complete) {
    sched_resp->dag_id = wf_response->dag_id;
    cout << "The response is incomplete" << endl;
    this->myMutex.unlock();
    stat_out("MA DAG","End workflow request");
    return sched_resp;
  }

  // By default use the Round Robbin scheduler
  if (mySched == NULL) {
    mySched = new RoundRobbin_MaDag_Sched();
  }

  sched_resp->wf_node_sched_seq = mySched->schedule(wf_response, reader);
  sched_resp->dag_id = wf_response->dag_id;

  this->myMutex.unlock();

  stat_out("MA DAG","End workflow request");

  return sched_resp;
}

/**
 * Another workflow submission function *
 * return only the ordering of the nodes execution *
 */
wf_ordering_t *
MaDag_impl::get_wf_ordering(const corba_wf_desc_t& wf_desc) {
  wf_ordering_t * wf_ordering = new wf_ordering_t;
  // TO COMPLETE
  return wf_ordering;  
}

/**
 * set the remaining dag
 */
void
MaDag_impl::remainingDag(const char * dag_descr) {
}

/**
 * set the scheduler for the MA DAG
 */
void
MaDag_impl::set_sched(const madag_sched_t madag_sched) {
  switch (madag_sched) {
  case round_robbin_sched:
    if (mySched != NULL) {
      delete (mySched);
    }
    mySched = new RoundRobbin_MaDag_Sched();
    break;

  case heft_sched:
    if (mySched != NULL) {
      delete (mySched);
    }
    mySched = new HEFT_MaDag_Sched();
    break;

  default:
    break;
  } // end switch 
}


/**
 * enable the multi-workflow support
 * sched is by default set MWF_DEFAULT
 */
void
MaDag_impl::enable_multi_wf(bool enable,
			    const madag_mwf_sched_t sched) {
  if (enable)
    this->multi_wf = true;
  else
    return;

  switch (sched) {
  case MWF_FAIRNESS:
    this->metaSched = new MultiWfFairness_Sched();
    break;
  case MWF_DEFAULT:
    this->metaSched = new WfMetaSched();
    break;
  default:
    this->metaSched = new WfMetaSched();
  } // end switch
}

/*
 * inform the MA DAG that node execution is done
 */
void 
MaDag_impl::setAsDone(const char* dagId, const char* nodeId) {
  if ( (this->multi_wf) &&
       (this->metaSched != NULL) ) {
    TRACE_TEXT(TRACE_MAIN_STEPS, 
	       "The node " << nodeId << 
	       " in the dag " << dagId << " is done" << endl);
    this->metaSched->setNodeAsDone(dagId, nodeId);
  }
}

/**
 * inform the ma dag that the dag execution is complete
 */
void 
MaDag_impl::setDagAsDone(const char * dag_id) {
  if ( (this->multi_wf) &&
       (this->metaSched != NULL) ) {
    TRACE_TEXT(TRACE_MAIN_STEPS, 
	       "Remove the dag " << dag_id << " from the clients table" << endl);
    this->metaSched->removeClient(dag_id);
  }
}

/*
 * register a dag to the ma dag
 */
void
MaDag_impl::registerClt(const char* dag_id, const char* client_ref) {
  if (this->multi_wf == false)
    cout << "this->multi_wf  == false" << endl;
  if (this->metaSched == NULL)
    cout << "this->metaSched == NULL" << endl;
  if ( (this->multi_wf) &&
       (this->metaSched != NULL) ) {
    CORBA::Object_ptr obj = 
      ORBMgr::stringToObject(client_ref);
    if (obj == NULL) {
      WARNING("The new client reference is not valid, this request is ignored");
      return;
    }
    CltReoMan_var clt = CltReoMan::_narrow(obj);
    if (clt == NULL) {
      WARNING("Unable to get a client reference");
      return;
    }
    // store the dag and the client ref in the meta scheduler table
    TRACE_TEXT(TRACE_MAIN_STEPS, 
	       "Registring a new dag client: "  << dag_id << endl);
    this->metaSched->addClient(clt, dag_id);
  }
}
