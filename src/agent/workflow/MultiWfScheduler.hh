/****************************************************************************/
/* The base class for multi-workflow scheduler                     */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.21  2009/02/06 14:50:35  bisnard
 * setup exceptions
 *
 * Revision 1.20  2009/01/16 13:41:22  bisnard
 * added common base class DagScheduler to simplify dag events handling
 * improved exception management
 *
 * Revision 1.19  2008/12/02 10:21:03  bisnard
 * use MetaDags to handle multi-dag submission and execution
 *
 * Revision 1.18  2008/10/14 13:24:49  bisnard
 * use new class structure for dags (DagNode,DagNodePort)
 *
 * Revision 1.17  2008/09/30 09:25:34  bisnard
 * use Node::initProfileSubmit to create the diet profile before requesting node estimation to MA
 *
 * Revision 1.16  2008/09/04 14:33:55  bisnard
 * - New option for MaDag to select platform type (servers
 * with same service list or not)
 * - Optimization of the multiwfscheduler to avoid requests to
 * MA for server availability
 *
 * Revision 1.15  2008/07/12 00:22:28  rbolze
 * add function getInterRoundDelay()
 * use this function when the maDag start to display this value.
 * display the dag_id when compute the ageFactor in AgingHEFT
 * add some stats info :
 * 	queuedNodeCount
 * 	change MA DAG to MA_DAG
 *
 * Revision 1.14  2008/07/08 15:52:03  bisnard
 * Set interRoundDelay as parameter of workflow scheduler
 *
 * Revision 1.13  2008/06/25 10:05:44  bisnard
 * - Waiting priority set when node is put back in waiting queue
 * - Node index in wf_response stored in Node class (new attribute submitIndex)
 * - HEFT scheduler uses SeD ref instead of hostname
 * - Estimation vector and ReqID passed to client when SeD chosen by MaDag
 * - New params in execNodeOnSeD to provide ReqId and estimation vector
 * to client for solve request
 *
 * Revision 1.12  2008/06/19 10:18:54  bisnard
 * new heuristic AgingHEFT for multi-workflow scheduling
 *
 * Revision 1.11  2008/06/18 15:03:09  bisnard
 * use milliseconds instead of seconds in timestamps
 * new handler method when node is waiting in queue
 * set NodeRun class as friend to protect handler methods
 *
 * Revision 1.10  2008/06/03 13:37:09  bisnard
 * Multi-workflow sched now keeps nodes in the ready nodes queue
 * until a ressource is available to ensure comparison is done btw
 * nodes of different workflows (using sched-specific metric).
 *
 * Revision 1.9  2008/05/30 14:16:25  bisnard
 * obsolete MultiDag (not used anymore for multi-wf)
 *
 * Revision 1.8  2008/05/30 13:22:19  bisnard
 * added micro-delay between workflow node executions to avoid interf
 *
 * Revision 1.7  2008/05/16 12:30:20  bisnard
 * MaDag returns dagID to client after dag submission
 * (used for node execution)
 *
 * Revision 1.6  2008/04/30 07:37:01  bisnard
 * use relative timestamps for estimated and real completion time
 * make MultiWfScheduler abstract and add HEFT MultiWf scheduler
 *
 * Revision 1.5  2008/04/28 12:12:44  bisnard
 * new NodeQueue implementation for FOFT
 * manage thread join after node execution
 * compute slowdown for FOFT
 *
 * Revision 1.4  2008/04/21 14:31:45  bisnard
 * moved common multiwf routines from derived classes to MultiWfScheduler
 * use wf request identifer instead of dagid to reference client
 * use nodeQueue to manage multiwf scheduling
 * renamed WfParser as DagWfParser
 *
 * Revision 1.3  2008/04/14 13:44:29  bisnard
 * - Parameter 'used' obsoleted in MultiWfScheduler::submit_wf & submit_pb_set
 *
 * Revision 1.2  2008/04/14 09:10:37  bisnard
 *  - Workflow rescheduling (CltReoMan) no longer used with MaDag v2
 *  - AbstractWfSched and derived classes no longer used with MaDag v2
 *
 * Revision 1.1  2008/04/10 09:13:29  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 ****************************************************************************/

