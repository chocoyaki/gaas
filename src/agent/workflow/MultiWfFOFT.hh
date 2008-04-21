/****************************************************************************/
/* Another multi-workflow scheduler                                         */
/* This scheduler is based on Fairness On Finish Time algorithm             */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
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
     * Workflow submission function.
     * @param wf_desc workflow string description
     * @param dag_id the dag ID
     */
    virtual bool
    submit_wf (const corba_wf_desc_t& wf_desc, int dag_id,
               MasterAgent_var parent,
               CltMan_var cltMan);

  protected:
    /**
     * The scheduling of each dag using a scheduling algorithm 'R'
     */
    map<Dag*, wf_node_sched_seq_t> sOwn;

    /**
     * The scheduling using the multiworkflow algorithm
     */
    map<Node*, wf_node_sched_t> sMulti;

    /**
     * Save the state of dags
     * See the class DagState
     */
    map<Dag*, DagState> dagsState;

    /**
     * Save the state of nodes
     * See the class NodeState
     */
    map<Node*, NodeState> nodesState;

    /**
     * Vector of unexecuted dags
     * must be sorted
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
     * the makespan of the DAG scheduled alone
     */
    double makespan;

    /**
     * Dag slowdown
     */
    double slowdown;

    /**
     * The number of executed (or scheduled) nodes
     */
    unsigned int executedNodes;
  };

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



