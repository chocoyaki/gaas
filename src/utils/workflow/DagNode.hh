/**
* @file DagNode.hh
* 
* @brief  The node class used for dag execution 
* 
* @author  Benjamin ISNARD (Benjamin.Isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENCE|                                                                
*/
/****************************************************************************/
/* The node class used for dag execution                                    */
/* Manages the execution profile, the data, the execution status and time   */
/****************************************************************************/

#ifndef _DAGNODE_HH_
#define _DAGNODE_HH_

#include <iostream>
#include <string>
#include <vector>

#ifdef WIN32
#include <time.h>
#else
#include <sys/time.h>
#endif
#include <ctime>

// DIET core headers
#include "DIET_grpc.h"
#include "MasterAgentImpl.hh"

// workflow related headers
#include "WfNode.hh"
#include "NodeQueue.hh"
#include "Thread.hh"
#include "DagScheduler.hh"
#include "DagNodeLauncher.hh"
#ifdef WIN32
#define DIET_API_LIB __declspec(dllexport)
#else
#define DIET_API_LIB
#endif

/*****************************************************************************/
/*                         CLASS WfDataException                             */
/*****************************************************************************/
class DIET_API_LIB WfDataException {
public:
  enum WfDataErrorType { eNOTFOUND,
                         eNOTAVAIL,
                         eWRONGTYPE,
                         eINVALID_CONTAINER,
                         eID_UNDEF,
                         eVOID_DATA,
                         eREADFILERROR,
                         eINVALID_VALUE };

  WfDataException(WfDataErrorType t, const std::string& info)
    : why(t), info(info) {
  }

  WfDataErrorType
  Type() const {
    return this->why;
  }

  const std::string&
  Info() const {
    return this->info;
  }

  std::string
  ErrorMsg() const;

private:
  WfDataErrorType why;
  std::string info;
};

/****************************************************************************/
/*                                                                          */
/*                            class DagNode                                 */
/*                                                                          */
/****************************************************************************/

class Dag;
class FWorkflow;
class FProcNode;

class DIET_API_LIB DagNode : public WfNode  {
public:
  /**
   * The Dag Node default constructor
   * @param id         the node id
   * @param dag        the parent dag
   * @param wf         the workflow that generated this dag node (optional)
   */
  DagNode(const std::string& id, Dag *dag = NULL, FWorkflow* wf = NULL);

  /**
   * Dag Node event message types
   */
  enum eventMsg_e {
    READY,
    START,
    FINISH,
    FAILED
  };

  /**
   * DagNode destructor
   */
  virtual ~DagNode();

  /**
   * Get the node complete id (dagName-nodeName) *
   */
  std::string
  getCompleteId();

  /**
   * set the node Dag reference
   */
  void
  setDag(Dag *dag);

  /**
   * get the node Dag reference
   */
  Dag *
  getDag() const;

  /**
   * get the node Wf reference
   * (may return NULL if node does not belong to a functional workflow)
   */
  FWorkflow *
  getWorkflow() const;

  /**
   * set the node problem name (ie service name)
   */
  void
  setPbName(const std::string& pbName);

  /**
   * get the problem name
   */
  const std::string&
  getPbName() const;

  /**
   * create a new port
   */
  WfPort *
  newPort(std::string portId, unsigned int ind,
          WfPort::WfPortType portType, WfCst::WfDataType dataType,
          unsigned int depth) throw(WfStructException);

  /**
   * Add a new predecessor
   * (may check some constraints before adding the predecessor effectively)
   * @param node  ptr to the predecessor
   * @param fullNodeId  contains the id of the predecessor eventually prefixed (if external)
   */
  virtual void
  addNodePredecessor(WfNode * node, const std::string& fullNodeId);

  /**
   * Returns a string description of the node
   */
  virtual std::string
  toString() const;

  /**
   * returns an XML representation of a node (includes current state)
   * @param output the out stream
   */
  void
  toXML(std::ostream& output) const;

  /******************************/
  /* DIET Profile Mgmt          */
  /******************************/

  /**
   * Return the node profile
   */
  diet_profile_t *
  getProfile();

