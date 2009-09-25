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
 * Revision 1.12  2009/09/25 12:39:14  bisnard
 * modified includes to reduce inter-dependencies
 *
 * Revision 1.11  2009/02/06 14:50:35  bisnard
 * setup exceptions
 *
 * Revision 1.10  2008/10/14 13:24:49  bisnard
 * use new class structure for dags (DagNode,DagNodePort)
 *
 * Revision 1.9  2008/07/24 23:59:05  rbolze
 * oops I have commit the AgingHEFT with exponentiel factor.
 *
 * Revision 1.8  2008/07/24 21:08:11  rbolze
 * New multi-wf heuristic FCFS (First Come First Serve)
 *
 * Revision 1.7  2008/07/12 00:22:28  rbolze
 * add function getInterRoundDelay()
 * use this function when the maDag start to display this value.
 * display the dag_id when compute the ageFactor in AgingHEFT
 * add some stats info :
 * 	queuedNodeCount
 * 	change MA DAG to MA_DAG
 *
 * Revision 1.6  2008/07/10 11:42:20  bisnard
 * Fix bug 68 memory loss during workflow execution
 *
 * Revision 1.5  2008/06/25 10:05:44  bisnard
 * - Waiting priority set when node is put back in waiting queue
 * - Node index in wf_response stored in Node class (new attribute submitIndex)
 * - HEFT scheduler uses SeD ref instead of hostname
 * - Estimation vector and ReqID passed to client when SeD chosen by MaDag
 * - New params in execNodeOnSeD to provide ReqId and estimation vector
 * to client for solve request
 *
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
#include "Dag.hh"
#include "DagNode.hh"

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
MultiWfHEFT::handlerNodeDone(DagNode * node) {
  // does nothing
}

/**** CLASS MultiWfAgingHEFT ****/

/**
 * Intra-dag scheduling
 */
void
MultiWfAgingHEFT::intraDagSchedule(Dag * dag, MasterAgent_var MA)
    throw (MaDag::ServiceNotFound, MaDag::CommProblem) {
  // Call the MA to get estimations for all services
  wf_response_t * wf_response = this->getProblemEstimates(dag, MA);

  // Prioritize the nodes (with intra-dag scheduler)
  this->mySched->setNodesPriority(wf_response, dag);

  // Initialize the earliest finish time for all nodes
  std::vector<DagNode*>& orderedNodes = dag->getNodesByPriority();
  double startTime = this->getRelCurrTime();
  this->mySched->setNodesEFT(orderedNodes, wf_response, dag, startTime);
  delete &orderedNodes;

  // Store the HEFT Priority of nodes
  for (map <string,DagNode *>::iterator iter = dag->begin(); iter != dag->end();
       iter++) {
    DagNode * node = (DagNode*) iter->second;
    this->nodesHEFTPrio[node] = node->getPriority();
  }

  // Initialize the earliest finish time and makespan of the dag
  this->dagsState[dag].EFT       = dag->getEFT();
  this->dagsState[dag].makespan  = this->dagsState[dag].EFT - startTime;
  TRACE_TEXT(TRACE_ALL_STEPS, "[AHEFT] Init (Dag " << dag->getId() << ") EFT = "
      << this->dagsState[dag].EFT << " / makespan = "
      << this->dagsState[dag].makespan << endl);

  // Cleanup
  delete wf_response;
}

void
MultiWfAgingHEFT::handlerNodeDone(DagNode * node) {
  // does nothing
}

/**
 * Computes the priority of node that depends on the age of the Dag
 * Note: the age of the dag is re-computed for all nodes
 */
void
MultiWfAgingHEFT::setExecPriority(DagNode * node) {
  double dagAge     = this->getRelCurrTime() - node->getDag()->getStartTime();
  //float  ageFactor  = exp((float) (dagAge / this->dagsState[node->getDag()].makespan) + 1);
  float  ageFactor  = (float) (dagAge / this->dagsState[node->getDag()].makespan) + 1;
  node->setPriority((double) (this->nodesHEFTPrio[node] * ageFactor));
  TRACE_TEXT(TRACE_ALL_STEPS,"[AHEFT] Node priority set to " << node->getPriority()
      << " (dag" << node->getDag()->getId()
      << " age = " << dagAge
      << "/factor = " << ageFactor << ")" << endl);
}

/**
 * Set priority before inserting back in the ready queue
 */
void
MultiWfAgingHEFT::setWaitingPriority(DagNode * node) {
  node->setPriority(this->nodesHEFTPrio[node]);
}
