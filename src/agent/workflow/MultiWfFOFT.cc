/**
* @file MultiWfFOFT.cc
* 
* @brief  Another multi-workflow scheduler based on Fairness On Finish Time algorithm
* 
* @author - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr) 
*         - Benjamin Isnard (Benjamin.Isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/
/* $Id$
 * $Log$
 * Revision 1.20  2011/01/20 18:31:50  bdepardo
 * Prefer prefix ++/-- operators for non-primitive types.
 * Removed unused variable
 *
 * Revision 1.19  2009/09/25 12:39:13  bisnard
 * modified includes to reduce inter-dependencies
 *
 * Revision 1.18  2009/02/06 14:50:35  bisnard
 * setup exceptions
 *
 * Revision 1.17  2009/01/16 13:41:22  bisnard
 * added common base class DagScheduler to simplify dag events handling
 * improved exception management
 *
 * Revision 1.16  2008/10/14 13:24:49  bisnard
 * use new class structure for dags (DagNode, DagNodePort)
 *
 * Revision 1.15  2008/07/10 11:42:20  bisnard
 * Fix bug 68 memory loss during workflow execution
 *
 * Revision 1.14  2008/06/25 10:05:44  bisnard
 * - Waiting priority set when node is put back in waiting queue
 * - Node index in wf_response stored in Node class (new attribute submitIndex)
 * - HEFT scheduler uses SeD ref instead of hostname
 * - Estimation vector and ReqID passed to client when SeD chosen by MaDag
 * - New params in execNodeOnSeD to provide ReqId and estimation vector
 * to client for solve request
 *
 * Revision 1.13  2008/06/19 10:18:54  bisnard
 * new heuristic AgingHEFT for multi-workflow scheduling
 *
 * Revision 1.12  2008/06/18 15:04:22  bisnard
 * initialize dag scheduling time in multi-wf scheduler
 * update slowdown when node is waiting in the ready nodes queue
 *
 * Revision 1.11  2008/06/03 13:37:09  bisnard
 * Multi-workflow sched now keeps nodes in the ready nodes queue
 * until a ressource is available to ensure comparison is done btw
 * nodes of different workflows (using sched-specific metric).
 *
 * Revision 1.10  2008/06/01 14:06:57  rbolze
 * replace most ot the cout by adapted function from debug.cc
 * there are some left ...
 *
 * Revision 1.9  2008/05/30 14:39:57  bisnard
 * cleanup
 *
 * Revision 1.8  2008/05/30 14:25:35  bisnard
 * cleanup
 *
 * Revision 1.7  2008/05/30 14:22:48  bisnard
 * obsolete MultiDag
 *
 * Revision 1.6  2008/04/30 07:37:01  bisnard
 * use relative timestamps for estimated and real completion time
 * make MultiWfScheduler abstract and add HEFT MultiWf scheduler
 *
 * Revision 1.5  2008/04/28 12:12:44  bisnard
 * new NodeQueue implementation for FOFT
 * manage thread join after node execution
 * compute slowdown for FOFT
 *
 * Revision 1.4  2008/04/21 14:31:45  bisnard
 * moved common multiwf routines from derived classes to MultiWfScheduler
 * use wf request identifer instead of dagid to reference client
 * use nodeQueue to manage multiwf scheduling
 * renamed WfParser as DagWfParser
 *
 * Revision 1.3  2008/04/14 13:44:29  bisnard
 * - Parameter 'used' obsoleted in MultiWfScheduler::submit_wf & submit_pb_set
 *
 * Revision 1.2  2008/04/14 09:10:37  bisnard
 *  - Workflow rescheduling (CltReoMan) no longer used with MaDag v2
 *  - AbstractWfSched and derived classes no longer used with MaDag v2
 *
 * Revision 1.1  2008/04/10 09:13:29  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 ****************************************************************************/

#include "debug.hh"
#include "MultiWfFOFT.hh"
#include "Dag.hh"
#include "DagNode.hh"

using namespace madag;

/****************************************************************************/
/*                         PUBLIC METHODS                                   */
/****************************************************************************/

