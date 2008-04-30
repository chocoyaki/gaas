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
 * Revision 1.4  2008/04/30 07:32:24  bisnard
 * use relative timestamps for estimated and real completion time
 *
 * Revision 1.3  2008/04/28 11:54:52  bisnard
 * new methods setNodePriorities & setNodesEFT replacing schedule
 * nodes sort done in separate method in Dag class
 *
 * Revision 1.2  2008/04/21 14:31:45  bisnard
 * moved common multiwf routines from derived classes to MultiWfScheduler
 * use wf request identifer instead of dagid to reference client
 * use nodeQueue to manage multiwf scheduling
 * renamed WfParser as DagWfParser
 *
 * Revision 1.1  2008/04/10 08:15:54  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 ****************************************************************************/

#include "HEFTScheduler.hh"
#include "debug.hh"

using namespace madag;

/****************************************************************************/
/*                         PUBLIC METHODS                                   */
/****************************************************************************/

HEFTScheduler::HEFTScheduler() {
}

HEFTScheduler::~HEFTScheduler() {
}

/**
 * Set the priority of nodes using b-level algorithm
 */
void
HEFTScheduler::setNodesPriority(const wf_response_t * wf_response, Dag * dag) {

  TRACE_TEXT (TRACE_ALL_STEPS, "HEFT : Setting nodes priority" << endl);
  // Initialize the node weights (priority attr)
  this->computeNodeWeights(wf_response, dag);

  // Ranking
  Node * n = NULL;
  for (std::map <std::string, Node *>::iterator p = dag->begin();
       p != dag->end();
       p++) {
         n = (Node *)(p->second);
         if (n->isAnExit())
           rank(n);
  }
} // end setNodesPriority

/**
 * Initialize the Earliest Finish Time for all nodes
 * This version computes the EFT using the best server found and taking into
 * account the availability of the servers, considering that the current dag
 * has exclusive access to them.
 * Note that if there are many servers available for a given node, the
 * complexity of this method is <nb_servers> * <nb_predecessors> for each
 * node.
 */
void
HEFTScheduler::setNodesEFT(std::vector<Node *>& orderedNodes,
                           const wf_response_t * wf_response,
                           Dag * dag,
                           double& initTime,
                           double refTime) {
  // to store the availabilty of resources
  map<std::string, double> avail;
  // init the availability map (one entry per hostname)
  struct timeval current_time;
  gettimeofday(&current_time, NULL);
  initTime = current_time.tv_sec - refTime;
  for (unsigned int ix=0;
       ix < wf_response->wfn_seq_resp.length();
       ix++) {
    for (unsigned int jx=0;
	 jx < wf_response->wfn_seq_resp[ix].response.servers.length();
	 jx++) {
      string hostname(CORBA::string_dup(wf_response->wfn_seq_resp[ix].response.servers[jx].loc.hostName));
      // if new host init its availability to current time
      if (avail.find(hostname) == avail.end())
	avail[hostname] = initTime;
    }
  }
  TRACE_TEXT (TRACE_ALL_STEPS, "HEFT : start computing nodes EFT" << endl);
  // LOOP-1: for all dag nodes in the order provided
  for (std::vector<Node *>::iterator p = orderedNodes.begin();
       p != orderedNodes.end();
       p++) {
    Node *        n = (Node *) *p;
    unsigned int  pb_index = 0; // index of the service (problem) in the wf_response
    unsigned int  sed_ind = 0;  // index of the chosen server in the wf_response
    double        EFT = 0;      // earliest finish time for current node

    // find the problem index
    for (unsigned int ix=0; ix < wf_response->wfn_seq_resp.length(); ix++) {
      if (!strcmp(n->getPb().c_str(), wf_response->wfn_seq_resp[ix].node_id))
	pb_index = ix;
    } // end for ix

    /* LOOP-2 FOR ONE NODE => CHOOSES THE BEST SERVER & COMPUTES AFT */
    /* Remark: this loop can be very costly: <nb servers> * <nb predecessors> !! */
    for (unsigned int ix=0;
	 ix<wf_response->wfn_seq_resp[pb_index].response.servers.length();
	 ix++) {
      string ss(CORBA::string_dup(wf_response->wfn_seq_resp[pb_index].response.servers[ix].loc.hostName));
      double EST = avail[ss];
      for (unsigned int jx=0;
	   jx < n->prevNodesCount();
	   jx++) {
	EST = max(EST, AFT[n->getPrev(jx)->getCompleteId()]);
      } // end for jx
      if ( (
	    EST + wf_response->wfn_seq_resp[pb_index].response.servers[ix].estim.estValues[1].v_value < EFT )
	   || (EFT == 0)) {
	EFT =
	  EST + wf_response->wfn_seq_resp[pb_index].response.servers[ix].estim.estValues[1].v_value;
	sed_ind = ix;
      }
    } // end for ix

    /* END OF THE MAIN COMPUTATION LOOP */

    // update availability of chosen server
    string str(CORBA::string_dup(wf_response->wfn_seq_resp[pb_index].response.servers[sed_ind].loc.hostName));
    avail[str] = EFT;
    // update node scheduling info
    AFT[n->getCompleteId()] = EFT;
    n->setEstCompTime((long int)EFT);
    TRACE_TEXT (TRACE_ALL_STEPS, "HEFT : node " << n->getCompleteId()
        << " : EFT = " << EFT << endl);
  } // end LOOP-1
} // end setNodesEFT

