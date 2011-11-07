/**
 * @file WfScheduler.hh
 *
 * @brief  The MA DAG scheduler interface
 *
 * @author  Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#ifndef WFSCHEDULER_HH
#define WFSCHEDULER_HH

#include <vector>

#include "response.hh"

class Dag;
class DagNode;

namespace madag {
class WfScheduler {
public:
  /**
   * WfScheduler constructor
   */
  WfScheduler();

  /**
   * WfScheduler destructor
   */
  virtual ~WfScheduler();

  /**
   * Order the nodes using scheduling algorithm
   */
  virtual void
  setNodesPriority(const wf_response_t *wf_response, Dag *dag) = 0;

  /**
   * Computes the schedule using HEFT algorithm and updates node
   */
  virtual void
  setNodesEFT(std::vector<DagNode *> &orderedNodes,
              const wf_response_t *wf_response,
              Dag *dag,
              double initTime) = 0;
};
}

#endif  // _WFMETASCHED_HH_
