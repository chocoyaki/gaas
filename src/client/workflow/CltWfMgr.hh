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
 * Revision 1.12  2009/01/16 13:51:11  bisnard
 * corrected synchronization pbs between dag instanciation and execution
 *
 * Revision 1.11  2008/12/09 12:09:03  bisnard
 * added parameters to dag submit method to handle inter-dependent dags
 *
 * Revision 1.10  2008/12/02 14:17:48  bisnard
 * manage multi-dag cancellation when one dag fails
 *
 * Revision 1.9  2008/12/02 10:19:15  bisnard
 * functional workflow submission to MaDag
 *
 * Revision 1.8  2008/07/11 09:12:34  bisnard
 * Added exclusion blocks to avoid dag not found error
 *
 * Revision 1.7  2008/07/08 11:14:21  bisnard
 * Add dag makespan value in release message
 *
 * Revision 1.6  2008/06/26 15:00:18  bisnard
 * corba type mismatch
 *
 * Revision 1.5  2008/06/25 09:59:39  bisnard
 * new params in execNodeOnSeD to provide ReqId and estimation vector
 * to client for solve request
 *
 * Revision 1.4  2008/06/02 08:34:20  bisnard
 * Execute method (for wf node) now returns an error code in case of
 * communication failure with the SeD
 *
 * Revision 1.3  2008/06/01 09:20:37  rbolze
 * the remote method release now return a string which contains
 * feedback on the dag execution
 *
 * Revision 1.2  2008/05/16 12:32:10  bisnard
 * API function to retrieve all workflow results
 *
 * Revision 1.1  2008/04/30 07:26:10  bisnard
 * moved CltWfMgr into agent/workflow directory
 *
 * Revision 1.5  2008/04/28 12:08:16  bisnard
 * obsolete init_wf_call
 * changed constructor for Node (new param wfReqId)
 *
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
#include <string>

// IDL headers
#include "CltMan.hh"
#include "MasterAgent.hh"
#include "MaDag.hh"
#include "WfLogSrv.hh"

// DIET headers
#include "DIET_data.h"
#include "DIET_client.h"
#include "marshalling.hh"

#include "workflow/DagWfParser.hh"
#include "workflow/MetaDag.hh"
class FWorkflow;

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
   * @param reqID   request ID (from submit request)
   * @param ev      Estimation vector for this SeD (from submit request)
   */
  virtual CORBA::Long
  execNodeOnSed(const char * node_id,
                const char * dag_id,
                _objref_SeD* sed,
                const CORBA::ULong reqID,
                corba_estimation_t& ev);

  /**
   * Executes a node without specifying the Sed (CORBA method)
   *
   * @param node_id node identifier
   * @param dag_id  dag identifier
   */
  virtual CORBA::Long
  execNode(const char * node_id, const char * dag_id);

  /**
   * (CORBA method)
   * Release the waiting semaphore. This method is used by the MA DAG when workflow execution
   * is done by this agent
   */
  virtual char *
  release(const char * dag_id, bool successful);

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
   * Get a new workflow request ID (for multi-dag submit)
   */
  CORBA::Long
  getNewWfReqID();

  /**
   * Execute a dag using the MA DAG.
   * (if the wfReqID is set in the profile, then this call is part of several
   * interdependent dag calls)
   * @param profile dag description
   */
  diet_error_t
  wfDagCall(diet_wf_desc_t * profile);

  /**
   * Execute a functional workflow using the MA DAG
   * @param profile workflow description
   */
  diet_error_t
  wfFunctionalCall(diet_wf_desc_t * profile);

  /**
   * Display all results from a dag
   */
  diet_error_t
  printAllDagResults(diet_wf_desc_t* profile);

  /**
   * DIsplay all results from a functional wf
   */
  diet_error_t
  printAllFunctionalWfResults(diet_wf_desc_t* profile);

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

private:

  /***************************************************************************/
  /*                           PRIVATE methods                               */
  /***************************************************************************/

  /**
   * Private constructor
   */
  CltWfMgr();

  /**
   * Get current time (in milliseconds)
   */
  double
  getCurrTime();

  /**
   * Return the object IOR
   */
  const char *
  myIOR();

  /**
   * Return the DAG with a given identifier
   * @param dag_id Dag identifier
   * @return dag pointer or NULL if not found
   */
  Dag *
  getDag(std::string dag_id);

  /**
   * Initialize status for functional workflow
   */
  void
      initDagStatus(FWorkflow* wf);

  /**
   * Set the dag status regarding dags sent for a given functional workflow
   */
  void
      setWfSubmissionComplete(FWorkflow* wf);

  /**
   * Get the status regarding dags sent for a given functional workflow
   */
  bool
      isWfSubmissionComplete(FWorkflow* wf);

  /***************************************************************************/
  /*                          PRIVATE attributes                             */
  /***************************************************************************/

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
   * Local workflow request ID counter
   * (different from wf request ID on MaDag)
   */
  int cltWfReqId;

  /**
   * Map for profiles and their dags or workflows
   */
  std::map<diet_wf_desc_t *, NodeSet *> myProfiles;

  /**
   * Map for metadags
   */
  std::map<CORBA::Long, MetaDag*> myMetaDags;

  /**
   * Dags status for functional workflows
   */
  std::map<FWorkflow*,bool> allDagsSent;

  /**
   * Dag sent counter
   */
  int dagSentCount;

  /**
    * Critical section
   */
  omni_mutex myLock;

  /**
   * Synchronisation semaphores
   */
  omni_semaphore mySem;

  bool instanciationPending;
  /**
   * Reference time
   */
  struct timeval refTime;
};


#endif   /* not defined _CLTWFMGR._HH */

