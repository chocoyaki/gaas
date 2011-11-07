/**
 * @file MultiWfFCFS.cc
 *
 * @brief  Another multi-workflow scheduler based on First Come First Serve
 *
 * @author  Raphaël  Bolze  (raphael.bolze@ens-lyon.fr)
 *          Benjamin Isnard (Benjamin.Isnard@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#include <map>

#include "debug.hh"
#include "MultiWfFCFS.hh"
#include "Dag.hh"
#include "DagNode.hh"

using namespace madag;

/****************************************************************************/
/*                         PUBLIC METHODS                                   */
/****************************************************************************/

MultiWfFCFS::MultiWfFCFS(MaDag_impl *maDag)
  : MultiWfScheduler(maDag, MultiWfScheduler::MULTIWF_DAG_METRIC) {
  this->execQueue = new PriorityNodeQueue;
  TRACE_TEXT(TRACE_MAIN_STEPS, "Using FCFS multi-workflow scheduler\n");
}

MultiWfFCFS::~MultiWfFCFS() {
  delete dynamic_cast<PriorityNodeQueue *>(execQueue);
}

/****************************************************************************/
/*                         PROTECTED METHODS                                */
/****************************************************************************/

/**
 * Notify the scheduler that a node is done (called by runNode)
 * (does nothing for this heuristic, as the priority is calculated
 * each time a node is put into execution queue)
 */
void
MultiWfFCFS::handlerNodeDone(DagNode *node) {
  // does nothing
}

/**
 * set node priority before inserting into execution queue
 */
void
MultiWfFCFS::setExecPriority(DagNode *node) {
  node->setPriority(1 / node->getDag()->getStartTime());
}

/**
 * Set priority before inserting back in the ready queue
 */
void
MultiWfFCFS::setWaitingPriority(DagNode *node) {
  node->setPriority(this->nodesHEFTPrio[node]);
}
