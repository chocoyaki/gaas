/****************************************************************************/
/* Another multi-workflow scheduler                                         */
/* This scheduler is based on Shortest Remaining Processing Time            */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2008/07/17 13:33:09  bisnard
 * New multi-wf heuristic SRPT
 *
 */

#include <map>

#include "debug.hh"
#include "MultiWfSRPT.hh"

using namespace madag;

/****************************************************************************/
/*                         PUBLIC METHODS                                   */
/****************************************************************************/

MultiWfSRPT::MultiWfSRPT(MaDag_impl* maDag)
  : MultiWfScheduler(maDag, MultiWfScheduler::MULTIWF_DAG_METRIC) {
  this->execQueue = new PriorityNodeQueue;
  TRACE_TEXT(TRACE_MAIN_STEPS,"Using SRPT multi-workflow scheduler" << endl);
}

MultiWfSRPT::~MultiWfSRPT() {
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
MultiWfSRPT::handlerNodeDone(Node * node) {
  // does nothing
}

/**
 * set node priority before inserting into execution queue
 */
void
MultiWfSRPT::setExecPriority(Node * node) {
  Dag * dag = node->getDag();
  double  RPT = 0; // remaining processing time
  // loop over all dag nodes and add their computation time if not yet executed
  for (std::map<std::string,Node*>::iterator np = dag->begin();
       np != dag->end(); np++) {
    Node * curNode = (Node *) np->second;
    if (!curNode->isDone() && !curNode->hasFailed()) {
      RPT += curNode->getEstDuration();
      if (curNode->isRunning()) {
        double nodeStartTime = curNode->getRealStartTime();
        if (nodeStartTime < 0) {
          WARNING("Error in MultiWfSRPT::setExecPriority (missing node start time)");
          RPT -= curNode->getEstDuration();
        } else {
          RPT -= this->getRelCurrTime() - nodeStartTime; // remove already done
        }
      }
    }
  } // end loop (dag nodes)
  if (RPT > 0) {
    TRACE_TEXT(TRACE_ALL_STEPS,"[SRPT] Dag " << dag->getId()
        << " RPT = " << RPT << endl);
    node->setPriority(1 / RPT);
  }
}

/**
 * Set priority before inserting back in the ready queue
 */
void
MultiWfSRPT::setWaitingPriority(Node * node) {
  node->setPriority(this->nodesHEFTPrio[node]);
}
