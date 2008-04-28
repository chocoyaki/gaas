/****************************************************************************/
/* The MA DAG scheduler interface                                           */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.3  2008/04/28 11:54:53  bisnard
 * new methods setNodePriorities & setNodesEFT replacing schedule
 * nodes sort done in separate method in Dag class
 *
 * Revision 1.2  2008/04/21 14:31:45  bisnard
 * moved common multiwf routines from derived classes to MultiWfScheduler
 * use wf request identifer instead of dagid to reference client
 * use nodeQueue to manage multiwf scheduling
 * renamed WfParser as DagWfParser
 *
 * Revision 1.1  2008/04/10 08:16:15  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 ****************************************************************************/

#ifndef WFSCHEDULER_HH
#define WFSCHEDULER_HH

#include <map>
#include <string>

#include "response.hh"
#include "workflow/DagWfParser.hh"

namespace madag {
  class WfScheduler {
  public:
    /**
     * WfScheduler constructor
     */
    WfScheduler();

    /**
     * WfScheduler destructor
     */
    virtual ~WfScheduler();

    /**
     * Order the nodes using scheduling algorithm
     */
    virtual void
    setNodesPriority(const wf_response_t * wf_response, Dag * dag) = 0;

    /**
     * Computes the schedule using HEFT algorithm and updates node
     */
    virtual void
    setNodesEFT(std::vector<Node *>& orderedNodes,
                const wf_response_t * wf_response,
                Dag * dag,
                double& initTime) = 0;
    /**
     * Old scheduling methods
     * @deprecated
     */

//     virtual wf_node_sched_seq_t
//     schedule(const wf_response_t * wf_response,
//              DagWfParser& parser,
//              CORBA::Long dag_id) = 0;

    virtual wf_node_sched_seq_t
    schedule(const wf_response_t * wf_response,
             Dag * dag) = 0;

//     virtual wf_node_sched_seq_t
//     reSchedule(const wf_response_t * wf_response,
//                DagWfParser& parser) = 0;
//
//     virtual wf_node_sched_seq_t
//     reSchedule(const wf_response_t * wf_response,
//                Dag * dag) = 0;
//
    virtual wf_node_sched_t
    schedule(const wf_response_t * response,
             Node * n) = 0;

    virtual double
    getAFT(string nodeId) = 0;

  };

}

#endif // _WFMETASCHED_HH_
