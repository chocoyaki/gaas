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
     * Order the nodes using b-level algorithm
     */
    virtual std::vector<Node *>
    prioritizeNodes(const wf_response_t * wf_response, Dag * dag);

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
    map<string, double> AST;

  };

}

#endif   /* not defined _HEFTSCHEDULER_HH */



