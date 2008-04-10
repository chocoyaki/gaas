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

    virtual wf_node_sched_seq_t 
    schedule(const wf_response_t * wf_response,
             WfParser& reader,
             CORBA::Long dag_id);

    virtual wf_node_sched_seq_t 
    schedule(const wf_response_t * wf_response,
             Dag * dag);

    virtual wf_node_sched_seq_t 
    reSchedule(const wf_response_t * wf_response,
               WfParser& reader);

    virtual wf_node_sched_seq_t 
    reSchedule(const wf_response_t * wf_response,
               Dag* dag);

    virtual wf_node_sched_t
    schedule(const wf_response_t * response, 
             Node * n);

    virtual double
    getAFT(string nodeId);
  private:
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



