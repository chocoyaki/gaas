/****************************************************************************/
/* [Documentation Here!]                                                    */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id : 
 * $Log : 
 ****************************************************************************/

#include "WfMetaSched.hh"
#include "debug.hh"
#include "RoundRobbin_MaDag_Sched.hh"
#include "marshalling.hh"

WfMetaSched::WfMetaSched() 
  : AbsWfMetaSched() {
}

WfMetaSched::~WfMetaSched() {
  if (this->myMetaDag != NULL)
    delete this->myMetaDag;
  
  if (this->mySched != NULL)
    delete this->mySched;
}

void 
WfMetaSched::setSched(MaDag_sched * sched) {
}

/**
 * Workflow submission function. 
 */
wf_sched_response_t * 
WfMetaSched::submit_wf (const corba_wf_desc_t& wf_desc, int dag_id,
			  MasterAgent_var parent) {
  CltReoMan_var clt = NULL;
  // get the remaining dags from all the clients
  cout << endl <<
    "get the remaining dags from all the clients" <<
    endl << endl;
  for (map<string, CltReoMan_var>::iterator p = this->myClients.begin();
       p != this->myClients.end();
       ++p) {
    clt = (CltReoMan_var)(p->second);
    if (clt != NULL) {
      cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
      cout << clt->getremainingDag() << endl;
      cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    }
  }
  

  wf_sched_response_t * wf_resp = new wf_sched_response_t;
  wf_resp->dag_id = dag_id;

  cout << "The meta scheduler receive a new dag " << endl
       << wf_desc.abstract_wf << endl;

  WfExtReader reader(wf_desc.abstract_wf);
  reader.setup();
  reader.getDag()->setId(itoa(dag_id));
  this->myMetaDag->addDag(reader.getDag());
  cout << " %%%%%%%%%%%%%%%%%%%%%%%%%% " << endl << endl << endl;
  cout << "The Meta Dag XML representation " << endl;
  cout << this->myMetaDag->toXML() << endl;
  cout << " %%%%%%%%%%%%%%%%%%%%%%%%%% " << endl << endl << endl;

  // check the services
  Dag * dag = this->myMetaDag->getDag();
  dag->setAsTemp(true);
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

  cout << "WfMetaSched : send the problems sequence to the MA  ... " << endl;
  wf_response_t * wf_response = parent->submit_pb_set(pbs_seq, dag->size());
  cout << "... done" << endl;

  wf_resp->dag_id = wf_response->dag_id;
  wf_resp->firstReqID = wf_response->firstReqID;
  wf_resp->lastReqID = wf_response->lastReqID;
  wf_resp->ma_response = *wf_response;

  // construct the response/scheduling

  if ( ! wf_response->complete) {
    cout << "The response is incomplete" << endl;
    return wf_resp;
  }

  // By default use the Round Robbin scheduler
  if (mySched == NULL) {
    mySched = new RoundRobbin_MaDag_Sched();
  }

  wf_resp->wf_node_sched_seq = mySched->schedule(wf_response, dag);

  if (dag != NULL)
    delete (dag);

  return extract(dag_id, wf_resp);
}

