/****************************************************************************/
/* The MA DAG round robbin scheduler                                        */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id: 
 * $Log: 
 ****************************************************************************/

#include <vector>

#include "RoundRobbin_MaDag_Sched.hh"

using namespace std;

RoundRobbin_MaDag_Sched::RoundRobbin_MaDag_Sched() {
  
}

RoundRobbin_MaDag_Sched::~RoundRobbin_MaDag_Sched() {
}

wf_node_sched_seq_t * 
RoundRobbin_MaDag_Sched::schedule(const wf_response_t * wf_response,
				  WfReader& reader) {
  wf_node_sched_seq_t * sched_seq = new wf_node_sched_seq_t;
  sched_seq->length(0);

  unsigned int len = reader.pbs_list.size();

  vector<int> rr;
  unsigned int index;

  for (unsigned int ix=0; ix<len; ix++) {
    rr.push_back(0);
  }

  len = reader.nodes_list.size();  
  sched_seq->length(len);
  unsigned int ix = 0;
  for (map<string, corba_pb_desc_t>::iterator p = reader.nodes_list.begin();
       p != reader.nodes_list.end();
       ++p) {
    string nid = (string)(p->first);
    (*sched_seq)[ix].node_id = 
      CORBA::string_dup(nid.c_str());

    index = reader.indexOfPb(reader.nodes_list[nid]);
    // normally the pb index in pbs_list must be egal to the index in 
    // the wf_response
    (*sched_seq)[ix].server  = 
      wf_response->wfn_seq_resp[index].response.servers[rr[index]];

    rr[index] = 
      (rr[index] + 1) % wf_response->wfn_seq_resp[index].response.servers.length();
    ix++;
  }
  
  return sched_seq;

  return NULL;
}
 
wf_node_sched_seq_t * 
RoundRobbin_MaDag_Sched::reSchedule(const wf_response_t * wf_response,
				    WfReader& reader) {
  return NULL;
}

