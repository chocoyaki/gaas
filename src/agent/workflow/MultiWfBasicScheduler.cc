/**
 * @file MultiWfBasicScheduler.cc
 *
 * @brief  The Workflow Meta-schedulerg used when multi-workflow support is enabled
 *
 * @author  Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
 *          Benjamin Isnard (Benjamin.Isnard@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#include "MultiWfBasicScheduler.hh"
#include "Dag.hh"
#include "DagNode.hh"
#include "MaDagNodeLauncher.hh"
#include "debug.hh"
#include "marshalling.hh"
#include "MaDag_impl.hh"
#include "DIET_compat.hh"

using namespace madag;

MultiWfBasicScheduler::MultiWfBasicScheduler(MaDag_impl *maDag)
  : MultiWfScheduler(maDag, MultiWfScheduler::MULTIWF_NO_METRIC) {
  TRACE_TEXT(TRACE_MAIN_STEPS, "Using BASIC multi-workflow scheduler\n");
}

MultiWfBasicScheduler::~MultiWfBasicScheduler() {
}

/**
 * Execution method
 * (does not use execQueue)
 */
void *
MultiWfBasicScheduler::run() {
  TRACE_TEXT(TRACE_MAIN_STEPS, "MultiWfBasicScheduler is running\n");
  int nodeCount = 0;
  while (this->keepOnRunning) {
    TRACE_TEXT(TRACE_MAIN_STEPS, "\t ** Starting MultiWfBasicScheduler\n");
    myLock.lock();
    // Loop over all nodeQueues and run the first ready node
    // for each queue
    nodeCount = 0;
    std::list<OrderedNodeQueue *>::iterator qp = readyQueues.begin();
    while (qp != readyQueues.end()) {
      TRACE_TEXT(TRACE_ALL_STEPS, "Checking ready nodes queue:\n");
      OrderedNodeQueue *readyQ = *qp;
      DagNode *n = readyQ->popFirstNode();
      if (n != NULL) {
        TRACE_TEXT(TRACE_ALL_STEPS, "  #### Ready node : " << n->getCompleteId()
                                                           << " => execute\n");
        // EXECUTE NODE (NEW THREAD)
        // create a node launcher
        DagNodeLauncher *launcher = new MaDagNodeLauncher(n, this,
                                                          myMaDag->getCltMan(n
                                                                             ->
                                                                             getDag()
                                                                             ->getId()));
        n->start(launcher);
        nodeCount++;
        // Destroy queues if both are empty
        ChainedNodeQueue *waitQ = waitingQueues[readyQ];
        if (waitQ->isEmpty() && readyQ->isEmpty()) {
          TRACE_TEXT(TRACE_ALL_STEPS,
                     "Node Queues are empty: remove & destroy\n");
          qp = readyQueues.erase(qp);      // removes from the list
          this->deleteNodeQueue(readyQ);  // deletes both queues
          continue;
        }
        // DELAY between NODES (to avoid interference btw submits)
        diet::usleep(this->interRoundDelay * 1000);
      }
      ++qp;  // go to next queue
    }
    myLock.unlock();

    if (nodeCount == 0) {
      TRACE_TEXT(TRACE_MAIN_STEPS, "No ready nodes\n");
      this->mySem.wait();

      // WAIT UNTIL A NEW DAG IS SUBMITTED OR A NODE IS COMPLETED

      this->postWakeUp();
      this->checkDagsRelease();
    }
  }
  return NULL;
} // run

/**
 * Notify the scheduler that a node is done
 */
void
MultiWfBasicScheduler::handlerNodeDone(DagNode *node) {
  // does nothing for this class
}

/**
 * Create two chained node queues and return the ready queue
 *  - WAITING queue => READY queue
 */

OrderedNodeQueue *
MultiWfBasicScheduler::createNodeQueue(Dag *dag) {
  TRACE_TEXT(TRACE_ALL_STEPS, "Creating new node queues (basic)\n");
  OrderedNodeQueue *readyQ = new OrderedNodeQueue();
  ChainedNodeQueue *waitQ = new ChainedNodeQueue(readyQ);
  for (std::map <std::string, DagNode *>::iterator nodeIt = dag->begin();
       nodeIt != dag->end();
       ++nodeIt) {
    waitQ->pushNode(&(*nodeIt->second));
  }
  this->waitingQueues[readyQ] = waitQ;  // used to destroy waiting queue
  return readyQ;
} // createNodeQueue


/**
 * Delete the two chained node queues
 *  - WAITING queue => READY queue
 */
void
MultiWfBasicScheduler::deleteNodeQueue(OrderedNodeQueue *nodeQ) {
  TRACE_TEXT(TRACE_ALL_STEPS, "Deleting node queues\n");
  ChainedNodeQueue *waitQ = waitingQueues[nodeQ];
  waitingQueues.erase(nodeQ);     // removes from the map
  delete waitQ;
  delete nodeQ;
}
