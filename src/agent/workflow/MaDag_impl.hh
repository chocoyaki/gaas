/****************************************************************************/
/* The MA DAG CORBA object implementation header                            */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/

/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2008/04/14 09:14:29  bisnard
 *  - Workflow rescheduling (CltReoMan) no longer used with MaDag v2
 *  - AbstractWfSched and derived classes no longer used with MaDag v2
 *
 * Revision 1.1  2008/04/10 09:13:29  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 * Revision 1.6  2008/03/21 10:22:04  rbolze
 *  - add ping() function to the MaDag in order to be able test this component.
 * this is use by goDIET.
 *  - print IOR in stdout in the constructor of the MaDag.
 *
 * Revision 1.5b ??? aamar (added by bisnard)
 * Added functions submit_dag and submit_dag_in_multi
 *
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
#include "WfScheduler.hh"
#include "MultiDag.hh"
#include "MultiWfScheduler.hh"
#include "CltMan.hh"

typedef enum {
  madag_rr,
  madag_heft,
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

  /**
   * Workflow submission function
   *
   * @param dag_desc     workflow textual description
   * @param firstReqId   first request identifier 
   * @return             a dag scheduling
   */
  virtual dag_response_t* 
  submit_dag(const corba_wf_desc_t& dag_desc, CORBA::Long& firstReqId);
  
  /**
   * Workflow submission in multi-workflow mode
   *
   * @param  dag_desc   workflow textual description
   * @param  cltMgrRef  client workflow manager reference
   * @param  dag_id     submitted workflow identifier (obtained by getDagId function)
   * @return true if workflow submission succeed, otherwise false
   */
  
  virtual CORBA::Boolean
  submit_dag_in_multi(const corba_wf_desc_t& dag_desc, const char* cltMgrRef, CORBA::Long dag_id);

  /** 
   * Workflow submission function.
   * 
   * @param wf_desc submited workflow description
   * @param used  
   * @param multi_mode set if workflow is submited in multiworkflow mode
   * @return the scheduling response or execute the workflow if multiworkflow
   *         mode is selected
   */
  virtual wf_sched_response_t * 
  submit_wf (const corba_wf_desc_t& wf_desc, CORBA::Boolean used,
             CORBA::Boolean multi_mode,
             const char * cltMgrRef,
             CORBA::Long dag_id);

  /**
   * Get a new DAG identifier
   */ 
  virtual CORBA::Long
  getDagId();

  /** Used to test if it is alive. */
  virtual CORBA::Long
  ping();

  /**
   * Set the scheduler for the MA DAG
   *
   * @param madag_sched scheduler identifier
   */
  void
  setScheduler(const madag_sched_t madag_sched);

  /**
   * set the scheduler for multi workflow mode
   */
  void
  setMultiWfScheduler(const madag_mwf_sched_t madag_multi_sched);


protected:
  /**
   * Schedule workflow in a normal mode
   * @deprecated
   *
   * @param wf_desc submited workflow description
   */
  virtual wf_sched_response_t *
  monoMode_wf_sub(const corba_wf_desc_t& wf_desc,
                  const bool used,
                  const long dag_id);

  /**
   * Schedule workflow in multi-workflow mode
   *
   * @param wf_desc submited workflow description
   */
  bool
  multiMode_wf_sub(const corba_wf_desc_t& wf_desc,
                   const bool used,
                   CltMan_ptr cltMan,
                   const long dag_id);

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
  madag::WfScheduler * mySched; 
 
  /**
   * The meta-scheduler (used for multiworkflow support)
   */
  madag::MultiWfScheduler * myMultiWfSched; 

  /**
   * Lock to prevent concurrent access
   */
  omni_mutex myMutex ;

  /**
   * Dag identifier counter
   */
  CORBA::Long dagId;
};


#endif   /* not defined _MADAG_IMPL_HH */



