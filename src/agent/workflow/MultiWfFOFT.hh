/****************************************************************************/
/* Another multi-workflow scheduler                                         */
/* This scheduler is based on Fairness On Finish Time algorithm             */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
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

#ifndef _MULTIWFFOFT_HH_
#define _MULTIWFFOFT_HH_

#include <map>
#include <vector>

#include "workflow/Dag.hh"
#include "MultiWfScheduler.hh"

using namespace std;


namespace madag {
  class DagState;
  class NodeState;

  class MultiWfFOFT : public MultiWfScheduler {
  public:
    MultiWfFOFT(MaDag_impl* maDag);
    virtual ~MultiWfFOFT();

    /**
     * Old Workflow submission function.
     * @deprecated
     *
     * @param wf_desc workflow string description
     * @param dag_id the dag ID
     */
    virtual bool
    submit_wf (const corba_wf_desc_t& wf_desc, int dag_id,
               MasterAgent_var parent,
               CltMan_var cltMan);

    /**
     * Updates scheduler when a node has been executed
     */
    virtual void
        handlerNodeDone(Node * node);

  protected:

    /**
     * internal dag scheduling
     */
    virtual void
        intraDagSchedule(Dag * dag, MasterAgent_var MA)
        throw (NodeException);

    /**
     * set node priority before inserting into execution queue
     * @param node   the node to insert
     */
    virtual void
        setExecPriority(Node * node);

    /**
     * Save the state of dags
     */
    map<Dag*, DagState> dagsState;

    /*************** DEPRECATED ATTRIBUTES & METHODS ******************/
    /*********** (used by submit_wf) **********************************/

    /**
     * Save the state of nodes
     */
    map<Node*, NodeState> nodesState;

    /**
     * The scheduling of each dag using a scheduling algorithm 'R'
     * @deprecated
     */
    map<Dag*, wf_node_sched_seq_t> sOwn;

    /**
     * The scheduling using the multiworkflow algorithm
     * @deprecated
     */
    map<Node*, wf_node_sched_t> sMulti;

    /**
     * Vector of unexecuted dags
     * must be sorted
     * @deprecated
     */
    vector<Dag *> U;

    /**
     * Init the scheduling
     * 1. schedule each dag
     * 2. mark each dag as unexecuted, initialize the slowdown
     */
    virtual
    void init();

    /**
     * The main scheduling function
     */
    void
    fairnessOnFinishTime(const wf_response_t * wf_response);

    Node *
    get1stReadyNode(vector<Node*>& v);


    void
    sortU();
  };

  class DagState {
  public:
    DagState();

    /**
     * true if the dag is marked as executed otherwise false
     *
     */
    bool executed;

    /**
     * the earliest finish time of the DAG scheduled alone
     */
    double EFT;

    /**
     * the makespan of the DAG scheduled alone
     */
    double makespan;

    /**
     * the estimated global delay for the DAG
     */
    double estimatedDelay;

    /**
     * Dag slowdown (percentage of delay / makespan)
     */
    double slowdown;

    /**
     * The number of executed (or scheduled) nodes
     */
    unsigned int executedNodes;
  };

  // OBSOLETE
  class NodeState {
  public:
    NodeState();

    /**
     * true if the dag is marked as executed otherwise false
     */
    bool executed;

    /**
     * Node finish time based on the multi-workflow algorithm
     */
    double multiFT;

    /**
     * Node finish time when the dag is scheduled alone
     */
    double ownFT;
  };

}

#endif   /* not defined _MULTIWFFAIRNESS_SCHED_HH */