  /**
   * return the reqID of the node
   */
  diet_reqID_t
  getReqID();

  /**
   * Set the estimation class
   * (if dagNode belongs to an estimation class then
   * the scheduler will use the same estimation value as for any node
   * of the same class)
   */
  void
  setEstimationClass(const std::string& estimClassId);

  /**
   * Get the estimation class
   */
  const std::string&
  getEstimationClass();

  /**
   * set the submit index
   */
  void
  setSubmitIndex(int idx);

  /**
   * get the submit index
   */
  int
  getSubmitIndex();

  /**
   * creates a basic profile without parameters (common part)
   */
  void
  createProfile();

  /**
   * create the diet profile associated to the node (MADAG) *
   */
  void
  initProfileSubmit();

  /**
   * create the diet profile associated to the node (CLIENT) *
   */
  void
  initProfileExec() throw(WfDataException);

  /**
   * Store the persistent data IDs from profile to node
   */
  void
  storeProfileData();

  /**
   * free the node profile and all the output data
   */
  void
  freeProfileAndData(MasterAgent_var& MA);

  /**
   * Allocate a new char *
   * @param value the parameter value as a string
   */
  char *
  newChar(const std::string value = "");
  /**
   * Allocate a new short *
   * @param value the parameter value as a string
   */
  short *
  newShort(const std::string value = "");

  /**
   * Allocate a new int  *
   * @param value the parameter value as a string
   */
  int *
  newInt(const std::string value = "");

  /**
   * Allocate a new long *
   * @param value the parameter value as a string
   */
  long *
  newLong(const std::string value = "");

  /**
   * Allocate a new string *
   * @param value the parameter value as a string
   */
  char *
  newString(const std::string value = "");

  /**
   * Allocate a new float  *
   * @param value the parameter value as a string
   */
  float *
  newFloat(const std::string value = "");

  /**
   * Allocate a new double  *
   * @param value the parameter value as a string
   */
  double *
  newDouble(const std::string value = "");

  /**
   * Allocate a new filename  *
   * @param value the parameter value as a string
   */
  char *
  newFile(const std::string value = "");

  /**
   * Display all results of the node
   */
  void
  displayResults(std::ostream& output);


  /******************************/
  /* Scheduling                 */
  /******************************/

  /**
   * set the node priority *
   * @param priority the new priority of the node
   */
  void
  setPriority(double priority);

  /**
   * get the node priority *
   */
  double
  getPriority();

  /**
   * Set the NodeQueue when the node is inserted into it
   */
  void
  setNodeQueue(NodeQueue * nodeQ);

  /**
   * Remove node from the NodeQueue it belongs to (if applicable)
   * This is used in case of dag cancellation
   */
  void
  removeFromNodeQueue();

  /**
   * set the ref to the last nodeQueue occupied by the node
   */
  void
  setLastQueue(NodeQueue * queue);

  /**
   * get the ref to the last nodeQueue occupied by the node
   */
  NodeQueue *
  getLastQueue();

  /******************************/
  /* Timestamps (MaDag)         */
  /******************************/

  /**
   * set the estimated duration
   */
  void
  setEstDuration(double time);

  /**
   * get the estimated duration
   */
  double
  getEstDuration();

  /**
   * set the estimated completion time
   */
  void
  setEstCompTime(double time);

  /**
   * get the estimated completion time
   */
  double
  getEstCompTime();

  /**
   * set the real start time
   */
  void
  setRealStartTime(double time);

  /**
   * get the real start time
   */
  double
  getRealStartTime();

  /**
   * set the estimated delay
   */
  void
  setEstDelay(double delay);

  /**
   * get the estimated delay
   */
  double
  getEstDelay();

  /**
   * set the real completion time
   */
  void
  setRealCompTime(double time);

  /**
   * get the real completion time
   */
  double
  getRealCompTime();

  /**
   * get the real delay (positive) after execution
   * or 0 if no delay
   * or -1 if needed timestamps not set
   */
  double
  getRealDelay();

  /**********************************/
  /* Execution status               */
  /**********************************/

