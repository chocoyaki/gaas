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
 * Revision 1.11  2008/06/03 12:19:36  bisnard
 * Method to get MA ref
 *
 * Revision 1.10  2008/06/01 09:22:14  rbolze
 * add getter to retrieve the dietLogComponent
 *
 * Revision 1.9  2008/05/31 08:45:55  rbolze
 * add DietLogComponent to the maDagAgent
 *
 * Revision 1.8  2008/05/30 14:16:25  bisnard
 * obsolete MultiDag (not used anymore for multi-wf)
 *
 * Revision 1.7  2008/05/16 12:30:20  bisnard
 * MaDag returns dagID to client after dag submission
 * (used for node execution)
 *
 * Revision 1.6  2008/04/30 07:37:01  bisnard
 * use relative timestamps for estimated and real completion time
 * make MultiWfScheduler abstract and add HEFT MultiWf scheduler
 *
 * Revision 1.5  2008/04/28 11:56:51  bisnard
 * choose wf scheduler type when creating madag
 *
 * Revision 1.4  2008/04/21 14:31:45  bisnard
 * moved common multiwf routines from derived classes to MultiWfScheduler
 * use wf request identifer instead of dagid to reference client
 * use nodeQueue to manage multiwf scheduling
 * renamed WfParser as DagWfParser
 *
 * Revision 1.3  2008/04/14 13:45:10  bisnard
 * - Removed wf mono-mode submit
 * - Renamed submit_wf in processDagWf
 *
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
#include "MultiWfScheduler.hh"
#include "CltMan.hh"
#include "MaDag.hh"
#include "DietLogComponent.hh"

class MaDag_impl : public POA_MaDag,
		   public PortableServer::RefCountServantBase {
public:
  enum MaDagSchedType { BASIC, HEFT, FOFT };

  MaDag_impl(const char * name, const MaDagSchedType schedType);

  virtual ~MaDag_impl();

  /**
   * DAG Workflow processing
   * Manages the scheduling and orchestration of the submitted DAG
   * Delegates the execution of each individual task to the client
   * workflow mgr
   *
   * @param  dag_desc   workflow textual description
   * @param  cltMgrRef  client workflow manager reference
   * @param  wfReqId    submitted workflow identifier (obtained by
   * getWfReqId function)
   * @return the dag Id if workflow submission succeed, -1 if not
   */

  virtual CORBA::Long
      processDagWf(const corba_wf_desc_t& dag_desc, const char* cltMgrRef,
                   CORBA::Long wfReqId);
  /**
   * Multi DAG Workflow processing
   * Manages the scheduling and orchestration of a set of N identical DAGs
   * Delegates the execution of each individual task to the client workflow mgr
   *
   * @param  dag_desc     workflow textual description
   * @param  cltMgrRef    client workflow manager reference
   * @param  wfReqId      submitted workflow identifier (obtained by
   * getWfReqId function)
   * @return true if workflow submission succeed, otherwise false
   */

  virtual CORBA::Boolean
      processMultiDagWf(const corba_wf_desc_t& dag_desc, const char* cltMgrRef,
                        CORBA::Long wfReqId);

  /**
   * Get a new workflow request identifier
   * @return a new identifier to be used for a wf request
   */
  virtual CORBA::Long
      getWfReqId();

  /**
   * Get the client mgr for a given wfReqId
   * @param wfReqId workflow request identifier
   * @return client manager pointer (CORBA)
   */
  virtual CltMan_ptr
      getCltMan(int wfReqId);

  /** Used to test if it is alive. */
  virtual CORBA::Long
      ping();
   /**
   * Ptr to the DietLogComponent. This ptr can be NULL, so it has to
   * be checked every time it is used. If it is NULL, no monitoring
   * messages have to be sent.
   */
  DietLogComponent* dietLogComponent;

  /**
   *  Get the DietLogComponent
   */
  DietLogComponent*
      getDietLogComponent();

  /**
   * Get the MA
   */
  MasterAgent_var
      getMA() const;

protected:
  /**
   * set the client manager for a wf request
   */
  void
  setCltMan(int wfReqId, CltMan_ptr cltMan);

/**
   * setup the DietLogComponent
   */
  void
  setupDietLogComponent();

private:
  /**
   * The Ma Dag name (used for CORBA naming service binding)
   */
  std::string myName;

  /**
   * The master agent reference
   */
  MasterAgent_var myMA;

  /**
   * The client wf manager references
   */
  map<int, CltMan_ptr> cltMans;

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
  CORBA::Long wfReqIdCounter;

};


#endif   /* not defined _MADAG_IMPL_HH */



