/****************************************************************************/
/* The MA DAG HEFT scheduler                                                */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.4  2007/07/09 18:54:48  aamar
 * Adding Endianness support (CMake option).
 *
 * Revision 1.3  2006/11/27 09:53:00  aamar
 * Correct headers of source files used in workflow support.
 *
 ****************************************************************************/

#include "HEFT_MaDag_Sched.hh"

HEFT_MaDag_Sched::HEFT_MaDag_Sched() {
}

HEFT_MaDag_Sched::~HEFT_MaDag_Sched() {
}

/**
 * rank the node upward 
 */

void
HEFT_MaDag_Sched::rank(Node * n) {
  Node * succ = NULL;
  unsigned len = n->nextNb();
  for (unsigned int ix=0; ix<len; ix++) {
    succ = (Node*)(n->getNext(ix));
    if ((succ->getPriority() + WI[n->getId()]) > n->getPriority()) {
      n->setPriority(succ->getPriority() + WI[n->getId()]);
    }
  }
  len = n->prevNb();
  Node * prev = NULL;
  for (unsigned int ix=0; ix<len; ix++) {
    prev = (Node*)(n->getPrev(ix));
    if ((!prev->isDone()) && (!prev->isRunning()))
      rank(prev);
  }
}

wf_node_sched_seq_t 
HEFT_MaDag_Sched::schedule(const wf_response_t * response,
			   WfExtReader& reader) {
  Dag * myDag = reader.getDag();
  return this->schedule(response, myDag);
}

/**
 *
 */
wf_node_sched_seq_t 
HEFT_MaDag_Sched::schedule(const wf_response_t * response,
			   Dag * dag) {
  wf_node_sched_seq_t sched_seq;
  sched_seq.length(dag->size());

  Node * n = NULL;
  Dag * myDag = dag;

  cout << "The HEFT Scheduler starts execution " << endl;
  cout << "Linking the dag nodes ... " << endl;
  myDag->linkAllPorts();
  cout << "linking is done " << endl;

  // ranking
  cout << "HEFT : start ranking" << endl;
  for (std::map <std::string, Node *>::iterator p = myDag->begin();
       p != myDag->end();
       p++) {
    // compute WI[ix];
    n = (Node *)(p->second);
    WI[n->getId()] = 0;
    // found the corresponding response in wf_response
    for (unsigned int ix=0; ix<response->wfn_seq_resp.length(); ix++) {
      if (!strcmp(n->getPb().c_str(), response->wfn_seq_resp[ix].node_id)) {
	// compute WI[ix]
	double w = 0;
	for (unsigned int jx=0; 
	     jx<response->wfn_seq_resp[ix].response.servers.length(); 
	     jx++) {
	  cout << "estValues.length() " << 
	    response->wfn_seq_resp[ix].response.servers[jx].estim.estValues.length() 
	       << endl;
	  w += response->wfn_seq_resp[ix].response.servers[jx].estim.estValues[1].v_value;
	} // end for jx
	WI[n->getId()] = w/response->wfn_seq_resp[ix].response.servers.length();
      }
    }
    if (n->isAnExit())
      rank(n);
  }
  cout << "HEFT : ranking done" << endl;
  for (std::map <std::string, Node *>::iterator p = myDag->begin();
       p != myDag->end();
       p++) {
    n = (Node*)(p->second);
    cout << "--- Node ID = " << n->getId() << 
      ", node RANK = " << n->getPriority() << endl;
  }
  // init the availability map
  for (unsigned int ix=0;
       ix < response->wfn_seq_resp.length();
       ix++) {
    for (unsigned int jx=0;
	 jx < response->wfn_seq_resp[ix].response.servers.length(); 
	 jx++) {
      string hostname(CORBA::string_dup(response->wfn_seq_resp[ix].response.servers[jx].loc.hostName));
      // if new host init its availability to zero
      if (avail.find(hostname) == avail.end())
	avail[hostname] = 0;
    }
  }

  vector<Node*> sorted_list;
  Node * n1 = NULL;
  // Sort the nodes by nonincreasing rank
  cout << "HEFT : start sorting list" << endl;
  for (std::map <std::string, Node *>::iterator p = myDag->begin();
       p != myDag->end();
       p++) {
    n1 = (Node*)(p->second);
    // found where insert the node
    vector<Node*>::iterator p = sorted_list.begin();
    bool b = false;
    while ((p != sorted_list.end()) && (!b)) {
      Node * n2 = *p;
      if (n2->getPriority() < n1->getPriority())
	b = true;
      else
	p++;
    }
    sorted_list.insert(p, n1);
  }
  cout << "HEFT : sorting list done" << endl;

  unsigned int pb_index = 0;

  // init AFT and AST map;
  for (std::map <std::string, Node *>::iterator p = myDag->begin();
       p != myDag->end();
       p++) {
    n = (Node*)(p->second);
    AFT[n->getId()] = 0;
    AST[n->getId()] = 0;
  }
  // compute EST and EFT
  cout << "HEFT : start computing EST & EFT" << endl;
  unsigned int response_index = 0;
  while (sorted_list.size() > 0) {
    n = sorted_list[0];
    sorted_list.erase(sorted_list.begin());
    // found the problem index
    for (unsigned int ix=0; ix<response->wfn_seq_resp.length(); ix++) {
      if (!strcmp(n->getPb().c_str(), response->wfn_seq_resp[ix].node_id))
	pb_index = ix;
    } // end for ix
    unsigned int sed_ind = 0;
    double EFT = 0;
    for (unsigned int ix=0; 
	 ix<response->wfn_seq_resp[pb_index].response.servers.length(); 
	 ix++) {
      string ss(CORBA::string_dup(response->wfn_seq_resp[pb_index].response.servers[ix].loc.hostName));
      double EST = 
	avail[ss];
      for (unsigned int jx=0;
	   jx < n->prevNb();
	   jx++) {
	EST = max(EST, AFT[n->getPrev(jx)->getId()]);
      } // end for jx
      if ( (
	    EST + response->wfn_seq_resp[pb_index].response.servers[ix].estim.estValues[1].v_value < EFT ) 
	   || (EFT == 0)) {
	EFT = 
	  EST + response->wfn_seq_resp[pb_index].response.servers[ix].estim.estValues[1].v_value;
	sed_ind = ix;
      }
    } // end for ix
    //
    // n->setSeD(response->wfn_seq_resp[pb_index].response.servers[sed_ind].loc.ior);
    sched_seq[response_index].node_id = 
      CORBA::string_dup(n->getId().c_str());
    sched_seq[response_index].server = 
      response->wfn_seq_resp[pb_index].response.servers[sed_ind];
    response_index++;

    string str(CORBA::string_dup(response->wfn_seq_resp[pb_index].response.servers[sed_ind].loc.hostName));
    avail[str] = EFT;
    AFT[n->getId()] = EFT;
    AST[n->getId()] = EFT;
    n->setEstCompTime((long int)EFT);
  }
  cout << "HEFT : Computing AFT & AST done" << endl;


  return sched_seq;
}

