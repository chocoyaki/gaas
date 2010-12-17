/****************************************************************************/
/* The MA DAG HEFT scheduler                                                */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.17  2010/12/17 09:48:00  kcoulomb
 * * Set diet to use the new log with forwarders
 * * Fix a CoRI problem
 * * Add library version remove DTM flag from ccmake because deprecated
 *
 * Revision 1.16  2010/07/20 08:59:36  bisnard
 * Added event generation
 *
 * Revision 1.15  2010/07/12 16:14:11  glemahec
 * DIET 2.5 beta 1 - Use the new ORB manager and allow the use of SSH-forwarders for all DIET CORBA objects
 *
 * Revision 1.14  2010/03/08 13:42:46  bisnard
 * removed debug output
 *
 * Revision 1.13  2009/09/25 12:39:13  bisnard
 * modified includes to reduce inter-dependencies
 *
 * Revision 1.12  2009/06/23 09:25:38  bisnard
 * use new estimation vector entry (EST_EFT)
 *
 * Revision 1.11  2009/04/17 08:45:40  bisnard
 * renamed Node class as WfNode
 *
 * Revision 1.10  2008/10/14 13:24:48  bisnard
 * use new class structure for dags (DagNode,DagNodePort)
 *
 * Revision 1.9  2008/09/30 15:33:35  bisnard
 * Node::prevNodes and nextNodes data structures modified
 *
 * Revision 1.8  2008/06/25 10:05:44  bisnard
 * - Waiting priority set when node is put back in waiting queue
 * - Node index in wf_response stored in Node class (new attribute submitIndex)
 * - HEFT scheduler uses SeD ref instead of hostname
 * - Estimation vector and ReqID passed to client when SeD chosen by MaDag
 * - New params in execNodeOnSeD to provide ReqId and estimation vector
 * to client for solve request
 *
 * Revision 1.7  2008/06/18 15:01:33  bisnard
 * use new attribute estDuration to store job duration for each node
 * rename method to avoid confusion (getCompTimeEst)
 * initialize dag scheduling time in multi-wf scheduler
 *
 * Revision 1.6  2008/06/03 15:32:59  bisnard
 * Modify rank calculation
 *
 * Revision 1.5  2008/05/05 13:54:17  bisnard
 * new computation time estimation get/set functions
 *
 * Revision 1.4  2008/04/30 07:32:24  bisnard
 * use relative timestamps for estimated and real completion time
 *
 * Revision 1.3  2008/04/28 11:54:52  bisnard
 * new methods setNodePriorities & setNodesEFT replacing schedule
 * nodes sort done in separate method in Dag class
 *
 * Revision 1.2  2008/04/21 14:31:45  bisnard
 * moved common multiwf routines from derived classes to MultiWfScheduler
 * use wf request identifer instead of dagid to reference client
 * use nodeQueue to manage multiwf scheduling
 * renamed WfParser as DagWfParser
 *
 * Revision 1.1  2008/04/10 08:15:54  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 ****************************************************************************/

#include "est_internal.hh"
#include "HEFTScheduler.hh"
#include "DagNode.hh"
#include "debug.hh"
#include "ORBMgr.hh"

using namespace madag;

/****************************************************************************/
/*                         PUBLIC METHODS                                   */
/****************************************************************************/

HEFTScheduler::HEFTScheduler() {
}

HEFTScheduler::~HEFTScheduler() {
}

/**
 * Set the priority of nodes using b-level algorithm
 */
void
HEFTScheduler::setNodesPriority(const wf_response_t * wf_response, Dag * dag) {

  TRACE_TEXT (TRACE_ALL_STEPS, "HEFT : Setting nodes priority" << endl);
  // Initialize the node weights (priority attr)
  this->computeNodeWeights(wf_response, dag);

  // Ranking
  DagNode * n = NULL;
  for (std::map <std::string, DagNode *>::iterator p = dag->begin();
       p != dag->end();
       p++) {
         n = (DagNode *)(p->second);
         if (n->isAnExit())
           rank(n);
  }
} // end setNodesPriority

/**
 * Initialize the Earliest Finish Time for all nodes
 * This version computes the EFT using the best server found and taking into
 * account the availability of the servers, considering that the current dag
 * has exclusive access to them.
 * Note that if there are many servers available for a given node, the
 * complexity of this method is <nb_servers> * <nb_predecessors> for each
 * node.
 * @param orderedNodes the array of nodes to initialize
 * @param initTime the current time (relative to scheduler ref) in ms
 */
