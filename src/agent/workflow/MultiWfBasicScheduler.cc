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
 * Revision 1.16  2011/03/16 21:30:51  bdepardo
 * Add a mean to stop the main loop of schedulers
 *
 * Revision 1.15  2011/01/20 18:31:50  bdepardo
 * Prefer prefix ++/-- operators for non-primitive types.
 * Removed unused variable
 *
 * Revision 1.14  2010/08/27 07:41:53  bisnard
 * Fixed warning
 *
 * Revision 1.13  2009/09/25 12:42:09  bisnard
 * - use new DagNodeLauncher classes to manage threads
 * - added dag cancellation method
 *
 * Revision 1.12  2008/10/14 13:24:49  bisnard
 * use new class structure for dags (DagNode,DagNodePort)
 *
 * Revision 1.11  2008/06/25 10:05:44  bisnard
 * - Waiting priority set when node is put back in waiting queue
 * - Node index in wf_response stored in Node class (new attribute submitIndex)
 * - HEFT scheduler uses SeD ref instead of hostname
 * - Estimation vector and ReqID passed to client when SeD chosen by MaDag
 * - New params in execNodeOnSeD to provide ReqId and estimation vector
 * to client for solve request
 *
 * Revision 1.10  2008/06/03 13:37:09  bisnard
 * Multi-workflow sched now keeps nodes in the ready nodes queue
 * until a ressource is available to ensure comparison is done btw
 * nodes of different workflows (using sched-specific metric).
 *
 * Revision 1.9  2008/06/02 08:35:39  bisnard
 * Avoid MaDag crash in case of client-SeD comm failure
 *
 * Revision 1.8  2008/06/01 14:06:57  rbolze
 * replace most ot the cout by adapted function from debug.cc
 * there are some left ...
 *
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
#include "Dag.hh"
#include "DagNode.hh"
#include "MaDagNodeLauncher.hh"
#include "debug.hh"
#include "marshalling.hh"
#include "MaDag_impl.hh"

using namespace madag;

MultiWfBasicScheduler::MultiWfBasicScheduler(MaDag_impl* maDag)
  : MultiWfScheduler(maDag, MultiWfScheduler::MULTIWF_NO_METRIC) {
   TRACE_TEXT(TRACE_MAIN_STEPS,"Using BASIC multi-workflow scheduler" << endl);
}

MultiWfBasicScheduler::~MultiWfBasicScheduler() {
}

/**
 * Execution method
 * (does not use execQueue)
 */
void*
MultiWfBasicScheduler::run() {
  TRACE_TEXT(TRACE_MAIN_STEPS,"MultiWfBasicScheduler is running" << endl);
  int nodeCount = 0;
  while (this->keepOnRunning) {
    TRACE_TEXT(TRACE_MAIN_STEPS,"\t ** Starting MultiWfBasicScheduler" << endl);
    myLock.lock();
    // Loop over all nodeQueues and run the first ready node
    // for each queue
    nodeCount = 0;
    std::list<OrderedNodeQueue *>::iterator qp = readyQueues.begin();
    while (qp != readyQueues.end()) {
      TRACE_TEXT(TRACE_ALL_STEPS,"Checking ready nodes queue:" << endl);
      OrderedNodeQueue * readyQ = *qp;
      DagNode * n = readyQ->popFirstNode();
      if (n != NULL) {
        TRACE_TEXT(TRACE_ALL_STEPS,"  #### Ready node : " << n->getCompleteId()
            << " => execute" << endl);
        // EXECUTE NODE (NEW THREAD)
        // create a node launcher
        DagNodeLauncher *launcher = new MaDagNodeLauncher( n, this,
                                           myMaDag->getCltMan(n->getDag()->getId()));
        n->start(launcher);
        nodeCount++;
        // Destroy queues if both are empty
        ChainedNodeQueue * waitQ = waitingQueues[readyQ];
        if (waitQ->isEmpty() && readyQ->isEmpty()) {
	  TRACE_TEXT(TRACE_ALL_STEPS,"Node Queues are empty: remove & destroy" << endl);
          qp = readyQueues.erase(qp);      // removes from the list
          this->deleteNodeQueue(readyQ);  // deletes both queues
          continue;
        }
        // DELAY between NODES (to avoid interference btw submits)
        usleep(this->interRoundDelay * 1000);
      }
      ++qp; // go to next queue
    }
    myLock.unlock();

    if (nodeCount == 0) {
      TRACE_TEXT(TRACE_MAIN_STEPS,"No ready nodes" << endl);
      this->mySem.wait();

      // WAIT UNTIL A NEW DAG IS SUBMITTED OR A NODE IS COMPLETED

      this->postWakeUp();
      this->checkDagsRelease();

    }
  }
  return NULL;
}

/**
 * Notify the scheduler that a node is done
 */
void
MultiWfBasicScheduler::handlerNodeDone(DagNode * node) {
  // does nothing for this class
}

/**
 * Create two chained node queues and return the ready queue
 *  - WAITING queue => READY queue
 */

OrderedNodeQueue *
MultiWfBasicScheduler::createNodeQueue(Dag * dag)  {
  TRACE_TEXT(TRACE_ALL_STEPS, "Creating new node queues (basic)" << endl);
  OrderedNodeQueue *  readyQ  = new OrderedNodeQueue();
  ChainedNodeQueue *  waitQ   = new ChainedNodeQueue(readyQ);
  for (std::map <std::string, DagNode *>::iterator nodeIt = dag->begin();
       nodeIt != dag->end();
       ++nodeIt) {
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
