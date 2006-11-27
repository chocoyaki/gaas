/****************************************************************************/
/* The MA DAG CORBA object implementation header                            */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.5  2006/11/27 09:53:00  aamar
 * Correct headers of source files used in workflow support.
 *
 * Revision 1.4  2006/11/06 15:14:53  aamar
 * Workflow support: Correct some code about reqID
 *
 * Revision 1.3  2006/10/20 09:13:55  aamar
 * Changing the submit_wf prototype (the return type)
 * Add the following function to the MA DAG interface
 *   - void setAsDone, setDagAsDone, registerClt
 * Some changes.
 *
 * Revision 1.2  2006/07/10 10:00:00  aamar
 * - Adding the function remainingDag to the MA DAG interface
 * - Round Robbin and HEFT scheduling
 *
 * Revision 1.1  2006/04/14 13:43:33  aamar
 * header of the MA DAG CORBA object.
 *
 ****************************************************************************/

#ifndef _MADAG_IMPL_HH_
#define _MADAG_IMPL_HH_

#include <string>

#include "MaDag.hh"
#include "MasterAgent.hh"
#include "MaDag_sched.hh"
#include "MetaDag.hh"
#include "WfMetaSched.hh"

typedef enum {
  round_robbin_sched,
  heft_sched,
} madag_sched_t;

typedef enum {
  MWF_DEFAULT,
  MWF_FAIRNESS
} madag_mwf_sched_t;

class MaDag_impl : public POA_MaDag,
		   public PortableServer::RefCountServantBase {
public:
  MaDag_impl(const char * name);

  virtual ~MaDag_impl();

  /** Workflow submission function. */
  virtual wf_sched_response_t * 
  submit_wf (const corba_wf_desc_t& wf_desc, const bool used);

  /**
   * Another workflow submission function *
   * return only the ordering of the nodes execution *
   */
  virtual wf_ordering_t *
  get_wf_ordering(const corba_wf_desc_t& wf_desc);

  /**
   * set the remaining dag
   */
  virtual void
  remainingDag(const char * dag_descr);

  /*
   * inform the MA DAG that node execution is done
   */
  virtual void 
  setAsDone(const char* dag_id, const char* nodeId);

  /**
   * inform the ma dag that the dag execution is complete
   */
  virtual void 
  setDagAsDone(const char * dag_id);

  /*
   * register a dag to the ma dag
   */
  virtual void
  registerClt(const char* dag_id, const char* client_ref);

  /**
   * set the scheduler for the MA DAG
   */
  void
  set_sched(const madag_sched_t madag_sched);

  /**
   * enable the multi-workflow support
   * @param enable if true multi-workflow is enabled
   * @param madag_mwf_sched the multi-workflow scheduler to be used
   */
  void
  enable_multi_wf(bool enable,
		  const madag_mwf_sched_t madag_mwf_sched = MWF_DEFAULT);

private:
  /**
   * The Ma Dag name (used for CORBA naming service binding)
   */ 
  std::string myName;
  
  /**
   * The master agent reference
   */
  MasterAgent_var parent;
  
  /**
   * The Ma Dag scheduler
   */
  MaDag_sched * mySched; 
 
  /**
   * multi-workflow support 
   */
  bool multi_wf;

  /**
   * The meta-scheduler (used for multiworkflow support)
   */
  AbsWfMetaSched * metaSched; 

  /**
   * Lock to prevent concurrent access
   */
  omni_mutex myMutex ;
};


#endif   /* not defined _MADAG_IMPL_HH */