void
HEFTScheduler::setNodesEFT(std::vector<DagNode *>& orderedNodes,
                           const wf_response_t * wf_response,
                           Dag * dag,
                           double initTime) {
  // to store the availabilty of resources
  map<SeD_ptr, double> avail;
  TRACE_TEXT (TRACE_ALL_STEPS, "HEFT : start computing nodes EFT (init time = "
      << initTime << ")" << endl);
  // LOOP-1: for all dag nodes in the order provided
  for (std::vector<DagNode *>::iterator p = orderedNodes.begin();
       p != orderedNodes.end();
       p++) {
    DagNode *        n = (DagNode *) *p;
    unsigned int  pb_index = 0; // index of the service (problem) in the wf_response
    unsigned int  sed_ind = 0;  // index of the chosen server in the wf_response
    SeD_ptr       chosenSeDPtr; // ref to the chosen SeD
    double        EFT = 0;      // earliest finish time for current node

    // find the problem index
    pb_index = n->getSubmitIndex(); // this index was stored before submitting to MA

    /* LOOP-2 FOR ONE NODE => CHOOSES THE BEST SERVER & COMPUTES AFT */
    /* Remark: this loop can be very costly: <nb servers> * <nb predecessors> !! */
    for (unsigned int ix=0;
	 ix<wf_response->wfn_seq_resp[pb_index].response.servers.length();
	 ix++) {
      string ss(CORBA::string_dup(wf_response->wfn_seq_resp[pb_index].response.servers[ix].loc.hostName));
      string sedName = string(wf_response->wfn_seq_resp[pb_index].response.servers[ix].loc.SeDName);
      SeD_ptr curSeDPtr = ORBMgr::getMgr()->resolve<SeD, SeD_ptr>(SEDCTXT, sedName);
			
      // get availability of current server (loop to check SeD ref equivalence - this is used to
      // avoid conflicts if two SeDs share the same hostname)
      double EST = initTime; // earliest start time among all SeDs for this service
      for (map<SeD_ptr, double>::iterator availIter=avail.begin();
           availIter!=avail.end();
           availIter++) {
        if (curSeDPtr->_is_equivalent(availIter->first)) {
          EST = availIter->second;
          curSeDPtr = availIter->first;  // replace pointer to SeD by equivalent
          break;
        }
      }
      // increase earliest starting time if some dependencies with other nodes
      for (vector<WfNode*>::iterator prevIter = n->prevNodesBegin();
           prevIter != n->prevNodesEnd();
           ++prevIter) {
	EST = max(EST, AFT[(dynamic_cast<DagNode*>(*prevIter))->getCompleteId()]);
      }
      // choose server if it improves the EFT
      double nodeDuration = this->getNodeDurationEst(wf_response, pb_index, ix);
      if ( ( EST + nodeDuration < EFT ) || (EFT == 0)) {
	EFT = EST + nodeDuration;
	sed_ind = ix;
        chosenSeDPtr = curSeDPtr;
      }
    } // end for ix
    /* END OF THE MAIN COMPUTATION LOOP */

    // update availability of chosen server
    avail[chosenSeDPtr] = EFT;
    // update node scheduling info
    AFT[n->getCompleteId()] = EFT;
    n->setEstCompTime((long int)EFT);
    TRACE_TEXT (TRACE_ALL_STEPS, "HEFT : node " << n->getCompleteId()
        << " : EFT = " << EFT << endl);
  } // end LOOP-1
} // end setNodesEFT

/****************************************************************************/
/*                          PRIVATE METHODS                                 */
/****************************************************************************/
/**
 * Get the estimation of job duration from the SeD estimation vector
 * (TCOMP value)
 * IMPORTANT: the value must be in milliseconds
 */
double
HEFTScheduler::getNodeDurationEst(const wf_response_t * wf_response,
                              unsigned int pbIndex,
                              unsigned int srvIndex) {
  return diet_est_get_internal(&wf_response->wfn_seq_resp[pbIndex].response.servers[srvIndex].estim,
                               EST_TCOMP, 0);
}

/**
 * Computes the average value of node workload across the Seds
 */
void
HEFTScheduler::computeNodeWeights(const wf_response_t * wf_response,
                                  Dag * dag) {
  TRACE_TEXT (TRACE_ALL_STEPS, "HEFT : start computing weights (mean of estimates)" << endl);
  DagNode * n = NULL;
  int ix = 0; // index of the node response in wf_response
  for (std::map <std::string, DagNode *>::iterator p = dag->begin();
       p != dag->end();
       p++) {
    n = (DagNode *)(p->second);
    n->setEstDuration(0);
    ix = n->getSubmitIndex(); // the index was stored before submitting to MA
    double w = 0;
    unsigned int    nbServers = wf_response->wfn_seq_resp[ix].response.servers.length();
    for (unsigned int jx=0; jx < nbServers; jx++) {
          w += this->getNodeDurationEst(wf_response, ix, jx);
    } // end for jx
    if (nbServers > 0) n->setEstDuration( w / nbServers);
    TRACE_TEXT (TRACE_ALL_STEPS, " HEFT : node " << n->getCompleteId() << " weight :"
         << n->getEstDuration() << endl);
  } // end for nodes
} // end computeNodeWeights

/**
 * rank the nodes upward
 * (uses the estimation of job duration calculated for each node)
 */
void
HEFTScheduler::rank(DagNode * n) {  // RECURSIVE
  DagNode * succ = NULL;
  unsigned len = n->nextNodesNb();
  if (len == 0) { // exit node
    n->setPriority(n->getEstDuration());
  } else {
    // LOOP for all descendant nodes of n
    for (list<WfNode*>::iterator nextIter = n->nextNodesBegin();
        nextIter != n->nextNodesEnd();
        ++nextIter) {
      succ = dynamic_cast<DagNode*>(*nextIter);
      // add duration of current node and priority of descendant node and compare it to
      // priority of current node: if higher then change priority of current node
      if ((succ->getPriority() + n->getEstDuration()) > n->getPriority()) {
        n->setPriority(succ->getPriority() + n->getEstDuration());
      }
    }
  }
  TRACE_TEXT (TRACE_ALL_STEPS, " HEFT : priority of node " << n->getCompleteId()
      << " is " << n->getPriority() << endl);
  // LOOP for all preceding nodes of n
  for (vector<WfNode*>::iterator prevIter = n->prevNodesBegin();
       prevIter != n->prevNodesEnd();
       ++prevIter) {
    DagNode * prev = dynamic_cast<DagNode*>(*prevIter);
    // if preceding node is not already done or running then rank it
    if ((!prev->isDone()) && (!prev->isRunning()))
      rank(prev);
  }
}
