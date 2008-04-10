/****************************************************************************/
/* The MA DAG round robbin scheduler                                        */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2008/04/10 09:13:29  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 ****************************************************************************/

#ifndef _RRSCHEDULER_HH_
#define _RRSCHEDULER_HH_

#include "WfScheduler.hh"

namespace madag {
  class RRScheduler : public WfScheduler {
  public:
    RRScheduler();

    virtual ~RRScheduler();

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
               Dag * dag);

    /**
     * TO COMPLETE
     */
    virtual wf_node_sched_t
    schedule(const wf_response_t * response, 
             Node * n);

    virtual double
    getAFT(string nodeId);

  private:
  };

}

#endif   /* not defined _RRSCHEDULER_HH */



