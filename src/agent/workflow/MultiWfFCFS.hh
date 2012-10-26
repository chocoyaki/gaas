/**
 * @file MultiWfFCFS.hh
 *
 * @brief  Another multi-workflow scheduler based on First Come First Serve
 *
 * @author  Raphaël  Bolze  (raphael.bolze@ens-lyon.fr)
 *          Benjamin Isnard (Benjamin.Isnard@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef _MULTIWFFCFC_HH_
#define _MULTIWFFCFS_HH_

#include "MultiWfScheduler.hh"

namespace madag {
class MultiWfFCFS : public MultiWfScheduler {
public:
  explicit
  MultiWfFCFS(MaDag_impl *maDag);

  virtual ~MultiWfFCFS();

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

#endif   /* not defined _MULTIWFFCFS_HH */
