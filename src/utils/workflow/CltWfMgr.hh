/****************************************************************************/
/* Client workflow manager class                                            */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.4  2008/04/21 14:31:45  bisnard
 * moved common multiwf routines from derived classes to MultiWfScheduler
 * use wf request identifer instead of dagid to reference client
 * use nodeQueue to manage multiwf scheduling
 * renamed WfParser as DagWfParser
 *
 * Revision 1.3  2008/04/15 14:20:20  bisnard
 * - Postpone sed mapping until wf node is executed
 *
 * Revision 1.2  2008/04/14 09:10:40  bisnard
 *  - Workflow rescheduling (CltReoMan) no longer used with MaDag v2
 *  - AbstractWfSched and derived classes no longer used with MaDag v2
 *
 * Revision 1.1  2008/04/10 08:38:50  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 ****************************************************************************/

#ifndef _CLTWFMGR_HH_
#define _CLTWFMGR_HH_

// STL headers
#include <map>

// IDL headers
#include "CltMan.hh"
#include "MasterAgent.hh"
#include "MaDag.hh"
#include "WfLogSrv.hh"

// DIET headers
#include "DIET_data.h"
#include "DIET_client.h"
#include "marshalling.hh"

#include "DagWfParser.hh"

class CltWfMgr : public POA_CltMan,
                 public PortableServer::RefCountServantBase{
public:
  /**
   * Executes a node on a specified Sed (CORBA method)
   * (CURRENTLY NOT USED)
   *
   * @param node_id node identifier
   * @param dag_id  dag identifier
   * @param sed     SeD where execute service
   */
  virtual void
  execNodeOnSed(const char * node_id, const char * dag_id, _objref_SeD* sed);

  /**
   * Executes a node without specifying the Sed (CORBA method)
   *
   * @param node_id node identifier
   * @param dag_id  dag identifier
   */
  virtual void
  execNode(const char * node_id, const char * dag_id);

  /**
   * Release the waiting semaphore. This method is used by the MA DAG when workflow execution
   * is done by this agent
   */
  virtual void
  release(const char * dag_id);

  /**
   * Debug function
   */
  virtual void
  ping();

  /**
   * Give access to unique reference of CltWfMgr
   */
  static CltWfMgr *
  instance();

  /**
   * Set the MaDag reference
   *
   * @param maDagRef MaDag reference
   */
  void
  setMaDag(MaDag_var maDagRef);

  /**
   * Set the Master Agent reference
   *
   * @param ma Master Agent reference
   */
  void
  setMA(MasterAgent_var ma);

  /**
   * Set the workflow log service reference
   *
   * @param logSrv workflow log service reference
   */
  void
  setWfLogSrv(WfLogSrv_var logSrv);

  /**
   * Execute a workflow
   *
   * @param profile profile of workflow to execute
   */
  diet_error_t
  wf_call(diet_wf_desc_t* profile);

  /**
   * Get a scalar result from a workflow
   */
   int
   getWfOutputScalar(diet_wf_desc_t* profile,
                   const char * id,
		   void** value);

  /**
   * Get a string result from a workflow
   */
   int
   getWfOutputString(diet_wf_desc_t* profile,
                   const char * id,
		   char** value);

  /**
   * Get a file result from a workflow
   */
   int
   getWfOutputFile(diet_wf_desc_t* profile,
                   const char * id,
		   size_t* size, char** path);

  /**
   * Get a matrix result from a workflow
   */
   int
   getWfOutputMatrix(diet_wf_desc_t* profile,
                   const char * id,
		   void** value,
		   size_t* nb_rows,
		   size_t *nb_cols,
		   diet_matrix_order_t* order);


  /**
   * terminate a workflow session and free the memory
   *
   * @param profile profile of workflow to execute
   */
  void
  wf_free(diet_wf_desc_t * profile);


protected:
  /**
   * Map for profiles and their dags
   */
  std::map<diet_wf_desc_t *, Dag *> myProfiles;

  /**
   * Init the workflow processing
   *
   * @param profile workflow profile reference
   * @param pbs_seq problem sequence
   * @param dagSize dag node number
   */
  Dag *
  init_wf_call(diet_wf_desc_t * profile,
               corba_pb_desc_seq_t& pbs_seq,
               unsigned int& dagSize);

  /**
   * Execute a workflow using the MA DAG.
   *
   * @param profile workflow reference
   * @param mapping set if the scheduling is complete (priorities and mapping)
   *                or if it is just for setting nodes priorities
   */
  virtual diet_error_t
  wf_call_madag(diet_wf_desc_t * profile,
                bool mapping);


  /**
   * Return the object IOR
   */
  const char *
  myIOR();

  /**
   * Return the DAG with a given identifier
   *
   * @param dag_id Dag identifier
   */
  Dag *
  getDag(std::string dag_id);

private:
  /**
   * Unique instance reference
   */
  static CltWfMgr * myInstance;

  /**
   * MaDag CORBA object reference
   */
  MaDag_var myMaDag;

  /**
   * Master Agent reference
   */
  MasterAgent_var myMA;

  /**
   * Workflow log service CORBA object reference
   */
  WfLogSrv_var myWfLogSrv;

  /**
   * Defines if use or not MA DAG Scheduling
   */
//   bool useMaDagSched;

  /**
   * Private constructor
   */
  CltWfMgr();


  /**
   * Synchronisation semaphores
   */
   omni_semaphore mySem;

};


#endif   /* not defined _CLTWFMGR._HH */