wf_node_sched_seq_t 
HEFT_MaDag_Sched::reSchedule(const wf_response_t * wf_response,
			     WfExtReader& reader) {
  wf_node_sched_seq_t sched_seq;
  sched_seq.length(0);
  return sched_seq;
}


wf_node_sched_seq_t 
HEFT_MaDag_Sched::reSchedule(const wf_response_t * wf_response,
			     Dag * dag) {
  wf_node_sched_seq_t sched_seq;
  sched_seq.length(0);
  return sched_seq;
}


wf_node_sched_t
HEFT_MaDag_Sched::schedule(const wf_response_t * response, 
			   Node * n) {
  // found the problem index
  unsigned int pb_index = 0;
  for (unsigned int ix=0; ix<response->wfn_seq_resp.length(); ix++) {
    if (!strcmp(n->getPb().c_str(), response->wfn_seq_resp[ix].node_id)) {
      pb_index = ix;
      break;    
    }
  } // end for ix

  unsigned int sed_ind = 0;
  double EFT = 0;
  for (unsigned int ix=0; 
       ix<response->wfn_seq_resp[pb_index].response.servers.length(); 
       ix++) {
    string ss(CORBA::string_dup(response->wfn_seq_resp[pb_index].response.servers[ix].loc.hostName));
    double EST = 
      avail[ss];
    for (unsigned int jx=0;
	 jx < n->prevNb();
	 jx++) {
      EST = max(EST, AFT[n->getPrev(jx)->getId()]);
    } // end for jx
    if ( (
	  EST + response->wfn_seq_resp[pb_index].response.servers[ix].estim.estValues[1].v_value < EFT ) 
	 || (EFT == 0)) {
      EFT = 
	EST + response->wfn_seq_resp[pb_index].response.servers[ix].estim.estValues[1].v_value;
      sed_ind = ix;
    }
  } // end for ix

  // 
  wf_node_sched_t node_sched;
  node_sched.node_id = 
    CORBA::string_dup(n->getId().c_str());
  node_sched.server = 
    response->wfn_seq_resp[pb_index].response.servers[sed_ind];
  string ss(CORBA::string_dup(response->wfn_seq_resp[pb_index].response.servers[sed_ind].loc.hostName));
  avail[ss] = EFT;
  AFT[n->getId()] = EFT;
  AST[n->getId()] = EFT;
  n->setEstCompTime((long int)EFT);

  return node_sched;
}

double
HEFT_MaDag_Sched::getAFT(string nodeId) {
  if (this->AFT.find(nodeId) != this->AFT.end())
    return AFT[nodeId];
  return 0;
}
