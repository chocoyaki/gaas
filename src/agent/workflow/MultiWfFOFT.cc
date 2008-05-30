/****************************************************************************/
/* Another multi-workflow scheduler                                         */
/* This scheduler is based on Fairness On Finish Time algorithm             */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
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

using namespace madag;

/****************************************************************************/
/*                         PUBLIC METHODS                                   */
/****************************************************************************/

MultiWfFOFT::MultiWfFOFT(MaDag_impl* maDag) : MultiWfScheduler(maDag) {
  this->execQueue = new PriorityNodeQueue;
  cout << "Using FOFT multi-workflow scheduler" << endl;
}

MultiWfFOFT::~MultiWfFOFT() {
  delete dynamic_cast<PriorityNodeQueue *>(execQueue);
}

/**
 * DagState default constructor
 */
DagState::DagState() {
  this->executed = false;
  this->EFT = -1;
  this->makespan = -1;
  this->estimatedDelay = 0;
  this->slowdown = 0;
  this->executedNodes = 0;
}


/**
 * Notify the scheduler that a node is done (called by runNode)
 * Triggers the update of slowdown parameter for the dag of the node
 */
void
MultiWfFOFT::handlerNodeDone(Node * node) {
  DagState& curDagState   = this->dagsState[node->getDag()];
  double dagPrevEstDelay  = curDagState.estimatedDelay;
  if (node->getDag()->updateDelayRec(node, node->getRealDelay())) {
    double dagNewEstDelay   = node->getDag()->getEstDelay();
    // updates slowdown if the global delay for the dag is increased
    if (dagNewEstDelay > dagPrevEstDelay) {
       // updates the estimated delay
      curDagState.estimatedDelay = dagNewEstDelay;
      // slowdown is the percentage of delay relatively to initial makespan
      curDagState.slowdown = (double) 100 * dagNewEstDelay / curDagState.makespan;
      TRACE_FUNCTION(TRACE_ALL_STEPS, "updated slowdown = "
          << curDagState.slowdown << endl);
    }
  } else {
    cout << "Problem during updateDelayRec" << endl;
  }
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
    throw (NodeException) {
  // Call the MA to get estimations for all services
  wf_response_t * wf_response = this->getProblemEstimates(dag, MA);

  // Prioritize the nodes (with intra-dag scheduler)
  this->mySched->setNodesPriority(wf_response, dag);

  // Initialize the earliest finish time for all nodes
  double startTime = 0; // will contain the timestamp for scheduling starting time
  std::vector<Node*>& orderedNodes = dag->getNodesByPriority();
  this->mySched->setNodesEFT(orderedNodes, wf_response, dag, startTime, this->getRefTime());
  delete &orderedNodes;

  // Initialize the earliest finish time and makespan of the dag
  this->dagsState[dag].EFT       = dag->getEFT();
  this->dagsState[dag].makespan  = this->dagsState[dag].EFT - startTime;
  TRACE_FUNCTION(TRACE_ALL_STEPS, "Dag " << dag->getId() << " : EFT = "
      << this->dagsState[dag].EFT << " / makespan = " << this->dagsState[dag].makespan
      << endl);
}

/**
 * set node priority before inserting into execution queue
 */
void
MultiWfFOFT::setExecPriority(Node * node) {
  node->setPriority(this->dagsState[node->getDag()].slowdown);
  cout << "     Node priority (slowdown) set to " << node->getPriority()
      << " before exec. " << endl;
}
