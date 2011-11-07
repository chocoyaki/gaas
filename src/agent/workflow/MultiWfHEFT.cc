/**
 * @file MultiWfHEFT.cc
 *
 * @brief  Another multi-workflow scheduler based on HEFT algorithm
 *
 * @author  Benjamin Isnard (Benjamin.Isnard@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */



#include "MultiWfHEFT.hh"
#include "Dag.hh"
#include "DagNode.hh"

using namespace madag;

/****************************************************************************/
/*                         PUBLIC METHODS                                   */
/****************************************************************************/

/**** CLASS MultiWfHEFT ****/

MultiWfHEFT::MultiWfHEFT(MaDag_impl *maDag)
  : MultiWfScheduler(maDag, MultiWfScheduler::MULTIWF_NODE_METRIC) {
  this->execQueue = new PriorityNodeQueue;
  TRACE_TEXT(TRACE_MAIN_STEPS, "Using HEFT multi-workflow scheduler\n");
}

MultiWfHEFT::~MultiWfHEFT() {
  delete dynamic_cast<PriorityNodeQueue *>(execQueue);
}

/**** CLASS MultiWfAgingHEFT ****/

MultiWfAgingHEFT::MultiWfAgingHEFT(MaDag_impl *maDag)
  : MultiWfScheduler(maDag, MultiWfScheduler::MULTIWF_NODE_METRIC) {
  this->execQueue = new PriorityNodeQueue;
  TRACE_TEXT(TRACE_MAIN_STEPS, "Using AgingHEFT multi-workflow scheduler\n");
}

MultiWfAgingHEFT::~MultiWfAgingHEFT() {
  delete dynamic_cast<PriorityNodeQueue *>(execQueue);
}

/****************************************************************************/
/*                       PROTECTED METHODS                                  */
/****************************************************************************/

/**** CLASS MultiWfHEFT ****/

void
MultiWfHEFT::handlerNodeDone(DagNode *node) {
  // does nothing
}

/**** CLASS MultiWfAgingHEFT ****/

/**
 * Intra-dag scheduling
 */
void
MultiWfAgingHEFT::intraDagSchedule(Dag *dag, MasterAgent_var MA)
throw(MaDag::ServiceNotFound, MaDag::CommProblem) {
  // Call the MA to get estimations for all services
  wf_response_t *wf_response = this->getProblemEstimates(dag, MA);

  // Prioritize the nodes (with intra-dag scheduler)
  this->mySched->setNodesPriority(wf_response, dag);

  // Initialize the earliest finish time for all nodes
  std::vector<DagNode *> &orderedNodes = dag->getNodesByPriority();
  double startTime = this->getRelCurrTime();
  this->mySched->setNodesEFT(orderedNodes, wf_response, dag, startTime);
  delete &orderedNodes;

  // Store the HEFT Priority of nodes
  for (std::map<std::string, DagNode *>::iterator iter = dag->begin();
       iter != dag->end();
       ++iter) {
    DagNode *node = (DagNode *) iter->second;
    this->nodesHEFTPrio[node] = node->getPriority();
  }

  // Initialize the earliest finish time and makespan of the dag
  this->dagsState[dag].EFT = dag->getEFT();
  this->dagsState[dag].makespan = this->dagsState[dag].EFT - startTime;
  TRACE_TEXT(TRACE_ALL_STEPS,
             "[AHEFT] Init (Dag " << dag->getId() << ") EFT = "
                                  << this->dagsState[dag].EFT
                                  << " / makespan = "
                                  << this->dagsState[dag].
             makespan << "\n");

  // Cleanup
  delete wf_response;
} // intraDagSchedule

void
MultiWfAgingHEFT::handlerNodeDone(DagNode *node) {
  // does nothing
}

/**
 * Computes the priority of node that depends on the age of the Dag
 * Note: the age of the dag is re-computed for all nodes
 */
void
MultiWfAgingHEFT::setExecPriority(DagNode *node) {
  double dagAge = this->getRelCurrTime() - node->getDag()->getStartTime();
  // float  ageFactor  = exp((float) (dagAge / this->dagsState[node->getDag()].makespan) + 1);
  float ageFactor =
    (float) (dagAge / this->dagsState[node->getDag()].makespan) + 1;
  node->setPriority((double) (this->nodesHEFTPrio[node] * ageFactor));
  TRACE_TEXT(TRACE_ALL_STEPS,
             "[AHEFT] Node priority set to " << node->getPriority()
                                             << " (dag" <<
             node->getDag()->getId()
                                             << " age = " <<
             dagAge
                                             << "/factor = "
                                             << ageFactor << ")\n");
}

/**
 * Set priority before inserting back in the ready queue
 */
void
MultiWfAgingHEFT::setWaitingPriority(DagNode *node) {
  node->setPriority(this->nodesHEFTPrio[node]);
}
