/****************************************************************************/
/* The MA DAG HEFT scheduler                                                */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.5  2008/05/05 13:54:17  bisnard
 * new computation time estimation get/set functions
 *
 * Revision 1.4  2008/04/30 07:32:24  bisnard
 * use relative timestamps for estimated and real completion time
 *
 * Revision 1.3  2008/04/28 11:54:52  bisnard
 * new methods setNodePriorities & setNodesEFT replacing schedule
 * nodes sort done in separate method in Dag class
 *
 * Revision 1.2  2008/04/21 14:31:45  bisnard
 * moved common multiwf routines from derived classes to MultiWfScheduler
 * use wf request identifer instead of dagid to reference client
 * use nodeQueue to manage multiwf scheduling
 * renamed WfParser as DagWfParser
 *
 * Revision 1.1  2008/04/10 09:17:10  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 ****************************************************************************/

#ifndef _HEFTSCHEDULER_HH_
#define _HEFTSCHEDULER_HH_

#include "WfScheduler.hh"

namespace madag {
  class HEFTScheduler : public WfScheduler {
  public:
    HEFTScheduler();

    virtual ~HEFTScheduler();

    /**
     * Set the priority of nodes using b-level algorithm
     */
    virtual void
    setNodesPriority(const wf_response_t * wf_response, Dag * dag);

    /**
     * Set the EFT of nodes using HEFT algorithm
     *
     * @param orderedNodes  vector of nodes ordered by decreasing priority
     * @param wf_response   the estimates given by the MA
     * @param dag           the dag
     * @param initTime      dag starting time (set by method)
     * @param refTime       reference time (all timestamps are relative to this ref)
     */
    virtual void
    setNodesEFT(std::vector<Node *>& orderedNodes,
                const wf_response_t * wf_response,
                Dag * dag,
                double& initTime,
                double refTime);

    /**
     * Old scheduling methods
     * @deprecated
     */
//     virtual wf_node_sched_seq_t
//     schedule(const wf_response_t * wf_response,
//              DagWfParser& reader,
//              CORBA::Long dag_id);

    virtual wf_node_sched_seq_t
    schedule(const wf_response_t * wf_response,
             Dag * dag);

//     virtual wf_node_sched_seq_t
//     reSchedule(const wf_response_t * wf_response,
//                DagWfParser& reader);

//     virtual wf_node_sched_seq_t
//     reSchedule(const wf_response_t * wf_response,
//                Dag* dag);

    virtual wf_node_sched_t
    schedule(const wf_response_t * response,
             Node * n); // used by FOFT

    virtual double
    getAFT(string nodeId);

  private:

    /**
     * Get the value of estimated computation time from wf response
     * for a given service and a given server
     * @param wf_response the wf response structure
     * @param pbIndex the index of the problem (service)
     * @param srvIndex the index of the server
     */
    double
    getCompTimeEst(const wf_response_t * wf_response,
                   unsigned int pbIndex,
                   unsigned int srvIndex);

    /**
     * Computes the average value of node workload across the Seds
     */
    void
    computeNodeWeights(const wf_response_t * wf_response, Dag * dag);

    /**
     * rank the node upward
     * @param n the top node to rank.
     */
    void
    rank(Node * n);

    // the average wi
    map<string, double> WI;
    // AFT and AST map
    map<string, double> AFT;
    map<string, double> AST;  // obsolete

    // the availabilty of resources (obsolete)
    static map<std::string, double> avail;
  };

}

#endif   /* not defined _HEFTSCHEDULER_HH */



