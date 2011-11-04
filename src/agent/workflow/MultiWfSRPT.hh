/**
 * @file MultiWfSRPT.hh
 *
 * @brief  A multi-workflow scheduler based on Shortest Remaining Processing Time
 *
 * @author  Benjamin Isnard (Benjamin.Isnard@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef _MULTIWFSRPT_HH_
#define _MULTIWFSRPT_HH_

#include "MultiWfScheduler.hh"

namespace madag {
class MultiWfSRPT : public MultiWfScheduler {
public:
  explicit
  MultiWfSRPT(MaDag_impl *maDag);

  virtual ~MultiWfSRPT();

protected:
  /**
   * Updates scheduler when a node has been executed
   */
  virtual void
  handlerNodeDone(DagNode *node);

  /**
   * set node priority before inserting into execution queue
   */
  virtual void
  setExecPriority(DagNode *node);

  /**
   * set node priority before inserting back in the ready queue
   */
  virtual void
  setWaitingPriority(DagNode *node);
};
}

#endif   /* not defined _MULTIWFSRPT_HH */