  /**
   * Return true if the node is ready for execution, false otherwise
   */
  bool
  isReady() const;

  /**
   * set the node as ready for execution
   */
  void
  setAsReady(DagScheduler* scheduler = NULL);

  /**
   * Called when a previous node execution is done *
   */
  void
  prevNodeHasDone(DagScheduler* scheduler = NULL);

  /**
   * Another method to get if the node is ready for execution *
   */
  bool
  allPrevDone();

  /**
   * Test if the node is running *
   */
  bool
  isRunning() const;

  /**
   * Test if the execution is done *
   */
  bool
  isDone() const;

  /**
   * Test if the execution failed *
   */
  bool
  hasFailed() const;

  /**
   * Set the node status as done
   * @param scheduler workflow scheduler (for MADAG only)
   */
  void
  setAsDone(DagScheduler* scheduler = NULL);

  /**
   * Set the node status as failed
   * @param scheduler  workflow scheduler (for MADAG only)
   */
  void
  setAsFailed(DagScheduler* scheduler = NULL);

  /**
   * Set the status of the node given a string value of the status
   * @param statusStr   status value as provided by getStateAsString()
   */
  void
  setStatus(const std::string& statusStr);

  /**********************************/
  /* Execution thread               */
  /**********************************/

  /**
   * start the node execution
   */
  void
  start(DagNodeLauncher * launcher);

  /**
   * terminate node execution
   * Will wait until completed and delete the launcher
   */
  void
  terminate();


protected:

  void
  setPrev(int index, WfNode * node);

  /**
   * Get node status as an attribute value (for XML output)
   */
  std::string
  getStatusAsString() const;

  /**
   * Set node statuc as running *
   */
  void
  setAsRunning();

private:

  /*********************************************************************/
  /* private members                                                   */
  /*********************************************************************/

  /**
   * Node problem
   */
  std::string myPb;

  /**
   * NodeQueue ref (used to notify NodeQueue when state changes)
   */
  NodeQueue * myQueue;

  /**
   * NodeQueue ref (used to go back in the previous queue)
   */
  NodeQueue * lastQueue;

  /**
   * the number of previous nodes not finished *
   */
  int prevNodesTodoCount;

  /**
   * indicate if the task is done *
   */
  bool task_done;

  /**
   * indicate if execution of the task failed *
   */
  bool taskExecFailed;

  /**
   * Dag reference
   */
  Dag * myDag;

  /**
   * Workflow reference
   */
  FWorkflow * myWf;

  /**
   * problem profile *
   */
  diet_profile_t * profile;

  /**
   * node launcher *
   */
  DagNodeLauncher*  myLauncher;

  /*************************/
  /* problem parameters    */
  /*************************/
  std::vector<char*> charParams;
  std::vector<short*> shortParams;
  std::vector<int*> intParams;
  std::vector<long*> longParams;
  std::vector<char *> stringParams;
  std::vector<float*> floatParams;
  std::vector<double*> doubleParams;
  std::vector<char*> fileParams;
  /*************************/

  /**
   * the node priority *
   */
  double priority;

  /**
   * node running status *
   */
  //   bool node_running;

  /**
   * number of immediate next nodes that have end their execution *
   */
  unsigned int nextDone;

  /**
   * Estimated duration (in ms)
   */
  double estDuration;

  /**
   * Estimated completion time (in ms)
   */
  double estCompTime;

  /**
   * Estimated delay (in ms)
   */
  double estDelay;

  /**
   * Real start time (in ms)
   */
  double realStartTime;

  /**
   * Real completion time (in ms)
   */
  double realCompTime;

  /**
   * Submit index (stores the index in MA response)
   */
  int submitIndex;

  /**
   * Estimation class (if dagNode belongs to an estimation class then
   * the scheduler will use the same estimation value as for any node
   * of the same class)
   */
  std::string estimationClass;

  /**
   * Indicates that the node started a thread
   */
  bool isStarted;

  /**
   * Indicates that the node's thread was joined
   */
  bool isTerminated;
};

#endif  // _DAGNODE_HH_
