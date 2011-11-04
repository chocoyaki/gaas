/**
 * @file MultiWfScheduler.hh
 *
 * @brief  The base class for multi-workflow schedule
 *
 * @author  Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
 *          Benjamin Isnard (Benjamin.Isnard@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef _MULTIWFSCHEDULER_HH_
#define _MULTIWFSCHEDULER_HH_

#include <list>
#include <map>
#include <string>
#include "WfScheduler.hh"
#include "CltMan.hh"
#include "MasterAgent.hh"
#include "SeD.hh"
/* workflow utils */
#include "Thread.hh"
#include "NodeQueue.hh"
#include "DagScheduler.hh"
#include "DagNodeLauncher.hh"
#include "MetaDag.hh"
#include "MaDag.hh"

class MaDag_impl;

namespace madag {
class NodeRun;
class DagState;

class MultiWfScheduler : public Thread, public DagScheduler {
public:
  /**
   * Selector for node priority policy:
   * (depends on the metric used to compare nodes for execution priority)
   *   MULTIWF_NO_METRIC: when no metric is used (Basic scheduler)
   *   MULTIWF_NODE_METRIC: metric is for each node individually (eg b-level)
   *   MULTIWF_DAG_METRIC: metric is per dag (eg slowdown)
   */
  typedef enum {MULTIWF_NO_METRIC, MULTIWF_NODE_METRIC, MULTIWF_DAG_METRIC}
  nodePolicy_t;

  /**
   * Selector for platform type
   * (used to optimize the requests to the platform)
   *   PFM_ANY : (default) any platform
   *   PFM_SAME_SERVICES : platform containing hosts with same service list
   */
  typedef enum {PFM_ANY, PFM_SAME_SERVICES}
  pfmType_t;

  /**
   * Event message types
   */
  enum {CONSTR};

  MultiWfScheduler(MaDag_impl * maDag, nodePolicy_t nodePol);

  virtual ~MultiWfScheduler();

  /**
   * Get the MaDag object ref
   */
  const MaDag_impl *
  getMaDag() const;

  /**
   * set the intra-dag scheduler used by the MA DAG (by default it is HEFT)
   * @param sched the base scheduler to be used
   */
  virtual void
  setSched(WfScheduler *sched);

  /**
   * set the platform type
   * @param pfmType the type of platform (grid) used
   */
  virtual void
  setPlatformType(pfmType_t pfmType);

  /**
   * set the inter-round delay in milliseconds
   * this value (by default 100ms) is used to avoid burst of submit requests
   * that result in mapping several jobs to same SeD
   */
  virtual void
  setInterRoundDelay(int IRD_value);

  /**
   * get the inter-round delay in milliseconds
   * this value (by default 100ms) is used to avoid burst of submit requests
   * that result in mapping several jobs to same SeD
   */
  virtual const int
  getInterRoundDelay() const;

  /**
   * schedules a new DAG workflow
   * @param newDag  ref to a dag
   * @param metaDag ref to a metadag containing the dag (optional)
   */
  virtual void
  scheduleNewDag(Dag *newDag, MetaDag *metaDag = NULL)
  throw(MaDag::ServiceNotFound, MaDag::CommProblem);

  /**
   * Execution method
   */
  virtual void *
  run();

  /**
   * Stop the thread loop.
   */
  void
  stop(bool wait = true);

  /**
   * Dag cancellation
   */
  virtual void
  cancelDag(const std::string &dagId);

  /**
   * Returns a description
   */
  virtual std::string
  toString() const;

protected:

  /**
   * Get a dag ref given the dag id
   * (will not find a completed dag except if part of a non-completed metaDag)
   */
  Dag *
  getDag(const std::string &dagId) throw(MaDag::InvalidDag);

  /**
   * Get the MetaDag of a given dag
   */
  MetaDag *
  getMetaDag(Dag *dag);

  /**
   * Call MA to get server estimations for all services
   */
  wf_response_t *
  getProblemEstimates(Dag *dag, MasterAgent_var MA)
  throw(MaDag::ServiceNotFound, MaDag::CommProblem);

  /**
   * Call MA to get server estimations for one node
   */
  wf_response_t *
  getProblemEstimates(DagNode *node, MasterAgent_var MA)
  throw(MaDag::ServiceNotFound, MaDag::CommProblem);

  /**
   * internal dag scheduling
   * to set priority of nodes using estimations given by MA and the
   * algorithm specified by setSched
   *
   * @param dag     pointer to dag object
   * @param MA      ref to the master agent
   */
  virtual void
  intraDagSchedule(Dag *dag, MasterAgent_var MA)
  throw(MaDag::ServiceNotFound, MaDag::CommProblem);

