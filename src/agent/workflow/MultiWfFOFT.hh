/**
 * @file MultiWfFOFT.hh
 *
 * @brief  Another multi-workflow scheduler based on Fairness On Finish Time algorithm
 *
 * @author  Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
 *          Benjamin Isnard (Benjamin.Isnard@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef _MULTIWFFOFT_HH_
#define _MULTIWFFOFT_HH_

#include <map>
#include <vector>

#include "MultiWfScheduler.hh"

namespace madag {
class MultiWfFOFT : public MultiWfScheduler {
public:
  explicit
  MultiWfFOFT(MaDag_impl *maDag);
  virtual ~MultiWfFOFT();

protected:
  /**
   * Updates scheduler when a node has been executed
   */
  virtual void
  handlerNodeDone(DagNode *node);

  /**
   * internal dag scheduling
   */
  virtual void
  intraDagSchedule(Dag *dag, MasterAgent_var MA)
  throw(MaDag::ServiceNotFound, MaDag::CommProblem);

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

  /**
   * updates the delay for a given node and change the current dag
   * slowdown value accordingly if the dag delay is impacted
   * @param node
   * @param delay delay in ms
   */
  virtual void
  updateNodeDelay(DagNode *node, double delay);

private:
  /**
   * Save the state of dags
   */
  std::map<Dag *, DagState> dagsState;

  /**
   * Mark the nodes which priority must be re-calculated
   */
  std::map<DagNode *, bool> nodesFlag;
};
}

#endif   /* not defined _MULTIWFFAIRNESS_SCHED_HH */
