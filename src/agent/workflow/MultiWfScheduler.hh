/****************************************************************************/
/* The base abstract class for multi-workflow scheduler                     */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
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
#include "MultiDag.hh"
#include "CltMan.hh"
#include "MasterAgent.hh"
#include "workflow/Thread.hh"
#include "workflow/NodeQueue.hh"
#include "SeD.hh"

class MaDag_impl;

namespace madag {

  class MultiWfScheduler : public Thread {
  public:
    MultiWfScheduler(MaDag_impl * maDag);

    virtual ~MultiWfScheduler();

    /**
     * set the intra-dag scheduler used by the MA DAG (by default it is HEFT)
     * @param sched the base scheduler to be used
     */
    virtual void
        setSched(WfScheduler * sched);

    /**
     * schedules a new DAG workflow
     * @param wf_desc   workflow string description
     * @param wfReqId   workflow request identifier
     * @param MA        master agent (CORBA)
     */
    virtual bool
        scheduleNewDag(const corba_wf_desc_t& wf_desc, int wfReqId,
                       MasterAgent_var MA);

    /**
     * Workflow submission function
     * @deprecated
     *
     * @param wf_desc workflow string description
     * @param dag_id the dag ID
     */
//     virtual bool
//         submit_wf (const corba_wf_desc_t& wf_desc, int dag_id,
//                MasterAgent_var parent,
//                CltMan_var cltMan) = 0;

    /**
     * Execution method
     */
    virtual void *
        run();

    /**
     * Execute a post operation on synchronisation semaphore
     */
    virtual void
        wakeUp();

    /**
     * Get the MaDag object ref
     */
    MaDag_impl*
        getMaDag();

  protected:

    /**
     * The Meta-Dag
     */
    MultiDag * myMetaDag;


    /**
     * The Wf meta-scheduler scheduler
     */
    WfScheduler * mySched;

    /**
     * Nodes state
     * true if node is done
     * false otherwise
     */
    map<Node *, bool> nodesStates;

    // the availabilty of resources
    static map<string, double> avail;


    /**
     * Critical section of the scheduler
     */
    omni_mutex myLock;

    /**
     * Synchronisation semaphore
     */
    omni_semaphore mySem;

    /**
     * Execute a node
     */
    Thread *
        runNode(Node * node, SeD_var sed);

    /**
     * parse a new dag provided in xml text and create a dag object
     *
     * @param wf_desc   workflow string description
     * @return pointer to dag structure (to be destroyed by the caller)
     */
    Dag *
        parseNewDag(const corba_wf_desc_t& wf_desc)
        throw (XMLParsingException);

    /**
     * internal dag scheduling
     *
     * @param dag     pointer to dag object
     * @param MA      ref to the master agent
     * @return an ordered vector of nodes
     */
    virtual std::vector<Node *>
        intraDagSchedule(Dag * dag, MasterAgent_var MA)
        throw (NodeException);

    /**
     * create a new node queue based on a dag
     *
     * @param nodes   a vector of nodes
     * @param wfReqId workflow request identifier
     * @return pointer to a nodequeue structure (to be destroyed by the caller)
     */
    virtual NodeQueue *
        createNodeQueue(std::vector<Node *> nodes, const int wfReqId);

    /**
     * insert a new node queue into the pool of queues managed by the sched
     * @param nodeQ   a node queue
     */
    virtual void
        insertNodeQueue(NodeQueue * nodeQ);

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
     * Node queues
     */
    list<NodeQueue *> myQueues;

  }; // end class MultiWfScheduler

} // end namespace madag

#endif   /* not defined _ABSWFMETASCHED_HH */



