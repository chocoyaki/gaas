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
 * Revision 1.1  2008/04/10 08:15:54  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 ****************************************************************************/

#include "HEFTScheduler.hh"
#include "debug.hh"

using namespace madag;

HEFTScheduler::HEFTScheduler() {
}

HEFTScheduler::~HEFTScheduler() {
}

/**
 * rank the node upward 
 */

void
HEFTScheduler::rank(Node * n) {
  Node * succ = NULL;
  unsigned len = n->nextNodesCount();
  for (unsigned int ix=0; ix<len; ix++) {
    succ = (Node*)(n->getNext(ix));
    if ((succ->getPriority() + WI[n->getCompleteId()]) > n->getPriority()) {
      n->setPriority(succ->getPriority() + WI[n->getCompleteId()]);
    }
  }
  len = n->prevNodesCount();
  Node * prev = NULL;
  for (unsigned int ix=0; ix<len; ix++) {
    prev = (Node*)(n->getPrev(ix));
    if ((!prev->isDone()) && (!prev->isRunning()))
      rank(prev);
  }
}

wf_node_sched_seq_t 
HEFTScheduler::schedule(const wf_response_t * response,
			   WfParser& reader,
                           CORBA::Long dag_id) {
  Dag * myDag = reader.getDag();
  myDag->setId(itoa(response->dag_id));
  return this->schedule(response, myDag);
}

/**
 *
 */
wf_node_sched_seq_t 
HEFTScheduler::schedule(const wf_response_t * response,
			   Dag * dag) {
  struct timeval current_time;
  gettimeofday(&current_time, NULL);

  for (map<string, double>::iterator p = avail.begin();
       p != avail.end();
       p++) {
    if (p->second < current_time.tv_sec) {
      avail[p->first] = current_time.tv_sec;
    }
  }

  wf_node_sched_seq_t sched_seq;
  sched_seq.length(dag->size());

  Node * n = NULL;
  Dag * myDag = dag;
  myDag->linkAllPorts();

  TRACE_TEXT (TRACE_ALL_STEPS, "The MA DAG HEFT Scheduler starts  " << endl);

  // ranking
  TRACE_TEXT (TRACE_ALL_STEPS, "HEFT : start ranking" << endl);
  for (std::map <std::string, Node *>::iterator p = myDag->begin();
       p != myDag->end();
       p++) {
    // compute WI[ix];
    n = (Node *)(p->second);
    WI[n->getCompleteId()] = 0;
    // found the corresponding response in wf_response
    for (unsigned int ix=0; ix<response->wfn_seq_resp.length(); ix++) {
      if (!strcmp(n->getPb().c_str(), response->wfn_seq_resp[ix].node_id)) {
	// compute WI[ix]
	double w = 0;
	for (unsigned int jx=0; 
	     jx<response->wfn_seq_resp[ix].response.servers.length(); 
	     jx++) {
          /*
	  cout << "\t ** estValues.length() " << 
	    response->wfn_seq_resp[ix].response.servers[jx].estim.estValues.length() 
	       << endl;
          */
	  w += response->wfn_seq_resp[ix].response.servers[jx].estim.estValues[1].v_value;
	} // end for jx
	WI[n->getCompleteId()] = w/response->wfn_seq_resp[ix].response.servers.length();
      }
    }
  }
  for (std::map <std::string, Node *>::iterator p = myDag->begin();
       p != myDag->end();
       p++) {
    n = (Node *)(p->second);
    if (n->isAnExit())
      rank(n);
  }
  TRACE_TEXT (TRACE_ALL_STEPS, "HEFT : ranking done" << endl);

  for (std::map <std::string, Node *>::iterator p = myDag->begin();
       p != myDag->end();
       p++) {
    n = (Node*)(p->second);
    TRACE_TEXT (TRACE_ALL_STEPS,  "--- Node ID = " << n->getCompleteId() << 
                ", node RANK = " << n->getPriority() << endl);
  }
  // init the availability map
  for (unsigned int ix=0;
       ix < response->wfn_seq_resp.length();
       ix++) {
    for (unsigned int jx=0;
	 jx < response->wfn_seq_resp[ix].response.servers.length(); 
	 jx++) {
      string hostname(CORBA::string_dup(response->wfn_seq_resp[ix].response.servers[jx].loc.hostName));
      // if new host init its availability to current time
      if (avail.find(hostname) == avail.end())
	avail[hostname] = current_time.tv_sec;
    }
  }

  vector<Node*> sorted_list;
  Node * n1 = NULL;
  // Sort the nodes by nonincreasing rank
  TRACE_TEXT (TRACE_ALL_STEPS, "HEFT : start sorting list" << endl);
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
  TRACE_TEXT (TRACE_ALL_STEPS, "HEFT : sorting list done" << endl);

  unsigned int pb_index = 0;

  // init AFT and AST map;
  for (std::map <std::string, Node *>::iterator p = myDag->begin();
       p != myDag->end();
       p++) {
    n = (Node*)(p->second);
    AFT[n->getCompleteId()] = 0;
    AST[n->getCompleteId()] = 0;
  }
  // compute EST and EFT
  TRACE_TEXT (TRACE_ALL_STEPS, "HEFT : start computing EST & EFT" << endl);
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
	   jx < n->prevNodesCount();
	   jx++) {
	EST = max(EST, AFT[n->getPrev(jx)->getCompleteId()]);
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
      CORBA::string_dup(n->getCompleteId().c_str());
    sched_seq[response_index].priority = 
      n->getPriority();
    sched_seq[response_index].server = 
      response->wfn_seq_resp[pb_index].response.servers[sed_ind];
    response_index++;

    string str(CORBA::string_dup(response->wfn_seq_resp[pb_index].response.servers[sed_ind].loc.hostName));
    avail[str] = EFT;
    AFT[n->getCompleteId()] = EFT;
    AST[n->getCompleteId()] = EFT;
    n->setEstCompTime((long int)EFT);
  }
  TRACE_TEXT (TRACE_ALL_STEPS, "HEFT : Computing AFT & AST done" << endl);

  return sched_seq;
}

wf_node_sched_seq_t 
HEFTScheduler::reSchedule(const wf_response_t * wf_response,
			     WfParser& reader) {
  wf_node_sched_seq_t sched_seq;
  sched_seq.length(0);
  return sched_seq;
}


wf_node_sched_seq_t 
HEFTScheduler::reSchedule(const wf_response_t * wf_response,
			     Dag * dag) {
  wf_node_sched_seq_t sched_seq;
  sched_seq.length(0);
  return sched_seq;
}


wf_node_sched_t
HEFTScheduler::schedule(const wf_response_t * response, 
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
	 jx < n->prevNodesCount();
	 jx++) {
      EST = max(EST, AFT[n->getPrev(jx)->getCompleteId()]);
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
    CORBA::string_dup(n->getCompleteId().c_str());
  node_sched.server = 
    response->wfn_seq_resp[pb_index].response.servers[sed_ind];
  string ss(CORBA::string_dup(response->wfn_seq_resp[pb_index].response.servers[sed_ind].loc.hostName));
  avail[ss] = EFT;
  AFT[n->getCompleteId()] = EFT;
  AST[n->getCompleteId()] = EFT;
  n->setEstCompTime((long int)EFT);

  return node_sched;
}

double
HEFTScheduler::getAFT(string nodeId) {
  if (this->AFT.find(nodeId) != this->AFT.end())
    return AFT[nodeId];
  return 0;
}
