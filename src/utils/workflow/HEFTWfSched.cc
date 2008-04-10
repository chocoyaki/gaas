/****************************************************************************/
/* HEFT Workflow Scheduler                                                  */
/* This scheduler use the HEFT heuristic to execute the workflow            */
/* The order between ready nodes is random                                  */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2008/04/10 08:38:50  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 * Revision 1.4  2006/11/06 12:00:01  aamar
 * *** empty log message ***
 * 
 ****************************************************************************/

#include <unistd.h>
#include <iostream>
#include "HEFTWfSched.hh"

using namespace std;

HEFTWfSched::HEFTWfSched() {
  int status;
  status = sem_init(&mySem, 0, 0);
  if (status !=0) {
    std::cout << __FILE__ << ":" << __LINE__ << "- " << flush;
    perror("sem_init failed"); exit(status);
  }
}

HEFTWfSched::~HEFTWfSched() {
}


/**
 * rank the node upward 
 */

void
HEFTWfSched::rank(Node * n) {
  Node * succ = NULL;
  unsigned len = n->nextNodesCount();
  for (unsigned int ix=0; ix<len; ix++) {
    succ = (Node*)(n->getNext(ix));
    if ((succ->getPriority() + WI[n->getId()]) > n->getPriority()) {
      n->setPriority(succ->getPriority() + WI[n->getId()]);
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

/**
 * The scheduler execution method *
 * implicitely called by the runtime *
 */

void
HEFTWfSched::execute() {
  Node * n = NULL;
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
    for (unsigned int ix=0; ix<response.wfn_seq_resp.length(); ix++) {
      if (!strcmp(n->getPb().c_str(), response.wfn_seq_resp[ix].node_id)) {
	// compute WI[ix]
	double w = 0;
	for (unsigned int jx=0; 
	     jx<response.wfn_seq_resp[ix].response.servers.length(); 
	     jx++) {
	  cout << "estValues.length() " << 
	    response.wfn_seq_resp[ix].response.servers[jx].estim.estValues.length() 
	       << endl;
	  w += response.wfn_seq_resp[ix].response.servers[jx].estim.estValues[1].v_value;
	} // end for jx
	WI[n->getId()] = w/response.wfn_seq_resp[ix].response.servers.length();
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
       ix < response.wfn_seq_resp.length();
       ix++) {
    for (unsigned int jx=0;
	 jx < response.wfn_seq_resp[ix].response.servers.length(); 
	 jx++) {
      string hn(response.wfn_seq_resp[ix].response.servers[jx].loc.hostName);
      avail[hn] = 0;
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
  while (sorted_list.size() > 0) {
    n = sorted_list[0];
    sorted_list.erase(sorted_list.begin());
    // found the problem index
    for (unsigned int ix=0; ix<response.wfn_seq_resp.length(); ix++) {
      if (!strcmp(n->getPb().c_str(), response.wfn_seq_resp[ix].node_id))
	pb_index = ix;
    } // end for ix
    unsigned int sed_ind = 0;
    double EFT = 0;
    for (unsigned int ix=0; 
	 ix<response.wfn_seq_resp[pb_index].response.servers.length(); 
	 ix++) {
      string hn(response.wfn_seq_resp[pb_index].response.servers[ix].loc.hostName);
      double EST = 
	avail[hn];
      for (unsigned int jx=0;
	   jx < n->prevNodesCount();
	   jx++) {
	EST = max(EST, AFT[n->getPrev(jx)->getId()]);
      } // end for jx
      if ( (
	    EST + response.wfn_seq_resp[pb_index].response.servers[ix].estim.estValues[1].v_value < EFT ) 
	   || (EFT == 0)) {
	EFT = 
	  EST + response.wfn_seq_resp[pb_index].response.servers[ix].estim.estValues[1].v_value;
	sed_ind = ix;
      }
    } // end for ix
    string hn(response.wfn_seq_resp[pb_index].response.servers[sed_ind].loc.hostName);
    n->setSeD(response.wfn_seq_resp[pb_index].response.servers[sed_ind].loc.ior,
              hn);
    avail[hn] = EFT;
    AFT[n->getId()] = EFT;
    AST[n->getId()] = EFT;
    n->setEstCompTime((long int)EFT);
  }
  cout << "HEFT : Computing AFT & AST done" << endl;

  cout << "HEFT : start executing ready nodes" << endl;

  struct timeval tv;
  gettimeofday(&tv, NULL);
  myDag->setTheBeginning(tv);

  CORBA::Long reqID = response.firstReqID;
  while (true) {
    // get the ready nodes
    vector <Node*> readyNodes;
    this->getReadyNodes(this->myDag, readyNodes);
    unsigned len = readyNodes.size();
    for (uint ix=0; ix<len; ix++) {
      n = readyNodes[ix];
      n->start(reqID++);
    }

    if ((this->myCltReoMan != NULL) && 
	(this->myCltReoMan->isEnabled()) &&
	(!myDag->checkScheduling())) {
      cout << "the scheduling is incorrect" << endl << 
	"try to reschedule" << endl;
      this->myCltReoMan->reSchedule();
      int status;
      status = sem_wait(&mySem);
      if (status !=0) {
	std::cout << __FILE__ << ":" << __LINE__ << "- " << flush;
	perror("sem_wait failed"); exit(status);
      }
      
    }

    if (readyNodes.size() == 0) {
      if (myDag->isDone())
	break;
      else {
	// no ready nodes but some nodes are running
	usleep(50);
      }
    }
  }

  cout << "... workflow execution done (HEFT) " << endl;
}


/**
 * Rescheduling methods
 */
void
HEFTWfSched::reSchedule() {
  cout << "HEFT rescheduling " << endl;
  Node * n = NULL;
  for (std::map <std::string, Node *>::iterator p = myDag->begin();
       p != myDag->end();
       p++) {
    // compute WI[ix];
    n = (Node *)(p->second);
    if (n != NULL) {
      if (n->isDone() || (n->isRunning())) {
	cout << "Moving the node "<< n->getId() << " to Trahs" << endl;
	myDag->moveToTrash(n);
      }
    }
  } // end for
  cout << "Executing the new DAG" << endl;
  this->init();
  int status;
  status = sem_post(&mySem);
  if (status !=0) {
    std::cout << __FILE__ << ":" << __LINE__ << "- " << flush;
    perror("sem_post failed"); exit(status);
  }

}

void 
HEFTWfSched::init() {
  Node * n = NULL;
  // ranking
  cout << "HEFT : start ranking" << endl;
  for (std::map <std::string, Node *>::iterator p = myDag->begin();
       p != myDag->end();
       p++) {
    // compute WI[ix];
    n = (Node *)(p->second);
    WI[n->getId()] = 0;
    // found the corresponding response in wf_response
    for (unsigned int ix=0; ix<response.wfn_seq_resp.length(); ix++) {
      if (!strcmp(n->getPb().c_str(), response.wfn_seq_resp[ix].node_id)) {
	// compute WI[ix]
	cout << "compute WI[" << ix << "]" << endl;
	double w = 0;
	for (unsigned int jx=0; 
	     jx<response.wfn_seq_resp[ix].response.servers.length(); 
	     jx++) {
	  w += response.wfn_seq_resp[ix].response.servers[jx].estim.estValues[1].v_value;
	} // end for jx
	WI[n->getId()] = w/response.wfn_seq_resp[ix].response.servers.length();
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
       ix < response.wfn_seq_resp.length();
       ix++) {
    for (unsigned int jx=0;
	 jx < response.wfn_seq_resp[ix].response.servers.length(); 
	 jx++) {
      string hn(response.wfn_seq_resp[ix].response.servers[jx].loc.hostName);
      avail[hn] = 0;
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
  while (sorted_list.size() > 0) {
    n = sorted_list[0];
    sorted_list.erase(sorted_list.begin());
    // found the problem index
    for (unsigned int ix=0; ix<response.wfn_seq_resp.length(); ix++) {
      if (!strcmp(n->getPb().c_str(), response.wfn_seq_resp[ix].node_id))
	pb_index = ix;
    } // end for ix
    unsigned int sed_ind = 0;
    double EFT = 0;
    for (unsigned int ix=0; 
	 ix<response.wfn_seq_resp[pb_index].response.servers.length(); 
	 ix++) {
      string hn(response.wfn_seq_resp[pb_index].response.servers[ix].loc.hostName);
      double EST = 
	avail[hn];
      for (unsigned int jx=0;
	   jx < n->prevNodesCount();
	   jx++) {
	EST = max(EST, AFT[n->getPrev(jx)->getId()]);
      } // end for jx
      if ( (
	    EST + response.wfn_seq_resp[pb_index].response.servers[ix].estim.estValues[1].v_value < EFT ) 
	   || (EFT == 0)) {
	EFT = 
	  EST + response.wfn_seq_resp[pb_index].response.servers[ix].estim.estValues[1].v_value;
	sed_ind = ix;
      }
    } // end for ix
    string hn(response.wfn_seq_resp[pb_index].response.servers[sed_ind].loc.hostName);
    n->setSeD(response.wfn_seq_resp[pb_index].response.servers[sed_ind].loc.ior,
              hn);
    avail[hn] = EFT;
    AFT[n->getId()] = EFT;
    AST[n->getId()] = EFT;
    n->setEstCompTime((long int)EFT);
  }
  cout << "HEFT : Computing AFT & AST done" << endl;

  struct timeval tv;
  gettimeofday(&tv, NULL);
  myDag->setTheBeginning(tv);

}

/**
 * Get the ready nodes for execution
 */
void 
HEFTWfSched::getReadyNodes(Dag*& dag, vector<Node *>& readyNodes) {
	map<string, Node*> allReadyNodes;
	dag->getReadyNodes(allReadyNodes);
	vector<string> topQueues;
	AbstractWfSched::getTopSeDQueues(topQueues);
//	cout << "  ** ALL READY NODES " << endl;
//	for (map<string, Node*>::iterator p = allReadyNodes.begin();
//		p != allReadyNodes.end();
//		p++) {
//		cout << p->second->getId() << endl;
//	} // end for p
//	cout << "  ** TOP QUEUES " << endl;
//	for (unsigned int ix=0; ix<topQueues.size(); ix++) {
//		cout << "  ** in the top " << topQueues[ix] << endl;
//	} // end for ix
	for (unsigned int ix=0; ix<topQueues.size(); ix++) {
		if (allReadyNodes.find(topQueues[ix]) != allReadyNodes.end()) {
			readyNodes.push_back(allReadyNodes[topQueues[ix]]);
		} // end if
	} // end for
} // end getReadyNodes
