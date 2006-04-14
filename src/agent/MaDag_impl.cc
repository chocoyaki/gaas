/****************************************************************************/
/* [Documentation Here!]                                                    */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2006/04/14 13:43:59  aamar
 * source of the MA DAG CORBA object.
 *
 ****************************************************************************/

#include <iostream>

#include "MaDag_impl.hh"
#include "WfReader.hh"
#include "ORBMgr.hh"
#include "debug.hh"
#include "Parsers.hh"

using namespace std;

MaDag_impl::MaDag_impl(const char * name) :
  myName(name) {
  char* parentName = (char*)
    Parsers::Results::getParamValue(Parsers::Results::PARENTNAME);

  // check if the parent is NULL

  this->parent =
    MasterAgent::_duplicate(MasterAgent::_narrow(ORBMgr::getObjReference(ORBMgr::AGENT,
							     parentName)));
  if (CORBA::is_nil(this->parent)) {
    cout << "cannot locate the master agent " << parentName << endl;
  }

  TRACE_TEXT(TRACE_MAIN_STEPS,
	     "\nMA DAG " << this->myName << " created.\n");

  /* Bind the MA DAG to its name in the CORBA Naming Service */
  if (ORBMgr::bindObjToName(_this(), ORBMgr::MA_DAG, this->myName.c_str())) {
    cout << "could not declare myself as " << this->myName << endl;
  }
  else {
    cout << "binding MA_DAG "<< this->myName << " to naming service" << endl;
  }
}

/** Workflow submission function. */
wf_node_sched_seq_t * 
MaDag_impl::submit_wf (const corba_wf_desc_t& wf_desc) {
  wf_node_sched_seq_t * sched_seq = new wf_node_sched_seq_t;
  sched_seq->length(0);
  wf_response_t * wf_response = new wf_response_t;
  wf_response-> complete = true;
  cout << "The MaDag receives a workflow submission!" <<endl;

  // read the workflow description
  // transform the description to a data structure
  //
  WfReader reader(wf_desc.abstract_wf);
  reader.setup();

  // check the services
  //
  unsigned int len = reader.pbs_list.size();
  corba_pb_desc_seq_t pbs_seq;
  pbs_seq.length(len);
  for (unsigned int ix=0; ix< len; ix++) {
    pbs_seq[ix] = reader.pbs_list[ix];
  }

  cout << "send the problems sequence to the master agent ... " << endl;
  wf_response = this->parent->submit_pb_set(pbs_seq);
  cout << "... done" << endl;

#if 0
  corba_pb_desc_t  * pb_desc = NULL;
  corba_response_t * corba_response = NULL;

  reader.pbReset();
  while (reader.hasPbNext()) {
    pb_desc = reader.pbNext();
    if (pb_desc != NULL) {
      cout<<"pb path : "<< pb_desc->path << endl;
      // TO COMPLETE
      // submit to the MA
      corba_response = this->parent->submit(*pb_desc, 16);
      if (!corba_response || corba_response->servers.length() == 0) {
	cout << "no server found for the service " <<
	  pb_desc->path << endl;
	wf_response->complete = false;
	return wf_response;
      }
      else {
	// .....
	cout << corba_response->servers.length() << " were found for the pb "
	     << pb_desc->path << endl;
	// TO FIX (memory lack)
	wf_node_response_t * wf_node_resp = new wf_node_response_t;
	wf_node_resp->node_id = CORBA::string_dup(pb_desc->path);
	wf_node_resp->response = *corba_response;
	unsigned int len = wf_response->wfn_seq_resp.length();
	wf_response->wfn_seq_resp.length(len+1);
	wf_response->wfn_seq_resp[len] = *wf_node_resp;
	delete (corba_response);
      }
    }
    
  }
#endif // 0


  // construct the response/scheduling
  //

  if ( ! wf_response->complete) {
    cout << "The " << endl;
    return sched_seq;
  }
  
  len = reader.pbs_list.size();

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
