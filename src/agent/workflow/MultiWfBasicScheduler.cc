/****************************************************************************/
/* The Workflow Meta-scheduler                                              */
/* This scheduler is used when multi-workflow support is enabled            */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.7  2008/05/30 13:22:19  bisnard
 * added micro-delay between workflow node executions to avoid interf
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
 * Revision 1.3  2008/04/15 14:20:19  bisnard
 * - Postpone sed mapping until wf node is executed
 *
 * Revision 1.2  2008/04/14 13:44:29  bisnard
 * - Parameter 'used' obsoleted in MultiWfScheduler::submit_wf & submit_pb_set
 *
 * Revision 1.1  2008/04/10 09:13:29  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 ****************************************************************************/

#include "MultiWfBasicScheduler.hh"
#include "debug.hh"
#include "marshalling.hh"

using namespace madag;

MultiWfBasicScheduler::MultiWfBasicScheduler(MaDag_impl* maDag)
  : MultiWfScheduler(maDag) {
  cout << "Using BASIC multi-workflow scheduler" << endl;
}

MultiWfBasicScheduler::~MultiWfBasicScheduler() {
}

/**
 * Execution method
 * (does not use execQueue)
 */
void*
MultiWfBasicScheduler::run() {
  int nodeCount = 0;
  while (true) {
    cout << "\t ** Starting MultiWfBasicScheduler" << endl;
    this->myLock.lock();
    // Loop over all nodeQueues and run the first ready node
    // for each queue
    nodeCount = 0;
    std::list<OrderedNodeQueue *>::iterator qp = readyQueues.begin();
    while (qp != readyQueues.end()) {
      cout << "Checking ready nodes queue:" << endl;
      OrderedNodeQueue * readyQ = *qp;
      Node * n = readyQ->popFirstNode();
      if (n != NULL) {
        cout << "  #### Ready node : " << n->getCompleteId()
            << " (request #" << n->getWfReqId() << ") => execute" << endl;
        // EXECUTE NODE (NEW THREAD)
        n->setAsRunning();
        runNode(n, n->getSeD());
        nodeCount++;
        // Destroy queues if both are empty
        ChainedNodeQueue * waitQ = waitingQueues[readyQ];
        if (waitQ->isEmpty() && readyQ->isEmpty()) {
          cout << "Node Queues are empty: remove & destroy" << endl;
          qp = readyQueues.erase(qp);      // removes from the list
          this->deleteNodeQueue(readyQ);  // deletes both queues
          continue;
        }
        // DELAY between NODES (to avoid interference btw submits)
        usleep(this->interNodeDelay * 1000);
      }
      ++qp; // go to next queue
    }
    this->myLock.unlock();
    if (nodeCount == 0) {
      cout << "No ready nodes" << endl;
      this->mySem.wait();
      if (this->termNode) {
        this->termNodeThread->join();
        delete this->termNodeThread;
      }
    }
  }
}

/**
 * Notify the scheduler that a node is done
 */
void
MultiWfBasicScheduler::handlerNodeDone(Node * node) {
  // does nothing for this class
}

/**
 * Create two chained node queues and return the ready queue
 *  - WAITING queue => READY queue
 */

OrderedNodeQueue *
MultiWfBasicScheduler::createNodeQueue(Dag * dag)  {
  TRACE_TEXT (TRACE_ALL_STEPS, "Creating new node queues (basic)" << endl);
  OrderedNodeQueue *  readyQ  = new OrderedNodeQueue();
  ChainedNodeQueue *  waitQ   = new ChainedNodeQueue(readyQ);
  for (std::map <std::string, Node *>::iterator nodeIt = dag->begin();
       nodeIt != dag->end();
       nodeIt++) {
    waitQ->pushNode(&(*nodeIt->second));
  }
  this->waitingQueues[readyQ] = waitQ; // used to destroy waiting queue
  return readyQ;
}


/**
 * Delete the two chained node queues
 *  - WAITING queue => READY queue
 */
void
MultiWfBasicScheduler::deleteNodeQueue(OrderedNodeQueue * nodeQ) {
  TRACE_TEXT (TRACE_ALL_STEPS, "Deleting node queues" << endl);
  ChainedNodeQueue *  waitQ = waitingQueues[nodeQ];
  waitingQueues.erase(nodeQ);     // removes from the map
  delete waitQ;
  delete nodeQ;
}
