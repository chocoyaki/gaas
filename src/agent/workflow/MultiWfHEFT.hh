/**
 * @file MultiWfHEFT.hh
 *
 * @brief  Another multi-workflow scheduler based on HEFT algorithm
 *
 * @author  Benjamin Isnard (Benjamin.Isnard@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#ifndef _MULTIWFHEFT_HH_
#define _MULTIWFHEFT_HH_

#include <cmath>
#include <map>
#include "debug.hh"
#include "MultiWfScheduler.hh"



namespace madag {
/****************************************************************************/
/*                              Multi-HEFT                                  */
/****************************************************************************/

class MultiWfHEFT : public MultiWfScheduler {
public:
  explicit
  MultiWfHEFT(MaDag_impl *maDag);

  virtual ~MultiWfHEFT();

protected:
  /**
   * Updates scheduler when a node has been executed
   */
  virtual void
  handlerNodeDone(DagNode *node);
};  // end class MultiWfHEFT

/****************************************************************************/
/*                              Aging HEFT                                  */
/****************************************************************************/

class MultiWfAgingHEFT : public MultiWfScheduler {
public:
  explicit
  MultiWfAgingHEFT(MaDag_impl *maDag);
  virtual ~MultiWfAgingHEFT();

protected:
  /**
   * internal dag scheduling
   */
  virtual void
  intraDagSchedule(Dag *dag, MasterAgent_var MA)
  throw(MaDag::ServiceNotFound, MaDag::CommProblem);

  /**
   * Updates scheduler when a node has been executed
   */
  virtual void
  handlerNodeDone(DagNode *node);

  /**
   * set node priority before inserting into execution queue
   * (called by run method)
   * @param node   the node to insert
   */
  virtual void
  setExecPriority(DagNode *node);

  /**
   * set node priority before inserting back in the ready queue
   */
  virtual void
  setWaitingPriority(DagNode *node);

  /**
   * Save the state of dags
   */
  std::map<Dag *, DagState> dagsState;
};  // end class MultiWfAgingHEFT
}  // end namespace madag

#endif  // _MULTIWFHEFT_HH_
