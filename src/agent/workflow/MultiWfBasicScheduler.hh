/****************************************************************************/
/* The Workflow Meta-scheduler                                              */
/* This scheduler is used when multi-workflow support is enabled            */
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

#ifndef _MULTIWFBASICSCHEDULER_HH_
#define _MULTIWFBASICSCHEDULER_HH_

#include "WfScheduler.hh"
#include "MultiDag.hh"
#include "CltReoMan.hh"
#include "MultiWfScheduler.hh"

namespace madag {
  class MultiWfBasicScheduler : public MultiWfScheduler {
  public:
    MultiWfBasicScheduler();

    virtual ~MultiWfBasicScheduler();

    /**
     * set the scheduler used by the MA DAG
     * @param sched the base scheduler to be used
     */
    virtual void 
    setSched(WfScheduler * sched);

    /**
     * Workflow submission function. 
     * @param wf_desc workflow string description
     * @param dag_id the dag ID
     */
    virtual bool 
    submit_wf (const corba_wf_desc_t& wf_desc, int dag_id,
               MasterAgent_var parent,
               const bool used,
               CltMan_var cltMan);

    /**
     * Execution method
     */
    virtual
    void* run();

    /**
     * Execute a post operation on synchronisation semaphore
     */
    virtual void
    wakeUp();

  protected:
    /**
     * Synchronisation semaphore
     */
    omni_semaphore mySem;



  private:
  };

}

#endif   /* not defined _MULTIWFBASICSCHEDULER_HH */



