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
 * Revision 1.13  2009/02/06 14:50:35  bisnard
 * setup exceptions
 *
 * Revision 1.12  2009/01/16 13:41:22  bisnard
 * added common base class DagScheduler to simplify dag events handling
 * improved exception management
 *
 * Revision 1.11  2008/10/14 13:24:49  bisnard
 * use new class structure for dags (DagNode,DagNodePort)
 *
 * Revision 1.10  2008/06/25 10:05:44  bisnard
 * - Waiting priority set when node is put back in waiting queue
 * - Node index in wf_response stored in Node class (new attribute submitIndex)
 * - HEFT scheduler uses SeD ref instead of hostname
 * - Estimation vector and ReqID passed to client when SeD chosen by MaDag
 * - New params in execNodeOnSeD to provide ReqId and estimation vector
 * to client for solve request
 *
 * Revision 1.9  2008/06/19 10:18:54  bisnard
 * new heuristic AgingHEFT for multi-workflow scheduling
 *
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

#include "MultiWfScheduler.hh"

using namespace std;


namespace madag {

  class MultiWfFOFT : public MultiWfScheduler {
  public:
    MultiWfFOFT(MaDag_impl* maDag);
    virtual ~MultiWfFOFT();

  protected:

    /**
     * Updates scheduler when a node has been executed
     */
    virtual void
    handlerNodeDone(DagNode * node);

    /**
     * internal dag scheduling
     */
    virtual void
    intraDagSchedule(Dag * dag, MasterAgent_var MA)
      throw (MaDag::ServiceNotFound, MaDag::CommProblem);

    /**
     * set node priority before inserting into execution queue
     */
    virtual void
    setExecPriority(DagNode * node);

    /**
     * set node priority before inserting back in the ready queue
     */
    virtual void
    setWaitingPriority(DagNode * node);

    /**
     * updates the delay for a given node and change the current dag
     * slowdown value accordingly if the dag delay is impacted
     * @param node
     * @param delay delay in ms
     */
    virtual void
    updateNodeDelay(DagNode * node, double delay);

  private:

    /**
     * Save the state of dags
     */
    map<Dag*, DagState> dagsState;

    /**
     * Mark the nodes which priority must be re-calculated
     */
    map<DagNode*,bool> nodesFlag;

  };

}

#endif   /* not defined _MULTIWFFAIRNESS_SCHED_HH */



