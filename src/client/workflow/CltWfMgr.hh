/**
 * @file  CltWfMgr.hh
 *
 * @brief   Client workflow manager class
 *
 * @author   Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef _CLTWFMGR_HH_
#define _CLTWFMGR_HH_

// STL headers
#include <map>
#include <string>

// IDL headers
#include "CltMan.hh"
#include "MasterAgent.hh"
#include "MaDag.hh"
#ifdef USE_LOG_SERVICE
#include "DietLogComponent.hh"
#endif
#include "WfLogService.hh"

// DIET headers
#include "DIET_data.h"
#include "DIET_client.h"
#include "marshalling.hh"

// Forwarder part
#include "Forwarder.hh"
#include "CltManFwdr.hh"

class NodeSet;
class FWorkflow;
class Dag;
class MetaDag;

class CltWfMgr : public POA_CltMan,
public PortableServer::RefCountServantBase {
public:
  /**
   * Executes a node on a specified Sed (CORBA method)
   * @param node_id node identifier
   * @param dag_id  dag identifier
   * @param sed     SeD where execute service
   * @param reqID   request ID (from submit request)
   * @param ev      Estimation vector for this SeD (from submit request)
   */
  virtual CORBA::Long
  execNodeOnSed(const char *node_id,
                const char *dag_id,
                const char *sed,
                const CORBA::ULong reqID,
                corba_estimation_t &ev);

  /**
   * Executes a node without specifying the Sed (CORBA method)
   *
   * @param node_id node identifier
   * @param dag_id  dag identifier
   */
  virtual CORBA::Long
  execNode(const char *node_id, const char *dag_id);

  /**
   * (CORBA method)
   * Release the waiting semaphore. This method is used by the MA DAG when workflow execution
   * is done by this agent
   */
  virtual char *
  release(const char *dag_id, bool successful);

  /**
   * Debug function
   */
  virtual CORBA::Long
  ping();

  /**
   * Give access to unique reference of CltWfMgr
   */
  static CltWfMgr *
  instance();

  /**
   * Deletes the unique reference of CltWfMgr
   */
  static void
  terminate();

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
#ifdef USE_LOG_SERVICE
  /**
   * Set the log service reference
   *
   * @param logComponent log service reference
   */
  void
  setLogComponent(DietLogComponent *logComponent);
#endif
  /**
   * Set the workflow log service (GUI)
   * @param logService wf log service reference
   */
  void
  setWfLogService(WfLogService_var logService);

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
  wfDagCall(diet_wf_desc_t *profile);

  /**
   * Execute a functional workflow using the MA DAG
   * @param profile workflow description
   */
  diet_error_t
  wfFunctionalCall(diet_wf_desc_t *profile);

  /**
   * Cancel a running dag. This method returns immediately as it only set the dag's status.
   * Dag will continue running until currently running nodes are finished, and then
   * MaDag will send the release() call.
   */
  diet_error_t
  cancelDag(const char *dagId);

  /**
   * Display all results from a dag
   */
  diet_error_t
  printAllDagResults(diet_wf_desc_t *profile);

  /**
   * DIsplay all results from a functional wf
   */
  diet_error_t
  printAllFunctionalWfResults(diet_wf_desc_t *profile);

  /**
   * Read the workflow/dag execution transcript from a file (XML)
   * (file name is stored in profile)
   * If file does not exist, method does nothing
   */
  diet_error_t
  readWorkflowExecutionTranscript(diet_wf_desc_t *profile);

  /**
   * Store the workflow/dag execution transcript in a file (XML)
   * (file name is stored in profile)
   * (If file contains sth, content is overwritten)
   */
  diet_error_t
  saveWorkflowExecutionTranscript(diet_wf_desc_t *profile,
                                  const char *transcriptFileName);

  /**
   * Store the workflow data file (XML format) after workflow execution.
   * This file can be used as input for another execution of a workflow
   * to avoid re-uploading the data items to the DIET platform (the data
   * IDs are written as attributes of the data items)
   * (if file already exists, content will be overwritten)
   */
  diet_error_t
  saveWorkflowDataFile(diet_wf_desc_t *profile,
                       const char *dataFileName);

  /**
   * Get a scalar result from a dag
   */
  int
  getWfOutputScalar(diet_wf_desc_t *profile,
                    const char *id,
                    void **value);

  /**
   * Get a string result from a dag
   */
  int
  getWfOutputString(diet_wf_desc_t *profile,
                    const char *id,
                    char **value);

  /**
   * Get a file result from a dag
   */
  int
  getWfOutputFile(diet_wf_desc_t *profile,
                  const char *id,
                  size_t *size, char **path);

  /**
   * Get a matrix result from a dag
   */
  int
  getWfOutputMatrix(diet_wf_desc_t *profile,
                    const char *id,
                    void **value,
                    size_t *nb_rows,
                    size_t *nb_cols,
                    diet_matrix_order_t *order);

  /**
   * Get a container result from a dag
   */
  int
  getWfOutputContainer(diet_wf_desc_t *profile,
                       const char *id,
                       char **dataID);

  /**
   * Get sink results from a functional workflow
   * @return dataID contains the DAGDA ID of a container containing all results
   */
  int
  getWfSinkContainer(diet_wf_desc_t *profile,
                     const char *id,
                     char **dataID);

  /**
   * terminate a workflow session and free the memory
   *
   * @param profile profile of workflow to execute
   */
  void
  wf_free(diet_wf_desc_t *profile);

