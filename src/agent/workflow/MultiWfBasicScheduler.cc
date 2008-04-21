/****************************************************************************/
/* The Workflow Meta-scheduler                                              */
/* This scheduler is used when multi-workflow support is enabled            */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.4  2008/04/21 14:31:45  bisnard
 * moved common multiwf routines from derived classes to MultiWfScheduler
 * use wf request identifer instead of dagid to reference client
 * use nodeQueue to manage multiwf scheduling
 * renamed WfParser as DagWfParser
 *
 * Revision 1.3  2008/04/15 14:20:19  bisnard
 * - Postpone sed mapping until wf node is executed
 *
 * Revision 1.2  2008/04/14 13:44:29  bisnard
 * - Parameter 'used' obsoleted in MultiWfScheduler::submit_wf & submit_pb_set
 *
 * Revision 1.1  2008/04/10 09:13:29  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 ****************************************************************************/

#include "MultiWfBasicScheduler.hh"
#include "debug.hh"
#include "HEFTScheduler.hh"
#include "marshalling.hh"

using namespace madag;

MultiWfBasicScheduler::MultiWfBasicScheduler(MaDag_impl* maDag)
  : MultiWfScheduler(maDag) {
  this->start();
}

MultiWfBasicScheduler::~MultiWfBasicScheduler() {
}

/**
 * Workflow submission function.
 */
bool
MultiWfBasicScheduler::submit_wf (const corba_wf_desc_t& wf_desc, int dag_id,
                                  MasterAgent_var parent,
                                  CltMan_var cltMan) {
  this->myLock.lock();
  wf_sched_response_t * wf_resp = new wf_sched_response_t;
  wf_resp->dag_id = dag_id;

  cout << "The meta scheduler receive a new dag " << endl
       << wf_desc.abstract_wf << endl;

  DagWfParser reader(wf_desc.abstract_wf);
  reader.setup();
  reader.getDag()->setId(itoa(dag_id));
//  this->cltMans[itoa(dag_id)] = cltMan;
  this->myMetaDag->addDag(reader.getDag());
//   cout << " %%%%%%%%%%%%%%%%%%%%%%%%%% " << endl << endl << endl;
//   cout << "The Meta Dag XML representation (" << endl;
//   cout << this->myMetaDag->toXML() << endl;
//   cout << " %%%%%%%%%%%%%%%%%%%%%%%%%% " << endl << endl << endl;

  // check the services
  Dag * dag = this->myMetaDag->getDag(itoa(dag_id));
  dag->setAsTemp(true); // avoids destruction of the dag
  vector<diet_profile_t*> v = dag->getAllProfiles();
  unsigned int len = v.size();
  corba_pb_desc_seq_t pbs_seq;
  pbs_seq.length(len);
  for (unsigned int ix=0; ix< len; ix++) {
    cout << "marshalling pb " << ix << endl
	 << "pb_name = " << v[ix]->pb_name << endl
	 << "last_in = " << v[ix]->last_in << endl
	 << "last_inout = " << v[ix]->last_inout << endl
	 << "last_out = " << v[ix]->last_out << endl;

    mrsh_pb_desc(&pbs_seq[ix], v[ix]);
  }

  cout << "MultiWfBasicScheduler: send the problems sequence to the MA  ... "
       << endl;
  wf_response_t * wf_response = parent->submit_pb_set(pbs_seq, dag->size());
  cout << "... done" << endl;

  wf_resp->dag_id = wf_response->dag_id;
  wf_resp->firstReqID = wf_response->firstReqID;
  wf_resp->lastReqID = wf_response->lastReqID;
  wf_resp->ma_response = *wf_response;

  // construct the response/scheduling

  if ( ! wf_response->complete) {
    cout << "The response is incomplete - dag is cancelled" << endl;
    this->myMetaDag->removeDag(itoa(dag_id));
    this->myLock.unlock();
    return false;
  }

  // By default use the Round Robbin scheduler
  if (mySched == NULL) {
    //    mySched = new RoundRobbin_MaDag_Sched();
    mySched = new HEFTScheduler();
  }

  wf_node_sched_seq_t sched_seq = mySched->schedule(wf_response, dag);

//  this->myMetaDag->mapSeDs(sched_seq);  // was used to assign sed

  this->wakeUp();

  this->myLock.unlock();

  return true;
}

