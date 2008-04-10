/****************************************************************************/
/* The base abstract class for multi-workflow scheduler                     */
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

#ifndef _MULTIWFSCHEDULER_HH_
#define _MULTIWFSCHEDULER_HH_

#include <map>

#include "WfScheduler.hh"
#include "MultiDag.hh"
#include "CltReoMan.hh"
#include "CltMan.hh"
#include "workflow/Thread.hh"
#include "SeD.hh"

namespace madag {

  class MultiWfScheduler : public Thread {
  public:
    MultiWfScheduler();
  
    virtual ~MultiWfScheduler();

    /**
     * set the scheduler used by the MA DAG
     * @param sched the base scheduler to be used
     */
    virtual void 
    setSched(WfScheduler * sched) = 0;

    /**
     * Workflow submission function. 
     * @param wf_desc workflow string description
     * @param dag_id the dag ID
     */
    virtual bool 
    submit_wf (const corba_wf_desc_t& wf_desc, int dag_id,
               MasterAgent_var parent,
               const bool used,
               CltMan_var cltMan) = 0;

    /**
     * Add a new client reference
     * @param clt the new client.
     * @param dag the dag identifier.
     */
    virtual void
    addClient(CltReoMan_var clt, const string dag);

    /**
     * Remove a client from myClients map
     * @param dag id
     */
    virtual void
    removeClient(const string dag);

    /**
     * Set the node state as done
     * @param dagId the DAG identifier
     * @param nodeId the node identifier in the DAG
     */
    virtual void 
    setNodeAsDone(const char* dagId, const char* nodeId);

    /**
     * Execution method
     */
    virtual
    void* run();

    /**
     * Execute a node
     */
    Thread * 
    runNode(Node * node, SeD_var sed);

    /**
     * Execute a post operation on synchronisation semaphore
     */
    virtual void
    wakeUp() = 0;

    /**
     * Get the corresponding client manager
     */
    CltMan_ptr
    getCltMan(string dag_id);

  protected:

    /**
     * A map of dag with corresponding client ref
     */
    std::map<string, CltReoMan_var> myClients;

    /**
     * The Meta-Dag
     */
    MultiDag * myMetaDag;
  

    /**
     * The Wf meta-scheduler scheduler
     */
    WfScheduler * mySched; 

    /**
     * Nodes state 
     * true if node is done
     * false otherwise
     */
    map<Node *, bool> nodesStates;

    /**
     * extract only the response for the specified dag
     * @param dag_id the dag identifier
     * @param wf_resp the global scheduling for all the meta-dag
     */
    static wf_sched_response_t * 
    extract(int dag_id, wf_sched_response_t * wf_resp);

    // the availabilty of resources
    static map<string, double> avail;

    // The Client manager references
    map<string, CltMan_ptr> cltMans;

    /**
     * Critical section of the scheduler
     */
    omni_mutex myLock;

  };

}

#endif   /* not defined _ABSWFMETASCHED_HH */



