/****************************************************************************/
/* Another multi-workflow scheduler                                         */
/* This scheduler is based on First Come First Serve                        */
/*                                                                          */
/* Author(s):                                                               */
/* - Raphaël  Bolze  (raphael.bolze@ens-lyon.fr)                            */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $@Id$
 * $@Log$
 *
 */

#include <map>

#include "debug.hh"
#include "MultiWfFCFS.hh"

using namespace madag;

/****************************************************************************/
/*                         PUBLIC METHODS                                   */
/****************************************************************************/

MultiWfFCFS::MultiWfFCFS(MaDag_impl* maDag)
  : MultiWfScheduler(maDag, MultiWfScheduler::MULTIWF_DAG_METRIC) {
  this->execQueue = new PriorityNodeQueue;
  TRACE_TEXT(TRACE_MAIN_STEPS,"Using FCFS multi-workflow scheduler" << endl);
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
MultiWfFCFS::handlerNodeDone(DagNode * node) {
  // does nothing
}

/**
 * set node priority before inserting into execution queue
 */
void
MultiWfFCFS::setExecPriority(DagNode * node) {
  node->setPriority(1/node->getDag()->getStartTime());
}

/**
 * Set priority before inserting back in the ready queue
 */
void
MultiWfFCFS::setWaitingPriority(DagNode * node) {
  node->setPriority(this->nodesHEFTPrio[node]);
}
