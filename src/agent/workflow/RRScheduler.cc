/****************************************************************************/
/* The MA DAG round robbin scheduler                                        */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2008/04/10 09:13:29  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 ****************************************************************************/

#include <vector>

#include "RRScheduler.hh"

using namespace std;
using namespace madag;

RRScheduler::RRScheduler() {
  
}

RRScheduler::~RRScheduler() {
}

wf_node_sched_seq_t  
RRScheduler::schedule(const wf_response_t * wf_response,
				  WfParser& reader,
                                  CORBA::Long dag_id) {
  wf_node_sched_seq_t sched_seq;
  sched_seq.length(0);

  unsigned int len = reader.pbs_list.size();

  vector<int> rr;
  unsigned int index;

  for (unsigned int ix=0; ix<len; ix++) {
    rr.push_back(0);
  }

  len = reader.nodes_list.size();  
  sched_seq.length(len);
  unsigned int ix = 0;
  for (map<string, corba_pb_desc_t>::iterator p = reader.nodes_list.begin();
       p != reader.nodes_list.end();
       ++p) {
    string nid = (string)(p->first);
    sched_seq[ix].node_id = 
      CORBA::string_dup(nid.c_str());
    
    index = reader.indexOfPb(reader.nodes_list[nid]);
    // normally the pb index in pbs_list must be egal to the index in 
    // the wf_response
    sched_seq[ix].server  = 
      wf_response->wfn_seq_resp[index].response.servers[rr[index]];

    rr[index] = 
      (rr[index] + 1) % wf_response->wfn_seq_resp[index].response.servers.length();
    ix++;
  }
  
  return sched_seq;
}

wf_node_sched_seq_t 
RRScheduler::schedule(const wf_response_t * wf_response,
				  Dag * dag){
  wf_node_sched_seq_t sched_seq;
  sched_seq.length(0);

  unsigned int len = wf_response->wfn_seq_resp.length();
  cout << "***********" << len << endl;
  vector<int> rr;
  for (unsigned int ix=0; ix<len; ix++) {
    rr.push_back(0);
  }

  len = dag->size();  
  sched_seq.length(len);
  // The dag nodes
  vector<Node *> v = dag->getNodes();
  // The dag profiles
  vector<diet_profile_t*> profiles = dag->getAllProfiles();

  int index = -1;
  for (unsigned int ix = 0; ix < v.size(); ix++) {
    sched_seq[ix].node_id = 
      CORBA::string_dup(v[ix]->getId().c_str());
    
    index = -1;
    for (unsigned int jx = 0; jx < wf_response->wfn_seq_resp.length(); jx++) {
      if ( string(wf_response->wfn_seq_resp[jx].node_id) == 
	   (v[ix]->getPb()) ) {
	index = jx;
	break;
      }      
    }
    // normally the pb index in pbs_list must be egal to the index in 
    // the wf_response
    // not true in multi-workflow submission
    // The previous comment if for the old code version
    cout << "Node " << ix << " is mapped to server " << index << endl;
    sched_seq[ix].server  = 
      wf_response->wfn_seq_resp[index].response.servers[rr[index]];

    rr[index] = 
      (rr[index] + 1) % wf_response->wfn_seq_resp[index].response.servers.length();
  }
  
  return sched_seq;  
}
 
wf_node_sched_seq_t 
RRScheduler::reSchedule(const wf_response_t * wf_response,
				    WfParser& reader) {
  wf_node_sched_seq_t sched_seq;
  sched_seq.length(0);
  return sched_seq;
}

wf_node_sched_seq_t 
RRScheduler::reSchedule(const wf_response_t * wf_response,
				    Dag * dag) {
  wf_node_sched_seq_t sched_seq;
  sched_seq.length(0);
  return sched_seq;
}


/**
 * TO COMPLETE
 */
wf_node_sched_t
RRScheduler::schedule(const wf_response_t * response, 
				  Node * n) {
  wf_node_sched_t node_sched;
  return node_sched;
}

/**
 * TO COMPLETE
 */
double
RRScheduler::getAFT(string nodeId) {
  return 0;
}