#ifndef _MULTIWFSCHEDULER_HH_
#define _MULTIWFSCHEDULER_HH_

#include <map>

#include "WfScheduler.hh"
#include "CltMan.hh"
#include "MasterAgent.hh"
#include "SeD.hh"
/* workflow utils */
#include "Thread.hh"
#include "NodeQueue.hh"
#include "DagScheduler.hh"
#include "MetaDag.hh"
#include "MaDag.hh"

class MaDag_impl;

namespace madag {
  class NodeRun;
  class DagState;

  class MultiWfScheduler : public Thread, public DagScheduler {

  friend class NodeRun;

  public:
    /**
     * Selector for node priority policy:
     * (depends on the metric used to compare nodes for execution priority)
     *   MULTIWF_NO_METRIC: when no metric is used (Basic scheduler)
     *   MULTIWF_NODE_METRIC: metric is for each node individually (eg b-level)
     *   MULTIWF_DAG_METRIC: metric is per dag (eg slowdown)
     */
    typedef enum { MULTIWF_NO_METRIC, MULTIWF_NODE_METRIC, MULTIWF_DAG_METRIC }
      nodePolicy_t;

    /**
     * Selector for platform type
     * (used to optimize the requests to the platform)
     *   PFM_ANY : (default) any platform
     *   PFM_SAME_SERVICES : platform containing hosts with same service list
     */
    typedef enum {PFM_ANY, PFM_SAME_SERVICES }
      pfmType_t;

    MultiWfScheduler(MaDag_impl * maDag, nodePolicy_t nodePol);

    virtual ~MultiWfScheduler();

    /**
     * Get the MaDag object ref
     */
    const MaDag_impl*
        getMaDag() const;

    /**
     * set the intra-dag scheduler used by the MA DAG (by default it is HEFT)
     * @param sched the base scheduler to be used
     */
    virtual void
        setSched(WfScheduler * sched);

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
        scheduleNewDag(Dag * newDag, MetaDag * metaDag = NULL)
        throw (MaDag::ServiceNotFound, MaDag::CommProblem);

    /**
     * Execution method
     */
    virtual void *
        run();

  protected:

    /**
     * Get the MetaDag of a given dag
     */
    MetaDag *
        getMetaDag(Dag * dag);

    /**
     * Call MA to get server estimations for all services
     */
    wf_response_t *
        getProblemEstimates(Dag * dag, MasterAgent_var MA)
        throw (MaDag::ServiceNotFound, MaDag::CommProblem);

    /**
     * Call MA to get server estimations for one node
     */
    wf_response_t *
        getProblemEstimates(DagNode * node, MasterAgent_var MA)
        throw (MaDag::ServiceNotFound, MaDag::CommProblem);

    /**
     * internal dag scheduling
     * to set priority of nodes using estimations given by MA and the
     * algorithm specified by setSched
     *
     * @param dag     pointer to dag object
     * @param MA      ref to the master agent
     */
    virtual void
        intraDagSchedule(Dag * dag, MasterAgent_var MA)
        throw (MaDag::ServiceNotFound, MaDag::CommProblem);

    /**
     * create a new node queue based on a dag
     * (by default uses a priority-based nodeQueue)
     *
     * @param dag   a dag
     * @return pointer to a nodequeue structure (to be destroyed by the caller)
     */
    virtual OrderedNodeQueue *
        createNodeQueue(Dag * dag);

    /**
     * delete the node queue created in createNodeQueue
     *
     * @param nodeQ   pointer to the node queue created in createdNodeQueue
     */
    virtual void
        deleteNodeQueue(OrderedNodeQueue * nodeQ);

