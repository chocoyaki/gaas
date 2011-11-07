/**
 * @file MultiWfBasicScheduler.hh
 *
 * @brief  The Workflow Meta-schedulerg used when multi-workflow support is enabled
 *
 * @author  Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
 *          Benjamin Isnard (Benjamin.Isnard@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#ifndef _MULTIWFBASICSCHEDULER_HH_
#define _MULTIWFBASICSCHEDULER_HH_

#include "MultiWfScheduler.hh"

namespace madag {
class MultiWfBasicScheduler : public MultiWfScheduler {
public:
  explicit
  MultiWfBasicScheduler(MaDag_impl *maDag);

  virtual ~MultiWfBasicScheduler();

  /**
   * Execution method
   */
  virtual void *
  run();

  /**
   * Updates scheduler when a node has been executed
   * (does nothing)
   */
  virtual void
  handlerNodeDone(DagNode *node);

protected:
  /**
   * create simple ordered NodeQueue for ready nodes
   * (does not use priority of nodes)
   */
  virtual OrderedNodeQueue *
  createNodeQueue(Dag *dag);

  /**
   * delete the node queue created in createNodeQueue
   */
  virtual void
  deleteNodeQueue(OrderedNodeQueue *nodeQ);
};
}

#endif   /* not defined _MULTIWFBASICSCHEDULER_HH */