MultiWfFOFT::MultiWfFOFT(MaDag_impl* maDag)
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
MultiWfFOFT::intraDagSchedule(Dag * dag, MasterAgent_var MA)
  throw(MaDag::ServiceNotFound, MaDag::CommProblem) {
  // Call the MA to get estimations for all services
  wf_response_t * wf_response = this->getProblemEstimates(dag, MA);

  // Prioritize the nodes (with intra-dag scheduler)
  this->mySched->setNodesPriority(wf_response, dag);

  // Initialize the earliest finish time for all nodes
  std::vector<DagNode*>& orderedNodes = dag->getNodesByPriority();
  double startTime = this->getRelCurrTime();
  this->mySched->setNodesEFT(orderedNodes, wf_response, dag, startTime);
  delete &orderedNodes;

  // Initialize nodesFlag and nodesHEFTPrio
  for (std::map<std::string, DagNode *>::iterator iter = dag->begin();
       iter != dag->end();
       ++iter) {
    DagNode * node = (DagNode*) iter->second;
    this->nodesFlag[node] = false;
    this->nodesHEFTPrio[node] = node->getPriority();
  }

  // Initialize the earliest finish time and makespan of the dag
  this->dagsState[dag].EFT       = dag->getEFT();
  this->dagsState[dag].makespan  = this->dagsState[dag].EFT - startTime;
  TRACE_TEXT(TRACE_ALL_STEPS, "[FOFT] Init (Dag " << dag->getId() << ") EFT = "
             << this->dagsState[dag].EFT << " / makespan = "
             << this->dagsState[dag].makespan << "\n");

  // Cleanup
  delete wf_response;
}

/**
 * Notify the scheduler that a node is done (called by runNode)
 * Triggers the update of slowdown parameter for the dag of the node
 */
void
MultiWfFOFT::handlerNodeDone(DagNode * node) {
  this->updateNodeDelay(node, node->getRealDelay());
}

/**
 * set node priority before inserting into execution queue
 */
void
MultiWfFOFT::setExecPriority(DagNode * node) {
  // if flag is set (prev attempt to exec failed due to lack of ress.
  if (this->nodesFlag[node]) {
    double currDelay = this->getRelCurrTime() + node->getEstDuration() - node->getEstCompTime();
    if (currDelay > 0) {
      this->updateNodeDelay(node, currDelay);
      TRACE_TEXT(TRACE_MAIN_STEPS, "[FOFT] Waiting node "
                 << node->getCompleteId() << " delay updated (duration="
                 << node->getEstDuration() << "/EFT=" << node->getEstCompTime()
                 << "/delay=" << currDelay << ")\n");
    } else {
      TRACE_TEXT(TRACE_MAIN_STEPS, "[FOFT] Waiting node " << node->getCompleteId()
                 << " is on schedule\n");
    }
  }
  node->setPriority(this->dagsState[node->getDag()].slowdown);
}

/**
 * Set priority before inserting back in the ready queue
 * Set a flag to trigger slowdown calculation the next time this node's
 * priority is set.
 */
void
MultiWfFOFT::setWaitingPriority(DagNode * node) {
  node->setPriority(this->nodesHEFTPrio[node]);
  this->nodesFlag[node] = true;
}

/**
 * Updates the delay of a node
 * Updates the slowdown of the DAG if needed
 */
void
MultiWfFOFT::updateNodeDelay(DagNode * node, double delay) {
  DagState& curDagState   = this->dagsState[node->getDag()];
  double dagPrevEstDelay  = curDagState.estimatedDelay;
  // Recursively updates the delay for the node and its successors
  node->getDag()->updateDelayRec(node, delay);
  // Check new value of the dag estimated delay
  double dagNewEstDelay   = node->getDag()->getEstDelay();
  // updates slowdown if the global delay for the dag is increased
  if (dagNewEstDelay > dagPrevEstDelay) {
    // updates the estimated delay
    curDagState.estimatedDelay = dagNewEstDelay;
    // slowdown is the percentage of delay relatively to initial makespan
    curDagState.slowdown = (double) 100 * dagNewEstDelay / curDagState.makespan;
    TRACE_TEXT(TRACE_MAIN_STEPS, "[FOFT] Updated slowdown for dag "
               << node->getDag()->getId() << " = " << curDagState.slowdown << "\n");
  }
}
