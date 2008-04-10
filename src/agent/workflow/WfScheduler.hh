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
 * Revision 1.1  2008/04/10 08:16:15  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 ****************************************************************************/

#ifndef WFSCHEDULER_HH
#define WFSCHEDULER_HH

#include <map>
#include <string>

#include "response.hh"
#include "workflow/WfParser.hh"

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

    virtual wf_node_sched_seq_t
    schedule(const wf_response_t * wf_response,
             WfParser& parser,
             CORBA::Long dag_id) = 0;

    virtual wf_node_sched_seq_t
    schedule(const wf_response_t * wf_response,
             Dag * dag) = 0;

    virtual wf_node_sched_seq_t 
    reSchedule(const wf_response_t * wf_response,
               WfParser& parser) = 0;

    virtual wf_node_sched_seq_t 
    reSchedule(const wf_response_t * wf_response,
               Dag * dag) = 0;

    virtual wf_node_sched_t
    schedule(const wf_response_t * response, 
             Node * n) = 0;

    virtual double
    getAFT(string nodeId) = 0;

  protected:

    // the availabilty of resources
    static map<std::string, double> avail;

  };

}

#endif // _WFMETASCHED_HH_
