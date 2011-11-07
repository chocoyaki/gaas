/**
 * @file MultiWfFOFT.cc
 *
 * @brief  Another multi-workflow scheduler based on Fairness On Finish Time algorithm
 *
 * @author  Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
 *          Benjamin Isnard (Benjamin.Isnard@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#include "debug.hh"
#include "MultiWfFOFT.hh"
#include "Dag.hh"
#include "DagNode.hh"

using namespace madag;

/****************************************************************************/
/*                         PUBLIC METHODS                                   */
/****************************************************************************/

MultiWfFOFT::MultiWfFOFT(MaDag_impl *maDag)
  : MultiWfScheduler(maDag, MultiWfScheduler::MULTIWF_DAG_METRIC) {
  this->execQueue = new PriorityNodeQueue;
  TRACE_TEXT(TRACE_MAIN_STEPS, "Using FOFT multi-workflow scheduler\n");
}

MultiWfFOFT::~MultiWfFOFT() {
  delete dynamic_cast<PriorityNodeQueue *>(execQueue);
}

/****************************************************************************/
/*                         PROTECTED METHODS                                */
/****************************************************************************/

/**
 * Intra-dag scheduling
 * Compared to parent class, this method uses more functions from the scheduler to
 * initialize node data
 */
void
MultiWfFOFT::intraDagSchedule(Dag *dag, MasterAgent_var MA)
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

  // Initialize nodesFlag and nodesHEFTPrio
  for (std::map<std::string, DagNode *>::iterator iter = dag->begin();
       iter != dag->end();
       ++iter) {
    DagNode *node = (DagNode *) iter->second;
    this->nodesFlag[node] = false;
    this->nodesHEFTPrio[node] = node->getPriority();
  }

  // Initialize the earliest finish time and makespan of the dag
  this->dagsState[dag].EFT = dag->getEFT();
  this->dagsState[dag].makespan = this->dagsState[dag].EFT - startTime;
  TRACE_TEXT(TRACE_ALL_STEPS,
             "[FOFT] Init (Dag " << dag->getId() << ") EFT = "
                                 << this->dagsState[dag].EFT
                                 << " / makespan = "
                                 << this->dagsState[dag].
             makespan << "\n");

  // Cleanup
  delete wf_response;
} // intraDagSchedule

/**
 * Notify the scheduler that a node is done (called by runNode)
 * Triggers the update of slowdown parameter for the dag of the node
 */
void
MultiWfFOFT::handlerNodeDone(DagNode *node) {
  this->updateNodeDelay(node, node->getRealDelay());
}

/**
 * set node priority before inserting into execution queue
 */
void
MultiWfFOFT::setExecPriority(DagNode *node) {
  // if flag is set (prev attempt to exec failed due to lack of ress.
  if (this->nodesFlag[node]) {
    double currDelay = this->getRelCurrTime() + node->getEstDuration() -
                       node->getEstCompTime();
    if (currDelay > 0) {
      this->updateNodeDelay(node, currDelay);
      TRACE_TEXT(TRACE_MAIN_STEPS, "[FOFT] Waiting node "
                 << node->getCompleteId() << " delay updated (duration="
                 << node->getEstDuration() << "/EFT=" << node->getEstCompTime()
                 << "/delay=" << currDelay << ")\n");
    } else {
      TRACE_TEXT(TRACE_MAIN_STEPS,
                 "[FOFT] Waiting node " << node->getCompleteId()
                                        <<
                 " is on schedule\n");
    }
  }
  node->setPriority(this->dagsState[node->getDag()].slowdown);
} // setExecPriority

/**
 * Set priority before inserting back in the ready queue
 * Set a flag to trigger slowdown calculation the next time this node's
 * priority is set.
 */
void
MultiWfFOFT::setWaitingPriority(DagNode *node) {
  node->setPriority(this->nodesHEFTPrio[node]);
  this->nodesFlag[node] = true;
}

/**
 * Updates the delay of a node
 * Updates the slowdown of the DAG if needed
 */
void
MultiWfFOFT::updateNodeDelay(DagNode *node, double delay) {
  DagState &curDagState = this->dagsState[node->getDag()];
  double dagPrevEstDelay = curDagState.estimatedDelay;
  // Recursively updates the delay for the node and its successors
  node->getDag()->updateDelayRec(node, delay);
  // Check new value of the dag estimated delay
  double dagNewEstDelay = node->getDag()->getEstDelay();
  // updates slowdown if the global delay for the dag is increased
  if (dagNewEstDelay > dagPrevEstDelay) {
    // updates the estimated delay
    curDagState.estimatedDelay = dagNewEstDelay;
    // slowdown is the percentage of delay relatively to initial makespan
    curDagState.slowdown = (double) 100 * dagNewEstDelay / curDagState.makespan;
    TRACE_TEXT(TRACE_MAIN_STEPS, "[FOFT] Updated slowdown for dag "
               << node->getDag()->getId() << " = " << curDagState.slowdown <<
               "\n");
  }
} // updateNodeDelay
