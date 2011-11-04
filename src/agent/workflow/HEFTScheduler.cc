/**
 * @file HEFTScheduler.cc
 *
 * @brief  The MA DAG HEFT scheduler
 *
 * @author  Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


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
HEFTScheduler::setNodesPriority(const wf_response_t *wf_response, Dag *dag) {
  TRACE_TEXT(TRACE_ALL_STEPS, "HEFT : Setting nodes priority\n");
  // Initialize the node weights (priority attr)
  this->computeNodeWeights(wf_response, dag);

  // Ranking
  DagNode *n = NULL;
  for (std::map <std::string, DagNode *>::iterator p = dag->begin();
       p != dag->end();
       ++p) {
    n = (DagNode *) (p->second);
    if (n->isAnExit()) {
      rank(n);
    }
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
HEFTScheduler::setNodesEFT(std::vector<DagNode *> &orderedNodes,
                           const wf_response_t *wf_response,
                           Dag *dag,
                           double initTime) {
  // to store the availabilty of resources
  std::map<SeD_ptr, double> avail;
  TRACE_TEXT(TRACE_ALL_STEPS, "HEFT : start computing nodes EFT (init time = "
             << initTime << ")\n");
  // LOOP-1: for all dag nodes in the order provided
  for (std::vector<DagNode *>::iterator p = orderedNodes.begin();
       p != orderedNodes.end();
       ++p) {
    DagNode *n = (DagNode *) *p;
    unsigned int pb_index = 0;   // index of the service (problem) in the wf_response
    SeD_ptr chosenSeDPtr;        // ref to the chosen SeD
    double EFT = 0;             // earliest finish time for current node

    // find the problem index
    pb_index = n->getSubmitIndex();  // this index was stored before submitting to MA

    /* LOOP-2 FOR ONE NODE => CHOOSES THE BEST SERVER & COMPUTES AFT */
    /* Remark: this loop can be very costly: <nb servers> * <nb predecessors> !! */
    for (unsigned int ix = 0;
         ix < wf_response->wfn_seq_resp[pb_index].response.servers.length();
         ix++) {
      std::string ss(CORBA::string_dup(
                       wf_response->wfn_seq_resp[pb_index].response.servers[ix]
                       .loc.hostName));
      std::string sedName(
        wf_response->wfn_seq_resp[pb_index].response.servers[ix].loc.SeDName);
      SeD_ptr curSeDPtr =
        ORBMgr::getMgr()->resolve<SeD, SeD_ptr>(SEDCTXT, sedName);

      // get availability of current server
      // (loop to check SeD ref equivalence - this is used to
      // avoid conflicts if two SeDs share the same hostname)
      // earliest start time among all SeDs for this service
      double EST = initTime;

      for (std::map<SeD_ptr, double>::iterator availIter = avail.begin();
           availIter != avail.end();
           ++availIter) {
        if (curSeDPtr->_is_equivalent(availIter->first)) {
          EST = availIter->second;
          curSeDPtr = availIter->first;  // replace pointer to SeD by equivalent
          break;
        }
      }
      // increase earliest starting time if some dependencies with other nodes
      for (std::vector<WfNode *>::iterator prevIter = n->prevNodesBegin();
           prevIter != n->prevNodesEnd();
           ++prevIter) {
        EST =
          std::max(EST,
                   AFT[(dynamic_cast<DagNode *>(*prevIter))->getCompleteId()]);
      }
      // choose server if it improves the EFT
      double nodeDuration = this->getNodeDurationEst(wf_response, pb_index, ix);
      if ((EST + nodeDuration < EFT) || (EFT == 0)) {
        EFT = EST + nodeDuration;
        chosenSeDPtr = curSeDPtr;
      }
    } // end for ix
      /* END OF THE MAIN COMPUTATION LOOP */

    // update availability of chosen server
    avail[chosenSeDPtr] = EFT;
    // update node scheduling info
    AFT[n->getCompleteId()] = EFT;
    n->setEstCompTime((long int) EFT);
    TRACE_TEXT(TRACE_ALL_STEPS, "HEFT : node " << n->getCompleteId()
                                               << " : EFT = " << EFT << "\n");
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
HEFTScheduler::getNodeDurationEst(const wf_response_t *wf_response,
                                  unsigned int pbIndex,
                                  unsigned int srvIndex) {
  return diet_est_get_internal(
           &wf_response->wfn_seq_resp[pbIndex].response.servers[srvIndex].estim,
           EST_TCOMP, 0);
}

/**
 * Computes the average value of node workload across the Seds
 */
void
HEFTScheduler::computeNodeWeights(const wf_response_t *wf_response,
                                  Dag *dag) {
  TRACE_TEXT(TRACE_ALL_STEPS,
             "HEFT : start computing weights (mean of estimates)\n");
  DagNode *n = NULL;
  int ix = 0;  // index of the node response in wf_response
  for (std::map<std::string, DagNode *>::iterator p = dag->begin();
       p != dag->end();
       ++p) {
    n = (DagNode *) (p->second);
    n->setEstDuration(0);
    ix = n->getSubmitIndex();  // the index was stored before submitting to MA
    double w = 0;
    unsigned int nbServers =
      wf_response->wfn_seq_resp[ix].response.servers.length();
    for (unsigned int jx = 0; jx < nbServers; jx++) {
      w += this->getNodeDurationEst(wf_response, ix, jx);
    } // end for jx
    if (nbServers > 0) {
      n->setEstDuration(w / nbServers);
    }
    TRACE_TEXT(TRACE_ALL_STEPS,
               " HEFT : node " << n->getCompleteId() << " weight :"
                               << n->getEstDuration() << "\n");
  }
} // computeNodeWeights

/**
 * rank the nodes upward
 * (uses the estimation of job duration calculated for each node)
 */
void
HEFTScheduler::rank(DagNode *n) {   // RECURSIVE
  DagNode *succ = NULL;
  unsigned len = n->nextNodesNb();
  if (len == 0) { // exit node
    n->setPriority(n->getEstDuration());
  } else {
    // LOOP for all descendant nodes of n
    for (std::list<WfNode *>::iterator nextIter = n->nextNodesBegin();
         nextIter != n->nextNodesEnd();
         ++nextIter) {
      succ = dynamic_cast<DagNode *>(*nextIter);
      // add duration of current node and priority of descendant node and compare it to
      // priority of current node: if higher then change priority of current node
      if ((succ->getPriority() + n->getEstDuration()) > n->getPriority()) {
        n->setPriority(succ->getPriority() + n->getEstDuration());
      }
    }
  }
  TRACE_TEXT(TRACE_ALL_STEPS,
             " HEFT : priority of node " << n->getCompleteId()
                                         << " is " <<
             n->getPriority() << "\n");
  // LOOP for all preceding nodes of n
  for (std::vector<WfNode *>::iterator prevIter = n->prevNodesBegin();
       prevIter != n->prevNodesEnd();
       ++prevIter) {
    DagNode *prev = dynamic_cast<DagNode *>(*prevIter);
    // if preceding node is not already done or running then rank it
    if ((!prev->isDone()) && (!prev->isRunning())) {
      rank(prev);
    }
  }
} // rank
