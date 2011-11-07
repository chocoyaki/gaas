/**
 * @file HEFTScheduler.hh
 *
 * @brief  The MA DAG HEFT scheduler
 *
 * @author  Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#ifndef _HEFTSCHEDULER_HH_
#define _HEFTSCHEDULER_HH_

#include <map>
#include <string>
#include <vector>
#include "Dag.hh"
#include "WfScheduler.hh"

namespace madag {
class HEFTScheduler : public WfScheduler {
public:
  HEFTScheduler();

  virtual ~HEFTScheduler();

  /**
   * Set the priority of nodes using b-level algorithm
   */
  virtual void
  setNodesPriority(const wf_response_t *wf_response, Dag *dag);

  /**
   * Set the EFT of nodes using HEFT algorithm
   *
   * @param orderedNodes  vector of nodes ordered by decreasing priority
   * @param wf_response   the estimates given by the MA
   * @param dag           the dag
   * @param initTime      dag starting time in ms (relative)
   */
  virtual void
  setNodesEFT(std::vector<DagNode *> &orderedNodes,
              const wf_response_t *wf_response,
              Dag *dag,
              double initTime);

private:

  /**
   * Get the value of estimated job duration from wf response
   * for a given service and a given server
   * @param wf_response the wf response structure
   * @param pbIndex the index of the problem (service)
   * @param srvIndex the index of the server
   */
  double
  getNodeDurationEst(const wf_response_t *wf_response,
                     unsigned int pbIndex,
                     unsigned int srvIndex);

  /**
   * Computes the average value of node workload across the Seds
   */
  void
  computeNodeWeights(const wf_response_t *wf_response, Dag *dag);

  /**
   * rank the node upward
   * @param n the top node to rank.
   */
  void
  rank(DagNode *n);

  /**
   * Stores all the nodes earliest finish times
   */
  std::map<std::string, double> AFT;
};
}

#endif   /* not defined _HEFTSCHEDULER_HH */
