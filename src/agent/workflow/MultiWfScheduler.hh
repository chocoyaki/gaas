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
#include "workflow/Thread.hh"
#include "workflow/NodeQueue.hh"
#include "SeD.hh"

class MaDag_impl;

namespace madag {
  class NodeRun;

  class MultiWfScheduler : public Thread {
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
     * schedules a new DAG workflow
     *
     * @param wf_desc   workflow string description
     * @param wfReqId   workflow request identifier
     * @param MA        master agent (CORBA)
     * @return dag identifier (double)
     */
    virtual double
        scheduleNewDag(const corba_wf_desc_t& wf_desc, int wfReqId,
                       MasterAgent_var MA)
        throw (XMLParsingException, NodeException);

    /**
     * Execution method
     */
    virtual void *
        run();

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
        handlerNodeDone(Node * node) = 0;

    /**
     * Get the current time from scheduler reference clock
     * @return  current time in sec (from scheduler start)
     */
    double
        getRelCurrTime();

  protected:

    /**
     * The Wf meta-scheduler scheduler
     */
    WfScheduler * mySched;

    /**
     * Node queues for waiting nodes
     * (key = ref of ready queue)
     */
    map<NodeQueue *,ChainedNodeQueue *> waitingQueues;

    /**
     * Node queues for ready nodes
     */
    list<OrderedNodeQueue *> readyQueues;

    /**
     * Node queue for nodes to be executed
     */
    OrderedNodeQueue * execQueue;

    /**
     * Selector for node priority policy
     */
    nodePolicy_t nodePolicy;

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
    bool      termNode;

    /**
     * Execute a node
     */
    Thread *
        runNode(Node * node, SeD_var sed);

    /**
     * Parse a new dag provided in xml text and create a dag object
     *
     * @param wfReqId   workflow request ID
     * @param wf_desc   workflow string description
     * @return pointer to dag structure (to be destroyed by the caller)
     */
    Dag *
        parseNewDag(int wfReqId, const corba_wf_desc_t& wf_desc)
        throw (XMLParsingException);

    /**
     * Call MA to get server estimations for all services
     */
    wf_response_t *
        getProblemEstimates(Dag * dag, MasterAgent_var MA)
        throw (NodeException);

    /**
     * Call MA to get server estimations for all services for nodes of a queue
     */
    wf_response_t *
        getProblemEstimates(OrderedNodeQueue * queue, MasterAgent_var MA)
        throw (NodeException);

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
        throw (NodeException);

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
        setExecPriority(Node * node);

    /**
     * get the reference time (when scheduler started)
     * used to compute diff with this time
     */
    double
        getRefTime();

    /**
     * Inter-node delay (used to separate DIET submits)
     *(in milliseconds)
     */
    static long interNodeDelay;

  private:

    /**
      * MaDag reference
      */
    MaDag_impl * myMaDag;

    /**
     * Dag counter
     */
    static long dagIdCounter;

    /**
     * Reference time
     */
    double refTime;

  }; // end class MultiWfScheduler


/****************************************************************************/
/*                            CLASS NodeRun                                 */
/****************************************************************************/

  class NodeRun: public Thread {
    public:
      NodeRun(Node * node, SeD_var sed, MultiWfScheduler * scheduler, CltMan_ptr cltMan);

      void*
          run();

    private:
      Node *  myNode;
      SeD_var mySeD;
      MultiWfScheduler * myScheduler;
      CltMan_ptr myCltMan;
  }; // end class NodeRun

} // end namespace madag

#endif   /* not defined _ABSWFMETASCHED_HH */


