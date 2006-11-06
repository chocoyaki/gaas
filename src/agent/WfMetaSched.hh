/****************************************************************************/
/* The Workflow Meta-scheduler                                              */
/* This scheduler is used when multi-workflow support is enabled            */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id :
 * $Log :
 ****************************************************************************/

#ifndef _WFMETASCHED_HH_
#define _WFMETASCHED_HH_

#include "MaDag_sched.hh"
#include "MetaDag.hh"
#include "CltReoMan.hh"
#include "AbsWfMetaSched.hh"

class WfMetaSched : public AbsWfMetaSched {
public:
  WfMetaSched();

  virtual ~WfMetaSched();

  /**
   * set the scheduler used by the MA DAG
   * @param sched the base scheduler to be used
   */
  virtual void 
  setSched(MaDag_sched * sched);

  /**
   * Workflow submission function. 
   * @param wf_desc workflow string description
   * @param dag_id the dag ID
   */
  virtual wf_sched_response_t * 
  submit_wf (const corba_wf_desc_t& wf_desc, int dag_id,
	     MasterAgent_var parent,
	     const bool used);

private:
};


#endif   /* not defined _WFMETASCHED_HH */