private:
  std::string name;
  /***************************************************************************/
  /*                           PRIVATE methods                               */
  /***************************************************************************/

  /**
   * Private constructor
   */
  explicit
  CltWfMgr(const std::string &name);

  /**
   * Get current time (in milliseconds)
   */
  double
  getCurrTime();

  /**
   * Return the object IOR
   */
  /*const char *
     myIOR();*/
  const std::string &
  myName() const;


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
  initDagStatus(FWorkflow *wf);

  /**
   * Set the dag status regarding dags sent for a given functional workflow
   */
  void
  setWfSubmissionComplete(FWorkflow *wf);

  /**
   * Get the status regarding dags sent for a given functional workflow
   */
  bool
  isWfSubmissionComplete(FWorkflow *wf);

  /**
   * Common part of node execution
   */
  CORBA::Long
  execNodeCommon(const char *node_id,
                 const char *dag_id,
                 const char *sed,
                 const CORBA::ULong reqID,
                 corba_estimation_t &ev);

  /**
   * Common part of the dag submission
   * @param dagProfile  the diet profile containing the XML code for the Dag
   *                    (to be sent to MaDag) and eventually the wfReqId
   * @param dag a ref to a dag (may be NULL if parse is true)
   * @param parse will parse the XML if set to true
   * @param release will close the wfReqId on MaDag side if set to true
   */
  diet_error_t
  wfDagCallCommon(diet_wf_desc_t *dagProfile, Dag *dag,
                  bool parse, bool release);


  /***************************************************************************/
  /*                          PRIVATE attributes                             */
  /***************************************************************************/

  /**
   * Unique instance reference
   */
  static CltWfMgr *myInstance;

  /**
   * Default data file name
   */
  static std::string defaultDataFileName;

  /**
   * MaDag CORBA object reference
   */
  MaDag_var myMaDag;

  /**
   * Master Agent reference
   */
  MasterAgent_var myMA;

#ifdef USE_LOG_SERVICE
  /**
   * Log service reference
   */
  DietLogComponent *myLC;
#endif

  /**
   * Wf Log Service Ref
   */
  WfLogService_var myLS;

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
  std::map<CORBA::Long, MetaDag *> myMetaDags;

  /**
   * Dags status for functional workflows
   */
  std::map<FWorkflow *, bool> allDagsSent;

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

/* Forwarder part. */
class CltWfMgrFwdr : public POA_CltManFwdr,
public PortableServer::RefCountServantBase {
public:
  CltWfMgrFwdr(Forwarder_ptr fwdr, const char *objName);

  virtual CORBA::Long
  execNodeOnSed(const char *node_id, const char *dag_id,
                const char *sed, const CORBA::ULong reqID,
                corba_estimation_t &ev);

  virtual CORBA::Long
  execNode(const char *node_id, const char *dag_id);

  virtual char *
  release(const char *dag_id, bool successful);

  virtual CORBA::Long
  ping();

private:
  Forwarder_ptr forwarder;
  char *objName;
};



#endif   /* not defined _CLTWFMGR._HH */