  /**
   * create a new node queue based on a dag
   * (by default uses a priority-based nodeQueue)
   *
   * @param dag   a dag
   * @return pointer to a nodequeue structure (to be destroyed by the caller)
   */
  virtual OrderedNodeQueue *
  createNodeQueue(Dag *dag);

  /**
   * delete the node queue created in createNodeQueue
   *
   * @param nodeQ   pointer to the node queue created in createdNodeQueue
   */
  virtual void
  deleteNodeQueue(OrderedNodeQueue *nodeQ);

  /**
   * insert a new node queue into the pool of queues managed by the sched
   * @param nodeQ   a node queue
   */
  virtual void
  insertNodeQueue(OrderedNodeQueue *nodeQ);

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
   * Get the current time from scheduler reference clock
   * @return  current time in milliseconds (from scheduler start)
   */
  virtual double
  getRelCurrTime();

  /**
   * Execute a post operation on synchronisation semaphore
   * @param newDag  must be set to True if calling post for a new submission
   * @param node    must contain the node ref if calling post for end of node
   */
  virtual void
  wakeUp(bool newDag, DagNode *node = NULL);

  /**
   * Updates scheduler when a node has no (more) dependencies
   */
  virtual void
  handlerNodeReady(DagNode *node);

  /**
   * Updates scheduler when a node has been completed successfully
   */
  virtual void
  handlerNodeDone(DagNode *node) = 0;

  /**
   * Updates scheduler when a dag has been executed
   */
  virtual void
  handlerDagDone(Dag *dag);

  /**
   * Handles the node threads termination
   */
  virtual void
  postWakeUp();

  /**
   * Handles the dag termination
   */
  virtual void
  checkDagsRelease();

  /**
   * Release a dag on the client
   */
  virtual void
  releaseDag(Dag *dag);

  /**
   * The Wf meta-scheduler scheduler
   */
  WfScheduler *mySched;

  /**
   * DagNode queues for waiting nodes
   * (key = ref of ready queue)
   */
  std::map<NodeQueue *, ChainedNodeQueue *> waitingQueues;

  /**
   * DagNode queues for ready nodes
   */
  std::list<OrderedNodeQueue *> readyQueues;

  /**
   * DagNode queue for nodes to be executed
   */
  OrderedNodeQueue *execQueue;

  /**
   * Store the nodes HEFT priority
   */
  std::map<DagNode *, double> nodesHEFTPrio;

  /**
   * Selector for node priority policy
   */
  nodePolicy_t nodePolicy;

  /**
   * Selector for platform type
   */

  pfmType_t platformType;

  /**
   * Critical section of the scheduler
   */
  omni_mutex myLock;

  /**
   * Synchronisation semaphore
   */
  omni_semaphore mySem;

  /**
   * FIFO list for semaphore post information
   */
  typedef struct {
    bool isNewDag;
    DagNode *nodeRef;
  } wakeUpInfo_t;

  std::list<wakeUpInfo_t>  myWakeUpList;

  /**
   * Semaphore for access to WakeUpList and DagsTermList
   */
  omni_mutex myWakeUpLock;

  /**
   * MAP for dag termination
   */
  std::list<std::string> myDagsTermList;

  /**
   * Inter-round delay (used to separate DIET submits)
   **(in milliseconds)
   */
  int interRoundDelay;

private:
  friend class MultiWfBasicScheduler;
  friend class MaDagNodeLauncher;

  /**
   * MaDag reference
   */
  MaDag_impl *myMaDag;

  /**
   * Reference time
   */
  struct timeval refTime;

  /**
   * Map dag id => metadag ref (only if dag belongs to metadag)
   */
  std::map<std::string, MetaDag *>  myMetaDags;

  /**
   * Map dag id => dag ptr (for all dags)
   */
  std::map<std::string, Dag *>  myDags;

  /**
   * Should the thread keep on running?
   */
  bool keepOnRunning;
};  // end class MultiWfScheduler


/****************************************************************************/
/*                            CLASS DagState                                */
/****************************************************************************/
/**
 * This class is used by some derived classes of MultiWfScheduler to store
 * information about the status of the DAG. The Dag structure itself is not
 * used as this information is managed by the scheduler.
 */

class DagState {
public:
  DagState();

  /**
   * the earliest finish time of the DAG scheduled alone
   * Used by FOFT / AgingHEFT
   */
  double EFT;

  /**
   * the makespan of the DAG scheduled alone
   * Used by FOFT / AgingHEFT
   */
  double makespan;

  /**
   * the estimated global delay for the DAG
   * Used by FOFT
   */
  double estimatedDelay;

  /**
   * Dag slowdown (percentage of delay / makespan)
   * Used by FOFT
   */
  double slowdown;
};  // end class DagState
}  // end namespace madag

#endif   /* not defined _ABSWFMETASCHED_HH */
