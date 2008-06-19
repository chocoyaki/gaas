/****************************************************************************/
/* Another multi-workflow scheduler                                         */
/* This scheduler is based on HEFT algorithm                                */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/

/* $Id$
 * $Log$
 * Revision 1.4  2008/06/19 10:18:54  bisnard
 * new heuristic AgingHEFT for multi-workflow scheduling
 *
 * Revision 1.3  2008/06/03 13:37:09  bisnard
 * Multi-workflow sched now keeps nodes in the ready nodes queue
 * until a ressource is available to ensure comparison is done btw
 * nodes of different workflows (using sched-specific metric).
 *
 * Revision 1.2  2008/06/01 14:06:57  rbolze
 * replace most ot the cout by adapted function from debug.cc
 * there are some left ...
 *
 * Revision 1.1  2008/04/30 07:37:01  bisnard
 * use relative timestamps for estimated and real completion time
 * make MultiWfScheduler abstract and add HEFT MultiWf scheduler
 *
 */


#include "MultiWfHEFT.hh"


using namespace madag;

/****************************************************************************/
/*                         PUBLIC METHODS                                   */
/****************************************************************************/

/**** CLASS MultiWfHEFT ****/

MultiWfHEFT::MultiWfHEFT(MaDag_impl* maDag)
  : MultiWfScheduler(maDag, MultiWfScheduler::MULTIWF_NODE_METRIC) {
  this->execQueue = new PriorityNodeQueue;
  TRACE_TEXT(TRACE_MAIN_STEPS,"Using HEFT multi-workflow scheduler" << endl);
}

MultiWfHEFT::~MultiWfHEFT() {
  delete dynamic_cast<PriorityNodeQueue *>(execQueue);
}

/**** CLASS MultiWfAgingHEFT ****/

MultiWfAgingHEFT::MultiWfAgingHEFT(MaDag_impl* maDag)
  : MultiWfScheduler(maDag, MultiWfScheduler::MULTIWF_NODE_METRIC) {
  this->execQueue = new PriorityNodeQueue;
  TRACE_TEXT(TRACE_MAIN_STEPS,"Using AgingHEFT multi-workflow scheduler" << endl);
}

MultiWfAgingHEFT::~MultiWfAgingHEFT() {
  delete dynamic_cast<PriorityNodeQueue *>(execQueue);
}

/****************************************************************************/
/*                       PROTECTED METHODS                                  */
/****************************************************************************/

/**** CLASS MultiWfHEFT ****/

void
MultiWfHEFT::handlerNodeDone(Node * node) {
  // does nothing
}

/**** CLASS MultiWfAgingHEFT ****/

/**
 * Intra-dag scheduling
 */
void
MultiWfAgingHEFT::intraDagSchedule(Dag * dag, MasterAgent_var MA)
    throw (NodeException) {
  // Call the MA to get estimations for all services
  wf_response_t * wf_response = this->getProblemEstimates(dag, MA);

  // Prioritize the nodes (with intra-dag scheduler)
  this->mySched->setNodesPriority(wf_response, dag);

  // Initialize the earliest finish time for all nodes
  std::vector<Node*>& orderedNodes = dag->getNodesByPriority();
  double startTime = this->getRelCurrTime();
  this->mySched->setNodesEFT(orderedNodes, wf_response, dag, startTime);
  delete &orderedNodes;

  // Store the HEFT Priority of nodes
  for (map <string,Node *>::iterator iter = dag->begin(); iter != dag->end();
       iter++) {
    Node * node = (Node*) iter->second;
    this->nodesHEFTPrio[node] = node->getPriority();
  }

  // Initialize the earliest finish time and makespan of the dag
  this->dagsState[dag].EFT       = dag->getEFT();
  this->dagsState[dag].makespan  = this->dagsState[dag].EFT - startTime;
  TRACE_TEXT(TRACE_ALL_STEPS, "[AHEFT] Init (Dag " << dag->getId() << ") EFT = "
      << this->dagsState[dag].EFT << " / makespan = "
      << this->dagsState[dag].makespan << endl);
}

void
MultiWfAgingHEFT::handlerNodeDone(Node * node) {
  // does nothing
}

/**
 * Computes the priority of node that depends on the age of the Dag
 * Note: the age of the dag is re-computed for all nodes
 */
void
MultiWfAgingHEFT::setExecPriority(Node * node) {
  double dagAge     = this->getRelCurrTime() - node->getDag()->getStartTime();
  float  ageFactor  = (float) (dagAge / this->dagsState[node->getDag()].makespan) + 1;
  node->setPriority((double) (this->nodesHEFTPrio[node] * ageFactor));
  TRACE_TEXT(TRACE_ALL_STEPS,"[AHEFT] Node priority set to " << node->getPriority()
      << " (dag age = " << dagAge << "/factor = " << ageFactor << ")" << endl);
}