    /**
     * insert a new node queue into the pool of queues managed by the sched
     * @param nodeQ   a node queue
     */
    virtual void
        insertNodeQueue(OrderedNodeQueue * nodeQ);

    /**
     * set node priority before inserting into execution queue
     * (called by run method)
     * @param node   the node to insert
     */
    virtual void
        setExecPriority(DagNode * node);

    /**
     * set node priority before inserting back in the ready queue
     */
    virtual void
        setWaitingPriority(DagNode * node);

    /**
     * Get the current time from scheduler reference clock
     * @return  current time in milliseconds (from scheduler start)
     */
    virtual double
        getRelCurrTime();

    /**
     * Execute a post operation on synchronisation semaphore
     * and joins the node thread given as parameter
     * @param nodeThread  pointer to the calling thread
     */
    virtual void
        wakeUp(NodeRun * nodeThread);

    /**
     * Execute a post operation on synchronisation semaphore
     */
    virtual void
        wakeUp();

    /**
     * Updates scheduler when a node has been executed
     */
    virtual void
        handlerNodeDone(DagNode * node) = 0;

    /**
     * Updates scheduler when a dag has been executed
     */
    virtual void
        handlerDagDone(Dag * dag);

    /**
     * Execute a node on a given SeD
     */
    Thread *
        runNode(DagNode * node,
                SeD_var sed,
                int reqID,
                corba_estimation_t& ev);

    /**
     * Execute a node without specifying a SeD
     */
    Thread *
        runNode(DagNode * node);

    /**
     * The Wf meta-scheduler scheduler
     */
    WfScheduler * mySched;

    /**
     * DagNode queues for waiting nodes
     * (key = ref of ready queue)
     */
    map<NodeQueue *,ChainedNodeQueue *> waitingQueues;

    /**
     * DagNode queues for ready nodes
     */
    list<OrderedNodeQueue *> readyQueues;

    /**
     * DagNode queue for nodes to be executed
     */
    OrderedNodeQueue * execQueue;

    /**
     * Store the nodes HEFT priority
     */
    map<DagNode*,double> nodesHEFTPrio;

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
     * Terminating node thread pointer (for join)
     */
    NodeRun * termNodeThread;

    /**
     * Terminating node thread flag (for join)
     */
    bool termNode;

    /**
     * Inter-round delay (used to separate DIET submits)
     *(in milliseconds)
     */
    int interRoundDelay;

  private:

    /**
      * MaDag reference
      */
    MaDag_impl * myMaDag;

    /**
     * Reference time
     */
    struct timeval refTime;

    /**
     * Map dag ref => metadag ref
     */
    map<string,MetaDag*>  myMetaDags;

  }; // end class MultiWfScheduler


/****************************************************************************/
/*                            CLASS NodeRun                                 */
/****************************************************************************/
  /**
   * This class is used by the MaDag agent to run a separate thread for each
   * node execution. This thread will invoke the execute() method on the
   * client workflow manager (agent located on the client) and wait until the
   * execution is finished. Then it triggers the execution of successor nodes
   * and informs the multi-wf scheduler that the node is completed.
   */

  class NodeRun: public Thread {
    public:
      /**
       * Constructor used when SeD is provided
       */
      NodeRun(DagNode * node,
              SeD_var sed,
              int reqID,
              corba_estimation_t ev,
              MultiWfScheduler * scheduler,
              CltMan_ptr cltMan);

      /**
       * Constructor used when no SeD is provided
       */
      NodeRun(DagNode * node,
              MultiWfScheduler * scheduler,
              CltMan_ptr cltMan);

      /**
       * Run method
       */
      void*
          run();

    private:
      DagNode *           myNode;
      SeD_var             mySeD;
      int                 myReqID;
      corba_estimation_t  myEstVect;
      MultiWfScheduler *  myScheduler;
      CltMan_ptr          myCltMan;
  }; // end class NodeRun

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

  }; // end class DagState

} // end namespace madag

#endif   /* not defined _ABSWFMETASCHED_HH */



