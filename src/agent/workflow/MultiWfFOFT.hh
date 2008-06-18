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
 * Revision 1.8  2008/06/18 15:04:23  bisnard
 * initialize dag scheduling time in multi-wf scheduler
 * update slowdown when node is waiting in the ready nodes queue
 *
 * Revision 1.7  2008/05/30 14:39:57  bisnard
 * cleanup
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

#ifndef _MULTIWFFOFT_HH_
#define _MULTIWFFOFT_HH_

#include <map>
#include <vector>

#include "workflow/Dag.hh"
#include "MultiWfScheduler.hh"

using namespace std;


namespace madag {
  class DagState;

  class MultiWfFOFT : public MultiWfScheduler {
  public:
    MultiWfFOFT(MaDag_impl* maDag);
    virtual ~MultiWfFOFT();

  protected:

    /**
     * Updates scheduler when a node has been executed
     */
    virtual void
        handlerNodeDone(Node * node);

    /**
     * Updates scheduler when a node cannot be executed and is waiting
     * in the ready nodes queue
     */
    virtual void
        handlerNodeWaiting(Node * node);

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
     * updates the delay for a given node and change the current dag
     * slowdown value accordingly if the dag delay is impacted
     * @param node
     * @param delay delay in ms
     */
    virtual void
        updateNodeDelay(Node * node, double delay);

  private:

    /**
     * Save the state of dags
     */
    map<Dag*, DagState> dagsState;

    /**
     * Mark the nodes which priority must be re-calculated
     */
    map<Node*,bool> nodesFlag;

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

  };

}

#endif   /* not defined _MULTIWFFAIRNESS_SCHED_HH */