/****************************************************************************/
/*                          PRIVATE METHODS                                 */
/****************************************************************************/

/**
 * Computes the average value of node workload across the Seds
 * (result stored in the WI table)
 */
void
HEFTScheduler::computeNodeWeights(const wf_response_t * wf_response,
                                  Dag * dag) {
  TRACE_TEXT (TRACE_ALL_STEPS, "HEFT : start computing weights (mean of estimates)" << endl);
  Node * n = NULL;
  for (std::map <std::string, Node *>::iterator p = dag->begin();
       p != dag->end();
       p++) {
    n = (Node *)(p->second);
    WI[n->getCompleteId()] = 0;
    // found the corresponding response in wf_response
    for (unsigned int ix=0; ix<wf_response->wfn_seq_resp.length(); ix++) {
      if (!strcmp(n->getPb().c_str(), wf_response->wfn_seq_resp[ix].node_id)) {
	// compute WI[ix]
        double w = 0;
        for (unsigned int jx=0;
             jx < wf_response->wfn_seq_resp[ix].response.servers.length();
             jx++) {
          /*
          cout << "\t ** estValues.length() " <<
          response->wfn_seq_resp[ix].response.servers[jx].estim.estValues.length()
          << endl;
          */
          w += wf_response->wfn_seq_resp[ix].response.servers[jx].estim.estValues[1].v_value;
        } // end for jx
        WI[n->getCompleteId()] = w / wf_response->wfn_seq_resp[ix].response.servers.length();
        TRACE_TEXT (TRACE_ALL_STEPS, " HEFT : node " << n->getCompleteId() << " weight :"
            << WI[n->getCompleteId()] << endl);
      }
    } // end for ix
  } // end for nodes
} // end computeNodeWeights

/**
 * rank the nodes upward
 * (uses the WI table and updates the priority of each node)
 */
void
HEFTScheduler::rank(Node * n) {  // RECURSIVE
  Node * succ = NULL;
  unsigned len = n->nextNodesCount();
  // LOOP for all descendant nodes of n
  for (unsigned int ix=0; ix<len; ix++) {
    succ = (Node*)(n->getNext(ix));
    // add WI of current node and priority of descendant node and compare it to
    // priority of current node: if higher then change priority of current node
    if ((succ->getPriority() + WI[n->getCompleteId()]) > n->getPriority()) {
      n->setPriority(succ->getPriority() + WI[n->getCompleteId()]);
    }
  }
  TRACE_TEXT (TRACE_ALL_STEPS, " HEFT : priority of node " << n->getCompleteId()
      << " is " << n->getPriority() << endl);
  len = n->prevNodesCount();
  Node * prev = NULL;
  // LOOP for         all preceding nodes of n
  for (unsigned int ix=0; ix<len; ix++) {
    prev = (Node*)(n->getPrev(ix));
    // if preceding node is not already done or running then rank it
    if ((!prev->isDone()) && (!prev->isRunning()))
      rank(prev);
  }
}

/************************ OBSOLETE PART *********************************/

// wf_node_sched_seq_t
// HEFTScheduler::schedule(const wf_response_t * response,
// 			   DagWfParser& reader,
//                            CORBA::Long dag_id) {
//   Dag * myDag = reader.getDag();
//   myDag->setId(itoa(response->dag_id));
//   return this->schedule(response, myDag);
// }

// Initialize static member
map<std::string, double> HEFTScheduler::avail;

/**
 * Old scheduling method (deprecated)
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

    /* MAIN COMPUTATION LOOP FOR ONE NODE => CHOOSES THE BEST SERVER & COMPUTES AFT */
    /* Remark: this loop can be very costly: <nb servers> * <nb predecessors> !! */

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

    /* END OF THE MAIN COMPUTATION LOOP */

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

// wf_node_sched_seq_t
// HEFTScheduler::reSchedule(const wf_response_t * wf_response,
// 			     DagWfParser& reader) {
//   wf_node_sched_seq_t sched_seq;
//   sched_seq.length(0);
//   return sched_seq;
// }


// wf_node_sched_seq_t
// HEFTScheduler::reSchedule(const wf_response_t * wf_response,
// 			     Dag * dag) {
//   wf_node_sched_seq_t sched_seq;
//   sched_seq.length(0);
//   return sched_seq;
// }


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
